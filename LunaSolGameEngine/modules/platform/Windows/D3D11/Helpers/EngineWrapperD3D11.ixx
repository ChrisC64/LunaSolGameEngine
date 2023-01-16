module;
#include "LSEFramework.h"
export module D3D11.EngineWrapperD3D11;

import LSData;
import Engine.LSDevice;

export namespace LS
{
    [[nodiscard]]
    constexpr auto FromPixelColorFormat(PIXEL_COLOR_FORMAT format) -> DXGI_FORMAT
    {
        using enum PIXEL_COLOR_FORMAT;
        switch (format)
        {
        case UNKNOWN:
            throw std::runtime_error("Unknown pixel color format, cannot continue.\n");
        case ARGB8_UNKNOWN:
            throw std::runtime_error("Unknown format ARGB8_UNKNOWN\n");
        case ARGB8_SINT:
            return DXGI_FORMAT_R8G8B8A8_SINT;
        case ARGB8_UINT:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        case ARGB8_UNORM:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case ARGB8_SNORM:
            return DXGI_FORMAT_R8G8B8A8_SNORM;
        case ABGR8_UNKNOWN:
            return DXGI_FORMAT_B8G8R8A8_TYPELESS;
        case ABGR8_SINT:
            throw std::runtime_error("Unsupported format ABGR8_SINT\n");
        case ABGR8_UINT:
            throw std::runtime_error("Unsupported format ABGR8_UINT\n");
        case ABGR8_UNORM:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        case ABGR8_SNORM:
            throw std::runtime_error("Unsupported format ABGR8_SNORM\n");
        case RGBA8_UNKNOWN:
            return DXGI_FORMAT_R8G8B8A8_TYPELESS;
        case RGBA8_SINT:
            return DXGI_FORMAT_R8G8B8A8_SINT;
        case RGBA8_UINT:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        case RGBA8_UNORM:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case RGBA8_SNORM:
            return DXGI_FORMAT_R8G8B8A8_SNORM;
        case BGRA8_UNKNOWN:
            return DXGI_FORMAT_B8G8R8A8_TYPELESS;
        case BGRA8_SINT:
            throw std::runtime_error("Unsupoorted format BGRA8_SINT\n");
        case BGRA8_UINT:
            throw std::runtime_error("Unsupported format BGRA8_UINT\n");
        case BGRA8_UNORM:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        case BGRA8_SNORM:
            throw std::runtime_error("Unsupported format BGRA8_SNORM\n");
        case ARGB16_UNKNOWN:
            throw std::runtime_error("Unsupported format ARGB16_SNORM\n");
        case ARGB16_FLOAT:
            throw std::runtime_error("Unsupported format ARGB16_FLOAT\n");
        case ARGB16_SINT:
            throw std::runtime_error("Unsupported format ARGB16_SINT\n");
        case ARGB16_UINT:
            throw std::runtime_error("Unsupported format ARGB16_UINT\n");
        case ARGB16_UNORM:
            throw std::runtime_error("Unsupported format ARGB16_UNORM\n");
        case ARGB16_SNORM:
            throw std::runtime_error("Unsupported format ARGB16_UNORM\n");
        case ABGR16_UNKNOWN:
        case ABGR16_FLOAT:
        case ABGR16_SINT:
        case ABGR16_UINT:
        case ABGR16_UNORM:
        case ABGR16_SNORM:
            throw std::runtime_error("Unsupported format for ABGR16 types\n");
        case RGBA16_UNKNOWN:
            return DXGI_FORMAT_R16G16B16A16_TYPELESS;
        case RGBA16_FLOAT:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case RGBA16_SINT:
            return DXGI_FORMAT_R16G16B16A16_SINT;
        case RGBA16_UINT:
            return DXGI_FORMAT_R16G16B16A16_UINT;
        case RGBA16_UNORM:
            return DXGI_FORMAT_R16G16B16A16_UNORM;
        case RGBA16_SNORM:
            return DXGI_FORMAT_R16G16B16A16_SNORM;
        case BGRA16_UNKNOWN:
        case BGRA16_FLOAT:
        case BGRA16_SINT:
        case BGRA16_UINT:
        case BGRA16_UNORM:
        case BGRA16_SNORM:
            throw std::runtime_error("Unsupported format BGRA16 types\n");
        case ARGB32_UNKNOWN:
        case ARGB32_FLOAT:
        case ARGB32_SINT:
        case ARGB32_UINT:
        case ABGR32_UNKNOWN:
        case ABGR32_FLOAT:
        case ABGR32_SINT:
        case ABGR32_UINT:
            throw std::runtime_error("Unsupported format ABGR32 types\n");
        case RGBA32_UNKNOWN:
            return DXGI_FORMAT_R32G32B32A32_TYPELESS;
        case RGBA32_FLOAT:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case RGBA32_SINT:
            return DXGI_FORMAT_R32G32B32A32_SINT;
        case RGBA32_UINT:
            return DXGI_FORMAT_R32G32B32A32_UINT;
        case BGRA32_UNKNOWN:
        case BGRA32_FLOAT:
        case BGRA32_SINT:
        case BGRA32_UINT:
            throw std::runtime_error("Unsupported format BGRA32 typees\n");
        case DEPTH24_UNORM_STENCIL8_UINT:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case DEPTH32_FLOAT:
            return DXGI_FORMAT_D32_FLOAT;
        case DEPTH32_SFLOAT_STENCIL8_UINT:
            throw std::runtime_error("Unsupported format DEPTH32_SFLOAT_STENCIL8_UINT\n");
        case DEPTH32_STENCIL8X24_UINT:
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        default:
            throw std::runtime_error("Unknown PIXEL_COLOR_FORMAT passed, unable to find matching case.\n");
        }
    }

