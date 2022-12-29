#include "LSEFramework.h"

import D3D11.Device;
import Util.MSUtils;

namespace LS::Win32
{
    DeviceD3D11::~DeviceD3D11()
    {
#ifdef _DEBUG
        if (m_pDebug)
        {
            HRESULT hr = m_pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
            if (FAILED(hr))
                TRACE("Failed to report live objects!\n");
            LS::Utils::ComRelease(m_pDebug.GetAddressOf());
        }
#endif
    }

    void DeviceD3D11::CreateDevice(bool isSingleThreaded /*= false*/)
    {
        using namespace Microsoft::WRL;

        HRESULT hr;
        UINT creationFlags = 0;

#ifdef _DEBUG
        creationFlags = D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#else
        creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#endif
        if (isSingleThreaded)
            creationFlags |= D3D11_CREATE_DEVICE_SINGLETHREADED;

        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_SOFTWARE,
        };

        auto driverCount = ARRAYSIZE(driverTypes);
        // If Feature Level 11_1 then Direct3D Runtime is 11.1
        // If Feature Level is 12_0 then Direct3D Runtime can be either 11.3 OR Direct3D 12.
        // Note that Feature level does not mean run time level. 
        // Feature levels use an underscore to separate their minor revisions (12_0, 11_1, etc.)
        // While Direct3D Runtimes are denoted with a dot revision (12.0, 11.1, 11.0, etc.)
        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };

        ComPtr<ID3D11Device> device;
        ComPtr<ID3D11DeviceContext> context;

        // Create the device only at this point, store the device, feature level, and context
        for (auto driverTypeIndex = 0u; driverTypeIndex < driverCount; ++driverTypeIndex)
        {
            hr = D3D11CreateDevice(
                nullptr,
                driverTypes[0],
                nullptr,
                creationFlags,
                featureLevels,
                ARRAYSIZE(featureLevels),
                D3D11_SDK_VERSION,
                &device,
                &m_featureLevel,
                &context
            );

            if (SUCCEEDED(hr))
            {
                break;
            }
        }
        assert(device);
        assert(context);
        //TODO: Report an error or throw exception if device and context are not set

        hr = context.As(&m_pImmediateContext);
        Utils::ThrowIfFailed(hr, "Failed to create ID3D11DeviceContext5 object");

        // Try to initialzie the Device and Context
        hr = device.As(&m_pDevice);
        Utils::ThrowIfFailed(hr, "Failed to initialize the device!\n");

        Utils::ComRelease(device.GetAddressOf());
        Utils::ComRelease(context.GetAddressOf());
        Utils::SetDebugName(m_pDevice.Get(), "Device");
#ifdef _DEBUG
        m_pDevice.As(&m_pDebug);
#endif
        m_bIsInitialized = true;
    }

    void DeviceD3D11::CreateSwapchain(HWND winHandle, const LS::LSSwapchainInfo& swapchainInfo)
    {
        using namespace Microsoft::WRL;

        auto swDesc1 = BuildSwapchainDesc1(swapchainInfo);
        ComPtr<IDXGIDevice1> dxgiDevice1;
        auto hr = Utils::QueryInterfaceFor(m_pDevice, dxgiDevice1);
        Utils::ThrowIfFailed(hr, "Failed to find DXGI Device1");

        ComPtr<IDXGIAdapter> dxgiAdapter;
        hr = dxgiDevice1->GetAdapter(&dxgiAdapter);
        Utils::ThrowIfFailed(hr, "Failed to obtain adapter from DXGI Device1");

        ComPtr<IDXGIFactory2> factory;
        hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&factory));
        Utils::ThrowIfFailed(hr, "Failed to find IDXGIFactory2 for Adapter");

        ComPtr<IDXGISwapChain1> swapchain1;
        hr = factory->CreateSwapChainForHwnd(
            m_pDevice.Get(),
            winHandle,
            &swDesc1,
            nullptr,
            nullptr,
            swapchain1.GetAddressOf()
        );

        Utils::ThrowIfFailed(hr, "Failed to create swap chain for HWND in DX11 Device");
        swapchain1.As(&m_pSwapchain);
    }

    void DeviceD3D11::CreateSwapchain(const LS::LSWindowBase* window, LS::PIXEL_FORMAT format, uint32_t bufferSize)
    {
        LS::LSSwapchainInfo info{
            .BufferSize = bufferSize,
            .Width = window->GetWidth(),
            .Height = window->GetHeight(),
            .PixelFormat = format,
            .IsStereoScopic = false,
            .MSCount = 1,
            .MSQuality = 0
        };

        CreateSwapchain(static_cast<HWND>(window->GetHandleToWindow()), info);
    }

    HRESULT DeviceD3D11::CreateDeferredContext(ID3D11DeviceContext** ppDeferredContext)
    {
        if (!m_pDevice)
            return E_NOT_SET; // Device is not set
        return m_pDevice->CreateDeferredContext(0, ppDeferredContext);
    }
    
    HRESULT DeviceD3D11::CreateDeferredContext2(ID3D11DeviceContext2** ppDeferredContext)
    {
        if (!m_pDevice)
            return E_NOT_SET; // Device is not set
        return m_pDevice->CreateDeferredContext2(0, ppDeferredContext);
    }
    
    HRESULT DeviceD3D11::CreateDeferredContext3(ID3D11DeviceContext3** ppDeferredContext)
    {
        if (!m_pDevice)
            return E_NOT_SET; // Device is not set
        return m_pDevice->CreateDeferredContext3(0, ppDeferredContext);
    }

    Microsoft::WRL::ComPtr<ID3D11Device5> DeviceD3D11::GetDevice()
    {
        return m_pDevice;
    }

    Microsoft::WRL::ComPtr<ID3D11DeviceContext4> DeviceD3D11::GetImmediateContext()
    {
        return m_pImmediateContext;
    }

    Microsoft::WRL::ComPtr<IDXGISwapChain1> DeviceD3D11::GetSwapChain()
    {
        return m_pSwapchain;
    }

    DXGI_SWAP_CHAIN_DESC1 DeviceD3D11::BuildSwapchainDesc1(const LS::LSSwapchainInfo& info)
    {
        DXGI_SWAP_CHAIN_DESC1 swDesc1{};
        swDesc1.BufferCount = info.BufferSize;
        swDesc1.Height = info.Height;
        swDesc1.Width = info.Width;
        using enum LS::PIXEL_FORMAT;
        DXGI_FORMAT format;
        switch (info.PixelFormat)
        {
        case RGBA_8:
            format = DXGI_FORMAT_R8G8B8A8_UNORM;
            break;
        case BGRA_8:
            format = DXGI_FORMAT_B8G8R8A8_UNORM;
            break;
        case RGBA_16:
            format = DXGI_FORMAT_R16G16B16A16_UNORM;
            break;
        case BGRA_16:
            format = DXGI_FORMAT_R16G16B16A16_UNORM;
            break;
        default:
            format = DXGI_FORMAT_R8G8B8A8_UNORM;
            break;
        }
        swDesc1.Format = format;
        swDesc1.Stereo = false;
        swDesc1.SampleDesc.Count = 1;
        swDesc1.SampleDesc.Quality = 0;
        swDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swDesc1.Scaling = DXGI_SCALING_NONE;
        swDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swDesc1.Flags = 0;
        return swDesc1;
    }
}