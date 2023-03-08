module;
#include "LSEFramework.h"
export module Util.HLSLUtils;

import Data.LSShader;
import Data.LSTextureTypes;
import Data.LSDataTypes;

export namespace LS::Utils
{
    constexpr DXGI_FORMAT FindDXGIFormat(D3D11_SIGNATURE_PARAMETER_DESC desc) noexcept
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

    constexpr Nullable<std::vector<D3D11_INPUT_ELEMENT_DESC>> BuildFromReflection(std::span<std::byte> fileData)
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

        D3D11_SIGNATURE_PARAMETER_DESC desc{};
        D3D11_SHADER_DESC shaderDesc{};
        HRESULT hr = pReflector->GetDesc(&shaderDesc);
        if (FAILED(hr))
        {
            return std::nullopt;
        }

        std::vector<D3D11_INPUT_ELEMENT_DESC> vertexElems(shaderDesc.InputParameters);
        auto i = 0u;
        for (auto& elem : vertexElems)
        {
            D3D11_INPUT_ELEMENT_DESC inputElem;

            hr = pReflector->GetInputParameterDesc(i++, &desc);

            inputElem.SemanticName = std::move(desc.SemanticName);
            inputElem.SemanticIndex = desc.SemanticIndex;
            inputElem.InputSlot = 0;
            inputElem.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
            inputElem.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            inputElem.InstanceDataStepRate = 0;

            inputElem.Format = FindDXGIFormat(desc);
            assert(inputElem.Format != DXGI_FORMAT_UNKNOWN);
            if (inputElem.Format == DXGI_FORMAT_UNKNOWN)
            {
                throw std::runtime_error("Failed to parse the DXGI Format\n");
            }
            elem = std::move(inputElem);
        }

        if (vertexElems.empty())
            return std::nullopt;

        return vertexElems;
    }
    //TODO: Make better - handle cases for value 0 isn't used for things like POSITION/TEXCOORD because 0 is implied
    constexpr uint32_t FindSemanticIndex(std::string_view semanticName) noexcept
    {
        // Find the value of semantics 1 - 9 (We don't find any higher, but we should probably support for 
        // double digits at least. The max resources you can bind is 128, but does that mean we can have
        // POSITION128 as a possible semantic too? 
        int num = 0;// If it fails, 0 will be the value from the result below (keeps default value)
        auto getIndex = [&](int length)
        {
            auto v = std::from_chars(semanticName.data() + semanticName.size() - length, semanticName.data() + semanticName.size(), num);
            if (v.ec == std::errc::invalid_argument)
            {
                TRACE("Cannot find value for length of " << length << " in semantic name: " << semanticName.data() << "\n");
            }
            else if (v.ec == std::errc::result_out_of_range)
            {
                TRACE("Out of range error for length of " << length << " when searching for semantic index for semantic: " << semanticName.data() << "\n");
            }
        };
        int temp = -1;
        for (int i = 1u; i <= 3; i++)
        {
            getIndex(i);
            if (num == temp)
                break;
            temp = num;
        }
        return num;
    }


    constexpr DXGI_FORMAT FindDXGIFormat(SHADER_DATA_TYPE shaderData)
    {
        using SDT = LS::SHADER_DATA_TYPE;

        switch (shaderData)
        {
        case SDT::FLOAT:		return DXGI_FORMAT_R32_FLOAT;
        case SDT::FLOAT2:		return DXGI_FORMAT_R32G32_FLOAT;
        case SDT::FLOAT3:		return DXGI_FORMAT_R32G32B32_FLOAT;
        case SDT::FLOAT4:		return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case SDT::INT:			return DXGI_FORMAT_R32_SINT;
        case SDT::INT2:			return DXGI_FORMAT_R32G32_SINT;
        case SDT::INT3:			return DXGI_FORMAT_R32G32B32_SINT;
        case SDT::INT4:			return DXGI_FORMAT_R32G32B32A32_SINT;
        case SDT::UINT:			return DXGI_FORMAT_R32_UINT;
        case SDT::UINT2:		return DXGI_FORMAT_R32G32_UINT;
        case SDT::UINT3:		return DXGI_FORMAT_R32G32B32_UINT;
        case SDT::UINT4:		return DXGI_FORMAT_R32G32B32A32_UINT;
        case SDT::BOOL:			return DXGI_FORMAT_R32_SINT;// HLSL Bools are 4 bytes 
        default:
            throw std::runtime_error("Unsuported DXGI FORMAT for data type.\n");
            break;
        }
    }

    constexpr DXGI_FORMAT FindDXGIFormat(PIXEL_COLOR_FORMAT pixFormat)
    {
        using enum PIXEL_COLOR_FORMAT;
        switch (pixFormat)
        {
            // 8 RGBA
        case RGBA8_SINT: return DXGI_FORMAT_R8G8B8A8_SINT;
        case RGBA8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;
        case RGBA8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
        case RGBA8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
        case RGBA8_UNKNOWN: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
            // 16 RGBA
        case RGBA16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case RGBA16_SINT: return DXGI_FORMAT_R16G16B16A16_SINT;
        case RGBA16_UINT: return DXGI_FORMAT_R16G16B16A16_UINT;
        case RGBA16_UNORM: return DXGI_FORMAT_R16G16B16A16_UNORM;
        case RGBA16_SNORM: return DXGI_FORMAT_R16G16B16A16_SNORM;
            // 32 RGBA
        case RGBA32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case RGBA32_SINT: return DXGI_FORMAT_R32G32B32A32_SINT;
        case RGBA32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;
        case RGBA32_UNKNOWN: return DXGI_FORMAT_R32G32B32A32_TYPELESS;
            // 8 BGRA
        case BGRA8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
        case BGRA8_UNKNOWN: return DXGI_FORMAT_B8G8R8A8_TYPELESS;
        default:
            throw std::runtime_error("Unknown PIXEL COLOR FORMAT enum passed for DXGI FORMAT conversion.\n");
        }
    }

    constexpr Nullable<std::vector<D3D11_INPUT_ELEMENT_DESC>> BuildFromShaderElements(std::span<ShaderElement> elements) noexcept
    {
        if (elements.empty())
            return std::nullopt;

        auto trim = [](std::string_view semanticName, int semanticIndex, std::string& outStr)
        {
            if (semanticIndex < 10)
            {
                // check final position is not a letter first (EX: POSITION or POSITION0 are both valid for a 0 index)
                auto c = semanticName.end() - 1;
                if (std::isdigit(*c))// Last letter is a letter, not number
                {
                    semanticName.remove_suffix(1);
                }
            }
            else if (semanticIndex < 100)
            {
                semanticName.remove_suffix(2);
            }
            else if (semanticIndex < 999)
            {
                semanticName.remove_suffix(3);
            }
            outStr = std::string{ semanticName };
        };

        std::vector<D3D11_INPUT_ELEMENT_DESC> inputs;

        for (auto& se : elements)
        {
            auto inputDesc = D3D11_INPUT_ELEMENT_DESC{};

            inputDesc.SemanticIndex = FindSemanticIndex(se.SemanticName);
            trim(se.SemanticName, inputDesc.SemanticIndex, se.SemanticName);
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
}