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

CommandQueueDx12::CommandQueueDx12(const WRL::ComPtr<ID3D12Device4>& pDevice, D3D12_COMMAND_LIST_TYPE type, uint32_t queueSize /*= 100*/) : m_queue(queueSize),
m_pDevice(pDevice),
m_commListType(type),
m_fenceValue(0)
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    auto hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pCommandQueue));
    LS::Utils::ThrowIfFailed(hr, "Failed to create command queue in CommandQueueDx12 ctor");

    hr = m_pDevice->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
    LS::Utils::ThrowIfFailed(hr, "Failed to create fence in CommandQueueDx12 ctor");

    m_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(m_fenceEvent && "Failed to create fence event handle.");
}

auto CommandQueueDx12::ExecuteCommandList() -> uint64_t
{
    // Execute the command lists
    std::vector<ID3D12CommandList*> commands(m_queue.size());

    for (auto i = 0u; i < m_queue.size(); ++i)
    {
        commands[i] = m_queue[i]->GetCommandList().Get();
        m_queue[i]->IncrementFenceValue();
    }

    m_pCommandQueue->ExecuteCommandLists(static_cast<UINT>(commands.size()), commands.data());

    // Obtain the signal and set it to return to the user
    m_fenceValue = LS::Platform::Dx12::Signal(m_pCommandQueue, m_pFence, m_fenceValue);
    return m_fenceValue;
}

void CommandQueueDx12::WaitForGpu(uint64_t fenceValue, std::chrono::milliseconds duration)
{
    WaitForFenceValue(m_pFence, fenceValue, m_fenceEvent, duration);
    m_queue.clear();
}

void CommandQueueDx12::WaitForGpuEx(uint64_t fenceValue, HANDLE* handles, DWORD count, std::chrono::milliseconds duration)
{
    const std::vector<HANDLE> waitables(handles, handles + count);
    WaitForFenceValueMany(m_pFence, fenceValue, m_fenceEvent, waitables, duration);
    m_queue.clear();
}

void CommandQueueDx12::Flush() noexcept
{
    m_fenceValue = LS::Platform::Dx12::Flush(m_pCommandQueue, m_pFence, m_fenceValue, m_fenceEvent);
    m_queue.clear();
}

void CommandQueueDx12::FlushAndWaitMany(const std::vector<HANDLE>& handles) noexcept
{
    m_fenceValue = LS::Platform::Dx12::FlushAndWaitForMany(m_pCommandQueue, m_pFence, m_fenceValue, m_fenceEvent, handles);
    m_queue.clear();
}

void CommandQueueDx12::QueueCommands(std::vector<LS::Platform::Dx12::CommandListDx12*> commands) noexcept
{
    for (auto c : commands)
    {
        m_queue.push_back(c);
    }
}

void CommandQueueDx12::QueueCommand(LS::Platform::Dx12::CommandListDx12* const command) noexcept
{
    m_queue.push_back(command);
}