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
        SimpleWindow(uint32_t width, uint32_t height) 
        {
            Window = LS::BuildWindow(width, height, L"Simple Window");
            m_settings.Width = width;
            m_settings.Height = height;
            m_settings.MinFeatureLevel = D3D_FEATURE_LEVEL_12_0;
            m_settings.Hwnd = (HWND)Window->GetHandleToWindow();
            m_device = std::make_unique<LS::Platform::Dx12::DeviceD3D12>(m_settings);
        }

        ~SimpleWindow() = default;

        auto Initialize(int argCount = 0, char* argsV[] = nullptr) -> LS::System::ErrorCode override;
        void Run() override;

    private:
        WRL::ComPtr<IDXGIFactory4> m_pFactory;
        WRL::ComPtr<IDXGIAdapter1> m_pAdapter;
        WRL::ComPtr<IDXGISwapChain4> m_pSwapChain;
        WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
        WRL::ComPtr<ID3D12Resource> m_renderTargets[3];
        //WRL::ComPtr<ID3D12Fence> m_fence;
        UINT m_currFrameIndex;
        UINT m_rtvDescriptorSize;
        UINT m_fenceValue;
        HANDLE m_scWaitableHandle;
        HANDLE m_fenceEvent;

        Ref<LS::Platform::Dx12::DeviceD3D12> m_device;
        Ref<LS::Platform::Dx12::CommandQueueDx12> m_queue;
        Ref<LS::Platform::Dx12::CommandListDx12> m_commandList;
        LS::Platform::Dx12::D3D12Settings m_settings{};

        void LoadPipeline();
        void LoadAssets();
        void PopulateCommandList();
        void OnRender();
        void OnDestroy();
        void OnUpdate();
        void WaitForPreviousFrame();

        const uint32_t NumOfFrames = 3;
    };
}

module : private;

auto gt::dx12::SimpleWindow::Initialize(int argCount, char* argsV[]) -> LS::System::ErrorCode 
{
    LoadPipeline();
    LoadAssets();
    return LS::System::CreateSuccessCode();
}

void gt::dx12::SimpleWindow::Run()
{
    Window->Show();

    IsRunning = true;
    
    while (IsRunning)
    {
        Window->PollEvent();
        OnRender();
    }
}

void gt::dx12::SimpleWindow::LoadPipeline()
{
    UINT flag = 0;
#ifdef _DEBUG
    flag = (UINT)DXGI_CREATE_FACTORY_DEBUG;
#endif
    auto factory = LS::Win32::CreateFactory(flag).value();

    LS::Utils::ThrowIfFailed(factory.As(&m_pFactory), "Failed to create DXGI Factory");

    auto adapterOptional = LS::Win32::GetHardwareAdapter(m_pFactory.Get(), true);
    if (!adapterOptional)
    {
        return;
    }
    
    m_pAdapter = adapterOptional.value();

    if (!m_device->CreateDevice(m_pAdapter))
    {
        return;
    }

    WRL::ComPtr<ID3D12Device4> device4;
    auto device = m_device->GetDevice();
    auto deviceHr = device.As(&device4);
    if (FAILED(deviceHr))
    {
        return;
    }
    auto type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    m_queue = std::make_unique<LS::Platform::Dx12::CommandQueueDx12>(device4, type);
    m_commandList = std::make_unique<LS::Platform::Dx12::CommandListDx12>(device4.Get(), type, "main command list");

    // Setup swap chain
    const auto& window = Window;
    HWND hwnd = reinterpret_cast<HWND>(window->GetHandleToWindow());

    DXGI_SWAP_CHAIN_DESC1 swapchainDesc1{};
    swapchainDesc1.BufferCount = NumOfFrames;
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

    // Since we are using an HWND (Win32) system, we can create the swapchain for HWND 
    {
        WRL::ComPtr<IDXGISwapChain1> swapChain1 = nullptr;
        LS::Utils::ThrowIfFailed(m_pFactory->CreateSwapChainForHwnd(m_queue->GetCommandQueue().Get(), hwnd, &swapchainDesc1, nullptr, nullptr, &swapChain1));
        LS::Utils::ThrowIfFailed(swapChain1.As(&m_pSwapChain));

        // Helper function that displays our display's resolution and refresh rates and other information 
        LS::Win32::LogAdapters(m_pFactory.Get());

        // Don't allot ALT+ENTER fullscreen
        m_pFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

        m_pSwapChain->SetMaximumFrameLatency(NumOfFrames);
        m_currFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
        m_scWaitableHandle = m_pSwapChain->GetFrameLatencyWaitableObject();
    }

    // Create Descriptor Heaps for RTV/SRV // 

    // This is the RTV descriptor heap (render target view)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NumDescriptors = NumOfFrames;
        desc.NodeMask = 0;

        //auto hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_rtvHeap));
        auto hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_rtvHeap));
        LS::Utils::ThrowIfFailed(hr, "Failed to create descriptor heap for the RTV");

        // Handles have a size that varies by GPU, so we have to ask for the Handle size on the GPU before processing
        m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // Constant Buffer View/Shader Resource View/Unordered Access View types (this one is just the SRV)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NumDescriptors = 1;

        LS::Utils::ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srvHeap)), "Failed to create CBV/SRV/UAV descriptor heap");
    }

    // Create our Render Targets for each frame context //
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
        for (auto i = 0u; i < NumOfFrames; ++i)
        {
            LS::Utils::ThrowIfFailed(m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
            device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }
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
    m_commandList->ResetCommandList();
    m_commandList->TransitionResource(m_renderTargets[m_currFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_currFrameIndex, m_rtvDescriptorSize);
    const std::array<float, 4> clearColor{ 0.320f, 0.12f, 0.34f, 1.0f };
    m_commandList->Clear(clearColor, rtvHandle);

    m_commandList->TransitionResource(m_renderTargets[m_currFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->Close();
    m_queue->QueueCommand(m_commandList.get());
}

void gt::dx12::SimpleWindow::OnRender()
{
    PopulateCommandList();
    m_fenceValue = m_queue->ExecuteCommandList();
    LS::Utils::ThrowIfFailed(m_pSwapChain->Present(1, 0), "Failed to present frame");
    WaitForPreviousFrame();
}

void gt::dx12::SimpleWindow::OnDestroy()
{
    WaitForPreviousFrame();
    ::CloseHandle(m_fenceEvent);
}

void gt::dx12::SimpleWindow::OnUpdate()
{

}

void gt::dx12::SimpleWindow::WaitForPreviousFrame()
{
    m_queue->WaitForCommands(m_fenceValue);
    m_currFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}