import D3D11.PipelineFactory;

#include "LSEFramework.h"

import Engine.Common;
import D3D11Lib;
import LSData;

using namespace LS;
using namespace LS::Win32;

void D3D11PipelineFactory::Init(SharedRef<DeviceD3D11>& device) noexcept
{
    m_pDevice = device;
}

bool D3D11PipelineFactory::CreatePipelineState(const PipelineDescriptor& pipeline) noexcept
{
    assert(m_pDevice);
    if (!m_pDevice)
        return false;

    auto pipelineD3D = CreatePipelineD3D11(pipeline);

    m_pipelines.emplace_back(pipelineD3D);
    return true;
}
//TODO: Breka up and put conversion functions into stand alone functions so we can reuse them.
PipelineStateDX11 LS::Win32::D3D11PipelineFactory::CreatePipelineD3D11(const PipelineDescriptor& pipeline)
{
    namespace WRL = Microsoft::WRL;
    PipelineStateDX11 out;
    ID3D11Device5* device = m_pDevice->GetDevice().Get();
    auto rasterizer = CreateRasterizerState2(device, pipeline.RasterizeState);
    if (rasterizer)
    {
        WRL::ComPtr<ID3D11RasterizerState2> rast;
        rast.Attach(rasterizer.value());
        out.RasterizerState = rast;
    }

    auto blendState = CreateBlendState1(device, pipeline.BlendState);
    if (blendState)
    {
        WRL::ComPtr<ID3D11BlendState1> blend;
        blend.Attach(blendState.value());
        out.BlendState = blend;
    }

    // Depth Stencil 
    auto depthStencilState = CreateDepthStencilState(device, pipeline.DepthStencil);
    if (depthStencilState)
    {
        WRL::ComPtr<ID3D11DepthStencilState> state;
        state.Attach(depthStencilState.value());
        out.DepthStencilState = state;
    }
    // Shader Compilation //
    for (auto [type, data] : pipeline.Shaders)
    {
        using enum SHADER_TYPE;
        switch (type)
        {
        case VERTEX:
        {
            WRL::ComPtr<ID3D11VertexShader> pShader;
            HRESULT hr = CompileVertexShaderFromByteCode(device, data, &pShader);
            if (FAILED(hr))
            {
                break;
            }
            out.VertexShader = pShader;
            break;
        }
        case PIXEL:
        {
            WRL::ComPtr<ID3D11PixelShader> pShader;
            HRESULT hr = CompilePixelShaderFromByteCode(device, data, &pShader);
            if (FAILED(hr))
            {
                break;
            }
            out.PixelShader = pShader;
            break;
        }
        case GEOMETRY:
            break;
        case COMPUTE:
            break;
        case HULL:
            break;
        case DOM:
            break;
        default:
            break;
        }
    }
    // Topology Conversion //
    switch (pipeline.Topology)
    {
        using enum PRIMITIVE_TOPOLOGY;
    case TRIANGLE_LIST:
        out.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        break;
    case TRIANGLE_STRIP:
        out.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        break;
    case TRIANGLE_FAN:
        out.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;//D3D11 Doesn't support Fan, OGL only
        break;
    case TRIANGLE_LIST_ADJ:
        out.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
        break;
    case TRIANGLE_STRIP_ADJ:
        out.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
        break;
    case POINT_LIST:
        out.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
        break;
    case LINE_LIST:
        out.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        break;
    case LINE_STRIP:
        out.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
        break;
    case LINE_LIST_ADJ:
        out.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
        break;
    case LINE_STRIP_ADJ:
        out.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
        break;
    default:
        out.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        break;
    }

    auto findUVW = [](TEX_ADDRESS_MODE mode) -> D3D11_TEXTURE_ADDRESS_MODE
    {
        using enum TEX_ADDRESS_MODE;
        switch (mode)
        {
        case WRAP: return D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
        case MIRROR: return D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_MIRROR;
        case CLAMP: return D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
        case BORDER: return D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_BORDER;
        case MIRROR_ONCE: return D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
        default: return D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
        }
    };

    auto convertCompareFunc = [](EVAL_COMPARE compare) -> D3D11_COMPARISON_FUNC
    {
        using enum EVAL_COMPARE;
        switch (compare)
        {
        case NEVER_PASS:
            return D3D11_COMPARISON_NEVER;
        case LESS_PASS:
            return D3D11_COMPARISON_LESS;
        case LESSS_EQUAL_PASS:
            return D3D11_COMPARISON_LESS_EQUAL;
        case EQUAL:
            return D3D11_COMPARISON_EQUAL;
        case NOT_EQUAL:
            return D3D11_COMPARISON_NOT_EQUAL;
        case GREATER_PASS:
            return D3D11_COMPARISON_GREATER;
        case GREATER_EQUAL_PASS:
            return D3D11_COMPARISON_EQUAL;
        case ALWAYS_PASS:
            return D3D11_COMPARISON_ALWAYS;
        default:
            return D3D11_COMPARISON_NEVER;
        }
    };

    auto findFilter = [](const TextureRenderState& state) -> D3D11_FILTER
    {
        using enum TEX_FILTER_MODE;
        using TYPE = TEX_FILTER_MODE_TYPE;
        auto min = state.MIN_FILTER;
        auto mag = state.MAG_FILTER;
        auto mip = state.MIP_FILTER;

        // Pre-defined states //
        if (state.METHOD != TEX_FILTER_METHOD::USER_MISC)
        {
            if (state.FILTER_TYPE == TYPE::DEFAULT)
            {
                if (state.METHOD == TEX_FILTER_METHOD::POINT)
                {
                    return D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_POINT;
                }
                if (state.METHOD == TEX_FILTER_METHOD::BILINEAR)
                {
                    return D3D11_FILTER::D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
                }
                if (state.METHOD == TEX_FILTER_METHOD::TRILINEAR)
                {
                    return D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
                }
                if (state.METHOD == TEX_FILTER_METHOD::ANISOTROPIC)
                {
                    return D3D11_FILTER::D3D11_FILTER_ANISOTROPIC;
                }
            }
            else if (state.FILTER_TYPE == TYPE::MINIMUM)
            {
                if (state.METHOD == TEX_FILTER_METHOD::POINT)
                {
                    return D3D11_FILTER::D3D11_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
                }
                if (state.METHOD == TEX_FILTER_METHOD::BILINEAR)
                {
                    return D3D11_FILTER::D3D11_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
                }
                if (state.METHOD == TEX_FILTER_METHOD::TRILINEAR)
                {
                    return D3D11_FILTER::D3D11_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
                }
                if (state.METHOD == TEX_FILTER_METHOD::ANISOTROPIC)
                {
                    return D3D11_FILTER::D3D11_FILTER_MINIMUM_ANISOTROPIC;
                }
            }
            else if (state.FILTER_TYPE == TYPE::MAXIMUM)
            {
                if (state.METHOD == TEX_FILTER_METHOD::POINT)
                {
                    return D3D11_FILTER::D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
                }
                if (state.METHOD == TEX_FILTER_METHOD::BILINEAR)
                {
                    return D3D11_FILTER::D3D11_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
                }
                if (state.METHOD == TEX_FILTER_METHOD::TRILINEAR)
                {
                    return D3D11_FILTER::D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
                }
                if (state.METHOD == TEX_FILTER_METHOD::ANISOTROPIC)
                {
                    return D3D11_FILTER::D3D11_FILTER_MAXIMUM_ANISOTROPIC;
                }
            }
            else if (state.FILTER_TYPE == TYPE::COMPARE)
            {
                if (state.METHOD == TEX_FILTER_METHOD::POINT)
                {
                    return D3D11_FILTER::D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
                }
                if (state.METHOD == TEX_FILTER_METHOD::BILINEAR)
                {
                    return D3D11_FILTER::D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
                }
                if (state.METHOD == TEX_FILTER_METHOD::TRILINEAR)
                {
                    return D3D11_FILTER::D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
                }
                if (state.METHOD == TEX_FILTER_METHOD::ANISOTROPIC)
                {
                    return D3D11_FILTER::D3D11_FILTER_COMPARISON_ANISOTROPIC;
                }
            }
        }

        // USER settings //
        switch (state.FILTER_TYPE)
        {
        case TYPE::DEFAULT:
        {
            if (min == POINT && mag == POINT && mip == POINT)
            {
                return D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_POINT;
            }
            else if (min == POINT && mag == POINT && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
            }
            else if (min == POINT && mag == LINEAR && mip == POINT)
            {
                return D3D11_FILTER::D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
            }
            else if (min == POINT && mag == LINEAR && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
            }
            else if (min == LINEAR && mag == POINT && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
            }
            else if (min == LINEAR && mag == LINEAR && mip == POINT)
            {
                return D3D11_FILTER::D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            }
            else if (min == LINEAR && mag == LINEAR && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            }
            else
            {
                return D3D11_FILTER::D3D11_FILTER_ANISOTROPIC;
            }
        }
        break;
        case TYPE::MINIMUM:
        {
            if (min == POINT && mag == POINT && mip == POINT)
            {
                return D3D11_FILTER::D3D11_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
            }
            else if (min == POINT && mag == POINT && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
            }
            else if (min == POINT && mag == LINEAR && mip == POINT)
            {
                return D3D11_FILTER::D3D11_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
            }
            else if (min == POINT && mag == LINEAR && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
            }
            else if (min == LINEAR && mag == POINT && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
            }
            else if (min == LINEAR && mag == LINEAR && mip == POINT)
            {
                return D3D11_FILTER::D3D11_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
            }
            else if (min == LINEAR && mag == LINEAR && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
            }
            else
            {
                return D3D11_FILTER::D3D11_FILTER_MINIMUM_ANISOTROPIC;
            }
        }
        break;
        case TYPE::MAXIMUM:
        {
            if (min == POINT && mag == POINT && mip == POINT)
            {
                return D3D11_FILTER::D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
            }
            else if (min == POINT && mag == POINT && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
            }
            else if (min == POINT && mag == LINEAR && mip == POINT)
            {
                return D3D11_FILTER::D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
            }
            else if (min == POINT && mag == LINEAR && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
            }
            else if (min == LINEAR && mag == POINT && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
            }
            else if (min == LINEAR && mag == LINEAR && mip == POINT)
            {
                return D3D11_FILTER::D3D11_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
            }
            else if (min == LINEAR && mag == LINEAR && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
            }
            else
            {
                return D3D11_FILTER::D3D11_FILTER_MAXIMUM_ANISOTROPIC;
            }
        }
        break;
        case TYPE::COMPARE:
        {
            if (min == POINT && mag == POINT && mip == POINT)
            {
                return D3D11_FILTER::D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
            }
            else if (min == POINT && mag == POINT && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
            }
            else if (min == POINT && mag == LINEAR && mip == POINT)
            {
                return D3D11_FILTER::D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
            }
            else if (min == POINT && mag == LINEAR && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
            }
            else if (min == LINEAR && mag == POINT && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
            }
            else if (min == LINEAR && mag == LINEAR && mip == POINT)
            {
                return D3D11_FILTER::D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
            }
            else if (min == LINEAR && mag == LINEAR && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
            }
            else
            {
                return D3D11_FILTER::D3D11_FILTER_COMPARISON_ANISOTROPIC;
            }
        }
        break;
        default:
        {
            if (min == POINT && mag == POINT && mip == POINT)
            {
                return D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_POINT;
            }
            else if (min == POINT && mag == POINT && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
            }
            else if (min == POINT && mag == LINEAR && mip == POINT)
            {
                return D3D11_FILTER::D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
            }
            else if (min == POINT && mag == LINEAR && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
            }
            else if (min == LINEAR && mag == POINT && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
            }
            else if (min == LINEAR && mag == LINEAR && mip == POINT)
            {
                return D3D11_FILTER::D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            }
            else if (min == LINEAR && mag == LINEAR && mip == LINEAR)
            {
                return D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            }
            else
            {
                return D3D11_FILTER::D3D11_FILTER_ANISOTROPIC;
            }
        }
        break;
        }
    };

    // Samplers //
    for (auto& [slot, sampler] : pipeline.Samplers)
    {
        D3D11_SAMPLER_DESC samplerDesc{};
        samplerDesc.MaxAnisotropy = sampler.AnisotropyLevel;
        samplerDesc.ComparisonFunc = convertCompareFunc(sampler.Evaluator);
        samplerDesc.MaxLOD = sampler.MaxLOD;
        samplerDesc.MinLOD = sampler.MinLOD;
        auto& color = sampler.TextureRenderState.BorderColor;
        samplerDesc.BorderColor[0] = color[0];
        samplerDesc.BorderColor[1] = color[1];
        samplerDesc.BorderColor[2] = color[2];
        samplerDesc.BorderColor[3] = color[3];
        samplerDesc.Filter = findFilter(sampler.TextureRenderState);
        samplerDesc.AddressU = findUVW(sampler.TextureRenderState.ADDRESS_U);
        samplerDesc.AddressV = findUVW(sampler.TextureRenderState.ADDRESS_V);
        samplerDesc.AddressW = findUVW(sampler.TextureRenderState.ADDRESS_W);
        samplerDesc.MipLODBias = sampler.MipLODBias;

        auto samplerState = CreateSamplerState(device, samplerDesc);
        if (samplerState)
        {
            WRL::ComPtr<ID3D11SamplerState> pSampler;
            pSampler.Attach(samplerState.value());
            out.Samplers.emplace_back(slot, pSampler);
        }
    }
    // Textures //
    for (auto& [slot, texture] : pipeline.Textures)
    {
        auto texDesc = CreateTexture2DDesc(texture);

        WRL::ComPtr<ID3D11Texture2D> pTexture;
        D3D11_SUBRESOURCE_DATA subRes{};
        subRes.pSysMem = texture.Data.data();
        subRes.SysMemPitch = static_cast<UINT>(texture.Width * 4);
        auto result = device->CreateTexture2D(&texDesc, &subRes, &pTexture);
        if (FAILED(result))
        {
            throw std::runtime_error("Failed to create texture2D object\n");
        }
        out.Textures.emplace_back(slot, pTexture);
    }
    // Buffers //
    return out;
}
