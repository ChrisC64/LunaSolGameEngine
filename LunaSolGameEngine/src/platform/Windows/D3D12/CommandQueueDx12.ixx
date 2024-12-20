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
import <span>;
import <thread>;
import <mutex>;

import D3D12Lib.CommandListDx12;
import Engine.EngineCodes;

namespace WRL = Microsoft::WRL;

export namespace LS::Platform::Dx12
{
    class CommandQueueDx12
    {
    public:
        explicit CommandQueueDx12(D3D12_COMMAND_LIST_TYPE type);
        CommandQueueDx12(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type);
        ~CommandQueueDx12();

        CommandQueueDx12(const CommandQueueDx12&) = delete;
        CommandQueueDx12& operator=(const CommandQueueDx12&) = delete;

        CommandQueueDx12(CommandQueueDx12&& other) = default;
        CommandQueueDx12& operator=(CommandQueueDx12&& other) = default;

        [[nodiscard]] auto Initialize(ID3D12Device* pDevice) noexcept -> LS::System::ErrorCode;
        /**
         * @brief Submits work to the GPU
         * @return The new fence value that is associated to this command queue's work.
         */
        [[nodiscard]] auto ExecuteCommandList() -> uint64_t;
        void WaitForGpu(uint64_t fenceValue, std::chrono::milliseconds duration = std::chrono::milliseconds::max()) noexcept;
        void WaitForGpuEx(uint64_t fenceValue, HANDLE* handles, DWORD count, std::chrono::milliseconds duration = std::chrono::milliseconds::max()) noexcept;
        void Flush() noexcept;
        void FlushAndWaitMany(const std::vector<HANDLE>& handles) noexcept;
        void QueueCommands(std::span<CommandListDx12*> commands) noexcept;
        void QueueCommand(const CommandListDx12* const command) noexcept;

        [[nodiscard]] auto GetCommandQueue() const noexcept -> const WRL::ComPtr<ID3D12CommandQueue>&
        {
            return m_pCommandQueue;
        }
        
        [[nodiscard]] auto GetFence() const noexcept -> const WRL::ComPtr<ID3D12Fence>&
        {
            return m_pFence;
        }

        [[nodiscard]] bool IsFenceComplete(uint64_t fenceValue);

    private:
        using CommandQueue = std::queue<const CommandListDx12*>;
        
        CommandQueue                            m_queue;
        WRL::ComPtr<ID3D12Device>               m_pDevice;
        WRL::ComPtr<ID3D12CommandQueue>         m_pCommandQueue;
        WRL::ComPtr<ID3D12Fence>                m_pFence;

        HANDLE                                  m_fenceEvent;
        uint64_t                                m_fenceValue;
        D3D12_COMMAND_LIST_TYPE                 m_commListType;
        std::mutex                              m_queueMutex;
        void PollForCurrentFenceValue();
        void Shutdown() noexcept;
        void ClearQueue(CommandQueue& queue);
    };
}

module : private;

import <cassert>;
import Win32.ComUtils;
import Engine.EngineCodes;
import D3D12Lib.D3D12Utils.Commands;

namespace WRL = Microsoft::WRL;
using namespace LS::Platform::Dx12;

void CommandQueueDx12::ClearQueue(CommandQueue& queue)
{
    const std::lock_guard<std::mutex> lock(m_queueMutex);
    for(; !queue.empty(); queue.pop())
    { 
    }
}

CommandQueueDx12::CommandQueueDx12(D3D12_COMMAND_LIST_TYPE type) : m_commListType(type),
m_pDevice(nullptr),
m_fenceValue(0)
{

}

CommandQueueDx12::CommandQueueDx12(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type) : m_pDevice(pDevice),
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

    hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
    m_fenceValue++;
    if (FAILED(hr))
    {
        return LS::System::CreateFailCode("Failed to create fence in CommandQueueDx12::Initialize");
    }

    m_fenceEvent = CreateEventEx(nullptr, TEXT("CommandQueue_FenceEvent"), false, EVENT_ALL_ACCESS);
    assert(m_fenceEvent && "Failed to create fence event handle.");
    return LS::System::CreateSuccessCode();
}

auto CommandQueueDx12::ExecuteCommandList() -> uint64_t
{
    // Execute the command lists
    std::vector<ID3D12CommandList*> commands(m_queue.size());

    for (auto i = 0; !m_queue.empty(); m_queue.pop(), ++i)
    {
        commands[i] = m_queue.front()->GetCommandListConst().Get();
    }

    m_pCommandQueue->ExecuteCommandLists(static_cast<UINT>(commands.size()), commands.data());

    // Obtain the signal and set it to return to the user
    ::Signal2(m_pCommandQueue, m_pFence, m_fenceValue);
    return m_fenceValue;
}

void CommandQueueDx12::WaitForGpu(uint64_t fenceValue, std::chrono::milliseconds duration) noexcept
{
    ::WaitForFenceValue(m_pFence, fenceValue, m_fenceEvent, duration);
}

void CommandQueueDx12::WaitForGpuEx(uint64_t fenceValue, HANDLE* handles, DWORD count, std::chrono::milliseconds duration) noexcept
{
    const std::vector<HANDLE> waitables(handles, handles + count);
    ::WaitForFenceValueMany(m_pFence, fenceValue, m_fenceEvent, waitables, duration);
}

void CommandQueueDx12::Flush() noexcept
{
    m_fenceValue = ::Flush(m_pCommandQueue, m_pFence, m_fenceValue, m_fenceEvent);
    ClearQueue(m_queue);
}

void CommandQueueDx12::FlushAndWaitMany(const std::vector<HANDLE>& handles) noexcept
{
    m_fenceValue = ::FlushAndWaitForMany(m_pCommandQueue, m_pFence, m_fenceValue, m_fenceEvent, handles);
    ClearQueue(m_queue);
}

void CommandQueueDx12::QueueCommands(std::span<CommandListDx12*> commands) noexcept
{
    const std::lock_guard<std::mutex> lock(m_queueMutex);
    for (auto c : commands)
    {
        m_queue.push(c);
    }
}

void CommandQueueDx12::QueueCommand(const CommandListDx12* const command) noexcept
{
    const std::lock_guard<std::mutex> lock(m_queueMutex);
    m_queue.push(command);
}

void CommandQueueDx12::Shutdown() noexcept
{
    Flush();
    CloseHandle(m_fenceEvent);
}

void CommandQueueDx12::PollForCurrentFenceValue()
{
    m_fenceValue = std::max(m_fenceValue, m_pFence->GetCompletedValue());
}

[[nodiscard]] bool CommandQueueDx12::IsFenceComplete(uint64_t fenceValue)
{
    if (fenceValue > m_fenceValue)
    {
        PollForCurrentFenceValue();
    }

    return fenceValue <= m_fenceValue;
}