#include <te/shader/detail/dxc_backend.hpp>
#include <te/shader/detail/handle_impl.hpp>

#if defined(TE_SHADER_HAVE_DXC) && TE_SHADER_HAVE_DXC
#include <dxcapi.h>
#include <wrl/client.h>
#include <string>
#include <vector>
#endif

namespace te::shader {

#if defined(TE_SHADER_HAVE_DXC) && TE_SHADER_HAVE_DXC

namespace {

bool compileHlslToDxilImpl(char const* source, size_t sourceLen, wchar_t const* entryPoint,
                           wchar_t const* targetProfile, std::vector<uint8_t>& outDxil,
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

    LPCWSTR args[] = {L"-E", entryPoint, L"-T", targetProfile};
    ComPtr<IDxcResult> result;
    HRESULT hr = compiler->Compile(&sourceBuffer, args, 4, includeHandler.Get(),
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

bool CompileHlslToDxil(ShaderHandleImpl* handle, std::string& outError) {
    if (!handle || handle->sourceCode_.empty()) {
        outError = "No HLSL source to compile";
        return false;
    }
    return compileHlslToDxilImpl(
        handle->sourceCode_.c_str(), handle->sourceCode_.size(),
        L"main", L"vs_6_0",  // default vertex shader
        handle->bytecodeBlob_, outError);
}

#else

bool CompileHlslToDxil(ShaderHandleImpl* /*handle*/, std::string& outError) {
    outError = "DXC not available (TE_RHI_D3D12 or directx-dxc not found)";
    return false;
}

#endif

}  // namespace te::shader
