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
        }

        ~SimpleWindow() = default;

        auto Initialize(int argCount = 0, char* argsV[] = nullptr) -> LS::System::ErrorCode override;
        void Run() override;

    private:
        WRL::ComPtr<IDXGIFactory4> m_pFactory;
        WRL::ComPtr<IDXGIAdapter1> m_pAdapter;
        WRL::ComPtr<ID3D12Device4> m_pDevice;
        WRL::ComPtr<IDXGISwapChain4> m_pSwapChain;
        WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
        WRL::ComPtr<ID3D12Resource> m_renderTargets[3];
        WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
        WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
        WRL::ComPtr<ID3D12CommandAllocator> m_commandAlloc;;
        WRL::ComPtr<ID3D12Fence> m_fence;
        UINT m_currFrameIndex;
        UINT m_rtvDescriptorSize;
        UINT m_fenceValue;
        HANDLE m_scWaitableHandle;
        HANDLE m_fenceEvent;

        //Ref<LS::Platform::Dx12::CommandQueueDx12> m_copyCommandQueue, m_directCommandQueue;

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
    WRL::ComPtr<ID3D12Debug> pdx12Debug = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
    {
        pdx12Debug->EnableDebugLayer();
    }

    flag = (UINT)DXGI_CREATE_FACTORY_DEBUG;
#endif
    auto factory = LS::Win32::CreateFactory(flag).value();

    LS::Utils::ThrowIfFailed(factory.As(&m_pFactory), "Failed to create DXGI Factory");

    LS::Win32::GetHardwareAdapter(m_pFactory, m_pAdapter, true);

    // Create device
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_0;
    LS::Utils::ThrowIfFailed(D3D12CreateDevice(m_pAdapter.Get(), featureLevel, IID_PPV_ARGS(&m_pDevice)), "Failed to creat device");

    // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef _DEBUG

    if (pdx12Debug)
    {
        WRL::ComPtr<ID3D12InfoQueue> pInfoQueue = nullptr;
        m_pDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
    }
#endif

    // Create the Command Queue //
    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    LS::Utils::ThrowIfFailed(m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)), "Failed to Create Command Queue");

    // Create Command Queues //
    /*m_copyCommandQueue = std::make_unique<LS::Platform::Dx12::CommandQueueDx12>(m_pDevice, D3D12_COMMAND_LIST_TYPE_COPY);
    m_directCommandQueue = std::make_unique<LS::Platform::Dx12::CommandQueueDx12>(m_pDevice, D3D12_COMMAND_LIST_TYPE_DIRECT);*/
    auto resultComm = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAlloc));
    LS::Utils::ThrowIfFailed(resultComm, "Failed to create command allocator");

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
        //LS::Utils::ThrowIfFailed(m_pFactory->CreateSwapChainForHwnd(m_directCommandQueue->GetCommandQueue().Get(), hwnd, &swapchainDesc1, nullptr, nullptr, &swapChain1));
        LS::Utils::ThrowIfFailed(m_pFactory->CreateSwapChainForHwnd(m_commandQueue.Get(), hwnd, &swapchainDesc1, nullptr, nullptr, &swapChain1));
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
        //desc.NodeMask = 1;

        auto hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_rtvHeap));
        LS::Utils::ThrowIfFailed(hr, "Failed to create descriptor heap for the RTV");
        // Handles have a size that varies by GPU, so we have to ask for the Handle size on the GPU before processing
        m_rtvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // Constant Buffer View/Shader Resource View/Unordered Access View types (this one is just the SRV)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NumDescriptors = 1;

        LS::Utils::ThrowIfFailed(m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srvHeap)), "Failed to create CBV/SRV/UAV descriptor heap");
    }

    // Create our Render Targets for each frame context //
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
        for (auto i = 0u; i < NumOfFrames; ++i)
        {
            LS::Utils::ThrowIfFailed(m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
            m_pDevice->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }
}

void gt::dx12::SimpleWindow::LoadAssets()
{
    LS::Utils::ThrowIfFailed(m_pDevice->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_commandList)),
        "Failed to create command list");

    // Create Fence object //
    LS::Utils::ThrowIfFailed(m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)), "Failed to create fence.");
    m_fenceValue = 1;

    // Create Event //
    static const wchar_t* eventName = L"Fence Event";
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, eventName);
    if (m_fenceEvent == nullptr)
    {
        LS::Utils::ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()), "Failed to create event handle.");
    }
}

void gt::dx12::SimpleWindow::PopulateCommandList()
{
    //auto commandList = m_directCommandQueue->GetCommandList();
    LS::Utils::ThrowIfFailed(m_commandAlloc->Reset(), "Failed to reset command allocator.");
    LS::Utils::ThrowIfFailed(m_commandList->Reset(m_commandAlloc.Get(), nullptr), "Failed to reset command allocator.");

    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_currFrameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandList->ResourceBarrier(1, &barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_currFrameIndex, m_rtvDescriptorSize);
    const float clearColor[] = { 0.0f, 0.12f, 0.34f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    CD3DX12_RESOURCE_BARRIER barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_currFrameIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->ResourceBarrier(1, &barrier2);

    LS::Utils::ThrowIfFailed(m_commandList->Close(), "Failed to close command list.");
}

void gt::dx12::SimpleWindow::OnRender()
{
    PopulateCommandList();
    ID3D12CommandList* commandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

    LS::Utils::ThrowIfFailed(m_pSwapChain->Present(1, 0), "Failed to present frame");

    WaitForPreviousFrame();
}

void gt::dx12::SimpleWindow::OnDestroy()
{
    //m_directCommandQueue->Flush();
    WaitForPreviousFrame();
    ::CloseHandle(m_fenceEvent);
}

void gt::dx12::SimpleWindow::OnUpdate()
{

}

void gt::dx12::SimpleWindow::WaitForPreviousFrame()
{
    // Signal will set the value from CPU to GPU side. This value sent over to the GPU
    // is the value it will retain until the work is done. Once that work is done,
    // it will be incremented on the GPU side to signal it's completion. 
    const UINT64 fence = m_fenceValue;
    LS::Utils::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence), "Failed to signal command queue.");
    m_fenceValue++;

    // The method of how you compare depends on what you're comparing with.
    // From what I gather, it goes a bit like this:
    // 1. You do the work of rendering, then when it's complete, you signal the GPU you're intent is completed.
    // 2. Afterward you tell the GPU the value of the signal you want to register to that work 
    // 3. When the GPU finishes that work, it will increment that value by +1
    // 4. When you're ready to do work again on the same resources for the next frame, you need to
    // verify that the work is complete by comparing the value you have CPU side and GPU side.
    // Below we use the ID3D12Fence object. That object is the GPU's communication, so it will be 
    // incremented when it's complete. Basically +1 the value of the signal we sent it.
    // So we need to compare that value to our CPU side and see if it's done. 
    //if (m_fenceValue >= m_fence->GetCompletedValue()) // Compares the CPU value (incremented) with the GPU which should be equal or greater than after GPU completed work
    if (m_fence->GetCompletedValue() < fence) // Compares the GPU fence value with the previous CPU fence value (before increment above)
    {
        LS::Utils::ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent), "Failed to set event on completion.");
        ::WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    m_currFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}