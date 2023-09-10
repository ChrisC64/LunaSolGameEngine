module;
#include <d3d12.h>
#include <Windows.h>
#include <wrl/client.h>
#include <dxgi1_6.h>
#include <dxgicommon.h>
#include <span>
#include <memory>
#include <string>
#include <string_view>
#include <array>
#pragma comment(lib, "dxguid.lib")

export module D3D12Lib:Device;

import :D3D12Common;
import :ResourceManagerD3D12;

import Platform.Win32Window;
import Engine.App;
import Engine.EngineCodes;

namespace WRL = Microsoft::WRL;

export namespace LS::Platform::Dx12
{
    class DeviceD3D12
    {
    public:
        DeviceD3D12(D3D12Settings&& settings);
        ~DeviceD3D12() = default;

        /**
         * @brief Initialize the D3D12 Device object
         * @param displayAdapter The display to use, if none if provided, it will find the first available display to use
         * @return true if operation was a success, false if an error occurred.
        */
        auto CreateDevice(WRL::ComPtr<IDXGIAdapter> displayAdapter = nullptr) noexcept -> LS::System::ErrorCode;
        
        auto CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_PRIORITY priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL) noexcept -> WRL::ComPtr<ID3D12CommandQueue>;

        /**
         * @brief Returns the number of physical adapters (nodes) with this device
         * @return number of adapters
        */
        UINT GetPhysicalAdapterCount() noexcept
        {
            return m_pDevice->GetNodeCount();
        }

    private:
        /**
         * @brief Find a compatible display from the objects provided that meets the minimum feature level in @link D3D12Settings
         * @param adapters list of display adapters to iterate through
         * @return optional value that may contain objects or none if there is no display that meets the requirement
        */
        auto FindCompatDisplay(std::span<WRL::ComPtr<IDXGIAdapter4>> adapters) noexcept -> Nullable<WRL::ComPtr<IDXGIAdapter4>>;
        void CreateSwapchain();

        void PrintDisplayAdapters();
        
        // Objects of Class // 
        D3D12Settings                   m_settings;
        HWND                            m_hwnd;
        uint64_t                        m_frameIndex = 0u;
        // ComPtr Objects // 
        WRL::ComPtr<ID3D12Device9>      m_pDevice = nullptr;
        WRL::ComPtr<ID3D12Debug5>       m_pDebug = nullptr;
        WRL::ComPtr<IDXGISwapChain4>    m_pSwapChain = nullptr;
        WRL::ComPtr<IDXGIFactory7>      m_pFactoryDxgi = nullptr;

    public: // Public inline functions (mainly getters/setters)
        D3D12Settings GetSettings() noexcept
        {
            return m_settings;
        }

        auto SwapchainWaitableHandle() noexcept -> HANDLE
        {
            return m_pSwapChain->GetFrameLatencyWaitableObject();
        }
    };
}

