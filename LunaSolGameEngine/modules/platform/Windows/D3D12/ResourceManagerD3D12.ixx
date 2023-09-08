module;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

export module D3D12Lib:ResourceManagerD3D12;

import :D3D12Common;
import Platform.Win32Window;
import Data.LSDataTypes;

namespace WRL = Microsoft::WRL;

export namespace LS::Platform::Dx12
{
    class ResourceManagerD3D12
    {
    public:
        ResourceManagerD3D12(WRL::ComPtr<ID3D12Device9>& device, SharedRef<D3D12Settings>& settings);
        ~ResourceManagerD3D12() = default;

        ResourceManagerD3D12(const ResourceManagerD3D12&) = delete;
        ResourceManagerD3D12& operator=(const ResourceManagerD3D12&) = delete;

        ResourceManagerD3D12(ResourceManagerD3D12&&) = default;
        ResourceManagerD3D12& operator=(ResourceManagerD3D12&&) = default;

        /**
         * @brief Create resources dependent on the D3D12 Device
        */
        void CreateDeviceDependentResources(WRL::ComPtr<ID3D12Device9>& pDevice) noexcept;

        /**
         * @brief Create resources that don't require the use of the D3D12 Device
        */
        void CreateDeviceIndependentResources() noexcept;

        /**
         * @brief Create resources that require the size of the window
        */
        void CreateWindowSizeDependentResources() noexcept;

        void CreateSwapChain(const Win32::Win32Window* window) noexcept;

        bool CreateCommandQueue() noexcept;

        void SetDebugDevice(WRL::ComPtr<ID3D12Debug5> Debug);

    private:
        // Window Dependent Resources //
        SharedRef<D3D12Settings>        m_pSettings;
        D3D12_VIEWPORT                  m_windowViewport;
        WRL::ComPtr<IDXGISwapChain4>    m_pSwapChain;


        // Device Dependent Resources
        WRL::ComPtr<ID3D12Device9>      m_pDevice;
        WRL::ComPtr<ID3D12Debug5>       m_pDebug;
        WRL::ComPtr<ID3D12CommandQueue> m_pCommandQueue = nullptr;
        WRL::ComPtr<IDXGIFactory7>      m_pFactoryDxgi = nullptr;

        // Device Independent Resources

    };
}