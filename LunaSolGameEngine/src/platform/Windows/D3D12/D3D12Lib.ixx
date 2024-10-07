module;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>

export module D3D12Lib;

export import D3D12Lib.D3D12Common;
export import D3D12Lib.Utils;
export import D3D12Lib.D3D12Utils.Descriptors;
export import D3D12Lib.D3D12Utils.Commands;
export import D3D12Lib.Device;
export import D3D12Lib.ResourceManagerD3D12;
export import D3D12Lib.CommandListDx12;
export import D3D12Lib.CommandQueueD3D12;
export import D3D12Lib.DescriptorHeapDx12;
export import D3D12Lib.FrameDx12;
export import D3D12Lib.FrameBufferDxgi;
export import D3D12Lib.RendererDX12;