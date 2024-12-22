module;
#include <fstream>
#include "engine/EngineLogDefines.h"

#ifdef LS_WIN32_BUILD
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

export module Helper.IO;
import <array>;
import <cstddef>;
import <filesystem>;
import <vector>;
import <optional>;
import <format>;
import <string_view>;

import Engine.EngineCodes;
import Engine.Defines;

export namespace LS::IO
{
    /**
     * @brief Flags for describing how a file should be opened
    */
    enum class FileFlags : uint16_t
    {
        Append = 0x1,//@brief add to the end of the file
        Binary = 0x2,//@brief open as a binary file
        Read = 0x4,//@brief Open for reading 
        Write = 0x8,//@brief Open for writing
        AtTheEnd = 0x10,//@brief Open with the position at the end of the file
        ClearContent = 0x20//@brief Clear contents of an existing file
    };

    auto GetParentPath() -> std::filesystem::path
    {
#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
        std::array<wchar_t, _MAX_PATH> modulePath{};
        if (!GetModuleFileName(nullptr, modulePath.data(), static_cast<DWORD>(modulePath.size())))
        {
            LS_LOG_ERROR(std::format(L"Module file not found, unable to find root directory"));
            return {};
        }
#endif
        std::filesystem::path modulefile(modulePath.begin(), modulePath.end());
        return modulefile.parent_path();
    }

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
    
    auto ReadFileSome(std::filesystem::path path, size_t len, size_t offset = 0u) -> Nullable<std::vector<std::byte>>
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
        if (offset + len > fileSize)
        {
            LS_LOG_ERROR(std::format(L"Unable to read from file. Offest ({}) + Length ({}) = {} is greater than file size: {}", offset, len, (offset + len), fileSize));
            return std::nullopt;
        }

        stream.seekg(offset);
        std::vector<std::byte> shaderData(len);
        stream.read(reinterpret_cast<char*>(shaderData.data()), len);
        stream.close();

        return shaderData;
    }
}