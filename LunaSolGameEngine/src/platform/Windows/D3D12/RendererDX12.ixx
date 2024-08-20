module;
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <directx/d3dx12.h>
#include <DirectXMath.h>
#include <d3dx12.h>

export module D3D12Lib.RendererDX12;
import <cstdint>;

import Engine.LSWindow;
import Engine.LSDevice;
import Engine.LSCamera;
import Engine.EngineCodes;
import Engine.Defines;

import D3D12Lib;
import Win32.Utils;
import Win32.ComUtils;
import DXGIHelper;

/**
 * @brief A implementation of a forward/immediate renderer for DX12 system.
 * This is a test to implement a hastly put together mode as I also want
 * to create a deferrred renderer system as well to play and understand with.
 *
 * This renderer will contain all the necessary components for users to do the following:
 * 1. Create window to draw on
 * 2. Simplify some of the complexities in "drawing" basic objects/shapes
 * 3. Manage the device's state as an FSM to perform various actions
 */

namespace WRL = Microsoft::WRL;

export namespace LS::Platform::Dx12
{
    enum class RendererState
    {
        UNINITIALIZED,// @brief Created, but hasn't been correctly setup
        INITIALIZED,// @brief User has successfully initialized the renderer
        CLEAR_RT, // @brief Clearing the render target's buffer with a color 
        PRESENT_RT, // @brief The drawing is done for one frmae, ready to present 
        WAIT_FOR_FRAME, // @brief Waiting on the GPU fence to draw the next frame
        COMMANDS_IN_FLIGHT, // @brief Commands are in flight to the GPU, executing command buffer
        PAUSED, // @brief Commands 
    };

    class RendererDX12
    {
    public:
        RendererDX12(uint32_t width, uint32_t height, uint32_t frameCount = 2) noexcept;
        ~RendererDX12() = default;

        RendererDX12& operator=(const RendererDX12&) = delete;
        RendererDX12(const RendererDX12&) = delete;

        RendererDX12(RendererDX12&&) = default;
        RendererDX12& operator=(RendererDX12&&) = default;

        [[nodiscard]]
        auto Initialize(const LSWindowBase* window) noexcept ->LS::System::ErrorCode;
        // Clear(float* rgba) -> void
        // BeginDraw();
        // EndDraw();
        // LoadShader(filepath) -> LS::System::ErrorCode;
        // LoadMesh(filepath) -> LS::System::ErrorCode;
    private:
        //D3D12Settings             m_settings;
        RendererState                           m_state = RendererState::UNINITIALIZED;
        uint64_t                                m_fenceValue = 0;
        LS::Platform::Dx12::DescriptorHeapDx12  m_heapRtv;
        LS::Platform::Dx12::DescriptorHeapDx12  m_heapSrv;
        FrameBufferDxgi                         m_frameBuffer;
        DeviceD3D12                             m_device;
        WRL::ComPtr<IDXGIFactory4>              m_pFactory;
        WRL::ComPtr<IDXGIAdapter1>              m_pAdapter;
        CommandListDx12                         m_commandList;
        CommandQueueDx12                        m_queue;
    };
}

module : private;

using namespace LS::Platform::Dx12;

RendererDX12::RendererDX12(uint32_t width, uint32_t height, uint32_t frameCount) noexcept : m_frameBuffer(frameCount,
    width, height,
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_SWAP_EFFECT_FLIP_DISCARD,
    DXGI_SCALING_NONE,
    DXGI_ALPHA_MODE_UNSPECIFIED,
    DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT),
    m_device(),
    m_pFactory(),
    m_heapRtv{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV, frameCount },
    m_heapSrv{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE },
    m_commandList(D3D12_COMMAND_LIST_TYPE_DIRECT, "main_cl"),
    m_queue(D3D12_COMMAND_LIST_TYPE_DIRECT)
{
}

auto RendererDX12::Initialize(const LSWindowBase* window) noexcept -> LS::System::ErrorCode
{
    UINT flag = 0;
#ifdef _DEBUG
    flag = (UINT)DXGI_CREATE_FACTORY_DEBUG;
#endif
    auto factory = LS::Win32::CreateDXGIFactory2(flag).value_or(nullptr);
    if (!factory)
    {
        return LS::System::ErrorCode("Failed to create DXGI Factory");
    }

    HRESULT hr = factory.As(&m_pFactory);
    hr = factory.As(&m_pFactory);
    if (FAILED(hr))
    {
        const auto msg = LS::Win32::HrToString(hr);
        return LS::System::ErrorCode(msg);
    }

    const auto adapterOptional = LS::Win32::GetHardwareAdapter(m_pFactory.Get());
    if (!adapterOptional)
    {
        return LS::System::ErrorCode("Failed to obtain adapter with hardware requirement.");
    }

    m_pAdapter = adapterOptional.value();

    auto ec = m_device.CreateDevice(m_pAdapter);
    if (!ec)
    {
        return ec;
    }

    ec = m_heapRtv.Initialize(m_device.GetDevice().Get());
    if (!ec)
    {
        return ec;
    }

    ec = m_heapSrv.Initialize(m_device.GetDevice().Get());
    if (!ec)
    {
        return ec;
    }

    WRL::ComPtr<ID3D12Device4> device4;
    auto device = m_device.GetDevice();
    auto deviceHr = device.As(&device4);
    if (FAILED(deviceHr))
    {
        return LS::System::ErrorCode("Failed to to create ID3D12Device4 interface.");
    }

    ec = m_queue.Initialize(m_device.GetDevice().Get());
    if (!ec)
    {
        return ec;
    }

    ec = m_commandList.Initialize(device4.Get());
    if (!ec)
    {
        return ec;
    }

    HWND hwnd = reinterpret_cast<HWND>(window->GetHandleToWindow());
    ec = m_frameBuffer.Initialize(m_pFactory,
        m_queue.GetCommandQueue().Get(),
        hwnd,
        m_heapRtv.GetHeapStartCpu(),
        m_device.GetDevice().Get());

    if (!ec)
    {
        return ec;
    }
    //TODO: Make optional or another setting, for now don't allow ALT+ENTER fullscreen swap
    m_pFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

    m_state = RendererState::INITIALIZED;
    return ec;
}
