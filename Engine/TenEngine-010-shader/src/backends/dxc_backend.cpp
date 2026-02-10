#include <te/shader/detail/dxc_backend.hpp>
#include <te/shader/detail/handle_impl.hpp>
#include <te/shader/types.hpp>

#if defined(TE_SHADER_HAVE_DXC) && TE_SHADER_HAVE_DXC
#include <dxcapi.h>
#include <wrl/client.h>
#include <string>
#include <vector>
#endif

namespace te::shader {

#if defined(TE_SHADER_HAVE_DXC) && TE_SHADER_HAVE_DXC

namespace {

wchar_t const* stageToDxcProfile(ShaderStage stage) {
    switch (stage) {
        case ShaderStage::Vertex: return L"vs_6_0";
        case ShaderStage::Fragment: return L"ps_6_0";
        case ShaderStage::Compute: return L"cs_6_0";
        case ShaderStage::Geometry: return L"gs_6_0";
        case ShaderStage::TessControl: return L"hs_6_0";
        case ShaderStage::TessEvaluation: return L"ds_6_0";
        default: return L"vs_6_0";
    }
}

void utf8EntryPointToWide(char const* entry, std::wstring& out) {
    out.clear();
    if (!entry || entry[0] == '\0') { out = L"main"; return; }
    size_t len = 0;
    while (entry[len] != '\0' && len < CompileOptions::kMaxEntryPointLen) ++len;
    out.resize(len);
    for (size_t i = 0; i < len; ++i) out[i] = static_cast<wchar_t>(static_cast<unsigned char>(entry[i]));
}

bool compileHlslToDxilImpl(char const* source, size_t sourceLen, wchar_t const* entryPoint,
                           wchar_t const* targetProfile, uint32_t optimizationLevel,
                           bool generateDebugInfo, std::vector<uint8_t>& outDxil,
                           std::string& outError) {
    using Microsoft::WRL::ComPtr;

    ComPtr<IDxcUtils> utils;
    ComPtr<IDxcCompiler3> compiler;
    ComPtr<IDxcIncludeHandler> includeHandler;
    if (FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)))) {
        outError = "DXC: Failed to create DxcUtils";
        return false;
    }
    if (FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)))) {
        outError = "DXC: Failed to create DxcCompiler";
        return false;
    }
    if (FAILED(utils->CreateDefaultIncludeHandler(&includeHandler))) {
        outError = "DXC: Failed to create include handler";
        return false;
    }

    ComPtr<IDxcBlobEncoding> sourceBlob;
    if (FAILED(utils->CreateBlob(source, static_cast<UINT32>(sourceLen), CP_UTF8, &sourceBlob))) {
        outError = "DXC: Failed to create source blob";
        return false;
    }

    DxcBuffer sourceBuffer{};
    sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
    sourceBuffer.Size = sourceBlob->GetBufferSize();
    sourceBuffer.Encoding = DXC_CP_UTF8;

    std::vector<std::wstring> optStrings;
    wchar_t const* oLevels[] = {L"-O0", L"-O1", L"-O2", L"-O3"};
    uint32_t oIdx = (optimizationLevel <= 3u) ? optimizationLevel : 1u;
    optStrings.push_back(oLevels[oIdx]);
    if (generateDebugInfo) optStrings.push_back(L"-Zi");
    std::vector<LPCWSTR> args = {L"-E", entryPoint, L"-T", targetProfile};
    for (auto const& s : optStrings) args.push_back(s.c_str());
    ComPtr<IDxcResult> result;
    HRESULT hr = compiler->Compile(&sourceBuffer, args.data(), static_cast<UINT32>(args.size()), includeHandler.Get(),
                                   IID_PPV_ARGS(&result));
    if (FAILED(hr)) {
        outError = "DXC: Compile failed";
        return false;
    }

    ComPtr<IDxcBlobUtf8> errors;
    result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
    if (errors && errors->GetStringLength() > 0) {
        outError = "DXC: ";
        outError += errors->GetStringPointer();
        return false;
    }

    ComPtr<IDxcBlob> dxilBlob;
    result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&dxilBlob), nullptr);
    if (!dxilBlob || dxilBlob->GetBufferSize() == 0) {
        outError = "DXC: No DXIL output";
        return false;
    }

    uint8_t const* data = static_cast<uint8_t const*>(dxilBlob->GetBufferPointer());
    size_t size = dxilBlob->GetBufferSize();
    outDxil.assign(data, data + size);
    return true;
}

}  // namespace

bool CompileHlslToDxil(ShaderHandleImpl* handle, CompileOptions const& options, std::string& outError) {
    if (!handle || handle->sourceCode_.empty()) {
        outError = "No HLSL source to compile";
        return false;
    }
    ShaderStage stage = (options.stage != ShaderStage::Unknown) ? options.stage : ShaderStage::Vertex;
    wchar_t const* profile = stageToDxcProfile(stage);
    std::wstring entryWide;
    utf8EntryPointToWide(options.entryPoint[0] != '\0' ? options.entryPoint : "main", entryWide);
    return compileHlslToDxilImpl(
        handle->sourceCode_.c_str(), handle->sourceCode_.size(),
        entryWide.c_str(), profile,
        options.optimizationLevel, options.generateDebugInfo,
        handle->bytecodeBlob_, outError);
}

#else

bool CompileHlslToDxil(ShaderHandleImpl* /*handle*/, CompileOptions const& /*options*/, std::string& outError) {
    outError = "DXC not available (TE_RHI_D3D12 or directx-dxc not found)";
    return false;
}

#endif

}  // namespace te::shader
