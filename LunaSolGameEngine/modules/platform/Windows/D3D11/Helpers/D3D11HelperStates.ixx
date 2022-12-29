module;
#include "LSEFramework.h"

export module D3D11.HelperStates;
import Engine.LSDevice;
import Util.MSUtils;
import Data.LSShader;

export namespace LS::Win32
{
    // TODO:Consider Building a PIMPL method instead for these creation methods?
    //TODO: The create methods from CommonStates return pointers to owned ComPtr objects in a shared resource pool.
    // This factory class means that I cannot just take the pointers as given unless I have it release the ownership or copy
    // the com pointers. I should re-evaluate how I would want to use them. 

    // Generator Methods //
    // Rasterizer States //
    [[nodiscard]]
    LSOptional<ID3D11RasterizerState2*> CreateRasterizerState2(ID3D11Device3* pDevice, const LS::LSDrawState& drawState) 
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

    [[nodiscard]]
    LSOptional<ID3D11RasterizerState*> CreateCullNoneState(ID3D11Device* pDevice) noexcept
    {
        DirectX::CommonStates common(pDevice);

        return common.CullNone();
    }
    
    [[nodiscard]]
    LSOptional<ID3D11RasterizerState*> CreateCullClockwiseState(ID3D11Device* pDevice) noexcept
    {
        DirectX::CommonStates common(pDevice);

        return common.CullClockwise();
    }
    
    [[nodiscard]]
    LSOptional<ID3D11RasterizerState*> CreateCullCounterClockwiseState(ID3D11Device* pDevice) noexcept
    {
        DirectX::CommonStates common(pDevice);

        return common.CullCounterClockwise();
    }
    
    [[nodiscard]]
    LSOptional<ID3D11RasterizerState*> CreateWireframeState(ID3D11Device* pDevice) noexcept
    {
        DirectX::CommonStates common(pDevice);

        return common.Wireframe();
    }

    // Depth Stencil States //
    [[nodiscard]]
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

    [[nodiscard]]
    LSOptional<ID3D11DepthStencilState*> CreateDefaultDepthState(ID3D11Device* pDevice) noexcept
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.DepthDefault();
    }

    [[nodiscard]]
    LSOptional<ID3D11DepthStencilState*> CreateNoDepthState(ID3D11Device* pDevice) noexcept
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.DepthNone();
    }
    
    [[nodiscard]]
    LSOptional<ID3D11DepthStencilState*> CreateDepthReadState(ID3D11Device* pDevice) noexcept
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.DepthRead();
    }
    
    [[nodiscard]]
    LSOptional<ID3D11DepthStencilState*> CreateDepthReverseZState(ID3D11Device* pDevice) noexcept
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.DepthReverseZ();
    }
    
    [[nodiscard]]
    LSOptional<ID3D11DepthStencilState*> CreateDepthReadReverseZState(ID3D11Device* pDevice) noexcept
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.DepthReadReverseZ();
    }
    
    // Blend State //
    [[nodiscard]]
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

    [[nodiscard]]
    LSOptional<ID3D11BlendState*> CreateAlphaBlendState(ID3D11Device* pDevice) noexcept
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates common(pDevice);
        return common.AlphaBlend();
    }
    
    [[nodiscard]]
    LSOptional<ID3D11BlendState*> CreateOpaqueState(ID3D11Device* pDevice) noexcept
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates common(pDevice);
        return common.Opaque();
    }
    
    [[nodiscard]]
    LSOptional<ID3D11BlendState*> CreateAdditiveState(ID3D11Device* pDevice) noexcept
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates common(pDevice);
        return common.Additive();
    }
    
    [[nodiscard]]
    LSOptional<ID3D11BlendState*> CreateNonPremultipliedState(ID3D11Device* pDevice) noexcept
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates common(pDevice);
        return common.NonPremultiplied();
    }

    // Sampler State //
    [[nodiscard]]
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

    [[nodiscard]]
    LSOptional<ID3D11SamplerState*> CreatePointWrapSampler(ID3D11Device* pDevice) noexcept
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.PointWrap();
    }

    [[nodiscard]]
    LSOptional<ID3D11SamplerState*> CreatePointClampSampler(ID3D11Device* pDevice) noexcept
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.PointClamp();
    }
    
    [[nodiscard]]
    LSOptional<ID3D11SamplerState*> CreateLinearWrapSampler(ID3D11Device* pDevice) noexcept
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.LinearWrap();
    }
    
    [[nodiscard]]
    LSOptional<ID3D11SamplerState*> CreateLinearClampSampler(ID3D11Device* pDevice) noexcept
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.LinearClamp();
    }
    
    [[nodiscard]]
    LSOptional<ID3D11SamplerState*> CreateAnisotropicWrapSampler(ID3D11Device* pDevice) noexcept
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.AnisotropicWrap();
    }
    
    [[nodiscard]]
    LSOptional<ID3D11SamplerState*> CreateAnisotropicClampSampler(ID3D11Device* pDevice) noexcept
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.AnisotropicClamp();
    }

    // Setter Methods //

    // Rasterizer States //
    constexpr void SetRasterizerState2(ID3D11DeviceContext4* pContext, ID3D11RasterizerState2* pRasterizerState) noexcept
    {
        assert(pContext);
        assert(pRasterizerState);
        pContext->RSSetState(pRasterizerState);
    }

    // Depth Stencil State //
    constexpr void SetBlendState1(ID3D11DeviceContext4* pContext, ID3D11DepthStencilState* pDepthStencilState, 
        uint32_t stencilRef = 1) noexcept
    {
        assert(pContext);
        assert(pDepthStencilState);
        pContext->OMSetDepthStencilState(pDepthStencilState, stencilRef);
    }

    // Blend State //
    constexpr void SetBlendState1(ID3D11DeviceContext4* pContext, ID3D11BlendState1* pBlendState, uint32_t sampleMask = 0xffffffff,
        std::array<float, 4> blendFactor = { 1.0f, 1.0f, 1.0f, 1.0f }) noexcept
    {
        assert(pContext);
        assert(pBlendState);
        pContext->OMSetBlendState(pBlendState, blendFactor.data(), sampleMask);
    }

    // Sampler Sate //
    constexpr void SetSamplerState(ID3D11DeviceContext4* pContext, std::span<ID3D11SamplerState**> pSamplerState, 
        LS::SHADER_TYPE type, uint32_t startSlot = 0, uint32_t numSamplers = 0) noexcept
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