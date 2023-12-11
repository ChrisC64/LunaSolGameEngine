module;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>

export module D3D12Lib;

export import :D3D12Common;
export import :D3D12Utils.Descriptors;
export import :D3D12Utils.Commands;
export import :Device;
export import :ResourceManagerD3D12;
export import :CommandListDx12;
export import :CommandQueueD3D12;
export import :FrameDx12;
export import :FrameBufferDxgi;