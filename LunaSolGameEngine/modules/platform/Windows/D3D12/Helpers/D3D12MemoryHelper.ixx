module;
#include <span>
#include <d3d12.h>
#include <directxtk12\BufferHelpers.h>
#include <directxtk12\DirectXHelpers.h>
#include <wrl\client.h>
export module D3D12.MemoryHelper;

import Engine.EngineCodes;

namespace WRL = Microsoft::WRL;

using namespace LS::System;

export namespace LS::Platform::Dx12
{
    /*[[nodiscard]] inline auto Allocate(ID3D12Device* pDevice, ID3D12GraphicsCommandList* cmdList,
        std::span<std::byte> pData) noexcept -> ErrorResult<WRL::ComPtr<ID3D12Resource>, LS::System::ErrorCode>
    {
        return LS::System::CreateFailCode("Not yet implemented");
    }*/
}