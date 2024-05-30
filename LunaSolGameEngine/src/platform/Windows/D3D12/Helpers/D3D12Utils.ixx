module;
#include <string>
#include <span>
#include <d3d12.h>
#include <directxtk12\BufferHelpers.h>
#include <directxtk12\DirectXHelpers.h>
#include <wrl\client.h>
#include <format>
export module D3D12Lib.Utils;

import Engine.EngineCodes;

namespace WRL = Microsoft::WRL;

using namespace LS::System;

export namespace LS::Platform::Dx12
{
    [[nodiscard]] constexpr auto Dx12ErroToString(HRESULT hr) noexcept -> const char*
    {
        switch (hr)
        {
        case D3D12_ERROR_ADAPTER_NOT_FOUND: return std::format("Adapter not found: {}", hr).c_str();
        case D3D12_ERROR_DRIVER_VERSION_MISMATCH: return std::format("Driver version mismatch: {}", hr).c_str();
        case DXGI_ERROR_INVALID_CALL: return std::format("Invalid call: {}", hr).c_str();
        case DXGI_ERROR_WAS_STILL_DRAWING: return std::format("Still drawing: {}", hr).c_str();
        case E_FAIL: return std::format("Fail: {}", hr).c_str();
        case E_INVALIDARG: return std::format("Invalid arg: {}", hr).c_str();
        case E_OUTOFMEMORY: return std::format("Out of memory: {}", hr).c_str();
        case E_NOTIMPL: return std::format("Not implemented: {}", hr).c_str();
        case S_FALSE: return std::format("Safe false error - a successful but nonstandard error: {}", hr).c_str();
        case S_OK: return std::format("Ok: {}", hr).c_str();
        default: return std::format("Unknown HRESULT code: {}", hr).c_str();
        }
    }
}