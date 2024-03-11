module;
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <string_view>
#include <filesystem>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <span>
#include <cassert>

#include <wrl/client.h>
#include <d3dcompiler.h>
#include <windows.h>
#include <d3d11_4.h>
#include "engine/EngineDefines.h"

export module D3D11.Utils;

import D3D11.LSTypeWrapper;
import LSEDataLib;
import Engine.LSDevice;

/**
 * @brief A list of common helper functions that enable us to perform common functions a little more easily. 
 * Any content related to memory management should be in the D3D11.MemoryHelper module
*/
export namespace LS::Win32
{
    struct ShaderResult
    {
        std::vector<std::byte> Data;
        std::string ErrMsg;
    };

    [[nodiscard]]
    inline auto BlobToString(ID3DBlob* blob) -> Nullable<std::string>
    {
        if (blob)
        {
            std::string out(reinterpret_cast<char*>(blob->GetBufferPointer()), blob->GetBufferSize());
            return out;
        }
        return std::nullopt;
    }

    /**
     * @brief Compiles a D3D HLSL shader from the file pointed.
     * @param filepath The file's location
     * @param entryPoint The name of the shader's entry point
     * @param targetProfile The target profile to build for
     * @param compilationFlags Any D3DCOMPILE_ compilation flags
     * @param ppData [out] contains the compiled source data
     * @param ppErrorMsg [out] contains an error message if the compilation fails
     * @return If the file is not found, HRESULT = ERROR_FILE_NOT_FOUND (2L) and if the compilation fails,
     * an error will be returned along with a message in the ppErrorMsg object. If successful, ppData will be initialized
     * with the data of the compiled source file.
    */
    [[nodiscard]]
    inline HRESULT CompileShaderFile(std::filesystem::path filepath, std::string_view entryPoint, std::string_view targetProfile,
        UINT compilationFlags, ID3DBlob** ppData, ID3DBlob** ppErrorMsg)
    {
        if (!std::filesystem::exists(filepath))
        {
            return ERROR_FILE_NOT_FOUND;
        }

        return D3DCompileFromFile(filepath.wstring().c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entryPoint.data(), targetProfile.data(), compilationFlags, 0, ppData, ppErrorMsg);
    }

    [[nodiscard]]
    inline auto CompileShaderFile(std::filesystem::path filepath, std::string_view entryPoint, std::string_view targetProfile,
        UINT compilationFlags) -> Nullable<ShaderResult>
    {
        if (!std::filesystem::exists(filepath))
        {
            return std::nullopt;
        }

        Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> pErrorMsg;

        auto hr = D3DCompileFromFile(filepath.wstring().c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entryPoint.data(), targetProfile.data(), compilationFlags, 0, &pBlob, &pErrorMsg);
        
        if (FAILED(hr))
        {
            if (pErrorMsg)
            {
                return ShaderResult{ .ErrMsg = { reinterpret_cast<char*>(pErrorMsg->GetBufferPointer()), pErrorMsg->GetBufferSize()} };
            }

            return std::nullopt;
        }

        if (pBlob)
        {
            std::byte* begin = (std::byte*)pBlob->GetBufferPointer();
            std::byte* end = begin + pBlob->GetBufferSize();
            return ShaderResult{ .Data = { begin, end } };
        }

        return std::nullopt;
    }

    /**
     * @brief Compiles a HLSL file from source data
     * @param sourceData The source data
     * @param entryPoint The entry point to the shader's main function
     * @param targetProfile The target profile to compile the shader as
     * @param compilationFlags A bitwise OR flag of D3DCOMPILE_ settings
     * @param ppData The compiled source data if successful otherwise it will be nullptr
     * @param ppErrorMsg An error messsage detailing why the compilation failed, nullptr if compilation successful
     * @return Returns S_OK if successful, otherwise an error code will be returned with a message of why the compilation failed.
     * If the vector is empty, no message will be set.
    */
    [[nodiscard]]
    constexpr HRESULT CompileShaderFromData(std::span<std::byte> sourceData, std::string_view entryPoint, std::string_view targetProfile,
        UINT compilationFlags, ID3DBlob** ppData, ID3DBlob** ppErrorMsg)
    {
        if (sourceData.empty())
        {
            return ERROR_INVALID_DATA;
        }

        return D3DCompile(sourceData.data(), sourceData.size(), nullptr, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entryPoint.data(), targetProfile.data(), compilationFlags, 0, ppData, ppErrorMsg);
    }

