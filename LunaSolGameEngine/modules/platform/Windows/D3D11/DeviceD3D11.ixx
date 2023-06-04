module;
#include <vector>
#include <memory>
#include <span>

#include <wrl/client.h>
#include <d3d11_4.h>
export module D3D11.Device;
import LSData;
import Engine.LSDevice;
import Engine.LSWindow;

namespace WRL = Microsoft::WRL;

export namespace LS::Win32
{
    class DeviceD3D11 final : public ILSDevice
    {
    public:
        DeviceD3D11() = default;
        ~DeviceD3D11();

        auto EnumerateDisplays() -> std::vector<WRL::ComPtr<IDXGIAdapter>>;
        void CreateDevice(WRL::ComPtr<IDXGIAdapter> displayAdapter = nullptr, bool isSingleThreaded = false);
        void CreateSwapchain(HWND winHandle, const LS::LSSwapchainInfo& swapchainInfo);
        void CreateSwapchain(const LS::LSWindowBase* window, PIXEL_COLOR_FORMAT format = PIXEL_COLOR_FORMAT::RGBA8_UNORM, uint32_t bufferSize = 2);
        void CreateSwapchainAsTexture(const LS::LSWindowBase* window, PIXEL_COLOR_FORMAT format = PIXEL_COLOR_FORMAT::RGBA8_UNORM, uint32_t bufferSize = 2);
        void PrintDisplays(const std::vector<WRL::ComPtr<IDXGIAdapter>>& adapters);

        HRESULT CreateDeferredContext(ID3D11DeviceContext** pDeferredContext) noexcept;
        HRESULT CreateDeferredContext2(ID3D11DeviceContext2** ppDeferredContext) noexcept;
        HRESULT CreateDeferredContext3(ID3D11DeviceContext3** ppDeferredContext) noexcept;
        HRESULT CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* elemDesc, uint32_t elemSize, std::span<std::byte> byteCode, ID3D11InputLayout** ppInputLayout);
        HRESULT CreateRenderTargetView(ID3D11Resource* pResource, ID3D11RenderTargetView** ppRTView, const D3D11_RENDER_TARGET_VIEW_DESC* rtvDesc = nullptr) noexcept;
        HRESULT CreateRenderTargetView1(ID3D11Resource* pResource, ID3D11RenderTargetView1** ppRTView, const D3D11_RENDER_TARGET_VIEW_DESC1* rtvDesc = nullptr) noexcept;
        HRESULT CreateDepthStencilView(ID3D11RenderTargetView* pRenderTargetView, ID3D11Resource* pResource, ID3D11DepthStencilView** ppDepthStencil, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc = nullptr) noexcept;
        HRESULT CreateDepthStencilViewForSwapchain(ID3D11RenderTargetView* pRenderTargetView, ID3D11DepthStencilView** ppDepthStencil, DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT) noexcept;
        HRESULT CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& depthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState) noexcept;
        HRESULT CreateBlendState(const D3D11_BLEND_DESC& blendDesc, ID3D11BlendState** ppBlendState) noexcept;
        HRESULT CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer) noexcept;

        WRL::ComPtr<ID3D11Device5>           GetDevice() const noexcept;
        WRL::ComPtr<ID3D11DeviceContext4>    GetImmediateContext() const noexcept;
        ID3D11DeviceContext*                 GetImmediateContextPtr() const noexcept;
        WRL::ComPtr<IDXGISwapChain1>         GetSwapChain() const noexcept;

        // Inherited by ILSDevice //
        [[nodiscard]] 
        virtual bool InitDevice(const LS::LSDeviceSettings& settings) noexcept final;
        
        [[nodiscard]] 
        virtual auto CreateContext() noexcept -> Nullable<Ref<LS::ILSContext>> final;
        virtual void Shutdown() noexcept final;

    private:
        bool                                            m_bIsInitialized = false;
        WRL::ComPtr<ID3D11Device5>                      m_pDevice = nullptr;
        WRL::ComPtr<ID3D11DeviceContext4>               m_pImmediateContext = nullptr;
        WRL::ComPtr<ID3D11Debug>                        m_pDebug = nullptr;
        WRL::ComPtr<IDXGISwapChain1>                    m_pSwapchain = nullptr;
        WRL::ComPtr<ID3D11Texture2D>                    m_pBackBufferFrame = nullptr;
        WRL::ComPtr<ID3D11DepthStencilView>             m_pDepthStencil = nullptr;// @brief depth stencil based on back buffer
        D3D_FEATURE_LEVEL                               m_featureLevel{};
    };
}