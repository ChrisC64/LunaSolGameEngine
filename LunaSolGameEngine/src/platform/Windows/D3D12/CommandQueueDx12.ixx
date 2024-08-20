module;
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include <queue>
#include <chrono>
#include <vector>
#ifdef max
#undef max
#endif

export module D3D12Lib.CommandQueueD3D12;

import D3D12Lib.CommandListDx12;
import Engine.EngineCodes;

namespace WRL = Microsoft::WRL;

export namespace LS::Platform::Dx12
{
    class CommandQueueDx12
    {
    public:
        CommandQueueDx12(D3D12_COMMAND_LIST_TYPE type);
        CommandQueueDx12(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type, uint32_t queueSize = 0);
        ~CommandQueueDx12();

        CommandQueueDx12(const CommandQueueDx12&) = delete;
        CommandQueueDx12& operator=(const CommandQueueDx12&) = delete;

        CommandQueueDx12(CommandQueueDx12&& other) = default;
        CommandQueueDx12& operator=(CommandQueueDx12&& other) = default;

        [[nodiscard]] auto Initialize(ID3D12Device* pDevice) noexcept -> LS::System::ErrorCode;
        [[nodiscard]] auto ExecuteCommandList() -> uint64_t;
        void WaitForGpu(uint64_t fenceValue, std::chrono::milliseconds duration = std::chrono::milliseconds::max());
        void WaitForGpuEx(uint64_t fenceValue, HANDLE* handles, DWORD count, std::chrono::milliseconds duration = std::chrono::milliseconds::max());
        void Flush() noexcept;
        void FlushAndWaitMany(const std::vector<HANDLE>& handles) noexcept;
        void QueueCommands(std::vector<LS::Platform::Dx12::CommandListDx12*> commands) noexcept;
        void QueueCommand(LS::Platform::Dx12::CommandListDx12* const command) noexcept;

        [[nodiscard]] auto GetCommandQueue() const noexcept -> WRL::ComPtr<ID3D12CommandQueue>
        {
            return m_pCommandQueue;
        }
        
        [[nodiscard]] auto GetFence() const noexcept -> WRL::ComPtr<ID3D12Fence>
        {
            return m_pFence;
        }

        void SetFenceEvent(HANDLE fenceEvent) noexcept
        {
            m_fenceEvent = fenceEvent;
        }

    private:
        using CommandQueue = std::vector<CommandListDx12*>;
        
        CommandQueue                            m_queue;
        WRL::ComPtr<ID3D12Device>               m_pDevice;
        WRL::ComPtr<ID3D12CommandQueue>         m_pCommandQueue;
        WRL::ComPtr<ID3D12Fence>                m_pFence;

        HANDLE                                  m_fenceEvent;
        uint64_t                                m_fenceValue;
        D3D12_COMMAND_LIST_TYPE                 m_commListType;

        void Shutdown() noexcept;
    };
}

module : private;

import <cassert>;
import Win32.ComUtils;
import Engine.EngineCodes;
import D3D12Lib.D3D12Utils.Commands;

namespace WRL = Microsoft::WRL;
using namespace LS::Platform::Dx12;

CommandQueueDx12::CommandQueueDx12(D3D12_COMMAND_LIST_TYPE type) : m_commListType(type),
m_pDevice(nullptr),
m_fenceValue(0)
{

}

CommandQueueDx12::CommandQueueDx12(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type, uint32_t queueSize /*= 0*/) : m_pDevice(pDevice),
m_commListType(type),
m_fenceValue(0)
{
    const auto ec = Initialize(pDevice);
    assert(ec && ec.Message().data());
    assert(m_fenceEvent && "Failed to create fence event handle.");
}

CommandQueueDx12::~CommandQueueDx12()
{
    Shutdown();
}

auto CommandQueueDx12::Initialize(ID3D12Device* pDevice) noexcept -> LS::System::ErrorCode
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = m_commListType;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    auto hr = pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pCommandQueue));
    if (FAILED(hr))
    {
        return LS::System::CreateFailCode("Failed to create command queue in CommandQueueDx12 ctor");
    }

    hr = pDevice->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
    if (FAILED(hr))
    {
        return LS::System::CreateFailCode("Failed to create fence in CommandQueueDx12::Initialize");
    }

    m_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(m_fenceEvent && "Failed to create fence event handle.");
    return LS::System::CreateSuccessCode();
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

void CommandQueueDx12::Shutdown() noexcept
{
    Flush();
    ::CloseHandle(m_fenceEvent);
}