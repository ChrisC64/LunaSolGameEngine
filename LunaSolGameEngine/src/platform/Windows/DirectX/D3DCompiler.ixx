module;
#include <d3dcompiler.h>
#include <wrl/client.h>

export module DirectXCommon.D3DCompiler;
import <filesystem>;
import <cstdint>;
import <cstddef>;
import <fmt/format.h>;
import <vector>;

export namespace LS::DX
{
    struct CompileResult
    {
        std::vector<std::byte> ByteCode;
        std::string ErrorMsg;

        bool HasError() const
        {
            return !ErrorMsg.empty();
        }
    };

    auto CompileShader(std::filesystem::path path, const char* entryPoint, const char* target, uint32_t compileFlags = 0) -> CompileResult&&
    {
        CompileResult result;
        if (!std::filesystem::exists(path))
        {
            result.ErrorMsg = std::format("File not found: {}", path.string());
            return std::move(result);
        }

        Microsoft::WRL::ComPtr<ID3DBlob> shader;
        Microsoft::WRL::ComPtr<ID3DBlob> error;
        D3DCompileFromFile(path.c_str(), nullptr, nullptr, entryPoint, target, compileFlags, 0, &shader, &error);

        if (error)
        {
            std::string msg(reinterpret_cast<const char*>(error->GetBufferPointer()), error->GetBufferSize());
            result.ErrorMsg = std::move(msg);
            return std::move(result);
        }

        std::byte* begin = reinterpret_cast<std::byte*>(shader->GetBufferPointer());
        std::copy(begin, begin + shader->GetBufferSize(), std::back_inserter(result.ByteCode));
        return std::move(result);
    }
}