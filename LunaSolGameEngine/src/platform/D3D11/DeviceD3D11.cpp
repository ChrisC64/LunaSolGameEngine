#include <d3d11_4.h>
#include <wrl/client.h>
#include <cassert>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <format>

#include "engine/EngineLogDefines.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
import D3D11.Device;
import D3D11.RenderFuncD3D11;
import D3D11.MemoryHelper;
import D3D11.Utils;
import D3D11.LSTypeWrapper;
import Win32.ComUtils;
import Engine.Logger;
import LSEDataLib;
namespace WRL = Microsoft::WRL;

using namespace LS::Win32;
using namespace LS::Platform::Dx11;

DeviceD3D11::~DeviceD3D11()
{
#ifdef _DEBUG
    if (m_pDebug)
    {
        HRESULT hr = m_pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        if (FAILED(hr))
        {
            LS_LOG_DEBUG("Failed to report live objects!\n");
        }
        Utils::ComRelease(m_pDebug.GetAddressOf());
    }
#endif
    Shutdown();
}

void DeviceD3D11::CreateDevice(WRL::ComPtr<IDXGIAdapter> displayAdapter, bool isSingleThreaded /*= false*/)
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

    WRL::ComPtr<ID3D11Device> device;
    WRL::ComPtr<ID3D11DeviceContext> context;
    IDXGIAdapter* adapter = displayAdapter.Get() == nullptr ? nullptr : displayAdapter.Get();
    // Create the device only at this point, store the device, feature level, and context
    for (auto driverTypeIndex = 0u; driverTypeIndex < driverCount; ++driverTypeIndex)
    {
        hr = D3D11CreateDevice(
            adapter,
            driverTypes[0],
            nullptr,
            creationFlags,
            &featureLevels[driverTypeIndex],
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

    hr = context.As(&m_pImmediateContext);
    Utils::ThrowIfFailed(hr, "Failed to create ID3D11DeviceContext5 object");

    // Try to initialzie the Device and Context
    hr = device.As(&m_pDevice);
    Utils::ThrowIfFailed(hr, "Failed to initialize the device!\n");

    Utils::SetDebugName(m_pDevice.Get(), "Device");
#ifdef _DEBUG
    m_pDevice.As(&m_pDebug);
#endif
    m_bIsInitialized = true;
}

void DeviceD3D11::CreateSwapchain(HWND winHandle, uint32_t width, uint32_t height,
    uint32_t frameBufferCount /*= 2*/, PIXEL_COLOR_FORMAT pixelColorFormat /*= PIXEL_COLOR_FORMAT::RGBA8_UNORM*/)
{
    using namespace Microsoft::WRL;
    auto swDesc1 = BuildSwapchainDesc1(frameBufferCount, width, height, pixelColorFormat);
    WRL::ComPtr<IDXGIDevice1> dxgiDevice1;
    auto hr = Utils::QueryInterfaceFor(m_pDevice, dxgiDevice1);
    Utils::ThrowIfFailed(hr, "Failed to find DXGI Device1");

    WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
    hr = dxgiDevice1->GetAdapter(&dxgiAdapter);
    Utils::ThrowIfFailed(hr, "Failed to obtain adapter from DXGI Device1");

    WRL::ComPtr<IDXGIFactory2> factory;
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

void DeviceD3D11::CreateSwapchain(const LSWindowBase* window, PIXEL_COLOR_FORMAT format /*PIXEL_COLOR_FORMAT::RGBA8_UNORM*/, uint32_t bufferSize /*2*/)
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

    CreateSwapchain(static_cast<HWND>(window->GetHandleToWindow()), info.Width, info.Height, info.BufferSize, info.PixelFormat);
}

void DeviceD3D11::CreateSwapchainAsTexture(const LS::LSWindowBase* window, PIXEL_COLOR_FORMAT format /*PIXEL_COLOR_FORMAT::RGBA8_UNORM*/, uint32_t bufferSize /*2*/)
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

    D3D11_TEXTURE2D_DESC rtDesc;
    rtDesc.Format = ToDxgiFormat(format);
    rtDesc.ArraySize = 1;
    rtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    rtDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    rtDesc.MipLevels = 1;
    rtDesc.MiscFlags = 0;
    rtDesc.Usage = D3D11_USAGE_DYNAMIC;
    rtDesc.Width = window->GetWidth();
    rtDesc.Height = window->GetHeight();
    rtDesc.SampleDesc.Count = info.MSCount;
    rtDesc.SampleDesc.Quality = info.MSQuality;

    auto result = m_pDevice->CreateTexture2D(&rtDesc, nullptr, &m_pBackBufferFrame);
    if (FAILED(result))
    {
        Utils::ThrowIfFailed(result, "Failed to generate back buffer as texture.");
    }
}

