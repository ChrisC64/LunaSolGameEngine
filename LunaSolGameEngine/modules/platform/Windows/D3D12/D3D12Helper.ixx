module;
#include <d3d12.h>
#include <span>
#include <cassert>
export module D3D12Lib:D3D12Helper;

export namespace LS::Win32
{
    constexpr void SetViewPorts(ID3D12GraphicsCommandList* command, std::span<D3D12_VIEWPORT> viewports) noexcept
    {
        assert(command);
        assert(viewports.size() <= UINT32_MAX);
        command->RSSetViewports((UINT)viewports.size(), &viewports.front());
    }

    constexpr void SetScissorRects(ID3D12GraphicsCommandList* command, std::span<D3D12_RECT> rects) noexcept
    {
        assert(command);
        assert(rects.size() <= UINT32_MAX);
        command->RSSetScissorRects((UINT)rects.size(), &rects.front());
    }


}