module;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <mutex>

export module D3D12Lib:ResourceManagerD3D12;

import :D3D12Common;

namespace WRL = Microsoft::WRL;

export namespace LS::Win32
{
    class ResourceManagerD3D12
    {
    public:
        ResourceManagerD3D12() = default;
        ~ResourceManagerD3D12() = default;

        ResourceManagerD3D12(const ResourceManagerD3D12&) = delete;
        ResourceManagerD3D12& operator=(const ResourceManagerD3D12&) = delete;

        ResourceManagerD3D12(ResourceManagerD3D12&&) = default;
        ResourceManagerD3D12& operator=(ResourceManagerD3D12&&) = default;

        /**
         * @brief Create resources dependent on the D3D12 Device
        */
        void CreateDeviceDependentResources(WRL::ComPtr<ID3D12Device9> pDevice) noexcept;

        /**
         * @brief Create resources that don't require the use of the D3D12 Device
        */
        void CreateDeviceIndependentResources() noexcept;

        /**
         * @brief Create resources that require the size of the window
        */
        void CreateWindowSizeDependentResources() noexcept;

        void CreateSwapChain(const D3D12Settings& settings) noexcept;

        bool CreateCommandQueue() noexcept;

    private:
        // Window Dependent Resources //
        WRL::ComPtr<IDXGISwapChain4> m_pSwapChain;
        D3D12_VIEWPORT m_windowViewport;

        // Device Dependent Resources

        // Device Independent Resources

    };
}