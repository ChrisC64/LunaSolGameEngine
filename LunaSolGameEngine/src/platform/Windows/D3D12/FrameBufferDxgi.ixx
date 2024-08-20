module;
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dx12_root_signature.h>
#include <wrl/client.h>

export module D3D12Lib.FrameBufferDxgi;
import <vector>;
import <cstdint>;
import <cassert>;
import <exception>;
import <format>;

import D3D12Lib.FrameDx12;
import Engine.EngineCodes;
import Win32.ComUtils;
import Win32.Utils;

namespace WRL = Microsoft::WRL;

export namespace LS::Platform::Dx12
{
    class FrameBufferDxgi
    {
    public:
        FrameBufferDxgi(uint32_t size, uint32_t width, uint32_t height,
            DXGI_FORMAT format,
            DXGI_SWAP_EFFECT swapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            DXGI_SCALING scaling = DXGI_SCALING_NONE, DXGI_ALPHA_MODE alphaMode = DXGI_ALPHA_MODE_UNSPECIFIED, 
            uint32_t flags = 0,
            DXGI_USAGE bufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            uint32_t sampleCount = 1, uint32_t sampleQuality = 0, 
            bool isStereo = false) noexcept :
            m_frames(size)
        {
            m_swapChainDesc = DXGI_SWAP_CHAIN_DESC1 {.Width = width,
                .Height = height,
                .Format = format,
                .Stereo = isStereo,
                .SampleDesc{.Count = sampleCount, .Quality = sampleQuality},
                .BufferUsage = bufferUsage,
                .BufferCount = size,
                .Scaling = scaling,
                .SwapEffect = swapEffect,
                .AlphaMode = alphaMode,
                .Flags = flags };
        }

        FrameBufferDxgi(const FrameBufferDxgi&) noexcept = default;
        FrameBufferDxgi(FrameBufferDxgi&&) noexcept = default;

        FrameBufferDxgi& operator=(const FrameBufferDxgi&) noexcept = default;
        FrameBufferDxgi& operator=(FrameBufferDxgi&&) noexcept = default;

        [[nodiscard]] auto Initialize(Microsoft::WRL::ComPtr<IDXGIFactory2> pFactory, ID3D12CommandQueue* commandQueue, 
            HWND hwnd,
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapStart, ID3D12Device* device,
            DXGI_SWAP_CHAIN_FULLSCREEN_DESC* fullscreenDesc = nullptr, IDXGIOutput* dxgiOutput = nullptr) -> LS::System::ErrorCode
        {
            assert(pFactory && "Factory cannot be null");
            pFactory;
            Microsoft::WRL::ComPtr<IDXGISwapChain1> temp;
            auto hr = pFactory->CreateSwapChainForHwnd(commandQueue, hwnd, &m_swapChainDesc, fullscreenDesc, dxgiOutput, &temp);
            if (FAILED(hr))
            {
                return LS::System::CreateFailCode("Failed to create swap chain for hwnd");
            }
            assert(temp && "Failed to create swapchain");
            temp.As(&m_pSwapChain);
            m_pSwapChain->SetMaximumFrameLatency(static_cast<UINT>(m_swapChainDesc.BufferCount));
            BuildFrames(rtvHeapStart, device);
            return LS::System::CreateSuccessCode();
        }
        // When we resize the frame buffer, these would be obsolete, right? I perhaps
        // should consider how these are shared. A pointer would be nullified, but a reference?
        // That would cause UB if the reference held on by another is no longer existing. 
        // Maybe a pointer to the frame should be used, easier and clearer to state "This could
        // be null at some point, check it..." though a weak_ptr also implies and enforces that
        // so maybe I just use those with shared_ptr initialized in the array, but that 
        // leaves me with three allocated pointers in some memory land, and does it make sense? 
        /**
         * @brief Get the frame at index, if out of range, will throw exception
         * @param pos index starts at 0 to n - 1 frames given to frame buffer
         * @return @link LS::Platform::Dx12::FrameDx12
        */
        [[nodiscard]] auto GetCurrentFrameAsRef() const noexcept -> const FrameDx12&
        {
            const auto pos = GetCurrentIndex();
            return m_frames.at(pos);
        }

        [[nodiscard]] auto GetCurrentFrameAsPtr() const noexcept -> const FrameDx12*
        {
            const auto pos = GetCurrentIndex();
            return &m_frames.at(pos);
        }

