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
        
        /**
         * @brief Create a command queue and return to the user
         * @param type D3D12_COMMAND_LIST_TYPE 
         * @param priority D3D12_COMMAND_QUEUE_PRIORITY defaults to D3D12_COMMAND_QUEUE_PRIORITY_NORMAL
         * @return An initialized ComPtr if successful, otherwise a nullptr if not. 
        */
        auto CreateCommandQueue(D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_PRIORITY priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL) noexcept -> WRL::ComPtr<ID3D12CommandQueue>;

        /**
         * @brief Create a Descriptor Heap and return to the user
         * @param type D3D12_DESCRIPTOR_HEAP_TYPE to set
         * @param numDescriptors The number of descriptors in the heap
         * @param isShaderVisible Whether this should be shader visible or not
         * @return An iniitlaized pointer of ID3D12DescriptorHeap if successful, otherwise a nullptr
        */
        auto CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, bool isShaderVisible = false) noexcept -> WRL::ComPtr<ID3D12DescriptorHeap>;

        /**
         * @brief Creates a command allocator 
         * @param type D3D12_COMMAND_LIST_TYPE to set it as
         * @return An initialized pointer to ID3D12CommandAllocator if successful, nullptr if failed
        */
        auto CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type) noexcept -> WRL::ComPtr<ID3D12CommandAllocator>;

        /**
         * @brief Creates a closed command list 
         * @param type D3D12_COMMAND_LIST_TYPE to create
         * @return An initialized ID3D12CommandList object if successful, nullptr if not
        */
        auto CreateCommandList(D3D12_COMMAND_LIST_TYPE type) noexcept -> WRL::ComPtr<ID3D12CommandList>;

        /**
         * @brief Creates a fence object initialized at 0 and has no flag options
         * @return A fence object, if any error occurs, will be nullptr
        */
        auto CreateFence(D3D12_FENCE_FLAGS flag = D3D12_FENCE_FLAG_NONE) noexcept -> WRL::ComPtr<ID3D12Fence>;

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

