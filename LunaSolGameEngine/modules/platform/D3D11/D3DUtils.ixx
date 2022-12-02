module;
#include "LSEFramework.h"
export module D3D11.Utils;

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
    inline HRESULT CompileShaderFile(std::filesystem::path filepath, std::string_view entryPoint, std::string_view targetProfile,
        uint64_t compilationFlags, ID3DBlob** ppData, ID3DBlob** ppErrorMsg)
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
    inline HRESULT CompileShaderFromData(std::vector<byte> sourceData, std::string_view entryPoint, std::string_view targetProfile,
        uint64_t compilationFlags, ID3DBlob** ppData, ID3DBlob** ppErrorMsg)
    {
        if (sourceData.empty())
        {
            return ERROR_EMPTY;
        }

        return D3DCompile(sourceData.data(), sourceData.size(), nullptr, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entryPoint.data(), targetProfile.data(), compilationFlags, 0, ppData, ppErrorMsg);
    }
}