    [[nodiscard]]
    constexpr auto FindD3D11Usage(BUFFER_USAGE usage) -> D3D11_USAGE
    {
        using enum BUFFER_USAGE;
        switch (usage)
        {
        case DEFAULT_RW:
            return D3D11_USAGE_DEFAULT;
        case CONSTANT:
            throw std::runtime_error("Unable to support BUFFER_USAGE::CONSTANT\n");
        case IMMUTABLE:
            return D3D11_USAGE_IMMUTABLE;
        case DYNAMIC:
            return D3D11_USAGE_DYNAMIC;
        case COPY_ONLY:
            return D3D11_USAGE_STAGING;
        default:
            throw std::runtime_error("Unable to support unknown buffer usage\n");
        }
    }

    [[nodiscard]]
    constexpr auto FindD3D11BindFlag(BUFFER_BIND_TYPE type) -> D3D11_BIND_FLAG
    {
        using enum BUFFER_BIND_TYPE;
        switch (type)
        {
            case UNKNOWN:
                throw std::runtime_error("Unknown bind flag passed\n");
            case VERTEX:
                return D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
            case INDEX:
                return D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
            case CONSTANT_BUFFER:
                return D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
            case SHADER_RESOURCE:
                return D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
            case STREAM_OUTPUT:
                return D3D11_BIND_FLAG::D3D11_BIND_STREAM_OUTPUT;
            case RENDER_TARGET:
                return D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
            case DEPTH_STENCIL:
                return D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
            case UNORDERED_ACCESS:
                return D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS;
            case DECODER:
                return D3D11_BIND_FLAG::D3D11_BIND_DECODER;
            case VIDEO_ENCODER:
                return D3D11_BIND_FLAG::D3D11_BIND_VIDEO_ENCODER;
            default:
                throw std::runtime_error("Unable to find matching case for BUFFER BIND TYPE flag\n");
        }
    }

    [[nodiscard]]
    constexpr auto FindD3D11CpuAccessFlag(CPU_ACCESS_FLAG flag) -> uint32_t
    {
        using enum CPU_ACCESS_FLAG;
        uint32_t out = 0;
        switch (flag)
        {
        case NOT_SET:
            out = 0;
            break;
        case READ_ONLY:
            out = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_READ;
            break;
        case WRITE_ONLY:
            out = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
            break;
        case READ_AND_WRITE:
            out = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
            break;
        default:
            out = 0;
            break;
        }
        return out;
    }
}