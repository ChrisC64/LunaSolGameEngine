#include "LSEFramework.h"

import D3D11.Device;
import D3D11.RenderD3D11;
import Util.MSUtils;
import LSData;
import Util.HLSLUtils;

namespace WRL = Microsoft::WRL;

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
            Utils::ComRelease(m_pDebug.GetAddressOf());
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

        hr = factory->CreateSwapChainForHwnd(
            m_pDevice.Get(),
            winHandle,
            &swDesc1,
            nullptr,
            nullptr,
            &m_pSwapchain
        );

        Utils::ThrowIfFailed(hr, "Failed to create swap chain for HWND in DX11 Device");
    }

    void DeviceD3D11::CreateSwapchain(const LSWindowBase* window, PIXEL_COLOR_FORMAT format, uint32_t bufferSize)
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

    HRESULT DeviceD3D11::CreateDeferredContext(ID3D11DeviceContext** ppDeferredContext) noexcept
    {
        if (!m_pDevice)
            return E_NOT_SET; // Device is not set
        return m_pDevice->CreateDeferredContext(0, ppDeferredContext);
    }

    HRESULT DeviceD3D11::CreateDeferredContext2(ID3D11DeviceContext2** ppDeferredContext) noexcept
    {
        if (!m_pDevice)
            return E_NOT_SET; // Device is not set
        return m_pDevice->CreateDeferredContext2(0, ppDeferredContext);
    }

    HRESULT DeviceD3D11::CreateDeferredContext3(ID3D11DeviceContext3** ppDeferredContext) noexcept
    {
        if (!m_pDevice)
            return E_NOT_SET; // Device is not set
        return m_pDevice->CreateDeferredContext3(0, ppDeferredContext);
    }

    HRESULT DeviceD3D11::CreateInputLayout(std::span<D3D11_INPUT_ELEMENT_DESC> inputs, std::span<std::byte> byteCode, ID3D11InputLayout** ppInputLayout)
    {
        if (!m_pDevice && inputs.size() <= std::numeric_limits<UINT>().max())
            return E_NOT_SET;
        return m_pDevice->CreateInputLayout(inputs.data(), static_cast<UINT>(inputs.size()), byteCode.data(), byteCode.size(), ppInputLayout);
    }

    HRESULT DeviceD3D11::CreateRenderTargetView(ID3D11Resource* pResource, ID3D11RenderTargetView** ppRTView,
        const D3D11_RENDER_TARGET_VIEW_DESC* rtvDesc) noexcept
    {
        assert(m_pDevice);
        assert(pResource);

        return m_pDevice->CreateRenderTargetView(pResource, rtvDesc, ppRTView);
    }

    HRESULT DeviceD3D11::CreateRenderTargetView1(ID3D11Resource* pResource, ID3D11RenderTargetView1** ppRTView, const D3D11_RENDER_TARGET_VIEW_DESC1* rtvDesc) noexcept
    {
        assert(m_pDevice);
        assert(pResource);

        return m_pDevice->CreateRenderTargetView1(pResource, rtvDesc, ppRTView);
    }

    HRESULT DeviceD3D11::CreateDepthStencilView(ID3D11RenderTargetView* pRenderTargetView, ID3D11Resource* pResource,
        ID3D11DepthStencilView** ppDepthStencil, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc /*= nullptr*/) noexcept
    {
        assert(m_pDevice);
        assert(pRenderTargetView);
        assert(pResource);
        return m_pDevice->CreateDepthStencilView(pResource, pDesc, ppDepthStencil);
    }

    HRESULT DeviceD3D11::CreateDepthStencilViewForSwapchain(ID3D11RenderTargetView* pRenderTargetView, ID3D11DepthStencilView** ppDepthStencil, DXGI_FORMAT format /*= DXGI_FORMAT_D24_UNORM_S8_UINT*/) noexcept
    {
        assert(m_pDevice);
        assert(pRenderTargetView);
        assert(m_pSwapchain);

        auto result = m_pSwapchain->GetBuffer(0, IID_PPV_ARGS(&m_pBackBufferFrame));
        if (FAILED(result))
            return result;

        D3D11_TEXTURE2D_DESC depthBufferDesc{};
        m_pBackBufferFrame->GetDesc(&depthBufferDesc);
        depthBufferDesc.Format = format;
        depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        WRL::ComPtr<ID3D11Texture2D> depthBuffer;
        result = m_pDevice->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer);
        if (FAILED(result))
            return result;

        return m_pDevice->CreateDepthStencilView(depthBuffer.Get(), nullptr, ppDepthStencil);
    }

    HRESULT DeviceD3D11::CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& depthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState) noexcept
    {
        assert(m_pDevice);

        return m_pDevice->CreateDepthStencilState(&depthStencilDesc, ppDepthStencilState);
    }

    HRESULT DeviceD3D11::CreateBlendState(const D3D11_BLEND_DESC& blendDesc, ID3D11BlendState** ppBlendState) noexcept
    {
        assert(m_pDevice);
        return m_pDevice->CreateBlendState(&blendDesc, ppBlendState);
    }

    HRESULT DeviceD3D11::CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer) noexcept
    {
        assert(m_pDevice);

        if (!pDesc)
        {
            return E_NOT_SET;
        }

        return m_pDevice->CreateBuffer(pDesc, pInitialData, ppBuffer);
    }

    Microsoft::WRL::ComPtr<ID3D11Device5> DeviceD3D11::GetDevice() const noexcept
    {
        return m_pDevice;
    }

    Microsoft::WRL::ComPtr<ID3D11DeviceContext4> DeviceD3D11::GetImmediateContext() const noexcept
    {
        return m_pImmediateContext;
    }

    ID3D11DeviceContext* DeviceD3D11::GetImmediateContextPtr() const noexcept
    {
        return m_pImmediateContext.Get();
    }

    Microsoft::WRL::ComPtr<IDXGISwapChain1> DeviceD3D11::GetSwapChain() const noexcept
    {
        return m_pSwapchain;
    }

    bool DeviceD3D11::InitDevice(const LS::LSDeviceSettings& settings) noexcept
    {
        try
        {
            CreateDevice();
            CreateSwapchain(reinterpret_cast<HWND>(settings.WindowHandle), settings.SwapchainInfo);
        }
        catch (const std::exception& ex)
        {
            std::cout << std::format("{}\n", ex.what());
            return false;
        }

        return true;
    }

    auto DeviceD3D11::CreateContext() noexcept -> LSOptional<Ref<LS::ILSContext>>
    {
        assert(m_pDevice);
        return {};
    }
}