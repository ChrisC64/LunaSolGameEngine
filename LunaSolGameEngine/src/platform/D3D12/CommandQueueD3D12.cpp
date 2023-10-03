#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include <queue>
#include <cassert>
#include <exception>
#include <chrono>

import D3D12Lib;
import Util.MSUtils;

using namespace LS::Platform::Dx12;
namespace WRL = Microsoft::WRL;

CommandQueue::CommandQueue(const WRL::ComPtr<ID3D12Device4>& pDevice, D3D12_COMMAND_LIST_TYPE type) : m_pDevice(pDevice),
m_commListType(type),
m_fenceValue(0)
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    auto hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pCommandQueue));
    LS::Utils::ThrowIfFailed(hr, "Failed to create command queue in CommandQueue ctor");

    hr = m_pDevice->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
    LS::Utils::ThrowIfFailed(hr, "Failed to create fence in CommandQueue ctor");

    m_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(m_fenceEvent && "Failed to create fence event handle.");
}

auto LS::Platform::Dx12::CommandQueue::GetCommandList() -> WRL::ComPtr<ID3D12GraphicsCommandList>
{
    WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
    WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
    
    if (m_commandAllocQueue.empty())
    {
        commandAllocator = CreateCommandAllocator();
    }
    else
    {   // Check if the queued up  command allocator is able to be used and reset
        if (m_pFence->GetCompletedValue() >= m_commandAllocQueue.front().FenceValue)
        {
            commandAllocator = m_commandAllocQueue.front().CommandAllocator;
            m_commandAllocQueue.pop();
            const auto hr = commandAllocator->Reset();
            LS::Utils::ThrowIfFailed(hr, "Failed ot reset command allocator from queue");
        }
    }

    if (m_commandListQueue.empty())
    {
        commandList = CreateCommandList(commandAllocator);
    }
    else
    {
        commandList = m_commandListQueue.front();
        m_commandListQueue.pop();
        const auto hr = commandList->Reset(commandAllocator.Get(), nullptr);
        LS::Utils::ThrowIfFailed(hr, "Failed to reset command list.");
    }

    const auto hr = commandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), commandAllocator.Get());
    LS::Utils::ThrowIfFailed(hr, "Failed to associate Command Allocator with Command List");
    return commandList;
}

auto CommandQueue::CreateCommandAllocator() -> WRL::ComPtr<ID3D12CommandAllocator>
{
    WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
    const auto hr = m_pDevice->CreateCommandAllocator(m_commListType, IID_PPV_ARGS(&commandAllocator));
    LS::Utils::ThrowIfFailed(hr, "Failed to create command allocator");
    return commandAllocator;
}

auto CommandQueue::CreateCommandList(WRL::ComPtr<ID3D12CommandAllocator>& allocator) -> WRL::ComPtr<ID3D12GraphicsCommandList>
{
    WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
    const auto hr = m_pDevice->CreateCommandList(0, m_commListType, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
    LS::Utils::ThrowIfFailed(hr, "Failed to create command list");
    return commandList;
}

auto LS::Platform::Dx12::CommandQueue::CreateCommandList() -> WRL::ComPtr<ID3D12GraphicsCommandList>
{
    WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
    const auto hr = m_pDevice->CreateCommandList1(0, m_commListType, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commandList));
    LS::Utils::ThrowIfFailed(hr, "Failed to create command closed command list");

    return commandList;
}

auto CommandQueue::ExecuteCommandList(WRL::ComPtr<ID3D12GraphicsCommandList>& commandList) -> uint64_t
{
    const auto closeHr = commandList->Close();
    LS::Utils::ThrowIfFailed(closeHr, "Failed to close the command list before executing the next one.");
    // Find the command allocator set as a private data interface member 
    WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
    UINT dataSize = sizeof(commandAllocator);
    const auto hr = commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, commandAllocator.Get());
    LS::Utils::ThrowIfFailed(hr, "Failed to find the command allocator, may not be set as private interface member.");

    // Execute the command lists
    ID3D12CommandList* const ppCommandLists[] = { commandList.Get() };
    m_pCommandQueue->ExecuteCommandLists(1, ppCommandLists);

    // Obtain the signal and set it to return to the user
    const uint64_t fenceValue = LS::Platform::Dx12::Signal(m_pCommandQueue, m_pFence, m_fenceValue);
    m_commandAllocQueue.emplace(CommandAllocatorEntry{ .FenceValue = fenceValue, .CommandAllocator = commandAllocator });
    m_commandListQueue.push(commandList);

    return fenceValue;
}

void CommandQueue::WaitForFenceValue(uint64_t fenceValue, std::chrono::milliseconds duration)
{
    if (m_pFence->GetCompletedValue() < fenceValue)
    {
        LS::Utils::ThrowIfFailed(m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent));
        ::WaitForSingleObject(m_fenceEvent, static_cast<DWORD>(duration.count()));
    }
}

void LS::Platform::Dx12::CommandQueue::Flush() noexcept
{
    LS::Platform::Dx12::Flush(m_pCommandQueue, m_pFence, m_fenceValue, m_fenceEvent);
}