    [[nodiscard]]
    constexpr HRESULT CreateVertexShaderFromByteCode(ID3D11Device* pDevice, std::span<std::byte> byteCode, ID3D11VertexShader** ppShader)
    {
        return pDevice->CreateVertexShader(byteCode.data(), byteCode.size(), nullptr, ppShader);
    }

    [[nodiscard]]
    constexpr HRESULT CreatePixelShaderFromByteCode(ID3D11Device* pDevice, std::span<std::byte> byteCode, ID3D11PixelShader** ppShader)
    {
        return pDevice->CreatePixelShader(byteCode.data(), byteCode.size(), nullptr, ppShader);
    }

    [[nodiscard]]
    constexpr HRESULT CreateGeometryShaderFromByteCode(ID3D11Device* pDevice, std::span<std::byte> byteCode, ID3D11GeometryShader** ppShader)
    {
        return pDevice->CreateGeometryShader(byteCode.data(), byteCode.size(), nullptr, ppShader);
    }

    [[nodiscard]]
    constexpr HRESULT CreateDomainShaderFromByteCode(ID3D11Device* pDevice, std::span<std::byte> byteCode, ID3D11DomainShader** ppShader)
    {
        return pDevice->CreateDomainShader(byteCode.data(), byteCode.size(), nullptr, ppShader);
    }

    [[nodiscard]]
    constexpr HRESULT CreateHullShaderFromByteCode(ID3D11Device* pDevice, std::span<std::byte> byteCode, ID3D11HullShader** ppShader)
    {
        return pDevice->CreateHullShader(byteCode.data(), byteCode.size(), nullptr, ppShader);
    }

    [[nodiscard]]
    constexpr HRESULT CreateComputeShaderFromByteCode(ID3D11Device* pDevice, std::span<std::byte> byteCode, ID3D11ComputeShader** ppShader)
    {
        return pDevice->CreateComputeShader(byteCode.data(), byteCode.size(), nullptr, ppShader);
    }

    [[nodiscard]]
    constexpr auto CreateVertexShaderD3D11(ID3D11Device* pDevice, std::span<std::byte> byteCode) -> Nullable<ID3D11VertexShader*>
    {
        ID3D11VertexShader* pShader;
        HRESULT hr = pDevice->CreateVertexShader(byteCode.data(), byteCode.size(), nullptr, &pShader);
        if (FAILED(hr))
        {
            return std::nullopt;
        }
        return pShader;
    }

    [[nodiscard]]
    constexpr auto CreatePixelShaderD3D11(ID3D11Device* pDevice, std::span<std::byte> byteCode) -> Nullable<ID3D11PixelShader*>
    {
        ID3D11PixelShader* pShader;
        HRESULT hr = pDevice->CreatePixelShader(byteCode.data(), byteCode.size(), nullptr, &pShader);
        if (FAILED(hr))
        {
            return std::nullopt;
        }
        return pShader;
    }

    [[nodiscard]]
    constexpr auto CreateGeometryShaderD3D11(ID3D11Device* pDevice, std::span<std::byte> byteCode) -> Nullable<ID3D11GeometryShader*>
    {
        ID3D11GeometryShader* pShader;
        HRESULT hr = pDevice->CreateGeometryShader(byteCode.data(), byteCode.size(), nullptr, &pShader);
        if (FAILED(hr))
        {
            return std::nullopt;
        }
        return pShader;
    }
    
    [[nodiscard]]
    constexpr auto CreateHullShaderD3D11(ID3D11Device* pDevice, std::span<std::byte> byteCode) -> Nullable<ID3D11HullShader*>
    {
        ID3D11HullShader* pShader;
        HRESULT hr = pDevice->CreateHullShader(byteCode.data(), byteCode.size(), nullptr, &pShader);
        if (FAILED(hr))
        {
            return std::nullopt;
        }
        return pShader;
    }
    
    [[nodiscard]]
    constexpr auto CreateComputeShaderD3D11(ID3D11Device* pDevice, std::span<std::byte> byteCode) -> Nullable<ID3D11ComputeShader*>
    {
        ID3D11ComputeShader* pShader;
        HRESULT hr = pDevice->CreateComputeShader(byteCode.data(), byteCode.size(), nullptr, &pShader);
        if (FAILED(hr))
        {
            return std::nullopt;
        }
        return pShader;
    }
    
