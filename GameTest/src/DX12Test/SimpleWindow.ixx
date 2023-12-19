module;
#include <filesystem>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <stdint.h>
#include <array>
#include <vector>
#include <span>
#include <algorithm>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <directx/d3dx12.h>
#include <thread>
#include <optional>
#include <stdexcept>
#include <mutex>
#include <ranges>
#include <semaphore>
#include <functional>
#include <DirectXMath.h>
#include <d3dx12.h>
#include <format>
#include <memory>
#include <array>
#include <cstdint>
#include <iostream>

export module DX12.SimpleWindow;

import LSData;
import Engine.App;
import D3D12Lib;
import Platform.Win32Window;
import Helper.LSCommonTypes;
import Helper.PipelineFactory;
import Util.MSUtils;
import DXGIHelper;

namespace gt::dx12
{
    namespace WRL = Microsoft::WRL;
    using namespace DirectX;

    export class SimpleWindow : public LS::LSApp
    {
    public:
        SimpleWindow(uint32_t width, uint32_t height) : m_frameBuffer(NUM_OF_FRAMES)
        {
            Window = LS::BuildWindow(width, height, L"Simple Window");
            m_settings.Width = width;
            m_settings.Height = height;
            m_settings.FeatureLevel = D3D_FEATURE_LEVEL_12_0;
            m_settings.Hwnd = (HWND)Window->GetHandleToWindow();
            m_device = std::make_unique<LS::Platform::Dx12::DeviceD3D12>(m_settings);

            UINT flag = 0;
#ifdef _DEBUG
            flag = (UINT)DXGI_CREATE_FACTORY_DEBUG;
#endif
            auto factory = LS::Win32::CreateFactory(flag).value();

            LS::Utils::ThrowIfFailed(factory.As(&m_pFactory), "Failed to create DXGI Factory");
        }

        ~SimpleWindow() = default;

        auto Initialize(const LS::LSCommandArgs& args) -> LS::System::ErrorCode override;
        void Run() override;

    private:
        const uint32_t NUM_OF_FRAMES = 3;

        WRL::ComPtr<IDXGIFactory4> m_pFactory;
        WRL::ComPtr<IDXGIAdapter1> m_pAdapter;
        LS::Platform::Dx12::DescriptorHeapDx12 m_heapRtv{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV, NUM_OF_FRAMES };
        LS::Platform::Dx12::DescriptorHeapDx12 m_heapSrv{D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE };
        uint64_t m_fenceValue;

        // My stuff to replace above // 
        Ref<LS::Platform::Dx12::DeviceD3D12> m_device;
        Ref<LS::Platform::Dx12::CommandQueueDx12> m_queue;
        Ref<LS::Platform::Dx12::CommandListDx12> m_commandList;
        LS::Platform::Dx12::D3D12Settings m_settings{};
        LS::Platform::Dx12::FrameBufferDxgi m_frameBuffer;

        [[nodiscard]] bool LoadPipeline();
        void LoadAssets();
        void PopulateCommandList();
        void OnRender();
        void OnDestroy();
        void OnUpdate();
        void WaitForPreviousFrame();
    };
}

module : private;

auto gt::dx12::SimpleWindow::Initialize(const LS::LSCommandArgs& args) -> LS::System::ErrorCode
{
    if (!LoadPipeline())
    {
        return LS::System::CreateFailCode("Failed to load pipeline");
    }
    LoadAssets();
    return LS::System::CreateSuccessCode();
}

void gt::dx12::SimpleWindow::Run()
{
    Window->Show();

    IsRunning = true;
    auto currWidth = Window->GetWidth();
    auto currHeight = Window->GetHeight();
    while (Window->IsOpen())
    {
        Window->PollEvent();
        m_frameBuffer.WaitOnFrameBuffer();
        if (currWidth != Window->GetWidth() || currHeight != Window->GetHeight())
        {
            m_queue->Flush();
            if (auto result = m_frameBuffer.ResizeFrames(Window->GetWidth(), Window->GetHeight(), m_heapRtv.GetHeapStartCpu(), m_device->GetDevice().Get()); !result)
            {
                throw std::runtime_error(std::format("Failed to resize frame buffer. Error: {}", result.Message()));
            }
            currWidth = Window->GetWidth();
            currHeight = Window->GetHeight();
        }
        
        OnRender();
        OnUpdate();
    }

    OnDestroy();
}

