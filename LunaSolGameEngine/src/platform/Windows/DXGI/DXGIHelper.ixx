module;
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <vector>
#include <cassert>
#include <optional>
#include <format>
#include <string>
#include <d3d12.h>
#include "engine/EngineLogDefines.h"
export module DXGIHelper;

import Engine.Logger;
import Engine.EngineCodes;
import Engine.Defines;
import Win32.Utils;
import D3D12Lib.D3D12Common;

namespace WRL = Microsoft::WRL;

export namespace LS::Win32
{
    /**
     * @brief Creates a DXGI Factory
     * @param flags DXGI_CREATE_FLAGS
     * @return @link Nullable object of a IDXGIFactory7 instance (see module @link LSdataTypes.ixx)
    */
    [[nodiscard]] auto CreateDXGIFactory2(UINT flags = 0u) noexcept -> Nullable<WRL::ComPtr<IDXGIFactory2>>;

    /**
     * @brief Create a swapchain for the DX12 implementation (requires factory and command queue)
     * @param settings Settings to apply to the SwapChain
     * @param factory IDXGIFactory2 that's used to create the swap chain
     * @param commandQueue Required to create the swap chainf or DX12
     * @return A swap chain if successful, otherwise a null object.
    */
    [[nodiscard]] auto CreateSwapchainForHwndDx12(const Platform::Dx12::D3D12Settings& settings,
        IDXGIFactory2* factory, ID3D12CommandQueue* commandQueue) noexcept -> Nullable<WRL::ComPtr<IDXGISwapChain1>>;

    /**
     * @brief Returns any high performance display adapters (discrete or external GPU supported)
     * @param pFactory The IDXGIFactory, must be convertable to a IDXGIFactory6 object
     * @pram requestHighPerformance To request the high performance GPU (true) or minimum GPU (false), the default is true
     * @return A vector of all Discrete GPU supported displays if any are found.
    */
    [[nodiscard]] auto EnumerateDiscreteGpuAdapters(IDXGIFactory6* pFactory, bool requestHighPerformance = true) noexcept -> std::vector<WRL::ComPtr<IDXGIAdapter4>>;

    /**
     * @brief Returns any display adapters
     * @param pFactory The IDXGIFactory interface object to use
     * @return A container containing all display adapters
    */
    [[nodiscard]] auto EnumerateDisplayAdapters(IDXGIFactory1* const pFactory) noexcept -> std::vector<WRL::ComPtr<IDXGIAdapter4>>;

    [[nodiscard]] auto GetHardwareAdapter(IDXGIFactory1* const pFactory, bool useHighPerformance = true) noexcept -> Nullable<WRL::ComPtr<IDXGIAdapter1>>;

    [[nodiscard]] auto GetWarpAdapter(IDXGIFactory1* const pFactory) noexcept -> Nullable<WRL::ComPtr<IDXGIAdapter>>;

    // Feature Check Supports //

    /**
     * @brief Checks if tearing support (i.e. FreeSync/G-Sync) is available
     * @param pFactory Windows DXGI Factory
     * @return An error code with any messages if support is available or not.
    */
    [[nodiscard]] auto CheckTearingSupport(IDXGIFactory5* const pFactory) noexcept -> LS::System::ErrorCode;

    void LogAdapters(IDXGIFactory4* const factory) noexcept;
    void LogAdapterOutput(IDXGIAdapter* const adapter) noexcept;
    void LogOutputDisplayModes(IDXGIOutput* const output, DXGI_FORMAT format) noexcept;

    /**
     * @brief Returns the given HRESULT error code as an error message
     * @param hr 
     * @return associated error message to the HRESULT error code for DXGI
     */
    constexpr auto HResultToDxgiError(HRESULT hr) noexcept -> const char*;
}

module : private;

using namespace LS::Win32;

auto LS::Win32::CreateDXGIFactory2(UINT flags) noexcept -> Nullable<Microsoft::WRL::ComPtr<IDXGIFactory2>>
{
    WRL::ComPtr<IDXGIFactory2> pOut;
    auto result = CreateDXGIFactory2(flags, IID_PPV_ARGS(&pOut));
    if (FAILED(result))
        return std::nullopt;
    return pOut;
}

