module;
#include <fstream>
#include <iostream>
#include <chrono>
#include <shared_mutex>
#include <string_view>
#include <string>
#include <format>
#include <memory>
#include <filesystem>
#include <utility>
#include "engine/EngineDefines.h"

export module Engine.Logger;

import Engine.EngineCodes;
import LSEDataLib;

export namespace LS::Log
{
#define LOGGER_CHECK if (!Logger) return
#define LEVEL_CHECK(x) if (x > Logger->GetLogLevel()) return

    enum class LOG_LEVEL : uint8_t
    {
        DEBUG = 0,//@brief Messages that are for debug use
        INFO, //@brief Messages that are informative but maybe not meant for debug usage
        WARN, //@brief Messages that should be looked at
        ERROR, //@brief Messages that contain critical errorrs
        ALL //@brief print out all messages
    };

    constexpr const char* ErrorAsChar(LOG_LEVEL level)
    {
        using enum LOG_LEVEL;
        switch (level)
        {
        case INFO:
            return "INFO";
        case DEBUG:
            return "DEBUG";
        case WARN:
            return "WARN";
        case ERROR:
            return "ERROR";
        default:
            return "UNKNOWN_LOG_LEVEL";
        }
    }

    constexpr const std::wstring ErrorAsWChar(LOG_LEVEL level)
    {
        using enum LOG_LEVEL;
        switch (level)
        {
        case INFO:
            return L"INFO";
        case DEBUG:
            return L"DEBUG";
        case WARN:
            return L"WARN";
        case ERROR:
            return L"ERROR";
        default:
            return L"UNKNOWN_LOG_LEVEL";
        }
    }

    class LSLogger
    {
    public:
        LSLogger();
        virtual ~LSLogger();

        virtual auto Init() noexcept -> LS::System::ErrorCode = 0;

        void Print(std::wstring_view msg) noexcept;
        void Print(std::string_view msg) noexcept;
        void Print(LOG_LEVEL level, std::wstring_view msg) noexcept;
        void Print(LOG_LEVEL level, std::string_view msg) noexcept;
        
        void PrintLine(std::wstring_view msg) noexcept;
        void PrintLine(std::string_view msg) noexcept;
        void PrintLine(LOG_LEVEL level, std::wstring_view msg) noexcept;
        void PrintLine(LOG_LEVEL level, std::string_view msg) noexcept;

        void SetLogLevel(LOG_LEVEL level) noexcept;
        auto GetLogLevel() noexcept -> LOG_LEVEL;
        void Flush() noexcept;

    protected:
        LOG_LEVEL m_logLevel = LOG_LEVEL::DEBUG;
        std::wostream m_stream;
    };

    class FileLogger : public LSLogger
    {
    public:
        FileLogger(std::filesystem::path file);
        auto Init() noexcept -> LS::System::ErrorCode;

    private:
        std::wofstream m_fileStream;
        std::filesystem::path m_path;
    };

    class ConsoleLogger : public LSLogger
    {
    public:
        ConsoleLogger();
        auto Init() noexcept -> LS::System::ErrorCode;
    };

    Ref<LSLogger> Logger;

    void TraceError(std::wstring_view msg);
    void TraceDebug(std::wstring_view msg);
    void TraceInfo(std::wstring_view msg);
    void TraceWarn(std::wstring_view msg);
    
    void TraceError(std::string_view msg);
    void TraceDebug(std::string_view msg);
    void TraceInfo(std::string_view msg);
    void TraceWarn(std::string_view msg);

    /**
     * @brief Initialize a log to the standard output or cerr based on LS::Log::LOG_LEVEL. 
     * Anything less than LS::Log::LOG_LEVEL::ERROR will print to the std::cout.
    */
    [[nodiscard]] auto InitLog(LOG_LEVEL level = LOG_LEVEL::DEBUG) noexcept -> LS::System::ErrorCode;

    /**
     * @brief Initialize a log to a file instead
     * @param filepath The file to use or create when logging
    */
    [[nodiscard]] auto InitLog(std::filesystem::path filepath, LOG_LEVEL level = LOG_LEVEL::DEBUG) noexcept -> LS::System::ErrorCode;

    void Flush() noexcept;
}

module : private;

LS::Log::LSLogger::LSLogger() : m_stream{ nullptr }
{
}

LS::Log::LSLogger::~LSLogger()
{
}

void LS::Log::LSLogger::Print(std::wstring_view msg) noexcept
{
    Print(m_logLevel, msg);
}

void LS::Log::LSLogger::Print(std::string_view msg) noexcept
{
}

void LS::Log::LSLogger::Print(LOG_LEVEL level, std::wstring_view msg) noexcept
{
    const auto time = std::chrono::system_clock::now();
    const auto fmtTime = std::chrono::current_zone()->to_local(time);
    const auto fmt = std::format(L"{} : [{}] || {}", fmtTime, ErrorAsWChar(level), msg);
    m_stream << fmt;
}

void LS::Log::LSLogger::Print([[maybe_unused]] LOG_LEVEL level, [[maybe_unused]] std::string_view msg) noexcept
{
}

