module;
#include <vector>
#include <memory>
#include <span>
#include <cassert>

#include <wrl/client.h>
#include <d3d11_4.h>

export module D3D11.Device;
import LSDataLib;
import Engine.LSDevice;
import Engine.LSWindow;
import Engine.Defines;

namespace WRL = Microsoft::WRL;

export namespace LS::Win32
{
    class DeviceD3D11 final : public ILSDevice
    {
    public:
        DeviceD3D11() = default;
        ~DeviceD3D11();

        void CreateDevice(WRL::ComPtr<IDXGIAdapter> displayAdapter = nullptr, bool isSingleThreaded = false);
        void DebugPrintLiveObjects();

        [[nodiscard]] auto CreateDeferredContext(ID3D11DeviceContext** pDeferredContext) noexcept -> HRESULT;
        [[nodiscard]] auto CreateDeferredContext2(ID3D11DeviceContext2** ppDeferredContext) noexcept -> HRESULT;
        [[nodiscard]] auto CreateDeferredContext3(ID3D11DeviceContext3** ppDeferredContext) noexcept -> HRESULT;
        [[nodiscard]] auto CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* elemDesc, uint32_t elemSize, std::span<std::byte> byteCode, ID3D11InputLayout** ppInputLayout) -> HRESULT;
        [[nodiscard]] auto CreateRenderTargetView(ID3D11Resource* pResource, ID3D11RenderTargetView** ppRTView, const D3D11_RENDER_TARGET_VIEW_DESC* rtvDesc = nullptr) noexcept -> HRESULT;
        [[nodiscard]] auto CreateRenderTargetView1(ID3D11Resource* pResource, ID3D11RenderTargetView1** ppRTView, const D3D11_RENDER_TARGET_VIEW_DESC1* rtvDesc = nullptr) noexcept -> HRESULT;
        [[nodiscard]] auto CreateDepthStencilView(ID3D11RenderTargetView* pRenderTargetView, ID3D11Resource* pResource, ID3D11DepthStencilView** ppDepthStencil, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc = nullptr) noexcept -> HRESULT;
        [[nodiscard]] auto CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& depthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState) noexcept -> HRESULT;
        [[nodiscard]] auto CreateBlendState(const D3D11_BLEND_DESC& blendDesc, ID3D11BlendState** ppBlendState) noexcept -> HRESULT;
        [[nodiscard]] auto CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer) noexcept -> HRESULT;
        [[nodiscard]] auto CreateVertexBuffer(const void* pData, uint32_t byteWidth, ID3D11Buffer** ppBuffer, D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT, uint32_t cpuAccess = 0, uint32_t miscFlags = 0, uint32_t structureByteStride = 0) noexcept -> HRESULT;
        [[nodiscard]] auto CreateIndexBuffer(const void* pData, uint32_t bytes, ID3D11Buffer** ppBuffer, D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT, uint32_t cpuAccess = 0, uint32_t miscFlags = 0, uint32_t structureByteStride = 0) noexcept -> HRESULT;
        [[nodiscard]] auto CreateConstantBuffer(const void* pData, uint32_t byteWidth, ID3D11Buffer** ppBuffer, D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT, uint32_t cpuAccess = 0, uint32_t miscFlags = 0, uint32_t structureByteStride = 0) noexcept -> HRESULT;

        [[nodiscard]] auto GetDevice() const noexcept -> WRL::ComPtr<ID3D11Device5>;
        [[nodiscard]] auto GetImmediateContext() const noexcept -> WRL::ComPtr<ID3D11DeviceContext4>;
        [[nodiscard]] auto GetIDXGDevice1() const noexcept -> Nullable<WRL::ComPtr<IDXGIDevice1>>;

        // Inherited by ILSDevice //
        [[nodiscard]] virtual auto InitDevice(const LS::LSDeviceSettings& settings, LS::LSWindowBase* pWindow) noexcept -> LS::System::ErrorCode final;
        
        [[nodiscard]] virtual auto CreateContext() noexcept -> Nullable<Ref<LS::ILSContext>> final;
        virtual void Shutdown() noexcept final;
        
    private:
        bool                                            m_bIsInitialized = false;
        WRL::ComPtr<ID3D11Device5>                      m_pDevice = nullptr;
        WRL::ComPtr<ID3D11DeviceContext4>               m_pImmediateContext = nullptr;
        WRL::ComPtr<ID3D11Debug>                        m_pDebug = nullptr;
        D3D_FEATURE_LEVEL                               m_featureLevel{};
    };
}

module : private;

import "engine/EngineLogDefines.h";

import D3D11.RenderFuncD3D11;
import D3D11.MemoryHelper;
import D3D11.Utils;
import D3D11.LSTypeWrapper;
import Win32.ComUtils;
import Engine.Logger;
import LSDataLib;

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

auto LS::Win32::DeviceD3D11::GetIDXGDevice1() const noexcept -> Nullable<WRL::ComPtr<IDXGIDevice1>>
{
    WRL::ComPtr<IDXGIDevice1> dxgiDevice1;
    auto hr = Utils::QueryInterfaceFor(m_pDevice, dxgiDevice1);
    if (FAILED(hr))
    {
        return std::nullopt;
    }

    return dxgiDevice1;
}

auto DeviceD3D11::InitDevice(const LS::LSDeviceSettings& settings, LS::LSWindowBase* pWindow) noexcept -> LS::System::ErrorCode
{
    try
    {
        CreateDevice();
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