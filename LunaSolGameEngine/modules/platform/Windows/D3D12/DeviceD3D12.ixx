module;
#include <d3d12.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wrl/client.h>
#include <dxgi1_6.h>
#include <dxgicommon.h>
#include <span>
#include <memory>
#include <string>
#include <string_view>
#include <array>
#include <cstdint>
#include <d3dx12.h>
#include "engine/EngineDefines.h"

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
    /**
     * @brief Manages the settings and features of the DirectX12 runtime. We initialize and grab the compatible DX12 device
     * and verify features available within the runtime are supported on the current graphics card. This is not meant
     * to be a "renderer" that performs drawing and other operations. At most, this is the communication link between
     * the user and the GPU device. 
    */
    class DeviceD3D12
    {
    public:
        DeviceD3D12(const D3D12Settings& settings);
        ~DeviceD3D12() = default;

        /**
         * @brief Initialize the D3D12 Device object
         * @param displayAdapter The display to use, if none if provided, it will find the first available display to use
         * @return true if operation was a success, false if an error occurred.
        */
        [[nodiscard]] auto CreateDevice(WRL::ComPtr<IDXGIAdapter> displayAdapter = nullptr) noexcept -> LS::System::ErrorCode;
        
        /**
         * @brief Returns the number of physical adapters (nodes) with this device
         * @return number of adapters
        */
        [[nodiscard]] UINT GetPhysicalAdapterCount() noexcept
        {
            return m_pDevice->GetNodeCount();
        }

        [[nodiscard]] auto GetSettings() const noexcept -> D3D12Settings
        {
            return m_settings;
        }

        [[nodiscard]] auto SwapchainWaitableHandle() const noexcept -> HANDLE
        {
            return m_pSwapChain->GetFrameLatencyWaitableObject();
        }

        [[nodiscard]] auto GetDevice() const noexcept -> WRL::ComPtr<ID3D12Device>
        {
            return m_pDevice;
        }

        [[nodiscard]] auto GetFeatureValidator() const noexcept -> CD3DX12FeatureSupport
        {
            return m_featureSupport;
        }

    private:
        /**
         * @brief Find a compatible display from the objects provided that meets the minimum feature level in @link D3D12Settings
         * @param adapters list of display adapters to iterate through
         * @return optional value that may contain objects or none if there is no display that meets the requirement
        */
        [[nodiscard]] auto FindCompatDisplay(std::span<WRL::ComPtr<IDXGIAdapter4>> adapters) noexcept -> Nullable<WRL::ComPtr<IDXGIAdapter4>>;

        void PrintDisplayAdapters();
        
        // Objects of Class // 
        D3D12Settings                   m_settings;
        HWND                            m_hwnd;

        // ComPtr Objects // 
        WRL::ComPtr<ID3D12Device1>      m_pDevice = nullptr;
        WRL::ComPtr<ID3D12Debug>        m_pDebug = nullptr;
        WRL::ComPtr<IDXGISwapChain2>    m_pSwapChain = nullptr;
        WRL::ComPtr<IDXGIFactory7>      m_pFactoryDxgi = nullptr;
        CD3DX12FeatureSupport           m_featureSupport;

    };
}

