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