auto LS::Win32::DeviceD3D11::ResizeSwapchain(uint32_t width, uint32_t height) noexcept -> HRESULT
{
    assert(m_pSwapchain);
    if (!m_pSwapchain)
        return E_POINTER;
    if (m_pBackBufferFrame)
    {
        m_pBackBufferFrame = nullptr;
    }

    return m_pSwapchain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
}

void LS::Win32::DeviceD3D11::DebugPrintLiveObjects()
{
#ifdef _DEBUG
    if (m_pDebug)
    {
        HRESULT hr = m_pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        if (FAILED(hr))
        {
            LS_LOG_DEBUG("Failed to report live objects!\n");
        }
        Utils::ComRelease(m_pDebug.GetAddressOf());
    }
#endif
}

auto DeviceD3D11::CreateDeferredContext(ID3D11DeviceContext** ppDeferredContext) noexcept -> HRESULT
{
    if (!m_pDevice)
        return E_NOT_SET; // Device is not set
    return m_pDevice->CreateDeferredContext(0, ppDeferredContext);
}

auto DeviceD3D11::CreateDeferredContext2(ID3D11DeviceContext2** ppDeferredContext) noexcept -> HRESULT
{
    if (!m_pDevice)
        return E_NOT_SET; // Device is not set
    return m_pDevice->CreateDeferredContext2(0, ppDeferredContext);
}

auto DeviceD3D11::CreateDeferredContext3(ID3D11DeviceContext3** ppDeferredContext) noexcept -> HRESULT
{
    if (!m_pDevice)
        return E_NOT_SET; // Device is not set
    return m_pDevice->CreateDeferredContext3(0, ppDeferredContext);
}

auto DeviceD3D11::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* elemDesc, uint32_t elemSize, std::span<std::byte> byteCode, ID3D11InputLayout** ppInputLayout) -> HRESULT
{
    if (!m_pDevice)
        return E_NOT_SET;
    return m_pDevice->CreateInputLayout(elemDesc, elemSize, byteCode.data(), byteCode.size(), ppInputLayout);
}

auto DeviceD3D11::CreateRTVFromBackBuffer(ID3D11RenderTargetView** ppRTV) noexcept -> HRESULT
{
    assert(m_pSwapchain);
    assert(m_pDevice);
    WRL::ComPtr<ID3D11Texture2D> backBuffer;

    HRESULT hr = m_pSwapchain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr))
    {
        LS_LOG_DEBUG(L"Failed to get back buffer from swap chain");
        return hr;
    }

    D3D11_TEXTURE2D_DESC swapDesc;
    backBuffer->GetDesc(&swapDesc);
    CD3D11_RENDER_TARGET_VIEW_DESC cdesc(backBuffer.Get(), D3D11_RTV_DIMENSION_TEXTURE2D, swapDesc.Format);

    return m_pDevice->CreateRenderTargetView(backBuffer.Get(), &cdesc, ppRTV);
}

auto DeviceD3D11::CreateRTVFromBackBuffer(ID3D11RenderTargetView1** ppRTV) noexcept -> HRESULT
{
    assert(m_pSwapchain);
    assert(m_pDevice);
    WRL::ComPtr<ID3D11Texture2D> backBuffer;

    HRESULT hr = m_pSwapchain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr))
    {
        LS_LOG_DEBUG(L"Failed to get back buffer from swap chain");
        return hr;
    }

    D3D11_TEXTURE2D_DESC swapDesc;
    backBuffer->GetDesc(&swapDesc);
    CD3D11_RENDER_TARGET_VIEW_DESC1 cdesc(backBuffer.Get(), D3D11_RTV_DIMENSION_TEXTURE2D, swapDesc.Format);

    return m_pDevice->CreateRenderTargetView1(backBuffer.Get(), &cdesc, ppRTV);
}

auto DeviceD3D11::CreateRenderTargetView(ID3D11Resource* pResource, ID3D11RenderTargetView** ppRTView,
    const D3D11_RENDER_TARGET_VIEW_DESC* rtvDesc) noexcept -> HRESULT
{
    assert(m_pDevice);
    assert(pResource);

    return m_pDevice->CreateRenderTargetView(pResource, rtvDesc, ppRTView);
}

auto DeviceD3D11::CreateRenderTargetView1(ID3D11Resource* pResource, ID3D11RenderTargetView1** ppRTView, const D3D11_RENDER_TARGET_VIEW_DESC1* rtvDesc) noexcept -> HRESULT
{
    assert(m_pDevice);
    assert(pResource);

    return m_pDevice->CreateRenderTargetView1(pResource, rtvDesc, ppRTView);
}

auto DeviceD3D11::CreateDepthStencilView([[maybe_unused]] ID3D11RenderTargetView* pRenderTargetView, ID3D11Resource* pResource,
    ID3D11DepthStencilView** ppDepthStencil, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc /*= nullptr*/) noexcept -> HRESULT
{
    assert(m_pDevice);
    assert(pRenderTargetView);
    assert(pResource);
    return m_pDevice->CreateDepthStencilView(pResource, pDesc, ppDepthStencil);
}

