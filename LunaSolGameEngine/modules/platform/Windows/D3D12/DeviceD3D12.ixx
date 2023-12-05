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
        DeviceD3D12(D3D12Settings&& settings);
        ~DeviceD3D12() = default;

        /**
         * @brief Initialize the D3D12 Device object
         * @param displayAdapter The display to use, if none if provided, it will find the first available display to use
         * @return true if operation was a success, false if an error occurred.
        */
        [[nodiscard]] auto CreateDevice(WRL::ComPtr<IDXGIAdapter> displayAdapter = nullptr) noexcept -> LS::System::ErrorCode;
        
        /**
         * @brief Create a command queue and return to the user
         * @param type D3D12_COMMAND_LIST_TYPE 
         * @param priority D3D12_COMMAND_QUEUE_PRIORITY defaults to D3D12_COMMAND_QUEUE_PRIORITY_NORMAL
         * @return An initialized ComPtr if successful, otherwise a nullptr if not. 
        */
        //[[nodiscard]] auto CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_PRIORITY priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL) noexcept -> WRL::ComPtr<ID3D12CommandQueue>;

        /**
         * @brief Create a Descriptor Heap and return to the user
         * @param type D3D12_DESCRIPTOR_HEAP_TYPE to set
         * @param numDescriptors The number of descriptors in the heap
         * @param isShaderVisible Whether this should be shader visible or not
         * @return An iniitlaized pointer of ID3D12DescriptorHeap if successful, otherwise a nullptr
        */
        //[[nodiscard]] auto CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, bool isShaderVisible = false) noexcept -> WRL::ComPtr<ID3D12DescriptorHeap>;

        /**
         * @brief Creates a command allocator 
         * @param type D3D12_COMMAND_LIST_TYPE to set it as
         * @return An initialized pointer to ID3D12CommandAllocator if successful, nullptr if failed
        */
        //[[nodiscard]] auto CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type) noexcept -> WRL::ComPtr<ID3D12CommandAllocator>;

        /**
         * @brief Creates a closed command list 
         * @param type D3D12_COMMAND_LIST_TYPE to create
         * @return An initialized ID3D12CommandList object if successful, nullptr if not
        */
        //[[nodiscard]] auto CreateCommandList(D3D12_COMMAND_LIST_TYPE type) noexcept -> WRL::ComPtr<ID3D12CommandList>;

        /**
         * @brief Creates a fence object initialized at 0 and has no flag options
         * @return A fence object, if any error occurs, will be nullptr
        */
        //[[nodiscard]] auto CreateFence(D3D12_FENCE_FLAGS flag = D3D12_FENCE_FLAG_NONE) noexcept -> WRL::ComPtr<ID3D12Fence>;

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

        [[nodiscard]] auto GetDeviceD3D12() const noexcept -> WRL::ComPtr<ID3D12Device>
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

