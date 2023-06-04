module;
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <string_view>
#include <filesystem>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <span>

#include <d3dcompiler.h>
#include <windows.h>
#include <d3d11_4.h>
export module D3D11.Utils;

import D3D11.EngineWrapperD3D11;
import LSData;
import Util.HLSLUtils;
import Engine.LSDevice;

export namespace LS::Win32
{
    /**
     * @brief Compiles a D3D HLSL shader from the file pointed.
     * @param filepath The file's location
     * @param entryPoint The name of the shader's entry point
     * @param targetProfile The target profile to build for
     * @param compilationFlags Any D3DCOMPILE_ compilation flags 
     * @param ppData [out] contains the compiled source data
     * @param ppErrorMsg [out] contains an error message if the compilation fails
     * @return If the file is not found, HRESULT = ERROR_FILE_NOT_FOUND (2L) and if the compilation fails,
     * an error will be returned along with a message in the ppErrorMsg object. If successful, ppData will be initialized
     * with the data of the compiled source file.
    */
    [[nodiscard]]
    inline HRESULT CompileShaderFile(std::filesystem::path filepath, std::string_view entryPoint, std::string_view targetProfile,
        UINT compilationFlags, ID3DBlob** ppData, ID3DBlob** ppErrorMsg)
    {
        if (!std::filesystem::exists(filepath))
        {
            return ERROR_FILE_NOT_FOUND;
        }
        return D3DCompileFromFile(filepath.wstring().c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, 
            entryPoint.data(), targetProfile.data(), compilationFlags, 0, ppData, ppErrorMsg);
    }

    /**
     * @brief Compiles a HLSL file from source data 
     * @param sourceData The source data 
     * @param entryPoint The entry point to the shader's main function
     * @param targetProfile The target profile to compile the shader as
     * @param compilationFlags A bitwise OR flag of D3DCOMPILE_ settings
     * @param ppData The compiled source data if successful otherwise it will be nullptr
     * @param ppErrorMsg An error messsage detailing why the compilation failed, nullptr if compilation successful
     * @return Returns S_OK if successful, otherwise an error code will be returned with a message of why the compilation failed.
     * If the vector is empty, no message will be set. 
    */
    [[nodiscard]]
    constexpr HRESULT CompileShaderFromData(std::vector<byte> sourceData, std::string_view entryPoint, std::string_view targetProfile,
        UINT compilationFlags, ID3DBlob** ppData, ID3DBlob** ppErrorMsg)
    {
        if (sourceData.empty())
        {
            return ERROR_EMPTY;
        }

        return D3DCompile(sourceData.data(), sourceData.size(), nullptr, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entryPoint.data(), targetProfile.data(), compilationFlags, 0, ppData, ppErrorMsg);
    }

    [[nodiscard]]
    constexpr HRESULT CompileVertexShaderFromByteCode(ID3D11Device* pDevice, std::span<std::byte> byteCode, ID3D11VertexShader** ppShader)
    {
        return pDevice->CreateVertexShader(byteCode.data(), byteCode.size(), nullptr, ppShader);
    }
    
    [[nodiscard]]
    constexpr HRESULT CompilePixelShaderFromByteCode(ID3D11Device* pDevice, std::span<std::byte> byteCode, ID3D11PixelShader** ppShader)
    {
        return pDevice->CreatePixelShader(byteCode.data(), byteCode.size(), nullptr, ppShader);
    }

    [[nodiscard]]
    inline auto CreateTexture2DDesc(const LSTextureInfo& texture,
        BUFFER_USAGE usage = BUFFER_USAGE::DEFAULT_RW, BUFFER_BIND_TYPE bindType = BUFFER_BIND_TYPE::SHADER_RESOURCE, CPU_ACCESS_FLAG cpuAccess = CPU_ACCESS_FLAG::NOT_SET, 
        uint32_t miscFlags = 0) -> D3D11_TEXTURE2D_DESC
    {
        D3D11_TEXTURE2D_DESC out{};
        out.Width = texture.Width;
        out.Height = texture.Height;
        out.MipLevels = texture.MipMapLevels;
        //Array size maps to number of mip maps because each mip map is the texture scaled 
        out.ArraySize = texture.MipMapLevels;
        out.Format = FromPixelColorFormat(texture.PixelFormat);
        out.SampleDesc = { .Count = texture.SampleCount, .Quality = texture.SampleQuality };
        out.Usage = FindD3D11Usage(usage);
        out.BindFlags = FindD3D11BindFlag(bindType);
        out.CPUAccessFlags = FindD3D11CpuAccessFlag(cpuAccess);
        out.MiscFlags = miscFlags;
        return out;
    }
}