auto LS::Win32::CreateSwapchainForHwndDx12(const Platform::Dx12::D3D12Settings& settings,
    IDXGIFactory2* factory, ID3D12CommandQueue* commandQueue) noexcept -> Nullable<WRL::ComPtr<IDXGISwapChain1>>
{
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc1{};
    swapchainDesc1.BufferCount = settings.FrameBufferCount;
    swapchainDesc1.Width = static_cast<UINT>(settings.Width);
    swapchainDesc1.Height = static_cast<UINT>(settings.Height);
    swapchainDesc1.Format = settings.PixelFormat;
    swapchainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc1.SampleDesc = { .Count = 1, .Quality = 0 };

    // If VSync is off, should use FIP_DISCARD not FLIP_SEQUENTIAL
    swapchainDesc1.SwapEffect = settings.SwapEffect;
    swapchainDesc1.AlphaMode = settings.AlphaMode;
    swapchainDesc1.Scaling = settings.Scaling;
    swapchainDesc1.Stereo = settings.IsStereoScopic ? TRUE : FALSE;

    // Feature Flags to support // 
    swapchainDesc1.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

    WRL::ComPtr<IDXGISwapChain1> swapchain;

    const auto hr = factory->CreateSwapChainForHwnd(commandQueue, settings.Hwnd,
        &swapchainDesc1, nullptr, nullptr, &swapchain);

    if (FAILED(hr))
    {
        LS_LOG_ERROR(std::format(L"Failed to create swap chain for HWND. Error Code: {}", HrToWString(hr)));
        return std::nullopt;
    }

    return swapchain;
}

auto LS::Win32::EnumerateDiscreteGpuAdapters(IDXGIFactory6* pFactory, bool requestHighPerformance /*= true*/) noexcept -> std::vector<WRL::ComPtr<IDXGIAdapter4>>
{
    assert(pFactory);
    WRL::ComPtr<IDXGIAdapter4> adapter;
    std::vector<WRL::ComPtr<IDXGIAdapter4>> out;

    auto preference = requestHighPerformance ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_MINIMUM_POWER;
    for (auto adapterIndex = 0u; SUCCEEDED(pFactory->EnumAdapterByGpuPreference(adapterIndex, preference, IID_PPV_ARGS(&adapter)));
        ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // Don't worry about software renderer (WARP)
            continue;
        }
        out.push_back(std::move(adapter));
    }

    return out;
}

auto LS::Win32::EnumerateDisplayAdapters(IDXGIFactory1* const pFactory) noexcept -> std::vector<WRL::ComPtr<IDXGIAdapter4>>
{
    assert(pFactory);
    std::vector<WRL::ComPtr<IDXGIAdapter4>> out;

    WRL::ComPtr<IDXGIAdapter1> adapter;
    for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }

        WRL::ComPtr<IDXGIAdapter4> move;
        if (FAILED(adapter.As(&move)))
        {
            break;
        }
        out.push_back(std::move(move));
    }

    return out;
}

auto LS::Win32::GetHardwareAdapter(IDXGIFactory1 * const pFactory, bool useHighPerformance /*= true*/) noexcept -> Nullable<WRL::ComPtr<IDXGIAdapter1>>
{
    WRL::ComPtr<IDXGIAdapter1> adapter;

    WRL::ComPtr<IDXGIFactory6> factory6;
    if (FAILED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
        return std::nullopt;
    }

    DXGI_GPU_PREFERENCE preference = useHighPerformance ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED;
    for (UINT adapterIndex = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(adapterIndex, preference, IID_PPV_ARGS(&adapter)));
        ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
            continue;
        // Check to see whether the adapter supports Direct3D 12, but don't create the
        // actual device yet.
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
        {
            break;
        }
    }

    if (adapter == nullptr)
    {
        for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
                continue;
            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    return adapter;
}

auto LS::Win32::GetWarpAdapter(IDXGIFactory1 * const  pFactory) noexcept -> Nullable<WRL::ComPtr<IDXGIAdapter>>
{
    WRL::ComPtr<IDXGIAdapter1> adapter;

    WRL::ComPtr<IDXGIFactory6> factory6;

    if (FAILED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
        return std::nullopt;
    }

    DXGI_GPU_PREFERENCE preference = DXGI_GPU_PREFERENCE_UNSPECIFIED;
    for (UINT adapterIndex = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference( adapterIndex, preference, IID_PPV_ARGS(&adapter)));
        ++adapterIndex)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    if (adapter.Get() == nullptr)
    {
        for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }
    }

    return adapter;
}

auto LS::Win32::CheckTearingSupport(IDXGIFactory5* const pFactory) noexcept -> LS::System::ErrorCode
{
    BOOL allowTearing = FALSE;

    const auto hr = pFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
    if (FAILED(hr))
    {
        const auto msg = HrToString(hr);
        return LS::System::CreateFailCode(msg);
    }

    return LS::System::CreateSuccessCode();
}

