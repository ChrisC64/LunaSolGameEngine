#pragma once
#include <cstdint>
#include <cassert>
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
import D3D11.Utils;
import DirectXCommon;

using namespace LS::Win32;
namespace WRL = Microsoft::WRL;

RenderD3D11::RenderD3D11(const LS::LSDeviceSettings& settings, LS::LSWindowBase* pWindow) : m_settings(settings),
m_window(pWindow)
{
}

RenderD3D11::~RenderD3D11()
{
    Shutdown();
}

auto LS::Win32::RenderD3D11::Initialize() noexcept -> LS::System::ErrorCode
{
    LS::System::ErrorCode ec = m_device.InitDevice(m_settings, m_window);
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
    assert(state && "state cannot be null");
    if (!state)
        return;

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
    assert(state->VertexShader && "A vertex shader cannot be null");
    context->VSSetShader(state->VertexShader.Get(), nullptr, 0);
    assert(state->PixelShader && "A pixel shader cannot be null");
    context->PSSetShader(state->PixelShader.Get(), nullptr, 0);

    if (state->GeometryShader)
    {
        context->GSSetShader(state->GeometryShader.Get(), nullptr, 0);
    }

    if (state->HullShader)
    {
        context->HSSetShader(state->HullShader.Get(), nullptr, 0);
    }

    if (state->DomainShader)
    {
        context->DSSetShader(state->DomainShader.Get(), nullptr, 0);
    }
    
    if (state->ComputeShader)
    {
        context->CSSetShader(state->ComputeShader.Get(), nullptr, 0);
    }

    // Load Shader Buffers //

    // Load Output Merger States //
    context->OMSetBlendState(state->BlendState.State.Get(), state->BlendState.BlendFactor, state->BlendState.SampleMask);

    const auto findRTs = [&state]() -> std::vector<ID3D11RenderTargetView*>
        {
            std::vector<ID3D11RenderTargetView*> views;
            for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
            {
                if (!state->RTStage.RTViews[i])
                    break;
                views.emplace_back(state->RTStage.RTViews[i].Get());
            }

            return views;
        };
    const auto rts = findRTs();
    // FIX-ME: Fix this issue as I removed View, it should be bound by the user, so let's split up some
    // rendering tasks instead of having one whole "operation" like this. 
    /*if (!state->DSStage.View)
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
    }*/

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
    if (m_renderTarget)
        m_renderTarget = nullptr;
    m_device.ResizeSwapchain(width, height);
    m_device.CreateRTVFromBackBuffer(&m_renderTarget);
    
}

void RenderD3D11::AttachToWindow(LS::LSWindowBase* window) noexcept
{
    
}

auto LS::Win32::RenderD3D11::GetDevice() noexcept -> ID3D11Device *
{
    return m_device.GetDevice().Get();
}

auto LS::Win32::RenderD3D11::GetDeviceCom() noexcept -> WRL::ComPtr<ID3D11Device>
{
    return m_device.GetDevice();
}

auto LS::Win32::RenderD3D11::GetSwapChainCom() noexcept -> WRL::ComPtr<IDXGISwapChain1>
{
    return m_device.GetSwapChain();
}

auto LS::Win32::RenderD3D11::GetDeviceContextCom() noexcept -> WRL::ComPtr<ID3D11DeviceContext>
{
    return m_device.GetImmediateContext();
}

auto LS::Win32::RenderD3D11::BuildInputLayout(std::span<std::byte> compiledByteCode) -> Nullable<WRL::ComPtr<ID3D11InputLayout>>
{
    const auto elements = BuildFromReflection(compiledByteCode);

    if (!elements)
    {
        return std::nullopt;
    }
    
    const auto& e = elements.value();
    WRL::ComPtr<ID3D11InputLayout> inputLayout;
    const auto result = m_device.CreateInputLayout(e.data(), static_cast<UINT>(e.size()), compiledByteCode, &inputLayout);
    if (FAILED(result))
    {
        return std::nullopt;
    }

    return inputLayout;
}
