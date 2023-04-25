module;
#include <d3d12.h>
#include <Windows.h>
#include <wrl/client.h>
#include <dxgi1_6.h>
#include <dxgicommon.h>

#pragma comment(lib, "dxguid.lib")
export module D3D12Lib:Device;
namespace WRL = Microsoft::WRL;

export namespace LS::Win32
{
    class DeviceD3D12
    {
    public:
        DeviceD3D12() = default;
        ~DeviceD3D12() = default;

        bool CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter> displayAdapter = nullptr)
        {
            return false;
        }

    private:
        WRL::ComPtr<ID3D12Device9> m_pDevice;

        WRL::ComPtr<ID3D12Debug5> m_pDebug;
    };
}

//module : private;
//using namespace LS::Win32;
//
//bool DeviceD3D12::CreateDevice(WRL::ComPtr<IDXGIAdapter> displayAdpater /* = nullptr*/)
//{
//#ifdef _DEBUG
//    WRL::ComPtr<ID3D12Debug> pdx12Debug = nullptr;
//    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
//        pdx12Debug->EnableDebugLayer();
//#endif
//
//    WRL::ComPtr<IDXGIFactory7> pFactory;
//    CreateDXGIFactory2(0u, IID_PPV_ARGS(&pFactory));
//
//    // Find the best graphics card (best performing one, with single GPU systems, this should be the default)
//    WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
//    GetHardwareAdapter(factory.Get(), &hardwareAdapter, false);
//
//    // If display adapter is not set, find the first available DX12 display supported
//    if (!displayAdpater)
//    {
//
//    }
//}