void LS::Win32::LogAdapters(IDXGIFactory4* factory) noexcept
{
    uint32_t i = 0;
    IDXGIAdapter* adapter = nullptr;
    std::vector<IDXGIAdapter*> adapterList;
    while (factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);
        Log::TraceInfo(std::format(L"*** Adapter {} ***\n", desc.Description));
        adapterList.emplace_back(adapter);
        ++i;
    }

    for (auto a : adapterList)
    {
        LogAdapterOutput(a);
        a->Release();
    }

    if (factory)
        factory->Release();
}

void LS::Win32::LogAdapterOutput(IDXGIAdapter* const adapter) noexcept
{
    uint32_t i = 0;
    IDXGIOutput* output = nullptr;
    while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_OUTPUT_DESC desc;
        output->GetDesc(&desc);

        Log::TraceInfo(std::format(L"\t|| Output ||\nName: {}\n", desc.DeviceName));
        LogOutputDisplayModes(output, DXGI_FORMAT_B8G8R8A8_UNORM);

        output->Release();
        ++i;
    }
}

void LS::Win32::LogOutputDisplayModes(IDXGIOutput* const output, DXGI_FORMAT format) noexcept
{
    UINT count = 0;
    UINT flags = 0;

    output->GetDisplayModeList(format, flags, &count, nullptr);
    std::vector<DXGI_MODE_DESC> modeList(count);
    output->GetDisplayModeList(format, flags, &count, &modeList[0]);

    for (auto& x : modeList)
    {
        UINT n = x.RefreshRate.Numerator;
        UINT d = x.RefreshRate.Denominator;
        std::wstring text = std::format(L"\tResolution (WxH): {}x{}\n\tRefresh Rate: {}", std::to_wstring(x.Width),
            std::to_wstring(x.Height), std::to_wstring(n / d));
        Log::TraceInfo(text);
    }
}

constexpr auto HResultToDxgiError(HRESULT hr) noexcept -> const char*
{
    using namespace LS::System;

    switch(hr)
    {
    case DXGI_ERROR_ACCESS_DENIED: return std::format("Access denied").c_str();
    case DXGI_ERROR_ACCESS_LOST: return std::format("Access lost: {}", hr).c_str();
    case DXGI_ERROR_ALREADY_EXISTS: return std::format("The element already exists: {}", hr).c_str();
    case DXGI_ERROR_CANNOT_PROTECT_CONTENT: return std::format("Cannot protect content: {}", hr).c_str();
    case DXGI_ERROR_DEVICE_HUNG: return std::format("Device hung: {}", hr).c_str();
    case DXGI_ERROR_DEVICE_REMOVED: return std::format("Device removed: {}", hr).c_str();
    case DXGI_ERROR_DEVICE_RESET: return std::format("Device reset: {}", hr).c_str();
    case DXGI_ERROR_DRIVER_INTERNAL_ERROR: return std::format("Driver internal error: {}", hr).c_str();
    case DXGI_ERROR_FRAME_STATISTICS_DISJOINT: return std::format("Frame statistics disjoint error: {}", hr).c_str();
    case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: return std::format("Graphics VIDPN source in use error: {}", hr).c_str();
    case DXGI_ERROR_INVALID_CALL: return std::format("Invalid call: {}", hr).c_str();
    case DXGI_ERROR_MORE_DATA: return std::format("Buffer supplied was too small for requested data: {}", hr).c_str();
    case DXGI_ERROR_NAME_ALREADY_EXISTS: return std::format("The supplied name already exists: {}", hr).c_str();
    case DXGI_ERROR_NONEXCLUSIVE: return std::format("Nonexclusive - Global resource in use and D3D devie cannot use the resource: {}", hr).c_str();
    case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE: return std::format("Not currently available: {}", hr).c_str();
    case DXGI_ERROR_NOT_FOUND: return std::format("Not found: {}", hr).c_str();
    case DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE: return std::format("Restrict output stale - swap chain was disconnected or changed: {}", hr).c_str();
    case DXGI_ERROR_SDK_COMPONENT_MISSING: return std::format("SDK Component Missing: {}", hr).c_str();
    case DXGI_ERROR_SESSION_DISCONNECTED: return std::format("Session disconnected: {}", hr).c_str();
    case DXGI_ERROR_UNSUPPORTED: return std::format("Requested functionality is unsupported by the device or driver: {}", hr).c_str();
    case DXGI_ERROR_WAIT_TIMEOUT: return std::format("Wait timeout - timeout interval has elapsed: {}", hr).c_str();
    case DXGI_ERROR_WAS_STILL_DRAWING: return std::format("Was still drawing - GPU was busy: {}", hr).c_str();
    case S_OK: return "OK";
    default:
        return std::format("Unknown Error Code Passed: {}", hr).c_str();
    }
}