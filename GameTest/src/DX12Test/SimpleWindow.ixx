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
#include <stdint.h>

export module DX12.SimpleWindow;

import <filesystem>;
import <array>;
import <vector>;
import <span>;
import <algorithm>;
import <thread>;
import <optional>;
import <stdexcept>;
import <mutex>;
import <ranges>;
import <semaphore>;
import <functional>;
import <format>;
import <memory>;
import <array>;
import <cstdint>;
import <iostream>;
import LSEngine;

namespace gt::dx12
{
    namespace WRL = Microsoft::WRL;
    using namespace DirectX;

    export class SimpleWindow : public LS::LSApp
    {
    public:
        SimpleWindow(uint32_t width, uint32_t height) : LSApp(width, height, L"Simple Window"),
            m_settingsDx12(),
            m_frameBuffer(NUM_OF_FRAMES,
                m_Window->GetWidth(), m_Window->GetHeight(),
                DXGI_FORMAT_R8G8B8A8_UNORM,
                DXGI_SWAP_EFFECT_FLIP_DISCARD,
                DXGI_SCALING_NONE,
                DXGI_ALPHA_MODE_UNSPECIFIED,
                DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)
        {
            m_settingsDx12.Width = m_Window->GetWidth();
            m_settingsDx12.Height = m_Window->GetHeight();
            m_settingsDx12.Hwnd = (HWND)m_Window->GetHandleToWindow();

            m_device = std::make_unique<LS::Platform::Dx12::DeviceD3D12>();

            UINT flag = 0;
#ifdef _DEBUG
            flag = (UINT)DXGI_CREATE_FACTORY_DEBUG;
#endif
            auto factory = LS::Win32::CreateDXGIFactory2(flag).value();

            LS::Utils::ThrowIfFailed(factory.As(&m_pFactory), "Failed to create DXGI Factory");
        }

        ~SimpleWindow() = default;

        auto Initialize(LS::SharedRef<LS::LSCommandArgs> args) -> LS::System::ErrorCode override;
        void Run() override;

    private:
        const uint32_t NUM_OF_FRAMES = 3;

        WRL::ComPtr<IDXGIFactory4> m_pFactory;
        WRL::ComPtr<IDXGIAdapter1> m_pAdapter;
        LS::Platform::Dx12::DescriptorHeapDx12 m_heapRtv{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV, NUM_OF_FRAMES };
        LS::Platform::Dx12::DescriptorHeapDx12 m_heapSrv{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE };
        uint64_t m_fenceValue;

        // My stuff to replace above // 
        LS::Ref<LS::Platform::Dx12::DeviceD3D12> m_device;
        LS::Ref<LS::Platform::Dx12::CommandQueueDx12> m_queue;
        LS::Ref<LS::Platform::Dx12::CommandListDx12> m_commandList;
        LS::Platform::Dx12::D3D12Settings m_settingsDx12{};
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

auto gt::dx12::SimpleWindow::Initialize(LS::SharedRef<LS::LSCommandArgs> args) -> LS::System::ErrorCode
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
    m_Window->Show();

    m_State = LS::APP_STATE::RUNNING;
    auto currWidth = m_Window->GetWidth();
    auto currHeight = m_Window->GetHeight();
    while (m_Window->IsOpen())
    {
        m_Window->PollEvent();
        m_frameBuffer.WaitOnFrameBuffer();
        if (currWidth != m_Window->GetWidth() || currHeight != m_Window->GetHeight())
        {
            m_queue->Flush();
            if (auto result = m_frameBuffer.ResizeFrames(m_Window->GetWidth(), m_Window->GetHeight(), m_heapRtv.GetHeapStartCpu(), m_device->GetDevice().Get()); !result)
            {
                throw std::runtime_error(std::format("Failed to resize frame buffer. Error: {}", result.Message()));
            }
            currWidth = m_Window->GetWidth();
            currHeight = m_Window->GetHeight();
        }

        OnRender();
        OnUpdate();
    }

    OnDestroy();
}

bool gt::dx12::SimpleWindow::LoadPipeline()
{
    auto adapterOptional = LS::Win32::GetHardwareAdapter(m_pFactory.Get());
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
    m_queue = std::make_unique<LS::Platform::Dx12::CommandQueueDx12>(m_device->GetDevice().Get(), type);
    m_commandList = std::make_unique<LS::Platform::Dx12::CommandListDx12>(device4.Get(), type, "main command list");

    // Create Descriptor Heaps for RTV/SRV // 
    // This is the RTV descriptor heap (render target view)
    m_heapRtv.Initialize(device.Get());

    // Constant Buffer View/Shader Resource View/Unordered Access View types (this one is just the SRV)
    m_heapSrv.Initialize(device.Get());

    // Create the swap chain //
    // Since we are using an HWND (Win32) system, we can create the swapchain for HWND 
    {
        // Setup swap chain
        const auto& window = m_Window;
        HWND hwnd = reinterpret_cast<HWND>(window->GetHandleToWindow());

        auto result = m_frameBuffer.Initialize(m_pFactory,
            m_queue->GetCommandQueue().Get(), hwnd,
            m_heapRtv.GetHeapStartCpu(), m_device->GetDevice().Get());
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
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_heapRtv.GetHeapStartCpu(), static_cast<int>(m_frameBuffer.GetCurrentIndex()), m_heapRtv.GetSize());
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