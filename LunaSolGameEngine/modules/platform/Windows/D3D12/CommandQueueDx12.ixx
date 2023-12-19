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

export module D3D12Lib:CommandQueueD3D12;

import :CommandListDx12;
import Engine.EngineCodes;

namespace WRL = Microsoft::WRL;

export namespace LS::Platform::Dx12
{
    class CommandQueueDx12
    {
    public:
        CommandQueueDx12(D3D12_COMMAND_LIST_TYPE type);
        CommandQueueDx12(const WRL::ComPtr<ID3D12Device4>& pDevice, D3D12_COMMAND_LIST_TYPE type, uint32_t queueSize = 0);
        ~CommandQueueDx12();

        CommandQueueDx12(const CommandQueueDx12&) = delete;
        CommandQueueDx12& operator=(const CommandQueueDx12&) = delete;

        CommandQueueDx12(CommandQueueDx12&& other) = default;
        CommandQueueDx12& operator=(CommandQueueDx12&& other) = default;

        [[nodiscard]] auto Initialize(ID3D12Device4* pDevice) noexcept -> LS::System::ErrorCode;
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
        // TODO: Other than creating the command queue, I don't seem to need it
        WRL::ComPtr<ID3D12Device4>              m_pDevice;
        WRL::ComPtr<ID3D12CommandQueue>         m_pCommandQueue;
        WRL::ComPtr<ID3D12Fence>                m_pFence;

        HANDLE                                  m_fenceEvent;
        uint64_t                                m_fenceValue;
        D3D12_COMMAND_LIST_TYPE                 m_commListType;

        void Shutdown() noexcept;
    };
}