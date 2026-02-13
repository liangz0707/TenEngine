/**
 * @file FileDialog.cpp
 * @brief Open file dialog implementation (Windows GetOpenFileName).
 */
#include <te/editor/FileDialog.h>
#include <te/editor/ImGuiBackend.h>
#include <te/core/platform.h>
#include <string>
#include <vector>

#if TE_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commdlg.h>
#pragma comment(lib, "comdlg32.lib")
#endif

namespace te {
namespace editor {

std::vector<std::string> OpenFileDialogMulti(char const* filterDesc, char const* filterSpec,
                                             char const* initialDir) {
  std::vector<std::string> result;
#if TE_PLATFORM_WINDOWS
  if (!filterDesc) filterDesc = "All supported";
  if (!filterSpec) filterSpec = "*.png;*.jpg;*.jpeg;*.tga;*.bmp;*.hdr;*.obj;*.fbx;*.gltf;*.*";

  std::vector<wchar_t> initialDirWide;
  if (initialDir && initialDir[0] != '\0') {
    int len = MultiByteToWideChar(CP_UTF8, 0, initialDir, -1, nullptr, 0);
    if (len > 0) {
      initialDirWide.resize(static_cast<size_t>(len), 0);
      MultiByteToWideChar(CP_UTF8, 0, initialDir, -1, initialDirWide.data(), len);
    }
  }

  std::vector<wchar_t> buf(32768, 0);
  OPENFILENAMEW ofn = {};
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = static_cast<HWND>(ImGuiBackend_GetWindowHandle());
  ofn.lpstrFilter = L"Image files (*.png;*.jpg;*.tga;*.bmp;*.hdr)\0*.png;*.jpg;*.jpeg;*.tga;*.bmp;*.hdr\0Mesh (*.obj;*.fbx;*.gltf)\0*.obj;*.fbx;*.gltf\0All files (*.*)\0*.*\0";
  ofn.lpstrFile = buf.data();
  ofn.nMaxFile = static_cast<DWORD>(buf.size());
  ofn.Flags = OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
  if (!initialDirWide.empty()) {
    ofn.lpstrInitialDir = initialDirWide.data();
  }

  if (!GetOpenFileNameW(&ofn))
    return result;

  std::wstring dir;
  std::vector<std::wstring> names;
  const wchar_t* p = buf.data();
  if (!p || !*p) return result;
  dir = p;
  p += dir.size() + 1;
  if (!*p) {
    size_t sep = dir.find_last_of(L"\\/");
    if (sep != std::wstring::npos) {
      names.push_back(dir.substr(sep + 1));
      dir = dir.substr(0, sep);
    } else {
      names.push_back(dir);
      dir.clear();
    }
  } else {
    while (*p) {
      names.push_back(p);
      p += names.back().size() + 1;
    }
  }

  for (std::wstring const& n : names) {
    std::wstring full = dir.empty() ? n : (dir + L"\\" + n);
    int len = WideCharToMultiByte(CP_UTF8, 0, full.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) continue;
    std::string utf8(static_cast<size_t>(len), '\0');
    WideCharToMultiByte(CP_UTF8, 0, full.c_str(), -1, &utf8[0], len, nullptr, nullptr);
    if (!utf8.empty() && utf8.back() == '\0') utf8.pop_back();
    result.push_back(utf8);
  }
#else
  (void)filterDesc;
  (void)filterSpec;
  (void)initialDir;
#endif
  return result;
}

}  // namespace editor
}  // namespace te
