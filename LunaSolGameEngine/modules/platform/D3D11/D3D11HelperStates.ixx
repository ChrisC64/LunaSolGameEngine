module;
#include "LSEFramework.h"

export module D3D11.HelperStates;
import Engine.LSDevice;
import Util.MSUtils;
import Data.LSShader;

export namespace LS::Win32
{
    // Generator Methods //
    // Rasterizer States //
    LSOptional<ID3D11RasterizerState2*> CreateRasterizerState2(ID3D11Device5* pDevice, const LS::LSDrawState& drawState)
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        D3D11_RASTERIZER_DESC2 desc{};
        desc.FillMode = drawState.Fill == FILL_STATE::FILL ? D3D11_FILL_SOLID : D3D11_FILL_WIREFRAME;
        switch (drawState.Cull)
        {
        case CULL_METHOD::NONE:
            desc.CullMode = D3D11_CULL_NONE;
            break;
        case CULL_METHOD::BACK:
            desc.CullMode = D3D11_CULL_BACK;
            break;
        case CULL_METHOD::FRONT:
            desc.CullMode = D3D11_CULL_FRONT;
            break;
        default:
            throw std::runtime_error("Unknown Cull Method passed.\n");
        }
        desc.FrontCounterClockwise = drawState.IsFrontCounterClockwise;
        desc.DepthClipEnable = drawState.IsDepthClipEnabled;

        using namespace Microsoft::WRL;

        ID3D11RasterizerState2* rsState;
        auto hr = pDevice->CreateRasterizerState2(&desc, &rsState);
        if (FAILED(hr))
        {
            Utils::ThrowIfFailed(hr, "Failed to create Rasterizer State2 Object");
        }

        return rsState;
    }

    // Depth Stencil States //
    LSOptional<ID3D11DepthStencilState*> CreateDepthStencilState(ID3D11Device5* pDevice, const D3D11_DEPTH_STENCIL_DESC depthDesc)
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        ID3D11DepthStencilState* pDepthState;
        HRESULT hr = pDevice->CreateDepthStencilState(&depthDesc, &pDepthState);
        if (FAILED(hr))
            Utils::ThrowIfFailed(hr, "Failed to create depth stencil state with DX11 Device");

        return pDepthState;
    }

    // Blend State //
    LSOptional<ID3D11BlendState1*> CreateBlendState1(ID3D11Device5* pDevice, const D3D11_BLEND_DESC1& blendDesc)
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        ID3D11BlendState1* pBlend;
        HRESULT hr = pDevice->CreateBlendState1(&blendDesc, &pBlend);
        if (FAILED(hr))
            Utils::ThrowIfFailed(hr, "Failed to create Blend Depth State 1 object with D3D11 Device");

        return pBlend;
    }

    // Sampler State //
    LSOptional<ID3D11SamplerState*> CreateSamplerState(ID3D11Device5* pDevice, const D3D11_SAMPLER_DESC& samplerDesc)
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        ID3D11SamplerState* pSampler;
        HRESULT hr = pDevice->CreateSamplerState(&samplerDesc, &pSampler);
        if (FAILED(hr))
            Utils::ThrowIfFailed(hr, "Failed to create sampler state from D3D11 Device");

        return pSampler;
    }

    // Setter Methods //

    // Rasterizer States //
    constexpr void SetRasterizerState2(ID3D11DeviceContext4* pContext, ID3D11RasterizerState2* pRasterizerState)
    {
        assert(pContext);
        assert(pRasterizerState);
        pContext->RSSetState(pRasterizerState);
    }

    // Depth Stencil State //
    constexpr void SetBlendState1(ID3D11DeviceContext4* pContext, ID3D11DepthStencilState* pDepthStencilState, uint32_t stencilRef = 1)
    {
        assert(pContext);
        assert(pDepthStencilState);
        pContext->OMSetDepthStencilState(pDepthStencilState, stencilRef);
    }

    // Blend State //
    constexpr void SetBlendState1(ID3D11DeviceContext4* pContext, ID3D11BlendState1* pBlendState, uint32_t sampleMask = 0xffffffff,
        std::array<float, 4> blendFactor = { 1.0f, 1.0f, 1.0f, 1.0f })
    {
        assert(pContext);
        assert(pBlendState);
        pContext->OMSetBlendState(pBlendState, blendFactor.data(), sampleMask);
    }

    // Sampler Sate //
    constexpr void SetSamplerState(ID3D11DeviceContext4* pContext, std::span<ID3D11SamplerState**> pSamplerState, LS::SHADER_TYPE type,
        uint32_t startSlot = 0, uint32_t numSamplers = 0)
    {
        assert(pContext);
        assert(pSamplerState.data());
        using enum LS::SHADER_TYPE;
        switch (type)
        {
        case VERTEX:
            pContext->VSSetSamplers(startSlot, numSamplers, pSamplerState.front());
            break;
        case PIXEL:
            pContext->PSSetSamplers(startSlot, numSamplers, pSamplerState.front());
            break;
        case GEOMETRY:
            pContext->GSSetSamplers(startSlot, numSamplers, pSamplerState.front());
            break;
        case COMPUTE:
            pContext->CSSetSamplers(startSlot, numSamplers, pSamplerState.front());
            break;
        case HULL:
            pContext->HSSetSamplers(startSlot, numSamplers, pSamplerState.front());
            break;
        case DOM:
            pContext->DSSetSamplers(startSlot, numSamplers, pSamplerState.front());
            break;
        default:
            //TODO: Handle error 
            break;
        }
    }
}