module;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>

export module D3D12Lib;

export import :D3D12Common;
export import :D3D12Utils.Descriptors;
export import :D3D12Utils.CommandList;
export import :Device;
export import :ResourceManagerD3D12;