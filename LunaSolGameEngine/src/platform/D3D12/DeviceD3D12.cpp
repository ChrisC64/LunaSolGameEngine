#include <dxgi1_6.h>
#include <d3d12.h>
#include <Windows.h>
#include <wrl/client.h>
#include <span>
#include <vector>
#include <optional>
#include <string>
#include <string>
#include <string_view>
#include <memory>
#include <cassert>
#include <stdexcept>
#include <exception>
#include <iostream>

import D3D12Lib;
import DXGIHelper;
import Data.LSDataTypes;
import Platform.Win32Window;


using namespace LS::Win32;
namespace WRL = Microsoft::WRL;

inline std::string HrToString(HRESULT hr)
{
    char s_str[64] = {};
    sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
    return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
    HRESULT Error() const { return m_hr; }
private:
    const HRESULT m_hr;
};

DeviceD3D12::DeviceD3D12(const D3D12Settings settings) : m_Settings(settings)
{
}

bool LS::Win32::DeviceD3D12::Initialize(uint32_t width, uint32_t height, std::wstring_view title, WRL::ComPtr<IDXGIAdapter> displayAdapter) noexcept
{
    if (!CreateDevice(displayAdapter))
    {
        return false;
    }

    InitWindow(width, height, title);
    return true;
}

bool DeviceD3D12::CreateDevice(WRL::ComPtr<IDXGIAdapter> displayAdpater /* = nullptr*/) noexcept
{
#ifdef _DEBUG
    WRL::ComPtr<ID3D12Debug> pdx12Debug = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
        pdx12Debug->EnableDebugLayer();
#endif

    WRL::ComPtr<IDXGIFactory7> pFactory;
    CreateDXGIFactory2(0u, IID_PPV_ARGS(&pFactory));

    // Find the best graphics card (best performing one, with single GPU systems, this should be the default)
    auto factoryOpt = CreateFactory();
    if (!factoryOpt)
    {
        return false;
    }

    m_pFactoryDxgi = factoryOpt.value();
    WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
    std::vector<WRL::ComPtr<IDXGIAdapter4>> adapters = EnumerateDiscreteGpuAdapters(m_pFactoryDxgi);

    HRESULT hr;
    // If display adapter is not set, find the first available DX12 display supported
    if (!displayAdpater)
    {
        // Find the display adapter that successfully builds to our requirements.
        auto adapter = FindCompatDisplay(adapters);
        if (!adapter)
        {
            return false;
        }
        displayAdpater = adapter.value();
    }

    hr = D3D12CreateDevice(displayAdpater.Get(), m_Settings.MinSettings.MinFeatureLevel, IID_PPV_ARGS(&m_pDevice));
    if (FAILED(hr))
    {
        std::cerr << "Failed to create device: " << HrToString(hr) << "\n";
        return false;
    }
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

    return CreateCommandQueue();
}

void DeviceD3D12::InitWindow(uint32_t width, uint32_t height, std::wstring_view title) noexcept
{
    assert(m_pDevice);
    assert(m_pFactoryDxgi);

    m_pWindow = std::make_unique<Win32Window>(width, height, title);

    m_pWindow->Show();
    // Our window is visible, we can create the swap chain to the window now 
    CreateSwapchain();
}

void LS::Win32::DeviceD3D12::TakeWindow(Ref<Win32Window>& window) noexcept
{
    assert(window);
    m_pWindow = std::move(window);
    CreateSwapchain();
}

// Private Methods for DeviceD3D12 //
auto DeviceD3D12::FindCompatDisplay(std::span<WRL::ComPtr<IDXGIAdapter4>> adapters) noexcept -> Nullable<WRL::ComPtr<IDXGIAdapter4>>
{
    for (auto adapter : adapters)
    {
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), m_Settings.MinSettings.MinFeatureLevel, __uuidof(ID3D12Device), nullptr)))
        {
            return adapter;
        }
    }
    return std::nullopt;
}

bool DeviceD3D12::CreateCommandQueue() noexcept
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 1;

    auto hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pCommandQueue));
    if (FAILED(hr))
    {
        std::cerr << "Failed to create command queue: " << HrToString(hr) << "\n";
        return false;
    }

    return true;
}

void LS::Win32::DeviceD3D12::CreateSwapchain()
{
    assert(m_pWindow);
    assert(m_pFactoryDxgi);
    assert(m_pCommandQueue);

    DXGI_SWAP_CHAIN_DESC1 swapchainDesc1{};
    swapchainDesc1.BufferCount = m_Settings.MinSettings.FrameBufferCount;
    swapchainDesc1.Width = m_pWindow->GetWidth();
    swapchainDesc1.Height = m_pWindow->GetHeight();
    swapchainDesc1.Format = m_Settings.MinSettings.PixelFormat;
    swapchainDesc1.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    swapchainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc1.SampleDesc.Count = 1;
    swapchainDesc1.SampleDesc.Quality = 0;
    swapchainDesc1.SwapEffect = m_Settings.ExSettings.SwapEffect;
    swapchainDesc1.AlphaMode = m_Settings.ExSettings.AlphaMode;
    swapchainDesc1.Scaling = m_Settings.ExSettings.Scaling;
    swapchainDesc1.Stereo = m_Settings.ExSettings.IsStereoScopic;

    //TODO: Currently doesn't set the DXGI_SWAPCHAIN_FULLSCREEN_DESC to handle full screen displays,
    // will need to add that in later. 
    WRL::ComPtr<IDXGISwapChain1> swapchain;
    auto hr = m_pFactoryDxgi->CreateSwapChainForHwnd(m_pCommandQueue.Get(), m_pWindow->Hwnd(), &swapchainDesc1, nullptr, nullptr, &swapchain);
    if (FAILED(hr))
    {
        throw HrException(hr);
    }

    hr = swapchain.As(&m_pSwapChain);
    if (FAILED(hr))
    {
        throw HrException(hr);
    }

    m_pSwapChain->SetMaximumFrameLatency(m_Settings.MinSettings.FrameBufferCount);
    m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}
