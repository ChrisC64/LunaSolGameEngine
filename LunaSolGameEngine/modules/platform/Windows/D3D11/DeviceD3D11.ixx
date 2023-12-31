module;
#include <vector>
#include <memory>
#include <span>

#include <wrl/client.h>
#include <d3d11_4.h>
export module D3D11.Device;
import LSEDataLib;
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

        void CreateDevice(WRL::ComPtr<IDXGIAdapter> displayAdapter = nullptr, bool isSingleThreaded = false);
        void CreateSwapchain(HWND winHandle, uint32_t width, uint32_t height, uint32_t frameBufferCount = 2, PIXEL_COLOR_FORMAT pixelColorFormat = PIXEL_COLOR_FORMAT::RGBA8_UNORM);
        void CreateSwapchain(const LS::LSWindowBase* window, PIXEL_COLOR_FORMAT format = PIXEL_COLOR_FORMAT::RGBA8_UNORM, uint32_t bufferSize = 2);
        void CreateSwapchainAsTexture(const LS::LSWindowBase* window, PIXEL_COLOR_FORMAT format = PIXEL_COLOR_FORMAT::RGBA8_UNORM, uint32_t bufferSize = 2);
        auto ResizeSwapchain(uint32_t width, uint32_t height) noexcept -> HRESULT;
        void DebugPrintLiveObjects();

        //TODO: Consider moving around params, if the "out" params have to stay (for now) then maybe move them to the back/end of the list
        [[nodiscard]] auto CreateDeferredContext(ID3D11DeviceContext** pDeferredContext) noexcept -> HRESULT;
        [[nodiscard]] auto CreateDeferredContext2(ID3D11DeviceContext2** ppDeferredContext) noexcept -> HRESULT;
        [[nodiscard]] auto CreateDeferredContext3(ID3D11DeviceContext3** ppDeferredContext) noexcept -> HRESULT;
        [[nodiscard]] auto CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* elemDesc, uint32_t elemSize, std::span<std::byte> byteCode, ID3D11InputLayout** ppInputLayout) -> HRESULT;
        [[nodiscard]] auto CreateRTVFromBackBuffer(ID3D11RenderTargetView** ppRTV) noexcept -> HRESULT;
        [[nodiscard]] auto CreateRTVFromBackBuffer(ID3D11RenderTargetView1** ppRTV) noexcept -> HRESULT;
        [[nodiscard]] auto CreateRenderTargetView(ID3D11Resource* pResource, ID3D11RenderTargetView** ppRTView, const D3D11_RENDER_TARGET_VIEW_DESC* rtvDesc = nullptr) noexcept -> HRESULT;
        [[nodiscard]] auto CreateRenderTargetView1(ID3D11Resource* pResource, ID3D11RenderTargetView1** ppRTView, const D3D11_RENDER_TARGET_VIEW_DESC1* rtvDesc = nullptr) noexcept -> HRESULT;
        [[nodiscard]] auto CreateDepthStencilView(ID3D11RenderTargetView* pRenderTargetView, ID3D11Resource* pResource, ID3D11DepthStencilView** ppDepthStencil, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc = nullptr) noexcept -> HRESULT;
        [[nodiscard]] auto CreateDepthStencilViewForSwapchain(ID3D11RenderTargetView* pRenderTargetView, ID3D11DepthStencilView** ppDepthStencil, DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT) noexcept -> HRESULT;
        [[nodiscard]] auto CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC& depthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState) noexcept -> HRESULT;
        [[nodiscard]] auto CreateBlendState(const D3D11_BLEND_DESC& blendDesc, ID3D11BlendState** ppBlendState) noexcept -> HRESULT;
        [[nodiscard]] auto CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer) noexcept -> HRESULT;
        [[nodiscard]] auto CreateVertexBuffer(const void* pData, uint32_t byteWidth, ID3D11Buffer** ppBuffer, D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT, uint32_t cpuAccess = 0, uint32_t miscFlags = 0, uint32_t structureByteStride = 0) noexcept -> HRESULT;
        [[nodiscard]] auto CreateIndexBuffer(const void* pData, uint32_t bytes, ID3D11Buffer** ppBuffer, D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT, uint32_t cpuAccess = 0, uint32_t miscFlags = 0, uint32_t structureByteStride = 0) noexcept -> HRESULT;
        [[nodiscard]] auto CreateConstantBuffer(const void* pData, uint32_t byteWidth, ID3D11Buffer** ppBuffer, D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT, uint32_t cpuAccess = 0, uint32_t miscFlags = 0, uint32_t structureByteStride = 0) noexcept -> HRESULT;

        [[nodiscard]] auto GetDevice() const noexcept -> WRL::ComPtr<ID3D11Device5>;
        [[nodiscard]] auto GetImmediateContext() const noexcept -> WRL::ComPtr<ID3D11DeviceContext4>;
        [[nodiscard]] auto GetImmediateContextPtr() const noexcept -> ID3D11DeviceContext*;
        [[nodiscard]] auto GetSwapChain() const noexcept -> WRL::ComPtr<IDXGISwapChain1>;

        // Inherited by ILSDevice //
        [[nodiscard]] virtual auto InitDevice(const LS::LSDeviceSettings& settings) noexcept -> LS::System::ErrorCode final;
        
        [[nodiscard]] virtual auto CreateContext() noexcept -> Nullable<Ref<LS::ILSContext>> final;
        virtual void Shutdown() noexcept final;
        
    private:
        bool                                            m_bIsInitialized = false;
        WRL::ComPtr<ID3D11Device5>                      m_pDevice = nullptr;
        WRL::ComPtr<ID3D11DeviceContext4>               m_pImmediateContext = nullptr;
        WRL::ComPtr<ID3D11Debug>                        m_pDebug = nullptr;
        WRL::ComPtr<IDXGISwapChain1>                    m_pSwapchain = nullptr;
        WRL::ComPtr<ID3D11Texture2D>                    m_pBackBufferFrame = nullptr;
        D3D_FEATURE_LEVEL                               m_featureLevel{};
    };
}