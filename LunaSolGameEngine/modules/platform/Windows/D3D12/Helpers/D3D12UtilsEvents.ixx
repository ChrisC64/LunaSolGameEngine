module;
#include <d3d12.h>
#include <chrono>
#include <wrl/client.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

// Undefien the min/max even though NOMINMAX should do so but isn't for some reason
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

export module D3D12Lib:D3D12Utils.Events;

namespace WRL = Microsoft::WRL;

export namespace LS::Platform::Dx12
{

}