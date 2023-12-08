module;
#include "imgui\imgui.h"
#include "imgui\backends\imgui_impl_win32.h"
#include "imgui\backends\imgui_impl_dx12.h"

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
#include <iostream>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
export module ImGuiWindowTest;

import LSData;
import Engine.App;
import D3D12Lib;
import Platform.Win32Window;
import Helper.LSCommonTypes;
import Helper.PipelineFactory;
import Util.MSUtils;
import DXGIHelper;


namespace gt::dx12::ImGuiSample
{
    namespace WRL = Microsoft::WRL;
    using namespace DirectX;

    const uint32_t NUM_FRAMES = 3;

    struct FrameContext
    {
        // Manages a heap for the command lists. This cannot be reset while the CommandList is still in flight on the GPU
        WRL::ComPtr<ID3D12CommandAllocator>     CommandAllocator;
        // Singal value between the GPU and CPU to perform synchronization. 
        uint64_t                                FenceValue;
    };

    static LRESULT WndProcImpl(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    
    export class ImGuiSample : public LS::LSApp
    {
    public:
        ImGuiSample(uint32_t width, uint32_t height)
        {
            Window = LS::BuildWindow(width, height, L"ImGui Sample");
        }

        ~ImGuiSample() = default;

        auto Initialize(int argCount = 0, char* argsV[] = nullptr) -> LS::System::ErrorCode override;
        void Run() override;
        void CreateRenderTargets();
        void CleanupRenderTargets();
        void CreateFrameContexts();
        void TransitionRTV(D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);
        void ClearRTV();
        void SubmitFrame();
        void WaitForLastSubmittedFrame();
        void OnDestroy();
        void BeginRenderSetup();
        void ResizeFrame(uint32_t width, uint32_t height);
    private:
        bool CreateDeviceD3D();

        WRL::ComPtr<IDXGIFactory4> m_pFactory;
        WRL::ComPtr<IDXGIAdapter1> m_pAdapter;
        WRL::ComPtr<ID3D12Device4> m_pDevice;
        WRL::ComPtr<IDXGISwapChain4> m_pSwapChain;
        WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
        WRL::ComPtr<ID3D12Resource> m_renderTargets[NUM_FRAMES];
        WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
        WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
        WRL::ComPtr<ID3D12Fence> m_fence;
        uint32_t m_currFrameIndex;
        uint32_t m_rtvDescriptorSize;
        uint64_t m_frameIndex;// records the number of frames that have elapsed over time
        HANDLE m_scWaitableHandle;
        HANDLE m_fenceEvent;
        std::array<FrameContext, NUM_FRAMES> m_frameContexts;
        FrameContext* m_currContext;
    };
}

module : private;

using namespace gt::dx12;

auto gt::dx12::ImGuiSample::ImGuiSample::Initialize(int argCount, char* argsV[]) -> LS::System::ErrorCode
{
    if (!CreateDeviceD3D())
        return LS::System::CreateFailCode("Failed to create device D3D");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init((HWND)Window->GetHandleToWindow());
    ImGui_ImplDX12_Init(m_pDevice.Get(), NUM_FRAMES, DXGI_FORMAT_R8G8B8A8_UNORM,
        m_srvHeap.Get(),
        // You'll need to designate a descriptor from your descriptor heap for Dear ImGui to use internally for its font texture's SRV
        m_srvHeap->GetCPUDescriptorHandleForHeapStart(),
        m_srvHeap->GetGPUDescriptorHandleForHeapStart());

    m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));

    CreateRenderTargets();
    CreateFrameContexts();

    auto win32Window = static_cast<LS::Win32::Win32Window*>(Window.get());
    using namespace std::placeholders;
    auto bind = std::bind(gt::dx12::ImGuiSample::WndProcImpl, _1, _2, _3, _4);

    win32Window->SetWndProcHandler(bind);
    return LS::System::CreateSuccessCode();

}

void gt::dx12::ImGuiSample::ImGuiSample::Run()
{
    Window->Show();
    ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    auto currWidth = Window->GetWidth();
    auto currHeight = Window->GetHeight();
    ImGuiIO& io = ImGui::GetIO();
    while (Window->IsOpen())
    {
        Window->PollEvent();
        auto mousePos = Window->GetMousePos();
        if (currWidth != Window->GetWidth() && currHeight != Window->GetHeight())
        {
            WaitForLastSubmittedFrame();
            CleanupRenderTargets();
            ResizeFrame(Window->GetWidth(), Window->GetHeight());
            CreateRenderTargets();
            currWidth = Window->GetWidth();
            currHeight = Window->GetHeight();
        }

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow(); // Show demo window! :)

        ImGui::Text("Width: %d", currWidth);
        ImGui::Text("Height: %d", currHeight);
        ImGui::Text("Mouse Pos: %d x %d", mousePos.x, mousePos.y);

        ImGui::Render();
        BeginRenderSetup();
        TransitionRTV(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        ClearRTV();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList.Get());

        TransitionRTV(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        
        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault(nullptr, (void*)m_commandList.Get());
        }

        SubmitFrame();
        WaitForLastSubmittedFrame();
    }

    OnDestroy();
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void gt::dx12::ImGuiSample::ImGuiSample::CreateRenderTargets()
{
    // The handle can now be used to help use build our RTVs - one RTV per frame/back buffer
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (auto i = 0u; i < NUM_FRAMES; ++i)
    {
        WRL::ComPtr<ID3D12Resource> backBuffer;
        LS::Utils::ThrowIfFailed(m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)), "Failed to create frame buffer");
        m_pDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
        backBuffer->SetName(std::format(L"Back buffer {}", i).c_str());
        m_renderTargets[i] = backBuffer;
        rtvHandle.Offset(1, m_rtvDescriptorSize);
    }
}

