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
#include <format>
#include "platform/Windows/Win32/WinApiUtils.h"

import D3D12Lib;
import DXGIHelper;
import Data.LSDataTypes;
import Platform.Win32Window;
import Engine.Logger;

using namespace LS::Win32;
namespace WRL = Microsoft::WRL;

class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
    HRESULT Error() const { return m_hr; }
private:
    const HRESULT m_hr;
};

DeviceD3D12::DeviceD3D12(SharedRef<D3D12Settings>& settings) : m_pSettings(settings),
    m_resManager(m_pDevice, m_pSettings)
{
}

bool LS::Win32::DeviceD3D12::Initialize(uint32_t width, uint32_t height, std::wstring_view title, WRL::ComPtr<IDXGIAdapter> displayAdapter) noexcept
{
    if (!CreateDevice(displayAdapter))
    {
        Log::TraceWarning(L"Failed to initialize the Direct3D 12 Device.");
        return false;
    }

    InitWindow(width, height, title);
    return true;
}

bool DeviceD3D12::CreateDevice(WRL::ComPtr<IDXGIAdapter> displayAdpater /* = nullptr*/) noexcept
{
#ifdef _DEBUG
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_pDebug))))
    {
        Log::TraceDebug(L"Enabling Debug Layer for D3D12 Device");
        m_pDebug->EnableDebugLayer();
    }
#endif

    WRL::ComPtr<IDXGIFactory7> pFactory;
    CreateDXGIFactory2(0u, IID_PPV_ARGS(&pFactory));

    // Find the best graphics card (best performing one, with single GPU systems, this should be the default)
    auto factoryOpt = CreateFactory();
    if (!factoryOpt)
    {
        Log::TraceError(L"Failed to create DXGI Factory, cannot proceed with device creation.");
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
            Log::TraceError(L"Failed to find a compatible display.");
            return false;
        }
        displayAdpater = adapter.value();
    }

    hr = D3D12CreateDevice(displayAdpater.Get(), m_pSettings->MinSettings.MinFeatureLevel, IID_PPV_ARGS(&m_pDevice));
    if (FAILED(hr))
    {
        auto string = HrToWString(hr);
        Log::TraceError(std::format(L"Failed to create device. Error Code: {}", string));
        return false;
    }
    // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef _DEBUG
    if (m_pDebug)
    {
        WRL::ComPtr<ID3D12InfoQueue> pInfoQueue = nullptr;
        m_pDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
        m_resManager.SetDebugDevice(m_pDebug);
    }
#endif
    
    return m_resManager.CreateCommandQueue();;
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

void LS::Win32::DeviceD3D12::MoveToWindow(Ref<Win32Window>& window) noexcept
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
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), m_pSettings->MinSettings.MinFeatureLevel, __uuidof(ID3D12Device), nullptr)))
        {
            return adapter;
        }
    }
    return std::nullopt;
}

void LS::Win32::DeviceD3D12::CreateSwapchain()
{
    assert(m_pWindow);
    if (!m_pWindow)
    {
        Log::TraceWarning(L"The window is null, cannot create swap chain");
        return;
    }
    m_resManager.CreateSwapChain(m_pWindow.get());
}

void LS::Win32::DeviceD3D12::PrintDisplayAdapters()
{
    LS::Win32::LogAdapters(m_pFactoryDxgi.Get());
}
