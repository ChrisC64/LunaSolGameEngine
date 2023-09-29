module;
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include <queue>
#include <chrono>

#ifdef max
#undef max
#endif

export module D3D12Lib:CommandQueueD3D12;

namespace WRL = Microsoft::WRL;

export namespace LS::Platform::Dx12
{
    class CommandQueue
    {
    public:
        CommandQueue(const WRL::ComPtr<ID3D12Device4>& pDevice, D3D12_COMMAND_LIST_TYPE type);
        ~CommandQueue() = default;

        CommandQueue(const CommandQueue&) = delete;
        CommandQueue& operator=(const CommandQueue&) = delete;

        CommandQueue(CommandQueue&& other) = default;
        CommandQueue& operator=(CommandQueue&& other) = default;

        auto GetCommandList() -> WRL::ComPtr<ID3D12GraphicsCommandList9>;

        auto CreateCommandAllocator() -> WRL::ComPtr<ID3D12CommandAllocator>;
        auto CreateCommandList(WRL::ComPtr<ID3D12CommandAllocator>& allocator) -> WRL::ComPtr<ID3D12GraphicsCommandList9>;
        auto CreateCommandList() -> WRL::ComPtr<ID3D12GraphicsCommandList9>;
        auto ExecuteCommandList(WRL::ComPtr<ID3D12GraphicsCommandList9>& commandList) -> uint64_t;
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

    private:
        struct CommandAllocatorEntry
        {
            uint64_t                            FenceValue;
            WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
        };

        using CommandAllocQueue = std::queue<CommandAllocatorEntry>;
        using CommandListQueue = std::queue<WRL::ComPtr<ID3D12GraphicsCommandList9>>;

        WRL::ComPtr<ID3D12Device4>              m_pDevice;
        WRL::ComPtr<ID3D12CommandQueue>         m_pCommandQueue;
        WRL::ComPtr<ID3D12Fence>                m_pFence;
        HANDLE                                  m_fenceEvent;
        uint64_t                                m_fenceValue;
        D3D12_COMMAND_LIST_TYPE                 m_commListType;

        CommandAllocQueue                       m_commandAllocQueue;
        CommandListQueue                        m_commandListQueue;
    };
}