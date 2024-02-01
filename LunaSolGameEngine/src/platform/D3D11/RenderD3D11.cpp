#pragma once
#include <cstdint>
#include <array>
#include <span>
#include <tuple>
#include <vector>
#include <wrl/client.h>
#include <d3d11_4.h>
#include "engine/EngineDefines.h"

import D3D11.RenderD3D11;
import D3D11.Device;
import D3D11.Utils;
import D3D11.PipelineFactory;
import D3D11.RenderFuncD3D11;
import DirectXCommon;

using namespace LS::Win32;
namespace WRL = Microsoft::WRL;

RenderD3D11::RenderD3D11(LS::LSDeviceSettings settings) : m_settings(settings),
m_window(settings.Window)
{
}

RenderD3D11::~RenderD3D11()
{
    Shutdown();
}

auto LS::Win32::RenderD3D11::Initialize() noexcept -> LS::System::ErrorCode
{
    LS::System::ErrorCode ec = m_device.InitDevice(m_settings);
    if (!ec)
    {
        return ec;
    }

    auto hresult = m_device.CreateRTVFromBackBuffer(&m_renderTarget);
    if (FAILED(hresult))
    {
        return LS::System::CreateFailCode(LS::Win32::Dx11ErrorToString(hresult));
    }
    m_context = m_device.GetImmediateContext();

    return LS::System::CreateSuccessCode();
}

void RenderD3D11::Clear(std::array<float, 4> clearColor) noexcept
{
    LS::Win32::ClearRT(m_context.Get(), m_renderTarget.Get(), clearColor);
}

void RenderD3D11::SetPipeline(const PipelineStateDX11* state) noexcept
{
    auto context = m_device.GetImmediateContext();

    const auto findStrides = [&state]() -> std::tuple<std::vector<uint32_t>, std::vector<uint32_t>, std::vector<ID3D11Buffer*>>
        {
            std::vector<uint32_t> offsets;
            std::vector<uint32_t> strides;
            std::vector<ID3D11Buffer*> buffers;
            for (const auto& b : state->VertexBuffers)
            {
                offsets.emplace_back(0);
                strides.emplace_back(b.BufferDesc.StructureByteStride);
                buffers.emplace_back(b.Buffer.Get());
            }

            return { offsets, strides, buffers };
        };

    // Input Assembler Stage Setup //
    const auto [s, o, b] = findStrides();
    context->IASetVertexBuffers(0, static_cast<UINT>(state->VertexBuffers.size()), b.data(), s.data(), o.data());

    if (state->IndexBuffer.Buffer)
    {
        context->IASetIndexBuffer(state->IndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    }

    context->IASetInputLayout(state->InputLayout.Get());
    context->IASetPrimitiveTopology(state->PrimitiveTopology);

    // Load Shaders //

    // Load Buffers //

    // Load Output Merger States //
    context->OMSetBlendState(state->BlendState.State.Get(), state->BlendState.BlendFactor, state->BlendState.SampleMask);

    const auto findRTs = [&state]() -> std::vector<ID3D11RenderTargetView*>
        {
            std::vector<ID3D11RenderTargetView*> views;
            for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
            {
                if (!state->RTViews[i])
                    break;
                views.emplace_back(state->RTViews[i].Get());
            }

            return views;
        };
    const auto rts = findRTs();
    if (!state->DSStage.View)
    {
        if (rts.size() == 0)
        {
            context->OMSetRenderTargets(0, nullptr, nullptr);
        }
        else
        {
            context->OMSetRenderTargets(static_cast<UINT>(rts.size()), rts.data(), nullptr);
        }
    }
    else
    {
        if (rts.size() == 0)
        {
            context->OMSetRenderTargets(0, nullptr, state->DSStage.View.Get());
        }
        else
        {
            context->OMSetRenderTargets(static_cast<UINT>(rts.size()), rts.data(), state->DSStage.View.Get());
        }
    }

    if (!state->DSStage.State)
    {
        context->OMSetDepthStencilState(nullptr, state->DSStage.StencilRef);
    }
    else
    {
        context->OMSetDepthStencilState(state->DSStage.State.Get(), state->DSStage.StencilRef);
    }
}

void RenderD3D11::LoadVertexBuffer(uint32_t startSlot, const PipelineStateDX11* state) noexcept
{
}

void RenderD3D11::LoadIndexBuffer(uint32_t offset, const PipelineStateDX11* state) noexcept
{
}

void RenderD3D11::Update(const LS::DX::DXCamera& camera) noexcept
{
}

void RenderD3D11::RenderObjects(std::span<LS::IRenderable*> objs) noexcept
{
}

void RenderD3D11::Draw() noexcept
{
    LS::Win32::Present1(m_device.GetSwapChain().Get(), 1);
}

void RenderD3D11::Shutdown() noexcept
{
}

void RenderD3D11::Resize(uint32_t width, uint32_t height) noexcept
{
}

void RenderD3D11::AttachToWindow(LS::LSWindowBase* window) noexcept
{
}