    [[nodiscard]]
    constexpr auto CreateDomainShaderD3D11(ID3D11Device* pDevice, std::span<std::byte> byteCode) -> Nullable<ID3D11DomainShader*>
    {
        ID3D11DomainShader* pShader;
        HRESULT hr = pDevice->CreateDomainShader(byteCode.data(), byteCode.size(), nullptr, &pShader);
        if (FAILED(hr))
        {
            return std::nullopt;
        }
        return pShader;
    }

    [[nodiscard]]
    inline auto CreateTexture2DDesc(const LSTextureInfo& texture, BUFFER_USAGE usage = BUFFER_USAGE::DEFAULT_RW, 
        BUFFER_BIND_TYPE bindType = BUFFER_BIND_TYPE::SHADER_RESOURCE, CPU_ACCESS_FLAG cpuAccess = CPU_ACCESS_FLAG::NOT_SET,
        uint32_t miscFlags = 0) -> D3D11_TEXTURE2D_DESC
    {
        D3D11_TEXTURE2D_DESC out{};
        out.Width = texture.Width;
        out.Height = texture.Height;
        out.MipLevels = texture.MipMapLevels;
        //Array size maps to number of mip maps because each mip map is the texture scaled 
        out.ArraySize = texture.MipMapLevels;
        out.Format = ToDxgiFormat(texture.PixelFormat);
        out.SampleDesc = { .Count = texture.SampleCount, .Quality = texture.SampleQuality };
        out.Usage = FindD3D11Usage(usage);
        out.BindFlags = FindD3D11BindFlag(bindType);
        out.CPUAccessFlags = FindD3D11CpuAccessFlag(cpuAccess);
        out.MiscFlags = miscFlags;
        return out;
    }

