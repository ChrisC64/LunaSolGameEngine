module;
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include "platform\Windows\Win32\WinApiUtils.h"
export module D3D12Lib:DescriptorHeapDx12;

import Engine.EngineCodes;
import Util.MSUtils;

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
                return LS::System::CreateFailCode(msg);
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