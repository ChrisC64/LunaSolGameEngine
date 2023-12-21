#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <cassert>
#include <format>
#include "platform/Windows/Win32/WinApiUtils.h"
#include "engine/EngineLogDefines.h"
#include "engine/EngineDefines.h"
import D3D12Lib;
import LSEDataLib;
import Platform.Win32Window;
import Engine.App;

namespace WRL = Microsoft::WRL;
using namespace LS::Win32;
using namespace LS::Platform::Dx12;

class HrException : public std::runtime_error
{
public:
    HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
    HRESULT Error() const { return m_hr; }
private:
    const HRESULT m_hr;
};

ResourceManagerD3D12::ResourceManagerD3D12(WRL::ComPtr<ID3D12Device9>& device, SharedRef<D3D12Settings>& settings) : m_pDevice(device),
    m_pSettings(settings)
{
}

void ResourceManagerD3D12::CreateDeviceDependentResources([[maybe_unused]] WRL::ComPtr<ID3D12Device9>& pDevice) noexcept
{
    assert(m_pDevice);
    if (!m_pDevice)
        return;

}

void ResourceManagerD3D12::CreateDeviceIndependentResources() noexcept
{
}

void ResourceManagerD3D12::CreateWindowSizeDependentResources() noexcept
{
}

void ResourceManagerD3D12::CreateSwapChain(const Win32Window* window) noexcept
{
    assert(window);
    assert(m_pFactoryDxgi);
    assert(m_pCommandQueue);

    DXGI_SWAP_CHAIN_DESC1 swapchainDesc1{};
    swapchainDesc1.BufferCount = m_pSettings->FrameBufferCount;
    swapchainDesc1.Width = window->GetWidth();
    swapchainDesc1.Height = window->GetHeight();
    swapchainDesc1.Format = m_pSettings->PixelFormat;
    swapchainDesc1.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    swapchainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc1.SampleDesc.Count = 1;
    swapchainDesc1.SampleDesc.Quality = 0;
    swapchainDesc1.SwapEffect = m_pSettings->SwapEffect;
    swapchainDesc1.AlphaMode = m_pSettings->AlphaMode;
    swapchainDesc1.Scaling = m_pSettings->Scaling;
    swapchainDesc1.Stereo = m_pSettings->IsStereoScopic;

    //TODO: Currently doesn't set the DXGI_SWAPCHAIN_FULLSCREEN_DESC to handle full screen displays,
    // will need to add that in later. 
    WRL::ComPtr<IDXGISwapChain1> swapchain;
    auto hr = m_pFactoryDxgi->CreateSwapChainForHwnd(m_pCommandQueue.Get(), window->Hwnd(), &swapchainDesc1, nullptr, nullptr, &swapchain);
    if (FAILED(hr))
    {
        LS_LOG_ERROR(std::format(L"Failed to create Swap Chain. Error: {}", HrToWString(hr)));
        return;
    }

    hr = swapchain.As(&m_pSwapChain);
    if (FAILED(hr))
    {
        LS_LOG_ERROR(std::format(L"Failed to initialize Swap Chain 4 interface com object. Error: {}", HrToWString(hr)));
        return;
    }

    m_pSwapChain->SetMaximumFrameLatency(m_pSettings->FrameBufferCount);
    LS::Global::FrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

bool ResourceManagerD3D12::CreateCommandQueue() noexcept
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 1;

    auto hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pCommandQueue));
    if (FAILED(hr))
    {
        LS_LOG_ERROR(std::format(L"Failed to create command queue: {}", HrToWString(hr)));
        return false;
    }

    return true;
}

void ResourceManagerD3D12::SetDebugDevice(WRL::ComPtr<ID3D12Debug5> Debug)
{
    m_pDebug = Debug;
}
