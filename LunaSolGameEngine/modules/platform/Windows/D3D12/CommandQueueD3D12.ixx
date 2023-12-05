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

namespace WRL = Microsoft::WRL;

export namespace LS::Platform::Dx12
{
    class CommandQueueDx12
    {
    public:
        CommandQueueDx12(const WRL::ComPtr<ID3D12Device4>& pDevice, D3D12_COMMAND_LIST_TYPE type);
        ~CommandQueueDx12() = default;

        CommandQueueDx12(const CommandQueueDx12&) = delete;
        CommandQueueDx12& operator=(const CommandQueueDx12&) = delete;

        CommandQueueDx12(CommandQueueDx12&& other) = default;
        CommandQueueDx12& operator=(CommandQueueDx12&& other) = default;

        auto GetCommandList() -> WRL::ComPtr<ID3D12GraphicsCommandList>;
        auto CreateCommandAllocator() -> WRL::ComPtr<ID3D12CommandAllocator>;
        auto CreateCommandList(WRL::ComPtr<ID3D12CommandAllocator>& allocator) -> WRL::ComPtr<ID3D12GraphicsCommandList>;
        auto CreateCommandList() -> WRL::ComPtr<ID3D12GraphicsCommandList>;
        auto ExecuteCommandList(WRL::ComPtr<ID3D12GraphicsCommandList>& commandList) -> uint64_t;
        void WaitForFenceValue(uint64_t fenceValue,
            std::chrono::milliseconds duration = std::chrono::milliseconds::max());
        void Flush() noexcept;

        auto GetCommandQueue() const noexcept -> WRL::ComPtr<ID3D12CommandQueue>
        {
            return m_pCommandQueue;
        }
        
        auto GetFence() const noexcept -> WRL::ComPtr<ID3D12Fence>
        {
            return m_pFence;
        }

        void SetFenceEvent(HANDLE fenceEvent) noexcept
        {
            m_fenceEvent = fenceEvent;
        }

        auto IsFenceComplete(uint64_t fenceValue) const noexcept -> bool;

        void QueueCommands(std::vector<const LS::Platform::Dx12::CommandListDx12*> commands) noexcept;
        auto ExecuteCommands() noexcept -> uint64_t;

    private:
        struct CommandAllocatorEntry
        {
            uint64_t                            FenceValue;
            WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
        };

        using CommandAllocQueue = std::queue<CommandAllocatorEntry>;
        using CommandListQueue = std::queue<WRL::ComPtr<ID3D12GraphicsCommandList>>;

        WRL::ComPtr<ID3D12Device4>              m_pDevice;
        WRL::ComPtr<ID3D12CommandQueue>         m_pCommandQueue;
        WRL::ComPtr<ID3D12Fence>                m_pFence
            ;
        HANDLE                                  m_fenceEvent;
        uint64_t                                m_fenceValue;
        D3D12_COMMAND_LIST_TYPE                 m_commListType;

        CommandAllocQueue                       m_commandAllocQueue;
        CommandListQueue                        m_commandListQueue;
    };
}