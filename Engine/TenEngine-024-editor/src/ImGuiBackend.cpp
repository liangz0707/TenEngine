/**
 * @file ImGuiBackend.cpp
 * @brief ImGui Win32 + D3D11 backend initialization and frame render.
 */
#include <te/core/platform.h>
#if TE_PLATFORM_WINDOWS

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <d3d11.h>
#include <dxgi.h>
#include <shellapi.h>
#include <te/core/check.h>
#ifdef CreateWindow
#undef CreateWindow
#endif
#include <te/application/Application.h>
#include <string>
#include <vector>

#pragma comment(lib, "d3d11.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace te {
namespace editor {

struct ImGuiBackendData {
  ID3D11Device* device = nullptr;
  ID3D11DeviceContext* ctx = nullptr;
  IDXGISwapChain* swapChain = nullptr;
  ID3D11RenderTargetView* rtv = nullptr;
  void* hwnd = nullptr;  // Main window for modal dialogs (e.g. file open)
  bool imguiInitialized = false;
  int width = 0;
  int height = 0;
};

static ImGuiBackendData g_data;
static std::vector<std::string> s_droppedPaths;

void ImGuiBackend_Resize(int width, int height);  // forward decl for NewFrame

static LRESULT CALLBACK WndProcWithDrop(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  LRESULT r = ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam);
  if (msg == WM_DROPFILES) {
    HDROP h = reinterpret_cast<HDROP>(wParam);
    UINT n = DragQueryFileW(h, 0xFFFFFFFF, nullptr, 0);
    for (UINT i = 0; i < n; i++) {
      wchar_t buf[MAX_PATH] = {};
      if (DragQueryFileW(h, i, buf, MAX_PATH) > 0) {
        int len = WideCharToMultiByte(CP_UTF8, 0, buf, -1, nullptr, 0, nullptr, nullptr);
        if (len > 0) {
          std::string utf8(static_cast<size_t>(len), '\0');
          WideCharToMultiByte(CP_UTF8, 0, buf, -1, &utf8[0], len, nullptr, nullptr);
          if (!utf8.empty() && utf8.back() == '\0') utf8.pop_back();
          s_droppedPaths.push_back(utf8);
        }
      }
    }
    DragFinish(h);
    return 0;
  }
  return r;
}

void ImGuiBackend_RegisterWndProcHandler(void* application) {
  if (application) {
    static_cast<te::application::IApplication*>(application)->SetWndProcHandler(
        reinterpret_cast<void*>(WndProcWithDrop));
  }
}

static bool CreateDeviceD3D(HWND hwnd, int width, int height) {
  DXGI_SWAP_CHAIN_DESC sd = {};
  sd.BufferCount = 2;
  sd.BufferDesc.Width = width;
  sd.BufferDesc.Height = height;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hwnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  UINT flags = 0;
#ifdef _DEBUG
  flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
  D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_11_0;
  HRESULT hr = D3D11CreateDeviceAndSwapChain(
    nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, &fl, 1,
    D3D11_SDK_VERSION, &sd, &g_data.swapChain, &g_data.device, nullptr, &g_data.ctx);
  if (FAILED(hr)) return false;

  ID3D11Texture2D* backBuffer = nullptr;
  hr = g_data.swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
  if (FAILED(hr)) return false;
  hr = g_data.device->CreateRenderTargetView(backBuffer, nullptr, &g_data.rtv);
  backBuffer->Release();
  if (FAILED(hr)) return false;

  g_data.ctx->OMSetRenderTargets(1, &g_data.rtv, nullptr);
  g_data.width = width;
  g_data.height = height;
  return true;
}

static void CleanupDeviceD3D() {
  if (g_data.rtv) { g_data.rtv->Release(); g_data.rtv = nullptr; }
  if (g_data.swapChain) { g_data.swapChain->Release(); g_data.swapChain = nullptr; }
  if (g_data.ctx) { g_data.ctx->Release(); g_data.ctx = nullptr; }
  if (g_data.device) { g_data.device->Release(); g_data.device = nullptr; }
}

bool ImGuiBackend_Init(void* hwnd, int width, int height) {
  if (!hwnd) return false;
  g_data.hwnd = hwnd;
  if (!CreateDeviceD3D(static_cast<HWND>(hwnd), width, height)) return false;

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  if (!ImGui_ImplWin32_Init(hwnd)) return false;
  if (!ImGui_ImplDX11_Init(g_data.device, g_data.ctx)) return false;
  DragAcceptFiles(static_cast<HWND>(hwnd), TRUE);

  g_data.imguiInitialized = true;
  return true;
}

void ImGuiBackend_Shutdown() {
  if (g_data.imguiInitialized) {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    g_data.imguiInitialized = false;
  }
  CleanupDeviceD3D();
}

void ImGuiBackend_NewFrame() {
  // Sync swap chain with current window client size (handles user resize/drag)
  RECT rect = {};
  if (g_data.hwnd && GetClientRect(static_cast<HWND>(g_data.hwnd), &rect)) {
    int clientW = rect.right - rect.left;
    int clientH = rect.bottom - rect.top;
    if (clientW > 0 && clientH > 0 && (clientW != g_data.width || clientH != g_data.height))
      ImGuiBackend_Resize(clientW, clientH);
  }
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
}

void ImGuiBackend_Render() {
  ImGui::Render();
  const float clear[4] = { 0.1f, 0.1f, 0.12f, 1.0f };
  g_data.ctx->OMSetRenderTargets(1, &g_data.rtv, nullptr);
  g_data.ctx->ClearRenderTargetView(g_data.rtv, clear);
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  g_data.swapChain->Present(1, 0);
}

void ImGuiBackend_Resize(int width, int height) {
  if (width <= 0 || height <= 0) return;
  if (!g_data.rtv || !g_data.swapChain) return;
  g_data.ctx->OMSetRenderTargets(0, nullptr, nullptr);
  g_data.rtv->Release();
  g_data.rtv = nullptr;
  g_data.swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
  ID3D11Texture2D* backBuffer = nullptr;
  g_data.swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
  g_data.device->CreateRenderTargetView(backBuffer, nullptr, &g_data.rtv);
  backBuffer->Release();
  g_data.ctx->OMSetRenderTargets(1, &g_data.rtv, nullptr);
  ImGui_ImplDX11_InvalidateDeviceObjects();
  ImGui_ImplDX11_CreateDeviceObjects();
  g_data.width = width;
  g_data.height = height;
}

bool ImGuiBackend_IsInitialized() { return g_data.imguiInitialized; }

std::vector<std::string> ImGuiBackend_GetAndClearDroppedPaths() {
  std::vector<std::string> out;
  out.swap(s_droppedPaths);
  return out;
}

void* ImGuiBackend_GetWindowHandle() {
  return g_data.hwnd;
}

}  // namespace editor
}  // namespace te

#else  // !TE_PLATFORM_WINDOWS

namespace te {
namespace editor {

void ImGuiBackend_RegisterWndProcHandler(void* /*application*/) {}

std::vector<std::string> ImGuiBackend_GetAndClearDroppedPaths() {
  return std::vector<std::string>();
}

void* ImGuiBackend_GetWindowHandle() {
  return nullptr;
}

}  // namespace editor
}  // namespace te

#endif  // TE_PLATFORM_WINDOWS