void LS::Log::LSLogger::PrintLine(std::wstring_view msg) noexcept
{
    PrintLine(m_logLevel, msg);
}

void LS::Log::LSLogger::PrintLine([[maybe_unused]] std::string_view msg) noexcept
{
}

void LS::Log::LSLogger::PrintLine(LOG_LEVEL level, std::wstring_view msg) noexcept
{
    const auto time = std::chrono::system_clock::now();
    const auto fmtTime = std::chrono::current_zone()->to_local(time);
    const auto fmt = std::format(L"{} : [{}] || {}\n", fmtTime, ErrorAsWChar(level), msg);
    m_stream << fmt;
}

void LS::Log::LSLogger::PrintLine([[maybe_unused]] LOG_LEVEL level, [[maybe_unused]] std::string_view msg) noexcept
{
}

void LS::Log::LSLogger::SetLogLevel(LOG_LEVEL level) noexcept
{
    m_logLevel = level;
}

auto LS::Log::LSLogger::GetLogLevel() noexcept -> LOG_LEVEL
{
    return m_logLevel;
}

void LS::Log::LSLogger::Flush() noexcept
{
    if (!m_stream)
        return;
    m_stream.flush();
}

void LS::Log::TraceError([[maybe_unused]] std::wstring_view msg)
{
    using enum LOG_LEVEL;
    LOGGER_CHECK;
    LEVEL_CHECK(ERROR);
    Logger->PrintLine(ERROR, msg);
}

void LS::Log::TraceDebug([[maybe_unused]] std::wstring_view msg)
{
    using enum LOG_LEVEL;
    LOGGER_CHECK;
    LEVEL_CHECK(DEBUG);
    Logger->PrintLine(DEBUG, msg);
}

void LS::Log::TraceInfo([[maybe_unused]] std::wstring_view msg)
{
    using enum LOG_LEVEL;
    LOGGER_CHECK;
    LEVEL_CHECK(INFO);
    Logger->PrintLine(INFO, msg);
}

void LS::Log::TraceWarn([[maybe_unused]] std::wstring_view msg)
{
    using enum LOG_LEVEL;
    LOGGER_CHECK;
    LEVEL_CHECK(WARN);
    Logger->PrintLine(WARN, msg);
}

void LS::Log::TraceError([[maybe_unused]] std::string_view msg)
{
    using enum LOG_LEVEL;
    LOGGER_CHECK;
    LEVEL_CHECK(ERROR);
    Logger->PrintLine(ERROR, msg);
}

void LS::Log::TraceDebug([[maybe_unused]] std::string_view msg)
{
    using enum LOG_LEVEL;
    LOGGER_CHECK;
    LEVEL_CHECK(DEBUG);
    Logger->PrintLine(DEBUG, msg);
}

void LS::Log::TraceInfo([[maybe_unused]] std::string_view msg)
{
    using enum LOG_LEVEL;
    LOGGER_CHECK;
    LEVEL_CHECK(INFO);
    Logger->PrintLine(INFO, msg);
}

void LS::Log::TraceWarn([[maybe_unused]] std::string_view msg)
{
    using enum LOG_LEVEL;
    LOGGER_CHECK;
    LEVEL_CHECK(WARN);
    Logger->PrintLine(WARN, msg);
}

auto LS::Log::InitLog(LOG_LEVEL level /*= LOG_LEVEL::DEBUG*/) noexcept -> LS::System::ErrorCode
{
    Logger = std::make_unique<ConsoleLogger>();
    Logger->SetLogLevel(level);
    return Logger->Init();
}

auto LS::Log::InitLog(std::filesystem::path filepath, LOG_LEVEL level /*= LOG_LEVEL::DEBUG*/) noexcept -> LS::System::ErrorCode
{
    Logger = std::make_unique<FileLogger>(filepath);
    Logger->SetLogLevel(level);
    return Logger->Init();
}

void LS::Log::Flush() noexcept
{
    LOGGER_CHECK;
    Logger->Flush();
}

LS::Log::FileLogger::FileLogger(std::filesystem::path file)
{
    if (!std::filesystem::exists(file.relative_path()))
    {
        std::filesystem::create_directories(file.relative_path());
    }

    m_fileStream.open(file, std::ios::out | std::ios::binary);
    if (!m_fileStream.is_open())
    {
        throw std::runtime_error("Failed to open file for logger.");
    }
    m_stream.rdbuf(m_fileStream.rdbuf());
}

auto LS::Log::FileLogger::Init() noexcept -> LS::System::ErrorCode
{
    if (!m_stream)
    {
        return LS::System::CreateFailCode("The stream is not initialized!", LS::System::ErrorCategory::IO);
    }
    return LS::System::CreateSuccessCode();
}

LS::Log::ConsoleLogger::ConsoleLogger()
{
    m_stream.rdbuf(std::wcout.rdbuf());
}

auto LS::Log::ConsoleLogger::Init() noexcept -> LS::System::ErrorCode
{
    if (!m_stream)
    {
        return LS::System::CreateFailCode("The stream is not initialized!", LS::System::ErrorCategory::IO);
    }
    return LS::System::CreateSuccessCode();
}