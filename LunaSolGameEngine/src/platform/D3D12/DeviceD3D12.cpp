#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dx12.h>
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
#include "engine\EngineLogDefines.h"
#include "platform/Windows/Win32/WinApiUtils.h"

import D3D12Lib;
import DXGIHelper;
import Data.LSDataTypes;
import Platform.Win32Window;
import Engine.Logger;
import Engine.EngineCodes;

using namespace LS::Win32;
using namespace LS::Platform::Dx12;
namespace WRL = Microsoft::WRL;

class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
    HRESULT Error() const { return m_hr; }
private:
    const HRESULT m_hr;
};

DeviceD3D12::DeviceD3D12(const D3D12Settings& settings) : m_settings(settings)
{
}

auto DeviceD3D12::CreateDevice(WRL::ComPtr<IDXGIAdapter> displayAdpater /* = nullptr*/) noexcept -> LS::System::ErrorCode
{
    UINT dxgiFlags = 0u;
#ifdef _DEBUG
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_pDebug))))
    {
        Log::TraceDebug(L"Enabling Debug Layer for D3D12 Device");
        m_pDebug->EnableDebugLayer();
    }

    dxgiFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    // Find the best graphics card (best performing one, with single GPU systems, this should be the default)
    if (FAILED(CreateDXGIFactory2(dxgiFlags, IID_PPV_ARGS(&m_pFactoryDxgi))))
    {
        Log::TraceError(L"Failed to create DXGI Factory, cannot proceed with device creation.");
        return LS::System::CreateFailCode("Failed to create DXGI Factory.");
    }

    WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
    WRL::ComPtr<IDXGIFactory7> pTemp;
    if (FAILED(m_pFactoryDxgi->QueryInterface(IID_PPV_ARGS(&pTemp))))
    {
        return LS::System::CreateFailCode("Failed to create IDXGIFactory7 interface needed for enumeration");
    }

    std::vector<WRL::ComPtr<IDXGIAdapter4>> adapters = EnumerateDiscreteGpuAdapters(pTemp.Get());

    HRESULT hr;
    // If display adapter is not set, find the first available DX12 display supported
    if (!displayAdpater)
    {
        // Find the display adapter that successfully builds to our requirements.
        auto adapter = FindCompatDisplay(adapters);
        if (!adapter)
        {
            Log::TraceError(L"Failed to find a compatible display.");
            return LS::System::CreateFailCode("Failed to find a compatible display.");
        }
        displayAdpater = adapter.value();
    }

    hr = D3D12CreateDevice(displayAdpater.Get(), m_settings.FeatureLevel, IID_PPV_ARGS(&m_pDevice));
    if (FAILED(hr))
    {
        auto string = HrToString(hr);
        auto wstring = HrToWString(hr);
        Log::TraceError(std::format(L"Failed to create device. Error Code: {}", wstring));
        return LS::System::CreateFailCode(std::format("Failed to create device. Error Code: {}", string));
    }

    if (FAILED(m_featureSupport.Init(m_pDevice.Get())))
    {
        return LS::System::CreateFailCode("Failed to create feature support validator.");
    }

    // Find the Max Feature Leavel and create the device to support it
    m_settings.FeatureLevel = m_featureSupport.MaxSupportedFeatureLevel();
    hr = D3D12CreateDevice(displayAdpater.Get(), m_settings.FeatureLevel, IID_PPV_ARGS(&m_pDevice));
    if (FAILED(hr))
    {
        auto hrString = HrToString(hr);
        auto hrWstring = HrToWString(hr);
        auto msg = std::format(L"Attempted to create device with largest feature level, but failed to create device. Error Code: {}", hrWstring);
        Log::TraceError(msg);
        return LS::System::CreateFailCode(std::format("Attempted to create device with largest feature level, but failed to create device. Error Code: {}", hrString));
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
    }
#endif

    return LS::System::CreateSuccessCode();
}

//auto LS::Platform::Dx12::DeviceD3D12::CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_PRIORITY priority) noexcept -> WRL::ComPtr<ID3D12CommandQueue>
//{
//    D3D12_COMMAND_QUEUE_DESC desc = {};
//    desc.Type = type;
//    desc.Priority = priority;
//    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
//    desc.NodeMask = 0;
//
//    WRL::ComPtr<ID3D12CommandQueue> pQueue;
//    const auto hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&pQueue));
//
//    if (FAILED(hr))
//    {
//        // Handle error
//        return nullptr;
//    }
//
//    return pQueue;
//}

//auto LS::Platform::Dx12::DeviceD3D12::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, bool isShaderVisible) noexcept -> WRL::ComPtr<ID3D12DescriptorHeap>
//{
//    assert(m_pDevice);
//
//    D3D12_DESCRIPTOR_HEAP_DESC desc{};
//    desc.NumDescriptors = numDescriptors;
//    desc.Type = type;
//    desc.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//    desc.NodeMask = 0u;
//
//    WRL::ComPtr<ID3D12DescriptorHeap> heap;
//    const auto hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap));
//
//    if (FAILED(hr))
//    {
//        return nullptr;
//    }
//
//    return heap;
//}

//auto LS::Platform::Dx12::DeviceD3D12::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type) noexcept -> WRL::ComPtr<ID3D12CommandAllocator>
//{
//    assert(m_pDevice);
//
//    WRL::ComPtr<ID3D12CommandAllocator> allocator;
//    const auto hr = m_pDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&allocator));
//    if (FAILED(hr))
//    {
//        const auto msg = HrToWString(hr);
//        LS_LOG_ERROR(std::format(L"Failed to create command allocator: {}", msg))
//        return nullptr;
//    }
//
//    return allocator;
//}

//auto LS::Platform::Dx12::DeviceD3D12::CreateCommandList(D3D12_COMMAND_LIST_TYPE type) noexcept -> WRL::ComPtr<ID3D12CommandList>
//{
//    assert(m_pDevice);
//    WRL::ComPtr<ID3D12CommandList> commandList;
//    const auto hr = m_pDevice->CreateCommandList1(0, type, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&commandList));
//    if (FAILED(hr))
//    {
//        const auto msg = HrToString(hr);
//        LS_LOG_ERROR(std::format("Failed to create command list: {}", msg));
//        return nullptr;
//    }
//    return commandList;
//}

//auto LS::Platform::Dx12::DeviceD3D12::CreateFence(D3D12_FENCE_FLAGS flag /*= D3D12_FENCE_FLAG_NONE*/) noexcept -> WRL::ComPtr<ID3D12Fence>
//{
//    assert(m_pDevice);
//    WRL::ComPtr<ID3D12Fence> fence;
//    const auto hr = m_pDevice->CreateFence(0, flag, IID_PPV_ARGS(&fence));
//    if (FAILED(hr))
//    {
//        const auto msg = HrToString(hr);
//        LS_LOG_ERROR(std::format("Failed to create fence: {}", msg));
//        return nullptr;
//    }
//    return fence;
//}

// Private Methods for DeviceD3D12 //
auto DeviceD3D12::FindCompatDisplay(std::span<WRL::ComPtr<IDXGIAdapter4>> adapters) noexcept -> Nullable<WRL::ComPtr<IDXGIAdapter4>>
{
    for (auto adapter : adapters)
    {
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), m_settings.FeatureLevel, __uuidof(ID3D12Device), nullptr)))
        {
            return adapter;
        }
    }
    return std::nullopt;
}

void DeviceD3D12::PrintDisplayAdapters()
{
    LS::Win32::LogAdapters(m_pFactoryDxgi.Get());
}