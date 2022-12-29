module;
#include "LSEFramework.h"
#include "LSBuffer.h"

export module D3D11.Utils;
import LSData;
import Util.HLSLUtils;

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
    inline HRESULT CompileShaderFromData(std::vector<byte> sourceData, std::string_view entryPoint, std::string_view targetProfile,
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
    inline HRESULT CompileVertexShaderFromByteCode(ID3D11Device* pDevice, std::span<std::byte> byteCode, ID3D11VertexShader** ppShader)
    {
        return pDevice->CreateVertexShader(byteCode.data(), byteCode.size(), nullptr, ppShader);
    }
    
    [[nodiscard]]
    inline HRESULT CompilePixelShaderFromByteCode(ID3D11Device* pDevice, std::span<std::byte> byteCode, ID3D11PixelShader** ppShader)
    {
        return pDevice->CreatePixelShader(byteCode.data(), byteCode.size(), nullptr, ppShader);
    }

    /*[[nodiscard]]
    inline D3D11_USAGE FindUsageFromLSBufferUsage(LS::BUFFER_USAGE bufferUsage)
    {
        using enum LS::BUFFER_USAGE;

        switch (bufferUsage)
        {
        case DEFAULT_RW: return D3D11_USAGE::D3D11_USAGE_DEFAULT;
        case CONSTANT: return D3D11_USAGE::D3D11_USAGE_DEFAULT;
        case IMMUTABLE: return D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
        case DYNAMIC: return D3D11_USAGE::D3D11_USAGE_DYNAMIC;
        case COPY_ONLY: return D3D11_USAGE::D3D11_USAGE_STAGING;
        default:
            throw std::runtime_error("Failed to find a suitable buffer usage type");
        }
    }*/

    /*std::vector<D3D11_INPUT_ELEMENT_DESC> BuildShaderInput(const LSShaderInputSignature& inputSignature)
    {
        std::vector<D3D11_INPUT_ELEMENT_DESC> elements;

        auto& layout = inputSignature.GetInputLayout();

        D3D11_INPUT_ELEMENT_DESC desc{};
        for (auto& l : layout)
        {
            desc.SemanticName = l.SemanticName.c_str();
            desc.SemanticIndex = l.SemanticIndex;
            desc.InputSlot = l.InputSlot;
            desc.AlignedByteOffset = l.OffsetAligned;
            desc.InputSlotClass = l.InputClass == INPUT_CLASS::VERTEX ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA;
            desc.InstanceDataStepRate = l.InstanceStepRate;
            desc.Format = Utils::FindDXGIFormat(l);
            elements.emplace_back(desc);
        }

        return elements;
    }*/
}