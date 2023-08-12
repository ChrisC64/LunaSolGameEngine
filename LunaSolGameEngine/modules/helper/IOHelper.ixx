module;
#include <cstddef>
#include <fstream>
#include <filesystem>
#include <vector>
#include <optional>
#include <format>
#include "engine/EngineLogDefines.h"
export module Helper.IO;

import Engine.EngineCodes;
import Data.LSDataTypes;

export namespace LS::IO
{
    /**
     * @brief Flags for describing how a file should be opened
    */
    enum class FileOpenFlags : uint16_t
    {
        Append = 0x1,//@brief add to the end of the file
        Binary = 0x2,//@brief open as a binary file
        Read = 0x4,//@brief Open for reading 
        Write = 0x8,//@brief Open for writing
        AtTheEnd = 0x10,//@brief Open with the position at the end of the file
        ClearContent = 0x20//@brief Clear contents of an existing file
    };

    auto ReadFile(std::filesystem::path path) -> Nullable<std::vector<std::byte>>
    {
        if (!std::filesystem::exists(path))
        {
            LS_LOG_ERROR(std::format(L"File NOT found: {}", path.wstring()));
            return std::nullopt;
        }

        std::fstream stream{ path, std::fstream::in | std::fstream::binary };
        if (!stream.is_open() && !stream.good())
        {
            LS_LOG_ERROR(std::format(L"File state not good, cannot read. File: {}", path.wstring()));
            return std::nullopt;
        }
        auto fileSize = std::filesystem::file_size(path);
        std::vector<std::byte> shaderData(fileSize);
        stream.read(reinterpret_cast<char*>(shaderData.data()), fileSize);
        stream.close();

        return shaderData;
    }
}