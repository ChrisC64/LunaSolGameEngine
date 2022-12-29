#pragma once

// STD C++ //
#include <memory>
#include <cstdint>
#include <cstddef>
#include <thread>
#include <vector>
#include <array>
#include <concepts>
#include <compare>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <optional>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <fstream>

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

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "Dxgi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3dcompiler")
#endif

#include "LSTypeDefs.h"
