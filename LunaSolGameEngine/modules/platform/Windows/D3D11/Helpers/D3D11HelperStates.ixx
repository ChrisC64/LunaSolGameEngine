module;
#include <d3d11_4.h>
#include <wrl/client.h>
#include <optional>
#include <directxtk/CommonStates.h>
#include <cassert>
#include <stdexcept>
#include <span>
#include <array>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "engine/EngineDefines.h"
export module D3D11.HelperStates;
import Engine.LSDevice;
import Util.MSUtils;
import LSEDataLib;

namespace WRL = Microsoft::WRL;

namespace LS::Win32
{
    SharedRef<DirectX::CommonStates> g_commonStates;
    bool g_isCommonStatesInitialized = false;
}

export namespace LS::Win32
{
    auto InitCommonStates(ID3D11Device* pDevice)
    {
        g_commonStates = std::make_shared<DirectX::CommonStates>(pDevice);
        if (g_commonStates)
            g_isCommonStatesInitialized = true;
    }

    auto GetCommonStates() -> SharedRef<DirectX::CommonStates>
    {
        assert(g_commonStates && "Please initialize the common states before calling the getter");
        if (!g_isCommonStatesInitialized)
            return nullptr;
        return g_commonStates;
    }
    // TODO: Avoid using the Create___ for common states and just return common states object for use

    // Generator Methods //
    // Rasterizer States //
    [[nodiscard]]
    auto CreateRasterizerState2(ID3D11Device3* pDevice, const LS::RasterizerInfo& drawState) -> Nullable<WRL::ComPtr<ID3D11RasterizerState2>>
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

        WRL::ComPtr<ID3D11RasterizerState2> rsState;
        auto hr = pDevice->CreateRasterizerState2(&desc, &rsState);
        if (FAILED(hr))
        {
            Utils::ThrowIfFailed(hr, "Failed to create Rasterizer State2 Object");
        }

