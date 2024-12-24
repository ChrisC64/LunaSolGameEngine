module;
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <directx-dxc/dxcapi.h>
#include <directx-dxc/d3d12shader.h>
export module DirectXCommon.D3DCompiler;
import <filesystem>;
import <cstdint>;
import <cstddef>;
import <format>;
import <vector>;

import Helper.IO;
import Engine.Shader;
import Engine.Defines;

namespace LS::DX
{
    static Microsoft::WRL::ComPtr<IDxcCompiler3> g_dxcCompiler;
    static Microsoft::WRL::ComPtr<IDxcUtils> g_dxcUtils;
    static Microsoft::WRL::ComPtr<IDxcIncludeHandler> g_dxcInclude;

    static void InitDxcCompiler()
    {
        if (g_dxcCompiler && g_dxcInclude && g_dxcUtils)
            return;

        HRESULT hr = ::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&g_dxcCompiler));
        if (FAILED(hr))
        {
            throw std::runtime_error("Failed to create DXC compiler");
        }

        hr = ::DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&g_dxcUtils));
        if (FAILED(hr))
        {
            throw std::runtime_error("Failed to create the DXC utils");
        }

        g_dxcUtils->CreateDefaultIncludeHandler(&g_dxcInclude);
    }

    static bool IsDxcToolsReady()
    {
        return g_dxcCompiler && g_dxcUtils && g_dxcInclude;
    }
}

export namespace LS::DX
{
    struct CompileResult
    {
        std::vector<std::byte> ByteCode;
        std::string ErrorMsg;

        bool HasError() const
        {
            return !ErrorMsg.empty();
        }
    };

    void InitCompilerTools()
    {
        InitDxcCompiler();
    }

    auto DxcLoadFile(std::filesystem::path file) -> LS::Nullable<std::vector<std::byte>>
    {
        if (!IsDxcToolsReady())
            InitDxcCompiler();

        Microsoft::WRL::ComPtr<IDxcBlobEncoding> data;
        HRESULT hr = g_dxcUtils->LoadFile(file.wstring().c_str(), NULL, &data);
        if (FAILED(hr))
        {
            return std::nullopt;
        }

        std::byte* begin = reinterpret_cast<std::byte*>(data->GetBufferPointer());
        std::vector<std::byte> outData(begin, begin + data->GetBufferSize());

        return outData;
    }

//    auto DxcCompileFile(std::filesystem::path file, const wchar_t* entryPoint, const wchar_t* target, const std::vector<LPCWSTR>& args = {}) -> LS::Nullable<std::vector<std::byte>>
//    {
//        if (!IsDxcToolsReady())
//            InitDxcCompiler();
//
//        std::vector<LPCWSTR> compileArgs
//        {
//            L"-E",
//            entryPoint,
//            L"-T",
//            target,
//#ifdef _DEBUG
//            DXC_ARG_DEBUG,
//            DXC_ARG_SKIP_OPTIMIZATIONS,
//#endif
//        };
//
//        for (auto a : args)
//        {
//            compileArgs.push_back(a);
//        }
//
//        Microsoft::WRL::ComPtr<IDxcBlobEncoding> data;
//        HRESULT hr = g_dxcUtils->LoadFile(file.wstring().c_str(), NULL, &data);
//        if (FAILED(hr))
//        {
//            return std::nullopt;
//        }
//
//        DxcBuffer sourceBuffer
//        {
//            .Ptr = data->GetBufferPointer(),
//            .Size = data->GetBufferSize(),
//            .Encoding = DXC_CP_ACP
//        };
//
//        Microsoft::WRL::ComPtr<IDxcResult> pResult;
//        hr = g_dxcCompiler->Compile(&sourceBuffer, compileArgs.data(), (uint32_t)compileArgs.size(), nullptr, IID_PPV_ARGS(&pResult));
//        if (FAILED(hr))
//        {
//            Microsoft::WRL::ComPtr<IDxcBlobUtf16> pErrors;
//            pResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
//            const LPCWSTR errorMessage = pErrors->GetStringPointer();
//            OutputDebugString(errorMessage);
//            return std::nullopt;
//        }
//        // Get the shader binary //
//        Microsoft::WRL::ComPtr<IDxcBlob> pShaderOut;
//        Microsoft::WRL::ComPtr<IDxcBlobUtf16> pShaderName;
//        pResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShaderOut), &pShaderName);
//        if (!pShaderOut)
//        {
//            OutputDebugString(std::format(L"Failed to get shader data: {}", file.wstring()).c_str());
//            return std::nullopt;
//        }
//
//        std::byte* begin = reinterpret_cast<std::byte*>(pShaderOut->GetBufferPointer());
//        std::vector<std::byte> outData(begin, begin + pShaderOut->GetBufferSize());
//
//        return outData;
//    }

    auto FxcCompileShader(std::filesystem::path path, const char* entryPoint, const char* target, uint32_t compileFlags = 0, D3D_SHADER_MACRO* defines = nullptr, ID3DInclude* include = nullptr) -> CompileResult
    {
        CompileResult result;
        if (!std::filesystem::exists(path))
        {
            result.ErrorMsg = std::format("File not found: {}", path.string());
            return result;
        }

        const auto optData = LS::IO::ReadFileSome(path, 4);
        if (!optData)
        {
            result.ErrorMsg = std::format("Could not read file: {}", path.string());
            return result;
        }

        const auto data = optData.value();
        const std::string key({ (char)data[0], (char)data[1], (char)data[2], (char)data[3] });
        if (key == "DXBC")
        {
            // this is already compiled, return bytecode back to user.
            const auto bcOpt = LS::IO::ReadFile(path);
            if (!bcOpt)
            {
                result.ErrorMsg = std::format("Unable to read file {}", path.string());
                return result;
            }

            result.ByteCode = std::move(bcOpt.value());
            return result;
        }

        Microsoft::WRL::ComPtr<ID3DBlob> shader;
        Microsoft::WRL::ComPtr<ID3DBlob> error;
        D3DCompileFromFile(path.c_str(), defines != nullptr ? defines : nullptr, include != nullptr ? include : nullptr, entryPoint, target, compileFlags, 0, &shader, &error);

        if (error)
        {
            std::string msg(reinterpret_cast<const char*>(error->GetBufferPointer()), error->GetBufferSize());
            result.ErrorMsg = std::move(msg);
            return result;
        }

        std::byte* begin = reinterpret_cast<std::byte*>(shader->GetBufferPointer());
        std::copy(begin, begin + shader->GetBufferSize(), std::back_inserter(result.ByteCode));
        return result;
    }

    auto FxcCompileShader(const LS::CompileShaderOpts& options) -> CompileResult
    {
        uint32_t flags = 0u;
        for (auto f : options.CompileFlags)
        {
            const uint32_t flag = std::stoul(f);
            flags |= flag;
        }

        return FxcCompileShader(options.FilePath, options.EntryPoint.c_str(), options.Target.c_str(), flags);
    }
}