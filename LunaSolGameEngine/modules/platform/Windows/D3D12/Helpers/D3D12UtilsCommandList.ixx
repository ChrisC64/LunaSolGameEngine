/**
 * @brief This file contains multiple ways to operate on and utilize the various 
 * CommandQueues and CommandList objects for DirectX12. 
*/
module;
#include <d3d12.h>
#include <cassert>
#include <span>
#include <format>
#include <wrl/client.h>
#include <d3dx12\d3dx12_barriers.h>
#include "engine\EngineLogDefines.h"
#include "platform\Windows\Win32\WinApiUtils.h"
#include <chrono>
//NOMINMAX doesn't seem to work here. Though this might be because of std
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif
export module D3D12Lib:D3D12Utils.Commands;

namespace WRL = Microsoft::WRL;

export namespace LS::Platform::Dx12
{
    // Creation Methods //

    [[nodiscard]] inline auto CreateCommandQueue(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_PRIORITY priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL) noexcept -> WRL::ComPtr<ID3D12CommandQueue>
    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = type;
        desc.Priority = priority;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;

        WRL::ComPtr<ID3D12CommandQueue> pQueue;
        const auto hr = pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&pQueue));

        if (FAILED(hr))
        {
            // Handle error
            return nullptr;
        }

        return pQueue;
    }

    /**
     * @brief Transitions a resource from current state (before) to next state (after)
     * @param before The current state this resource is in
     * @param after The state to transition into
     * @param pResource The resource
     * @return D3D12_RESOURCE_BARRIER
    */
    [[nodiscard]] inline auto CreateResourceTransition(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, ID3D12Resource* pResource) noexcept -> D3D12_RESOURCE_BARRIER
    {
        auto transition = CD3DX12_RESOURCE_BARRIER::Transition(pResource, before, after);
        return transition;
    }

    // SET FUNCTIONS // 
    inline void SetViewPorts(WRL::ComPtr<ID3D12GraphicsCommandList>& command, std::span<D3D12_VIEWPORT> viewports) noexcept
    {
        assert(command);
        assert(viewports.size() <= UINT32_MAX);
        command->RSSetViewports((UINT)viewports.size(), &viewports.front());
    }

    inline void SetScissorRects(WRL::ComPtr<ID3D12GraphicsCommandList>& command, std::span<D3D12_RECT> rects) noexcept
    {
        assert(command);
        assert(rects.size() <= UINT32_MAX);
        command->RSSetScissorRects((UINT)rects.size(), &rects.front());
    }

    /**
     * @brief Waits for the fence value and fires off the Fence Event when it is reached
     * @param fence THe fence to associate with the event
     * @param fenceValue The value to wait for (check Signal to retrieve this value)
     * @param fenceEvent The event to fire off after completion
     * @param duration How long to wait for the event before not abandoning
    */
    inline void WaitForFenceValue(WRL::ComPtr<ID3D12Fence>& fence, uint64_t fenceValue, HANDLE fenceEvent,
        std::chrono::milliseconds duration = std::chrono::milliseconds::max()) noexcept
    {
        if (fence->GetCompletedValue() < fenceValue)
        {
            const auto hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);

            if (FAILED(hr))
            {
                const auto msg = Win32::HrToString(hr);
                LS_LOG_ERROR(std::format("An error occurred when trying to set an Event On Completion: {}", hr));
            }

            ::WaitForSingleObject(fenceEvent, static_cast<DWORD>(duration.count()));
        }
    }

    /**
     * @brief Signals to the GPU to get the fence value and return it.
     * @param pQueue The queue to signal to
     * @param pFence A fence to pass into this command queue
     * @param fenceValue The current value to be incremented and pass as the signal to the GPU
     * @return A fence value that should be signaled by the CPU to assure all processes by the GPU are no longer "in-flight"
    */
    [[nodiscard]] inline auto Signal(WRL::ComPtr<ID3D12CommandQueue>& pQueue, WRL::ComPtr<ID3D12Fence>& pFence,
        const uint64_t fenceValue) noexcept -> uint64_t
    {
        uint64_t fenceValueForSignal = fenceValue + 1;
        const auto hr = pQueue->Signal(pFence.Get(), fenceValueForSignal);

        if (FAILED(hr))
        {
            const auto msg = Win32::HrToString(hr);
            LS_LOG_ERROR(std::format("An error occurred when signaling the fence value: {}", msg));
        }

        return fenceValueForSignal;
    }

    /**
     * @brief Tries to "flush" the command queue by immediately signaling the queue and then waiting for the event to finish
     * @param pQueue The command queue to "flush"
     * @param pFence The fence to use with
     * @param fenceValue The value to signal
     * @param fenceEvent The event to wait for
    */
    inline void Flush(WRL::ComPtr<ID3D12CommandQueue>& pQueue, WRL::ComPtr<ID3D12Fence>& pFence,
        uint64_t& fenceValue, HANDLE fenceEvent) noexcept
    {
        uint64_t fenceValueForSignal = Signal(pQueue, pFence, fenceValue);

        WaitForFenceValue(pFence, fenceValueForSignal, fenceEvent);
    }

    /**
     * @brief Notifies the GPU that the following resources are ready to begin their transitions. 
     * @param pCommList The Command list to use
     * @param barriers The barrier(s) to use 1 or more. 
    */
    inline void TransitionTo(ID3D12GraphicsCommandList* pCommList, std::span<D3D12_RESOURCE_BARRIER> barriers)
    {
        assert(pCommList);
        assert(barriers.size() > 0);
        pCommList->ResourceBarrier(barriers.size(), barriers.data());
    }
}