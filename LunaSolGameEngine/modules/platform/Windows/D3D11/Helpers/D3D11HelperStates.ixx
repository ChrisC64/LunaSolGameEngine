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

    // I am liking the looks of the auto [Function Signature] -> Return type setup here,
    // it allows a more coherent design of functions where I can have things lined up more neatly, and see (at a glance)
    // function names and parameters quickly without worrying about the return type until the end (which is almost never uniform)
    // basically because it looks neater and nicer lined up to me.
    [[nodiscard]]
    auto CreateRasterizerState2(ID3D11Device3* pDevice, const LS::RasterizerInfo& drawState) -> LSOptional<ID3D11RasterizerState2*>
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
    auto CreateCullNoneState(ID3D11Device* pDevice) noexcept -> LSOptional<ID3D11RasterizerState*>
    {
        DirectX::CommonStates common(pDevice);

        return common.CullNone();
    }
    
    [[nodiscard]]
    auto CreateCullClockwiseState(ID3D11Device* pDevice) noexcept -> LSOptional<ID3D11RasterizerState*>
    {
        DirectX::CommonStates common(pDevice);

        return common.CullClockwise();
    }
    
    [[nodiscard]]
    auto CreateCullCounterClockwiseState(ID3D11Device* pDevice) noexcept -> LSOptional<ID3D11RasterizerState*>
    {
        DirectX::CommonStates common(pDevice);

        return common.CullCounterClockwise();
    }
    
    [[nodiscard]]
    auto CreateWireframeState(ID3D11Device* pDevice) noexcept -> LSOptional<ID3D11RasterizerState*>
    {
        DirectX::CommonStates common(pDevice);

        return common.Wireframe();
    }

    // Depth Stencil States //
    [[nodiscard]]
    auto CreateDepthStencilState(ID3D11Device5* pDevice, const D3D11_DEPTH_STENCIL_DESC depthDesc) -> LSOptional<ID3D11DepthStencilState*>
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
    auto CreateDefaultDepthState(ID3D11Device* pDevice) noexcept -> LSOptional<ID3D11DepthStencilState*>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.DepthDefault();
    }

    [[nodiscard]]
    auto CreateNoDepthState(ID3D11Device* pDevice) noexcept -> LSOptional<ID3D11DepthStencilState*>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.DepthNone();
    }
    
    [[nodiscard]]
    auto CreateDepthReadState(ID3D11Device* pDevice) noexcept -> LSOptional<ID3D11DepthStencilState*>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.DepthRead();
    }
    
    [[nodiscard]]
    auto CreateDepthReverseZState(ID3D11Device* pDevice) noexcept -> LSOptional<ID3D11DepthStencilState*>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.DepthReverseZ();
    }
    
    [[nodiscard]]
    auto CreateDepthReadReverseZState(ID3D11Device* pDevice) noexcept -> LSOptional<ID3D11DepthStencilState*>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.DepthReadReverseZ();
    }
    
    // Blend State //
    [[nodiscard]]
    auto CreateBlendState1(ID3D11Device5* pDevice, const D3D11_BLEND_DESC1& blendDesc) -> LSOptional<ID3D11BlendState1*>
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
    auto CreateAlphaBlendState(ID3D11Device* pDevice) noexcept -> LSOptional<ID3D11BlendState*>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates common(pDevice);
        return common.AlphaBlend();
    }
    
    [[nodiscard]]
    auto CreateOpaqueState(ID3D11Device* pDevice) noexcept -> LSOptional<ID3D11BlendState*>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates common(pDevice);
        return common.Opaque();
    }
    
    [[nodiscard]]
    auto CreateAdditiveState(ID3D11Device* pDevice) noexcept -> LSOptional<ID3D11BlendState*>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates common(pDevice);
        return common.Additive();
    }
    
    [[nodiscard]]
    auto CreateNonPremultipliedState(ID3D11Device* pDevice) noexcept -> LSOptional<ID3D11BlendState*>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates common(pDevice);
        return common.NonPremultiplied();
    }

    // Sampler State //
    [[nodiscard]]
    auto CreateSamplerState(ID3D11Device5* pDevice, const D3D11_SAMPLER_DESC& samplerDesc) -> LSOptional<ID3D11SamplerState*>
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
    auto CreatePointWrapSampler(ID3D11Device* pDevice) noexcept -> LSOptional<ID3D11SamplerState*>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.PointWrap();
    }

    [[nodiscard]]
    auto CreatePointClampSampler(ID3D11Device* pDevice) noexcept -> LSOptional<ID3D11SamplerState*>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.PointClamp();
    }
    
    [[nodiscard]]
    auto CreateLinearWrapSampler(ID3D11Device* pDevice) noexcept -> LSOptional<ID3D11SamplerState*>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.LinearWrap();
    }
    
    [[nodiscard]]
    auto CreateLinearClampSampler(ID3D11Device* pDevice) noexcept -> LSOptional<ID3D11SamplerState*>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.LinearClamp();
    }
    
    [[nodiscard]]
    auto CreateAnisotropicWrapSampler(ID3D11Device* pDevice) noexcept -> LSOptional<ID3D11SamplerState*>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.AnisotropicWrap();
    }
    
    [[nodiscard]]
    auto CreateAnisotropicClampSampler(ID3D11Device* pDevice) noexcept -> LSOptional<ID3D11SamplerState*>
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