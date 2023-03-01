module;
#ifdef LS_WINDOWS_BUILD
// Windows Headers
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>
#include <atlbase.h>
#include <atlcomcli.h>
// DirectX/Windows Headers //
#include <d2d1helper.h>
#include <d2d1.h>
#include <wrl/client.h>
#include <d2d1.h>
#include <dwrite_3.h>
// D3D11 Headers //
#include <d3d11_4.h>

// DXGI Interfaces
#include <dxgi1_6.h>

// DirectX Libraries //
#include <DirectXMath.h>
#include <directxtk/CommonStates.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>

// libs to include //
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "Dxgi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3dcompiler")
#endif

#include <optional>
#include <string>
#include <memory>
export module Data.LSDataTypes;
// A list of common data types to define here for use within the engine itself

export
{
    // Typedefs //
// Reference Pointers //
    template <class T, typename Deleter = std::default_delete<T>>
    using Ref = std::unique_ptr<T, Deleter>;
    template <class T, typename Deleter>
    using Ref = std::unique_ptr<T, Deleter>;

    template <class S>
    using SharedRef = std::shared_ptr<S>;
    template<class T>
    using Nullable = std::optional<T>;
    using Guid = std::string;
    using Id = uint64_t;
#ifdef LS_WINDOWS_BUILD
    using mat4 = DirectX::XMFLOAT4X4;
#endif
}

export namespace LS
{
    enum class ERROR_LEVEL
    {
        INFO,
        WARN,
        FAIL,
        CRITICAL
    };
}