        return rsState;
    }

    [[nodiscard]]
    auto GetCullNoneState() noexcept -> Nullable<ID3D11RasterizerState*>
    {
        assert(g_commonStates && "Please initialize the common states before calling the getter");
        return g_commonStates->CullNone();
    }

    [[nodiscard]]
    auto GetCullClockwiseState() noexcept -> Nullable<ID3D11RasterizerState*>
    {
        assert(g_commonStates && "Please initialize the common states before calling the getter");
        return g_commonStates->CullClockwise();
    }

    [[nodiscard]]
    auto GetCullCounterClockwiseState() noexcept -> Nullable<ID3D11RasterizerState*>
    {
        assert(g_commonStates && "Please initialize the common states before calling the getter");
        return g_commonStates->CullCounterClockwise();
    }

    [[nodiscard]]
    auto GetWireframeState() noexcept -> Nullable<ID3D11RasterizerState*>
    {
        assert(g_commonStates && "Please initialize the common states before calling the getter");
        return g_commonStates->Wireframe();
    }

    // Depth Stencil States //
    [[nodiscard]]
    auto CreateDepthStencilState(ID3D11Device5* pDevice, const DepthStencil& depthStencil) -> Nullable<WRL::ComPtr<ID3D11DepthStencilState>>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        D3D11_DEPTH_STENCIL_DESC dsDesc;

        auto convertToDepthWriteMask = [](DEPTH_STENCIL_WRITE_MASK mask) -> D3D11_DEPTH_WRITE_MASK
        {
            using enum DEPTH_STENCIL_WRITE_MASK;
            switch (mask)
            {
            case ZERO:
                return D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ZERO;
            case ALL:
                return D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
            default:
                return D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ZERO;
            }
        };

        dsDesc.DepthEnable = depthStencil.IsDepthEnabled;
        dsDesc.StencilEnable = depthStencil.IsStencilEnabled;
        dsDesc.DepthWriteMask = convertToDepthWriteMask(depthStencil.DepthWriteMask);
        dsDesc.StencilReadMask = depthStencil.StencilReadMask;
        dsDesc.StencilWriteMask = depthStencil.StencilWriteMask;

        auto convertToDepthFunc = [](EVAL_COMPARE compare) -> D3D11_COMPARISON_FUNC
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
        dsDesc.DepthFunc = convertToDepthFunc(depthStencil.DepthComparison);

        auto convertDSOps = [](DEPTH_STENCIL_OPS ops) -> D3D11_STENCIL_OP
        {
            using enum DEPTH_STENCIL_OPS;
            switch (ops)
            {
                case KEEP:
                    return D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
                case ZERO:
                    return D3D11_STENCIL_OP::D3D11_STENCIL_OP_ZERO;
                case REPLACE:
                    return D3D11_STENCIL_OP::D3D11_STENCIL_OP_REPLACE;
                case INVERT:
                    return D3D11_STENCIL_OP::D3D11_STENCIL_OP_INVERT;
                case INCR_THEN_CLAMP:
                    return D3D11_STENCIL_OP::D3D11_STENCIL_OP_INCR;
                case INCR_THEN_WRAP:
                    return D3D11_STENCIL_OP::D3D11_STENCIL_OP_INCR_SAT;
                case DECR_THEN_CLAMP:
                    return D3D11_STENCIL_OP::D3D11_STENCIL_OP_DECR;
                case DECR_THEN_WRAP:
                    return D3D11_STENCIL_OP::D3D11_STENCIL_OP_DECR_SAT;
                default:
                    return D3D11_STENCIL_OP::D3D11_STENCIL_OP_KEEP;
            }
        };

        auto convertDepthStencilOp = [&](const DepthStencil::DepthStencilOps& ops) -> D3D11_DEPTH_STENCILOP_DESC
        {
            D3D11_DEPTH_STENCILOP_DESC out;
            out.StencilFailOp = convertDSOps(ops.StencilFailOp);
            out.StencilDepthFailOp = convertDSOps(ops.StencilPassDepthFailOp);
            out.StencilPassOp = convertDSOps(ops.StencilPassOp);
            out.StencilFunc = convertToDepthFunc(ops.StencilTestFunc);
            return out;
        };

        dsDesc.FrontFace = convertDepthStencilOp(depthStencil.FrontFace);
        dsDesc.BackFace = convertDepthStencilOp(depthStencil.BackFace);

        WRL::ComPtr<ID3D11DepthStencilState> pState;
        pDevice->CreateDepthStencilState(&dsDesc, &pState);

        return pState;
    }

    [[nodiscard]]
    auto CreateDepthStencilState(ID3D11Device5* pDevice, const D3D11_DEPTH_STENCIL_DESC depthDesc) -> Nullable<WRL::ComPtr<ID3D11DepthStencilState>>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        WRL::ComPtr<ID3D11DepthStencilState> pDepthState;
        HRESULT hr = pDevice->CreateDepthStencilState(&depthDesc, &pDepthState);
        if (FAILED(hr))
            Utils::ThrowIfFailed(hr, "Failed to create depth stencil state with DX11 Device");

        return pDepthState;
    }

    [[nodiscard]]
    auto GetDefaultDepthState() noexcept -> ID3D11DepthStencilState*
    {
        assert(g_commonStates && "Please initialize the common states before calling the getter");
        return g_commonStates->DepthDefault();
    }

    [[nodiscard]]
    auto GetNoDepthState() noexcept -> ID3D11DepthStencilState*
    {
        assert(g_commonStates && "Please initialize the common states before calling the getter");
        return g_commonStates->DepthNone();
    }

    [[nodiscard]]
    auto GetDepthReadState() noexcept -> ID3D11DepthStencilState*
    {
        assert(g_commonStates && "Please initialize the common states before calling the getter");
        return g_commonStates->DepthRead();
    }

    [[nodiscard]]
    auto GetDepthReverseZState() noexcept -> ID3D11DepthStencilState*
    {
        assert(g_commonStates && "Please initialize the common states before calling the getter");
        return g_commonStates->DepthReverseZ();
    }

    [[nodiscard]]
    auto GetDepthReadReverseZState() noexcept -> ID3D11DepthStencilState*
    {
        assert(g_commonStates && "Please initialize the common states before calling the getter");
        return g_commonStates->DepthReadReverseZ();
    }

    // Blend State //
    [[nodiscard]]
    auto CreateBlendState1(ID3D11Device5* pDevice, const LSBlendState& blendState) -> Nullable<ID3D11BlendState1*>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        CD3D11_BLEND_DESC1 blendDesc;
        blendDesc.AlphaToCoverageEnable = blendState.IsAlphaSampling;
        blendDesc.IndependentBlendEnable = blendState.IsIndepdentBlend;

        blendDesc.RenderTarget[0].BlendEnable = blendState.IsEnabled;

        auto convertBlendOperation = [](BLEND_OPERATION op) -> D3D11_BLEND_OP
        {
            using enum BLEND_OPERATION;
            switch (op)
            {
            case BLEND_ADD:
                return D3D11_BLEND_OP_ADD;
            case BLEND_SUB:
                return D3D11_BLEND_OP_SUBTRACT;
            case BLEND_REV_SUB:
                return D3D11_BLEND_OP_REV_SUBTRACT;
            case BLEND_MIN:
                return D3D11_BLEND_OP_MIN;
            case BLEND_MAX:
                return D3D11_BLEND_OP_MAX;
            default:
                return D3D11_BLEND_OP_ADD;
            }
        };

        auto convertBlend = [](BLEND_FACTOR factor) -> D3D11_BLEND
        {
            using enum BLEND_FACTOR;
            switch (factor)
            {
            case ZERO:
                return D3D11_BLEND::D3D11_BLEND_ZERO;
            case ONE:
                return D3D11_BLEND::D3D11_BLEND_ONE;
            case SRC_COLOR:
                return D3D11_BLEND::D3D11_BLEND_SRC_COLOR;
            case DEST_COLOR:
                return D3D11_BLEND::D3D11_BLEND_DEST_COLOR;
            case SRC_ALPHA:
                return D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
            case DEST_ALPHA:
                return D3D11_BLEND::D3D11_BLEND_DEST_ALPHA;
            case INV_SRC_COLOR:
                return D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
            case INV_SRC_ALPHA:
                return D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
            case INV_DEST_COLOR:
                return D3D11_BLEND::D3D11_BLEND_INV_DEST_COLOR;
            case INV_DEST_ALPHA:
                return D3D11_BLEND::D3D11_BLEND_INV_DEST_ALPHA;
            default:
                return D3D11_BLEND::D3D11_BLEND_ZERO;
            }
        };

        auto writeMask = [](COLOR_CHANNEL_MASK mask) -> uint8_t
        {
            using enum COLOR_CHANNEL_MASK;
            switch (mask)
            {
            case RED:
                return D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_RED;
            case GREEN:
                return D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_GREEN;
            case BLUE:
                return D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_BLUE;
            case ALPHA:
                return D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALPHA;
            case ALL:
                return D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;
            default:
                return D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;
            }
        };

        blendDesc.RenderTarget[0].SrcBlend = convertBlend(blendState.SrcBF);
        blendDesc.RenderTarget[0].DestBlend = convertBlend(blendState.DestBF);
        blendDesc.RenderTarget[0].BlendOp = convertBlendOperation(blendState.BlendOpRGB);
        blendDesc.RenderTarget[0].SrcBlendAlpha = convertBlend(blendState.AlphaSrcBF);
        blendDesc.RenderTarget[0].DestBlendAlpha = convertBlend(blendState.AlphaDestBF);
        blendDesc.RenderTarget[0].BlendOpAlpha = convertBlendOperation(blendState.BlendOpAlpha);
        blendDesc.RenderTarget[0].RenderTargetWriteMask = writeMask(blendState.Mask);


        ID3D11BlendState1* pBlend;
        HRESULT hr = pDevice->CreateBlendState1(&blendDesc, &pBlend);
        if (FAILED(hr))
            return std::nullopt;
        return pBlend;
    }

    [[nodiscard]]
    auto CreateBlendState1(ID3D11Device5* pDevice, const D3D11_BLEND_DESC1& blendDesc) -> Nullable<ID3D11BlendState1*>
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
    auto CreateAlphaBlendState(ID3D11Device* pDevice) noexcept -> Nullable<ID3D11BlendState*>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates common(pDevice);
        return common.AlphaBlend();
    }

    [[nodiscard]]
    auto CreateOpaqueState(ID3D11Device* pDevice) noexcept -> ID3D11BlendState*
    {
        assert(g_commonStates && "Please initialize the common states before calling the getter");
        return g_commonStates->Opaque();
    }

    [[nodiscard]]
    auto CreateAdditiveState(ID3D11Device* pDevice) noexcept -> ID3D11BlendState*
    {
        assert(g_commonStates && "Please initialize the common states before calling the getter");
        return g_commonStates->Additive();
    }

    [[nodiscard]]
    auto CreateNonPremultipliedState(ID3D11Device* pDevice) noexcept -> ID3D11BlendState*
    {
        assert(g_commonStates && "Please initialize the common states before calling the getter");
        return g_commonStates->NonPremultiplied();
    }

    // Sampler State //
    [[nodiscard]]
    auto CreateSamplerState(ID3D11Device5* pDevice, const D3D11_SAMPLER_DESC& samplerDesc) -> Nullable<ID3D11SamplerState*>
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
    auto CreatePointWrapSampler(ID3D11Device* pDevice) noexcept -> Nullable<ID3D11SamplerState*>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.PointWrap();
    }

    [[nodiscard]]
    auto CreatePointClampSampler(ID3D11Device* pDevice) noexcept -> Nullable<ID3D11SamplerState*>
    {
        assert(pDevice);
        if (!pDevice)
            return std::nullopt;

        DirectX::CommonStates commonState(pDevice);
        return commonState.PointClamp();
    }

    [[nodiscard]]
    auto CreateLinearWrapSampler(ID3D11Device* pDevice) noexcept -> ID3D11SamplerState*
    {
        assert(g_commonStates && "Please initialize the common states before calling the getter");
        return g_commonStates->LinearWrap();
    }

    [[nodiscard]]
    auto CreateLinearClampSampler(ID3D11Device* pDevice) noexcept -> ID3D11SamplerState*
    {
        assert(g_commonStates && "Please initialize the common states before calling the getter");
        return g_commonStates->LinearClamp();
    }

    [[nodiscard]]
    auto CreateAnisotropicWrapSampler(ID3D11Device* pDevice) noexcept -> ID3D11SamplerState*
    {
        assert(g_commonStates && "Please initialize the common states before calling the getter");
        return g_commonStates->AnisotropicWrap();
    }

    [[nodiscard]]
    auto CreateAnisotropicClampSampler(ID3D11Device* pDevice) noexcept -> ID3D11SamplerState*
    {
        assert(g_commonStates && "Please initialize the common states before calling the getter");
        return g_commonStates->AnisotropicClamp();
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
    constexpr void SetDepthStencilState(ID3D11DeviceContext4* pContext, ID3D11DepthStencilState* pDepthStencilState,
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
            break;
        }
    }
}