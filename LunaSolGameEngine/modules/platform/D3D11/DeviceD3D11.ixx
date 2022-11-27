module;
#include "LSEFramework.h"

export module D3D11.Device;
import Engine.LSDevice;

export namespace LS::Win32
{
    class DeviceD3D11
    {
    public:
        DeviceD3D11() = default;
        ~DeviceD3D11();

        void CreateDevice();
        void CreateSwapchain(HWND winHandle, const LS::LSSwapchainInfo& swapchainInfo);

    private:
        bool                                            m_bIsInitialized = false;
        Microsoft::WRL::ComPtr<ID3D11Device5>           m_pDevice = nullptr;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext4>    m_pContext = nullptr;
        Microsoft::WRL::ComPtr<ID3D11Debug>             m_pDebug = nullptr;
        Microsoft::WRL::ComPtr<IDXGISwapChain1>         m_pSwapchain = nullptr;
        D3D_FEATURE_LEVEL						        m_featureLevel{};

        DXGI_SWAP_CHAIN_DESC1 BuildSwapchainDesc1(const LS::LSSwapchainInfo& info);
    };
}