        /**
         * @brief Obtain a back buffer frame at given position (index starts at 0)
         * @param pos Backbuffer frame to obtain
         * @return Frame if available otherwise throws std::out_of_range exception
        */
        [[nodiscard]] auto GetFrameAt(uint32_t pos) const -> const FrameDx12*
        {
            if (pos >= m_frames.size())
            {
                throw std::out_of_range(std::format("Out of bounds Access! Position {} for Frame Count of {}", pos, m_swapChainDesc.BufferCount));
            }

            return &m_frames.at(pos);
        }

        /**
         * @brief The current frame buffer's back buffer index
         * @return
        */
        [[nodiscard]] auto GetCurrentIndex() const -> uint64_t
        {
            return m_pSwapChain->GetCurrentBackBufferIndex();
        }

        /**
         * @brief The number of frames that have been presented up to this point
         * @return Number of frames presented to the front of the swap chain
        */
        [[nodiscard]] auto GetTotalFrames() const -> uint64_t
        {
            return m_frameIndex;
        }

        [[nodiscard]] auto GetFrameLatencyWaitable() const -> HANDLE
        {
            return m_pSwapChain->GetFrameLatencyWaitableObject();
        }

        [[nodiscard]] auto Present() const -> LS::System::ErrorCode
        {
            // TODO: Clear whole screen, maybe later can optimize to clear a part of the screen
            // using this. 
            static const DXGI_PRESENT_PARAMETERS params{ .DirtyRectsCount = 0,
            .pDirtyRects = NULL, .pScrollRect = NULL, .pScrollOffset = NULL };

            auto hr = m_pSwapChain->Present1(m_syncInterval, 0, &params);
            if (FAILED(hr))
            {
                return LS::System::CreateFailCode(LS::Win32::HrToString(hr));
            }

            ++m_frameIndex;
            return LS::System::CreateSuccessCode();
        }

        void EnableVsync()
        {
            m_syncInterval = 1;
        }

        void DisableVsync()
        {
            m_syncInterval = 0;
        }

        void BuildFrames(D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapStart, ID3D12Device* device)
        {
            assert(m_pSwapChain && "Cannot build frames with NULL swap chain");
            assert(device && "The device cannot be null when building the frame buffer");
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeapStart);
            const auto size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

            for (auto i = 0u; i < m_swapChainDesc.BufferCount; ++i)
            {
                m_frames[i].InitFrame(m_pSwapChain.Get(), i);
                device->CreateRenderTargetView(m_frames[i].GetFrame().Get(), nullptr, rtvHandle);
                m_frames[i].SetDescriptorHandle(rtvHandle);
                rtvHandle.Offset(1, size);
                m_frames[i].SetDxgiFormat(m_swapChainDesc.Format);
            }
        }

        void CleanupFrames() noexcept
        {
            for (auto& frame : m_frames)
            {
                frame.SetFrame(nullptr);
            }

            m_frames.clear();
            m_frames.resize(m_swapChainDesc.BufferCount);
        }

        [[nodiscard]] auto ResizeFrames(uint32_t width, uint32_t height,
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapStart, ID3D12Device* device) noexcept -> LS::System::ErrorCode
        {
            CleanupFrames();

            auto hr = m_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, m_swapChainDesc.Flags);
            if (FAILED(hr))
            {
                return LS::System::CreateFailCode(LS::Win32::HrToString(hr));
            }

            BuildFrames(rtvHeapStart, device);

            return LS::System::CreateSuccessCode();
        }

        void WaitOnFrameBuffer() noexcept
        {
            auto result = WaitForSingleObjectEx(m_pSwapChain->GetFrameLatencyWaitableObject(), 1000, true);
            //TODO: implement error handling
            switch (result)
            {
            case WAIT_ABANDONED:
            {
                break;
            }
            case WAIT_IO_COMPLETION:
            {
                break;
            }
            case WAIT_OBJECT_0:
            {
                break;
            }
            case WAIT_TIMEOUT:
            {
                break;
            }
            case WAIT_FAILED:
            {
                break;
            }
            default:
                break;
            }
        }

    private:
        DXGI_SWAP_CHAIN_DESC1                    m_swapChainDesc;
        uint32_t                                 m_syncInterval = 1;
        mutable uint64_t                         m_frameIndex = 1;
        WRL::ComPtr<IDXGISwapChain3>             m_pSwapChain;
        WRL::ComPtr<IDXGIFactory2>               m_pFactory;
        std::vector<FrameDx12>                   m_frames;
    };
}