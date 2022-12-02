module;
#include "LSEFramework.h"

export module D3D11.Device;
import Engine.LSDevice;
import Engine.LSWindow;

export namespace LS::Win32
{
    class DeviceD3D11
    {
    public:
        DeviceD3D11() = default;
        ~DeviceD3D11();

        void CreateDevice(bool isSingleThreaded = false);
        void CreateSwapchain(HWND winHandle, const LS::LSSwapchainInfo& swapchainInfo);
        void CreateSwapchain(const LS::LSWindowBase* window, LS::PIXEL_FORMAT format = LS::PIXEL_FORMAT::RGBA_8, uint32_t bufferSize = 2);
        HRESULT CreateDeferredContext(ID3D11DeviceContext** pDeferredContext);
        HRESULT CreateDeferredContext2(ID3D11DeviceContext2** ppDeferredContext);
        HRESULT CreateDeferredContext3(ID3D11DeviceContext3** ppDeferredContext);

        Microsoft::WRL::ComPtr<ID3D11Device5>           GetDevice();
        Microsoft::WRL::ComPtr<ID3D11DeviceContext4>    GetImmediateContext();
        Microsoft::WRL::ComPtr<IDXGISwapChain1>         GetSwapChain();

    private:
        bool                                            m_bIsInitialized = false;
        Microsoft::WRL::ComPtr<ID3D11Device5>           m_pDevice = nullptr;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext4>    m_pImmediateContext = nullptr;
        Microsoft::WRL::ComPtr<ID3D11Debug>             m_pDebug = nullptr;
        Microsoft::WRL::ComPtr<IDXGISwapChain1>         m_pSwapchain = nullptr;
        D3D_FEATURE_LEVEL						        m_featureLevel{};

        DXGI_SWAP_CHAIN_DESC1 BuildSwapchainDesc1(const LS::LSSwapchainInfo& info);
    };
}