bool gt::dx12::SimpleWindow::LoadPipeline()
{
    auto adapterOptional = LS::Win32::GetHardwareAdapter(m_pFactory.Get(), true);
    if (!adapterOptional)
    {
        std::cout << "Failed to obtain adapter with hardware requirement.\n";
        return false;
    }

    m_pAdapter = adapterOptional.value();

    if (!m_device->CreateDevice(m_pAdapter))
    {
        std::cout << "Failed to create the DX12 Device class.\n";
        return false;
    }

    WRL::ComPtr<ID3D12Device4> device4;
    auto device = m_device->GetDevice();
    auto deviceHr = device.As(&device4);
    if (FAILED(deviceHr))
    {
        std::cout << "Failed to to create ID3D12Device4 interface.\n";
        return false;
    }
    auto type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    // TODO: Create an "Initialize" state to pass the device and setup these objects
    // instead of just in the default constructor
    m_queue = std::make_unique<LS::Platform::Dx12::CommandQueueDx12>(device4, type);
    m_commandList = std::make_unique<LS::Platform::Dx12::CommandListDx12>(device4.Get(), type, "main command list");

    // Setup swap chain
    const auto& window = Window;
    HWND hwnd = reinterpret_cast<HWND>(window->GetHandleToWindow());

    DXGI_SWAP_CHAIN_DESC1 swapchainDesc1{};
    swapchainDesc1.BufferCount = NUM_OF_FRAMES;
    swapchainDesc1.Width = window->GetWidth();
    swapchainDesc1.Height = window->GetHeight();
    swapchainDesc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc1.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    swapchainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc1.SampleDesc.Count = 1;
    swapchainDesc1.SampleDesc.Quality = 0;
    swapchainDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchainDesc1.Scaling = DXGI_SCALING_STRETCH;
    swapchainDesc1.Stereo = FALSE;

    // Create the swap chain //

    // Create Descriptor Heaps for RTV/SRV // 

    // This is the RTV descriptor heap (render target view)
    {
        m_heapRtv.Initialize(device.Get());
    }

    // Constant Buffer View/Shader Resource View/Unordered Access View types (this one is just the SRV)
    {
        m_heapSrv.Initialize(device.Get());
    }

    // Since we are using an HWND (Win32) system, we can create the swapchain for HWND 
    {
        auto result = m_frameBuffer.InitializeFrameBuffer(m_pFactory, m_queue->GetCommandQueue().Get(), hwnd, swapchainDesc1, m_heapRtv.GetHeapStartCpu(), m_device->GetDevice().Get());
        if (!result)
        {
            std::cout << result.Message();
            return false;
        }
        // Helper function that displays our display's resolution and refresh rates and other information 
        LS::Win32::LogAdapters(m_pFactory.Get());

        // Don't allot ALT+ENTER fullscreen
        m_pFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
    }

    return true;
}

void gt::dx12::SimpleWindow::LoadAssets()
{
    // Create Event //
    static const wchar_t* eventName = L"Fence Event";
    auto fenceEvent = CreateEvent(nullptr, FALSE, FALSE, eventName);
    if (fenceEvent == nullptr)
    {
        LS::Utils::ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), "Failed to create event handle.");
    }
    m_queue->SetFenceEvent(fenceEvent);
}

void gt::dx12::SimpleWindow::PopulateCommandList()
{
    auto frame = m_frameBuffer.GetCurrentFrameAsRef();
    m_commandList->ResetCommandList();
    m_commandList->TransitionResource(frame.GetFramePtr(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_heapRtv.GetHeapStartCpu(), m_frameBuffer.GetCurrentIndex(), m_heapRtv.GetSize());
    m_commandList->SetRenderTarget(rtvHandle);

    const std::array<float, 4> clearColor{ 0.0f, 0.12f, 0.34f, 1.0f };
    m_commandList->Clear(clearColor, rtvHandle);

    m_commandList->TransitionResource(frame.GetFramePtr(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->Close();
    m_queue->QueueCommand(m_commandList.get());
}

void gt::dx12::SimpleWindow::OnRender()
{
    PopulateCommandList();
    m_fenceValue = m_queue->ExecuteCommandList();
    if (auto result = m_frameBuffer.Present(); !result)
    {
        throw std::runtime_error(std::format("Error with Presenting Frame: {}", result.Message()));
    }
    WaitForPreviousFrame();
}

void gt::dx12::SimpleWindow::OnDestroy()
{
}

void gt::dx12::SimpleWindow::OnUpdate()
{

}

void gt::dx12::SimpleWindow::WaitForPreviousFrame()
{
    m_queue->WaitForGpu(m_fenceValue);
}