void gt::dx12::ImGuiSample::ImGuiSample::CleanupRenderTargets()
{
    for (auto i = 0u; i < NUM_FRAMES; ++i)
    {
        if (m_renderTargets[i])
            m_renderTargets[i] = nullptr;
    }
}

void gt::dx12::ImGuiSample::ImGuiSample::CreateFrameContexts()
{
    auto count = 0u;
    for (auto& fc : m_frameContexts)
    {
        auto resultComm = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&fc.CommandAllocator));
        LS::Utils::ThrowIfFailed(resultComm, "Failed to create command allocator");
        fc.CommandAllocator->SetName(std::format(L"FC Command Allocator {}", count).c_str());
        fc.FenceValue = 0;
        ++count;
    }

    auto result = m_pDevice->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_commandList));
    m_commandList->SetName(L"Command List");
    LS::Utils::ThrowIfFailed(result, "Failed to create command list");
}

void gt::dx12::ImGuiSample::ImGuiSample::TransitionRTV(D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_currFrameIndex].Get(),
        from, to);
    m_commandList->ResourceBarrier(1, &barrier);
}

void gt::dx12::ImGuiSample::ImGuiSample::ClearRTV()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_currFrameIndex, m_rtvDescriptorSize);
    const float clearColor[] = { 0.0f, 0.12f, 0.34f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
    m_commandList->SetDescriptorHeaps(1, ppHeaps);
}

void gt::dx12::ImGuiSample::ImGuiSample::SubmitFrame()
{
    LS::Utils::ThrowIfFailed(m_commandList->Close(), "Failed to close command list.");
    ID3D12CommandList* commandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

    //LS::Utils::ThrowIfFailed(m_pSwapChain->Present(1, 0), "Failed to present frame");
    m_pSwapChain->Present(1, 0);
}

void gt::dx12::ImGuiSample::ImGuiSample::WaitForLastSubmittedFrame()
{
    const auto fence = m_frameContexts[m_currFrameIndex].FenceValue;

    LS::Utils::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence), "Failed to signal command queue");
    m_frameContexts[m_currFrameIndex].FenceValue++;

    if (m_fence->GetCompletedValue() < fence)
    {
        LS::Utils::ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent), "Failed to set event on completion.");
        HANDLE waitableObjects[] = { m_scWaitableHandle, m_fenceEvent };
        DWORD number = 2;
        WaitForMultipleObjects(number, waitableObjects, TRUE, INFINITE);
    }

    m_currFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

void gt::dx12::ImGuiSample::ImGuiSample::OnDestroy()
{
    WaitForLastSubmittedFrame();
    ::CloseHandle(m_fenceEvent);
    ::CloseHandle(m_scWaitableHandle);
}

void gt::dx12::ImGuiSample::ImGuiSample::BeginRenderSetup()
{
    auto fc = &m_frameContexts[m_currFrameIndex];
    fc->CommandAllocator->Reset();
    m_commandList->Reset(fc->CommandAllocator.Get(), nullptr);
}

void gt::dx12::ImGuiSample::ImGuiSample::ResizeFrame(uint32_t width, uint32_t height)
{
    m_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
}

bool gt::dx12::ImGuiSample::ImGuiSample::CreateDeviceD3D()
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

    m_pAdapter = LS::Win32::GetHardwareAdapter(m_pFactory.Get(), true).value();

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
    for (int i = 0; i < NUM_FRAMES; ++i)
    {
        auto resultComm = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_frameContexts[i].CommandAllocator.ReleaseAndGetAddressOf()));
        LS::Utils::ThrowIfFailed(resultComm, "Failed to create command allocator");
    }

    // Setup swap chain
    const auto& window = Window;
    HWND hwnd = reinterpret_cast<HWND>(window->GetHandleToWindow());

    DXGI_SWAP_CHAIN_DESC1 swapchainDesc1{};
    swapchainDesc1.BufferCount = NUM_FRAMES;
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

        m_pSwapChain->SetMaximumFrameLatency(NUM_FRAMES);
        m_currFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
        m_scWaitableHandle = m_pSwapChain->GetFrameLatencyWaitableObject();
    }

    // Create Descriptor Heaps for RTV/SRV // 

    // This is the RTV descriptor heap (render target view)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NumDescriptors = NUM_FRAMES;
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
        for (auto i = 0u; i < NUM_FRAMES; ++i)
        {
            LS::Utils::ThrowIfFailed(m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
            m_pDevice->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    return true;
}

static LRESULT gt::dx12::ImGuiSample::WndProcImpl(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Let ImGui handle itself, but we want to still allow rest of default window behavior to go to our current
    // window handler for the rest of the messages we do handle. 
    ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
    return 0;
}