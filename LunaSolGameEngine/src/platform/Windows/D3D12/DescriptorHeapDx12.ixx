module;
#include <d3d12.h>
#include <d3dx12\d3dx12_root_signature.h>
#include <wrl/client.h>
export module D3D12Lib.DescriptorHeapDx12;
import <stdexcept>;
import <cstdint>;

import Engine.EngineCodes;
import Win32.ComUtils;
import Win32.Utils;

namespace WRL = Microsoft::WRL;

export namespace LS::Platform::Dx12
{
    class DescriptorHeapDx12
    {
    public:
        DescriptorHeapDx12(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE, uint32_t node = 0) :
            m_desc{ .Type = type, .NumDescriptors = numDescriptors, .Flags = flags, .NodeMask = node },
            m_descriptorSize()
        {   
        }

        auto Initialize(ID3D12Device* pDevice) noexcept -> LS::System::ErrorCode
        {
            auto hr = pDevice->CreateDescriptorHeap(&m_desc, IID_PPV_ARGS(&m_heap));
            if (FAILED(hr))
            {
                const auto msg = LS::Win32::HrToString(hr);
                return LS::System::ErrorCode(msg);
            }
            m_descriptorSize = pDevice->GetDescriptorHandleIncrementSize(m_desc.Type);
            return LS::System::CreateSuccessCode();
        }

        auto GetType() const noexcept -> D3D12_DESCRIPTOR_HEAP_TYPE
        {
            return m_desc.Type;
        }

        auto GetSize() const noexcept -> uint32_t
        {
            return m_descriptorSize;
        }

        auto GetHeap() const noexcept -> ID3D12DescriptorHeap*
        {
            return m_heap.Get();
        }

        auto GetHeapStartCpu() const noexcept -> D3D12_CPU_DESCRIPTOR_HANDLE
        {
            return m_heap->GetCPUDescriptorHandleForHeapStart();
        }

        auto GetHeapStartGpu() const noexcept -> D3D12_GPU_DESCRIPTOR_HANDLE
        {
            //TODO: If not shader-visible then this returns null, maybe should just assert that
            return m_heap->GetGPUDescriptorHandleForHeapStart();
        }

        auto GetCpuHeapAt(uint32_t offset) const noexcept -> D3D12_CPU_DESCRIPTOR_HANDLE
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE heapHandle(GetHeapStartCpu(), offset, m_descriptorSize);
            return heapHandle;
        }

        auto GetGpuHeapAt(uint32_t offset) const noexcept -> D3D12_GPU_DESCRIPTOR_HANDLE
        {
            CD3DX12_GPU_DESCRIPTOR_HANDLE heapHandle(GetHeapStartGpu(), offset, m_descriptorSize);
            return heapHandle;
        }

        void SetName(LPCWSTR name) noexcept
        {
#ifdef _DEBUG
            m_heap->SetName(name);
#endif
        }

    private:
        WRL::ComPtr<ID3D12DescriptorHeap> m_heap;
        uint32_t m_descriptorSize;
        const D3D12_DESCRIPTOR_HEAP_DESC m_desc;
    };
}