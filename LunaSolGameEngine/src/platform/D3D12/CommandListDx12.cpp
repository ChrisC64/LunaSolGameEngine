#include <cstdint>
#include <array>
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl/client.h>
#include <string_view>
#include <string>
#include <cassert>
#include <format>

import D3D12Lib;

import Util.MSUtils;

using namespace LS::Platform::Dx12;

CommandListDx12::CommandListDx12(ID3D12Device4* pDevice, D3D12_COMMAND_LIST_TYPE type, std::string_view name) : m_type(type),
m_fenceValue(0),
m_pCommandList(nullptr),
m_pAllocator(nullptr),
m_name(std::move(name))
{
    pDevice->CreateCommandList1(0, type, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_pCommandList));
    assert(m_pCommandList && "Failed to create command list");

    pDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&m_pAllocator));
    assert(m_pAllocator && "Failed to create command allocator");
}

void CommandListDx12::ResetCommandList() noexcept
{
    LS::Utils::ThrowIfFailed(m_pAllocator->Reset(), std::format("Failed to reset command allocator for: {}", m_name.c_str()));
    LS::Utils::ThrowIfFailed(m_pCommandList->Reset(m_pAllocator.Get(), nullptr), std::format("Failed to reset command allocator: {}", m_name.c_str()));
}

void CommandListDx12::Close() noexcept
{
    LS::Utils::ThrowIfFailed(m_pCommandList->Close(), std::format("Failed to close command list: {}", m_name.c_str()));
}

void CommandListDx12::Clear(const std::array<float, 4>& clearColor, const D3D12_CPU_DESCRIPTOR_HANDLE rtv) noexcept
{
    m_pCommandList->ClearRenderTargetView(rtv, clearColor.data(), 0, nullptr);
}

void CommandListDx12::ClearDepthStencil(const D3D12_CPU_DESCRIPTOR_HANDLE dsv, float clearValue, uint8_t stencilValue)
{
    m_pCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clearValue, 0xFF, 0, nullptr);
}

void CommandListDx12::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, before, after);
    m_pCommandList->ResourceBarrier(1, &barrier);
}