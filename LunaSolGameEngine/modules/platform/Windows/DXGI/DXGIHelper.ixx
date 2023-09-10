module;
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <vector>
#include <cassert>
#include <optional>
#include <format>
#include <string>
#include <d3d12.h>
#include "platform\Windows\Win32\WinApiUtils.h"

export module DXGIHelper;
export import Data.LSDataTypes;

import Engine.Logger;
import Engine.EngineCodes;
import D3D12Lib;

namespace WRL = Microsoft::WRL;

export namespace LS::Win32
{
    /**
     * @brief Creates a DXGI Factory
     * @param flags DXGI_CREATE_FLAGS
     * @return @link Nullable object of a IDXGIFactory7 instance (see module @link LSdataTypes.ixx)
    */
    [[nodiscard]] auto CreateFactory(UINT flags = 0u) noexcept -> Nullable<WRL::ComPtr<IDXGIFactory7>>;

    [[nodiscard]] auto CreateSwapchainForHwnd(const Platform::Dx12::D3D12Settings& settings,
        const WRL::ComPtr<IDXGIFactory2>& factory, WRL::ComPtr<ID3D12CommandQueue>& commandQueue) noexcept -> Nullable<WRL::ComPtr<IDXGISwapChain1>>;

    /**
     * @brief Returns any high performance display adapters (discrete or external GPU supported)
     * @param pFactory The IDXGIFactory, must be convertable to a IDXGIFactory6 object
     * @pram requestHighPerformance To request the high performance GPU (true) or minimum GPU (false), the default is true
     * @return A vector of all Discrete GPU supported displays if any are found.
    */
    [[nodiscard]] auto EnumerateDiscreteGpuAdapters(WRL::ComPtr<IDXGIFactory7>& pFactory, bool requestHighPerformance = true) noexcept -> std::vector<WRL::ComPtr<IDXGIAdapter4>>;

    /**
     * @brief Returns any display adapters
     * @param pFactory The IDXGIFactory interface object to use
     * @return A container containing all display adapters
    */
    [[nodiscard]] auto EnumerateDisplayAdapters(WRL::ComPtr<IDXGIFactory7>& pFactory) noexcept -> std::vector<WRL::ComPtr<IDXGIAdapter4>>;



    // Feature Check Supports //

    /**
     * @brief Checks if tearing support (i.e. FreeSync/G-Sync) is available
     * @param pFactory Windows DXGI Factory 
     * @return An error code with any messages if support is available or not.
    */
    [[nodiscard]] auto CheckTearingSupport(const WRL::ComPtr<IDXGIFactory5>& pFactory) noexcept -> LS::System::ErrorCode;

    void LogAdapters(IDXGIFactory4* factory) noexcept;
    void LogAdapterOutput(IDXGIAdapter* adapter) noexcept;
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format) noexcept;
}

module : private;

using namespace LS::Win32;

auto LS::Win32::CreateFactory(UINT flags) noexcept -> Nullable<Microsoft::WRL::ComPtr<IDXGIFactory7>>
{
    WRL::ComPtr<IDXGIFactory7> pOut;
    auto result = CreateDXGIFactory2(flags, IID_PPV_ARGS(&pOut));
    if (FAILED(result))
        return std::nullopt;
    return pOut;
}

auto LS::Win32::CreateSwapchainForHwnd(const Platform::Dx12::D3D12Settings& settings, 
    const WRL::ComPtr<IDXGIFactory2>& factory, WRL::ComPtr<ID3D12CommandQueue>& commandQueue) noexcept -> Nullable<WRL::ComPtr<IDXGISwapChain1>>
{
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc1{};
    swapchainDesc1.BufferCount = settings.MinSettings.FrameBufferCount;
    swapchainDesc1.Width = static_cast<UINT>(settings.MinSettings.ScreenWidth);
    swapchainDesc1.Height = static_cast<UINT>(settings.MinSettings.ScreenHeight) ;
    swapchainDesc1.Format = settings.MinSettings.PixelFormat;
    swapchainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc1.SampleDesc = { .Count = 1, .Quality = 0 };

    // If VSync is off, should use FIP_DISCARD not FLIP_SEQUENTIAL
    swapchainDesc1.SwapEffect = settings.ExSettings.SwapEffect;
    swapchainDesc1.AlphaMode = settings.ExSettings.AlphaMode;
    swapchainDesc1.Scaling = settings.ExSettings.Scaling;
    swapchainDesc1.Stereo = settings.ExSettings.IsStereoScopic ? TRUE : FALSE;
    // Feature Flags to support // 
    swapchainDesc1.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

    WRL::ComPtr<IDXGISwapChain1> swapchain;

    const auto hr = factory->CreateSwapChainForHwnd(commandQueue.Get(), settings.MinSettings.Hwnd,
        &swapchainDesc1, nullptr, nullptr, &swapchain);

    if (FAILED(hr))
    {
        return std::nullopt;
    }

    return swapchain;
}

auto LS::Win32::EnumerateDiscreteGpuAdapters(Microsoft::WRL::ComPtr<IDXGIFactory7>& pFactory, bool requestHighPerformance /*= true*/) noexcept -> std::vector<WRL::ComPtr<IDXGIAdapter4>>
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
            // Don't select the Basic Render Driver adapter.
            //TODO: Maybe add optional support for software adapter types 
            continue;
        }
        out.push_back(std::move(adapter));
    }

    return out;
}

auto LS::Win32::EnumerateDisplayAdapters(Microsoft::WRL::ComPtr<IDXGIFactory7>& pFactory) noexcept -> std::vector<WRL::ComPtr<IDXGIAdapter4>>
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
            // Don't select the Basic Render Driver adapter.
            //TODO: Maybe add optional support for software adapter types 
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

auto LS::Win32::CheckTearingSupport(const WRL::ComPtr<IDXGIFactory5>& pFactory) noexcept -> LS::System::ErrorCode
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
}

void LS::Win32::LogAdapterOutput(IDXGIAdapter* adapter) noexcept
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

void LS::Win32::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format) noexcept
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