    constexpr auto FindDXGIFormat(D3D11_SIGNATURE_PARAMETER_DESC desc) noexcept -> DXGI_FORMAT
    {
        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
        if (desc.Mask == 1)
        {
            if (desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
                format = DXGI_FORMAT_R32_UINT;
            else if (desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
                format = DXGI_FORMAT_R32_SINT;
            else if (desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
                format = DXGI_FORMAT_R32_FLOAT;
        }
        else if (desc.Mask <= 3)
        {
            if (desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
                format = DXGI_FORMAT_R32G32_UINT;
            else if (desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
                format = DXGI_FORMAT_R32G32_SINT;
            else if (desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
                format = DXGI_FORMAT_R32G32_FLOAT;
        }
        else if (desc.Mask <= 7)
        {
            if (desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
                format = DXGI_FORMAT_R32G32B32_UINT;
            else if (desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
                format = DXGI_FORMAT_R32G32B32_SINT;
            else if (desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
                format = DXGI_FORMAT_R32G32B32_FLOAT;
        }
        else if (desc.Mask <= 15)
        {
            if (desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
                format = DXGI_FORMAT_R32G32B32A32_UINT;
            else if (desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
                format = DXGI_FORMAT_R32G32B32A32_SINT;
            else if (desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
                format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        }

        return format;
    }

    inline auto BuildFromReflection(std::span<std::byte> fileData) -> Nullable<std::vector<D3D11_INPUT_ELEMENT_DESC>>
    {
        if (fileData.empty())
        {
            return std::nullopt;
        }

        Microsoft::WRL::ComPtr<ID3D11ShaderReflection> pReflector;

        D3DReflect(fileData.data(), fileData.size(), IID_ID3D11ShaderReflection, &pReflector);

        if (!pReflector)
        {
            return std::nullopt;
        }

        D3D11_SHADER_DESC shaderDesc{};
        HRESULT hr = pReflector->GetDesc(&shaderDesc);
        if (FAILED(hr))
        {
            return std::nullopt;
        }

        std::vector<D3D11_INPUT_ELEMENT_DESC> vertexElems(shaderDesc.InputParameters);

        for (auto p = 0u; p < vertexElems.size(); ++p)
        {
            D3D11_SIGNATURE_PARAMETER_DESC desc{};
            D3D11_INPUT_ELEMENT_DESC elem{};
            hr = pReflector->GetInputParameterDesc(p, &desc);

            elem.SemanticName = desc.SemanticName;
            elem.SemanticIndex = desc.SemanticIndex;
            elem.InputSlot = 0;
            elem.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
            elem.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            elem.InstanceDataStepRate = 0;

            elem.Format = FindDXGIFormat(desc);
            assert(elem.Format != DXGI_FORMAT_UNKNOWN);
            if (elem.Format == DXGI_FORMAT_UNKNOWN)
            {
                throw std::runtime_error("Failed to parse the DXGI Format\n");
            }
            vertexElems.at(p) = elem;
        }

        if (vertexElems.empty())
            return std::nullopt;

        return vertexElems;
    }

    //TODO: Make better - handle cases for value 0 isn't used for things like POSITION/TEXCOORD because 0 is implied
    //TODO: Just redo this wohle thing, I think we cand o this better.
    inline auto FindSemanticIndex(std::string_view semanticName) noexcept -> uint32_t
    {
        uint32_t offset = 0;
        uint32_t end = static_cast<uint32_t>(semanticName.size()) - 1u;
        if (std::isdigit(semanticName.back()) == 0)
            return 0;

        for (auto i = end; i > 0; --i)
        {
            if (std::isdigit(semanticName.at(i)))
                continue;
            offset = i + 1u;
            break;
        }
        // Case where the text is "POSITION" no 0 is detected, then offset should be greater than end because of this
        if (offset > end)
            return 0u;
        auto substr = semanticName.substr(offset, end);
        size_t pos;
        auto result = 0u;
        try
        {
            result = std::stoul(substr.data(), &pos);
        }
        catch (std::invalid_argument ex)
        {
            return 0u;
        }
        catch (std::out_of_range ex)
        {
            return 0u;
        }

        return result;
    }

    constexpr auto BuildFromShaderElements(std::span<ShaderElement> elements) noexcept -> Nullable<std::vector<D3D11_INPUT_ELEMENT_DESC>>
    {
        if (elements.empty())
            return std::nullopt;

        std::vector<D3D11_INPUT_ELEMENT_DESC> inputs;

        for (auto& se : elements)
        {
            auto inputDesc = D3D11_INPUT_ELEMENT_DESC{};

            inputDesc.SemanticIndex = se.SemanticIndex;;
            inputDesc.SemanticName = se.SemanticName.data();
            inputDesc.AlignedByteOffset = se.OffsetAligned;
            inputDesc.InputSlot = se.InputSlot;
            inputDesc.InputSlotClass = se.InputClass == INPUT_CLASS::VERTEX ? D3D11_INPUT_PER_VERTEX_DATA
                : D3D11_INPUT_PER_INSTANCE_DATA;
            inputDesc.Format = FindDXGIFormat(se.ShaderData);

            inputs.emplace_back(inputDesc);
        }

        if (inputs.empty())
            return std::nullopt;

        return inputs;
    }

    /**
     * @brief Helps prepare for a screen resize by discarding necessary states that need to be removed when handling a window resize event
     * @param pContext 
    */
    constexpr void ClearDeviceDependentResources(ID3D11DeviceContext* pContext)
    {
        assert(pContext);
        if (!pContext)
            return;
        pContext->ClearState();
        pContext->OMSetRenderTargets(0, nullptr, nullptr);
        pContext->Flush();
    }

    [[nodiscard]] constexpr auto HresultToDx11Error(HRESULT hr) noexcept -> const char*
    {
        switch (hr)
        {
        case D3D11_ERROR_FILE_NOT_FOUND: return std::format("File not found: {}", hr).c_str();
        case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS: return std::format("Too many unique state objects: {}", hr).c_str();
        case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS: return std::format("Too many unique view objects: {}", hr).c_str();
        case D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD: return std::format("Context map without intial discard: {}", hr).c_str();
        case DXGI_ERROR_INVALID_CALL: return std::format("Invalid call: {}", hr).c_str();
        case DXGI_ERROR_WAS_STILL_DRAWING: return std::format("Still drawing: {}", hr).c_str();
        case E_FAIL: return std::format("Fail: {}", hr).c_str();
        case E_INVALIDARG: return std::format("Invalid argument passed: {}", hr).c_str();
        case E_OUTOFMEMORY: return std::format("Out of memory: {}", hr).c_str();
        case E_NOTIMPL: return std::format("Not implemented: {}", hr).c_str();
        case S_FALSE: return std::format("Successful fail - the error is okay, but not standard: {}", hr).c_str();
        case S_OK: return std::format("Ok: {}", hr).c_str();
        default: return std::format("Unknown value passed: {}", hr).c_str();
        }
    }
}