auto DeviceD3D11::CreateDepthStencilViewForSwapchain([[maybe_unused]] ID3D11RenderTargetView* pRenderTargetView, ID3D11DepthStencilView** ppDepthStencil, DXGI_FORMAT format /*= DXGI_FORMAT_D24_UNORM_S8_UINT*/) noexcept -> HRESULT
{
    assert(m_pDevice);
    assert(pRenderTargetView);
    assert(m_pSwapchain);
    //TODO: Why did I want to save the back buffer frame? Maybe reconsider and remove. 
    auto result = m_pSwapchain->GetBuffer(0, IID_PPV_ARGS(&m_pBackBufferFrame));
#ifdef _DEBUG
    std::string debug = "Back Buffer Texture2D";
    m_pBackBufferFrame->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)debug.size(), debug.data());
#endif
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

auto DeviceD3D11::CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& depthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState) noexcept -> HRESULT
{
    assert(m_pDevice);

    return m_pDevice->CreateDepthStencilState(&depthStencilDesc, ppDepthStencilState);
}

auto DeviceD3D11::CreateBlendState(const D3D11_BLEND_DESC& blendDesc, ID3D11BlendState** ppBlendState) noexcept -> HRESULT
{
    assert(m_pDevice);
    return m_pDevice->CreateBlendState(&blendDesc, ppBlendState);
}

auto DeviceD3D11::CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer) noexcept -> HRESULT
{
    assert(m_pDevice);

    if (!pDesc)
    {
        return E_NOT_SET;
    }

    return m_pDevice->CreateBuffer(pDesc, pInitialData, ppBuffer);
}

auto LS::Win32::DeviceD3D11::CreateVertexBuffer(const void* pData, uint32_t byteWidth, ID3D11Buffer** ppBuffer,
    D3D11_USAGE usage, uint32_t cpuAccess, uint32_t miscFlags, uint32_t structureByteStride) noexcept -> HRESULT
{
    assert(m_pDevice);

    return CreateVertexBufferD3D11(m_pDevice.Get(), ppBuffer, pData, byteWidth, usage, cpuAccess, miscFlags, structureByteStride);
}

auto LS::Win32::DeviceD3D11::CreateIndexBuffer(const void* pData, uint32_t bytes, ID3D11Buffer** ppBuffer, D3D11_USAGE usage, uint32_t cpuAccess, uint32_t miscFlags, uint32_t structureByteStride) noexcept -> HRESULT
{
    assert(m_pDevice);

    return CreateIndexBufferD3D11(m_pDevice.Get(), ppBuffer, pData, bytes, usage, cpuAccess, miscFlags, structureByteStride);;
}

auto LS::Win32::DeviceD3D11::CreateConstantBuffer(const void* pData, uint32_t byteWidth, ID3D11Buffer** ppBuffer, D3D11_USAGE usage, uint32_t cpuAccess, uint32_t miscFlags, uint32_t structureByteStride) noexcept -> HRESULT
{
    assert(m_pDevice);

    return CreateConstantBufferD3D11(m_pDevice.Get(), ppBuffer, pData, byteWidth, usage, cpuAccess, miscFlags, structureByteStride);
}

auto DeviceD3D11::GetDevice() const noexcept -> Microsoft::WRL::ComPtr<ID3D11Device5>
{
    return m_pDevice;
}

auto DeviceD3D11::GetImmediateContext() const noexcept -> Microsoft::WRL::ComPtr<ID3D11DeviceContext4>
{
    return m_pImmediateContext;
}

auto DeviceD3D11::GetImmediateContextPtr() const noexcept -> ID3D11DeviceContext*
{
    return m_pImmediateContext.Get();
}

auto DeviceD3D11::GetSwapChain() const noexcept -> Microsoft::WRL::ComPtr<IDXGISwapChain1>
{
    return m_pSwapchain;
}

auto DeviceD3D11::InitDevice(const LS::LSDeviceSettings& settings, LS::LSWindowBase* pWindow) noexcept -> LS::System::ErrorCode
{
    try
    {
        HWND winHandle = nullptr;
        CreateDevice();
        if (pWindow)
        {
            winHandle = reinterpret_cast<HWND>(pWindow->GetHandleToWindow());
            CreateSwapchain(winHandle, settings.Width, settings.Height, settings.FrameBufferCount, settings.PixelFormat);
        }
    }
    catch (const std::exception& ex)
    {
        m_bIsInitialized = false;
        return LS::System::CreateFailCode(ex.what());
    }

    m_bIsInitialized = true;
    return LS::System::CreateSuccessCode();
}

auto DeviceD3D11::CreateContext() noexcept -> Nullable<Ref<LS::ILSContext>>
{
    assert(m_pDevice);
    return {};
}

void DeviceD3D11::Shutdown() noexcept
{

}
