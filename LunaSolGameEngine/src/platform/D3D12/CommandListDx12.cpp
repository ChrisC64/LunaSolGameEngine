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

import Win32.ComUtils;
import Engine.EngineCodes;
import Win32.Utils;

using namespace LS::Platform::Dx12;

CommandListDx12::CommandListDx12(D3D12_COMMAND_LIST_TYPE type, std::string_view name) : m_type(type),
    m_fenceValue(0),
    m_pCommandList(nullptr),
    m_pAllocator(nullptr),
    m_name(std::move(name))
{
}

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

auto CommandListDx12::Initialize(ID3D12Device4* pDevice) noexcept -> LS::System::ErrorCode
{
    auto hr = pDevice->CreateCommandList1(0, m_type, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_pCommandList));
    assert(m_pCommandList && "Failed to create command list");

    if (FAILED(hr))
    {
        const auto msg = LS::Win32::HrToString(hr);
        return LS::System::CreateFailCode(std::format("Failed to create command list. Error {}", msg));
    }

    hr = pDevice->CreateCommandAllocator(m_type, IID_PPV_ARGS(&m_pAllocator));
    assert(m_pAllocator && "Failed to create command allocator");
    if (FAILED(hr))
    {
        const auto msg = LS::Win32::HrToString(hr);
        return LS::System::CreateFailCode(std::format("Failed to create command allocator. Error {}", msg));
    }

    return LS::System::CreateSuccessCode();
}

void CommandListDx12::ResetCommandList() noexcept
{
    m_pAllocator->Reset();
    m_pCommandList->Reset(m_pAllocator.Get(), nullptr);

    //LS::Utils::ThrowIfFailed(m_pAllocator->Reset(), std::format("Failed to reset command allocator for: {}", m_name.c_str()));
    //LS::Utils::ThrowIfFailed(m_pCommandList->Reset(m_pAllocator.Get(), nullptr), std::format("Failed to reset command allocator: {}", m_name.c_str()));
}

void CommandListDx12::Close() noexcept
{
    m_pCommandList->Close();
    //LS::Utils::ThrowIfFailed(m_pCommandList->Close(), std::format("Failed to close command list: {}", m_name.c_str()));
}

void CommandListDx12::Clear(const std::array<float, 4>& clearColor, const D3D12_CPU_DESCRIPTOR_HANDLE rtv) noexcept
{
    m_pCommandList->ClearRenderTargetView(rtv, clearColor.data(), 0, nullptr);
}

void CommandListDx12::ClearDepthStencil(const D3D12_CPU_DESCRIPTOR_HANDLE dsv, float clearValue, uint8_t stencilValue)
{
    m_pCommandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clearValue, stencilValue, 0, nullptr);
}

void CommandListDx12::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, before, after);
    m_pCommandList->ResourceBarrier(1, &barrier);
}

void CommandListDx12::SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, const D3D12_CPU_DESCRIPTOR_HANDLE* depthStencilHandle /*= nullptr*/) noexcept
{
    m_pCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, depthStencilHandle);
}

void CommandListDx12::SetPipelineState(ID3D12PipelineState* state) noexcept
{
    m_pCommandList->SetPipelineState(state);
}

void CommandListDx12::SetGraphicsRootSignature(ID3D12RootSignature* rootSignature) noexcept
{
    m_pCommandList->SetGraphicsRootSignature(rootSignature);
}

void CommandListDx12::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology) noexcept
{
    m_pCommandList->IASetPrimitiveTopology(topology);
}

void CommandListDx12::SetVertexBuffers(uint32_t startSlot, uint32_t numViews, const D3D12_VERTEX_BUFFER_VIEW* views) noexcept
{
    m_pCommandList->IASetVertexBuffers(startSlot, numViews, views);
}

void CommandListDx12::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* indexBuffer) noexcept
{
    m_pCommandList->IASetIndexBuffer(indexBuffer);
}

void CommandListDx12::SetViewports(uint32_t numViewports, const D3D12_VIEWPORT* viewports) noexcept
{
    m_pCommandList->RSSetViewports(numViewports, viewports);
}

void CommandListDx12::SetScissorRects(uint32_t numRects, const D3D12_RECT* rects) noexcept
{
    m_pCommandList->RSSetScissorRects(numRects, rects);
}

void CommandListDx12::SetGraphicsRoot32BitConstant(uint32_t rootParamIndx, uint32_t srcData, uint32_t destOffsetIn32BitValues) noexcept
{
    m_pCommandList->SetGraphicsRoot32BitConstant(rootParamIndx, srcData, destOffsetIn32BitValues);
}

void CommandListDx12::SetGraphicsRoot32BitConstants(uint32_t rootParamIndx, uint32_t num32BitValueToSet, const void* pData, uint32_t destOffsetIn32BitValues) noexcept
{
    m_pCommandList->SetGraphicsRoot32BitConstants(rootParamIndx, num32BitValueToSet, pData, destOffsetIn32BitValues);
}

void CommandListDx12::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation /*= 0*/, int32_t baseVertexLocation /*= 0*/, uint32_t startInstanceLocation /*= 0*/) noexcept
{
    m_pCommandList->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}
