module;
#include <ostream>
#include <fstream>
#include <iostream>
#include <chrono>
#include <shared_mutex>
#include <string_view>
#include <string>
#include <format>
#include <memory>
#include <filesystem>

export module Engine.Logger;

import Data.LSDataTypes;

export namespace LS::Log
{
#define LOGGER_CHECK if (!Logger) return
#define LEVEL_CHECK(x) if (x <= LoggingLevel) return

    enum class LOG_LEVEL
    {
        DEBUG = 0,//@brief Messages that will only populate in DEBUG
        INFO, //@brief Messages that will populate outside debug mode
        WARN, //@brief Messages that should be looked at
        ERROR //@brief Messages that contain critical errorrs
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
    //TODO: This is fun to consider, but maybe I'll go with a logging library for now.
    //Mainly, threading and performance could be an issue in actually evaluating the effectiveness
    // of the engine in other areas. Plus, I'm not sure how I want to handle things, do I want
    // to just allow any stream type be used in debug/release? Letting users choose makes sense afterall.
    // and how do I want to handle that? 
    class LSLogger
    {
    private:
        mutable std::shared_mutex LogMutex;
    public:
        LSLogger() = default;

        ~LSLogger() = default;

        void Print(std::wostream& stream, LOG_LEVEL level, std::wstring_view msg) const noexcept;
        void PrintLine(std::wostream& stream, LOG_LEVEL level, std::wstring_view msg) const noexcept;
    };

    Ref<LSLogger> Logger;
    LOG_LEVEL LoggingLevel = LOG_LEVEL::DEBUG;
    std::wofstream LogFile;

    void TraceError(std::wstring_view msg);
    void TraceDebug(std::wstring_view msg);
    void TraceInfo(std::wstring_view msg);
    void TraceWarning(std::wstring_view msg);

    /**
     * @brief Initialize a log to the standard output 
    */
    void InitLog();

    /**
     * @brief Initialize a log to a file instead
     * @param filepath The file to use or create when logging
    */
    void InitLog(std::filesystem::path filepath);

    /**
     * @brief Sets the log level to show messages of a certain level and higher only
     * @param level The MINIMUM level to show only
    */
    void SetLogLevel(LOG_LEVEL level);
}

module : private;

void LS::Log::LSLogger::Print(std::wostream& stream, LOG_LEVEL level, std::wstring_view msg) const noexcept
{
    std::unique_lock lock(LogMutex);
    const auto time = std::chrono::system_clock::now();
    const auto fmtTime = std::chrono::current_zone()->to_local(time);
    const auto fmt = std::format(L"{} : [{}] || {}", fmtTime, ErrorAsWChar(level), msg);
    stream << fmt;
}

void LS::Log::LSLogger::PrintLine(std::wostream& stream, LOG_LEVEL level, std::wstring_view msg) const noexcept
{
    std::unique_lock lock(LogMutex);
    const auto time = std::chrono::system_clock::now();
    const auto fmtTime = std::chrono::current_zone()->to_local(time);
    const auto fmt = std::format(L"{} : [{}] || {}\n", fmtTime, ErrorAsWChar(level), msg);
    stream << fmt;
}

void LS::Log::TraceError([[maybe_unused]] std::wstring_view msg)
{
    using enum LOG_LEVEL;
    LOGGER_CHECK;
    LEVEL_CHECK(ERROR);
#if _DEBUG
        Logger->PrintLine(std::wcerr, ERROR, msg);
#else
        Logger->PrintLine(LogFile, ERROR, msg);
#endif
}

void LS::Log::TraceDebug([[maybe_unused]] std::wstring_view msg)
{
    using enum LOG_LEVEL;
    LOGGER_CHECK;
    LEVEL_CHECK(DEBUG);
#if _DEBUG
    Logger->PrintLine(std::wcout, DEBUG, msg);
#else
    Logger->PrintLine(LogFile, DEBUG, msg);
#endif
}

void LS::Log::TraceInfo([[maybe_unused]] std::wstring_view msg)
{
    using enum LOG_LEVEL;
    LOGGER_CHECK;
    LEVEL_CHECK(INFO);
#if _DEBUG
        Logger->PrintLine(std::wcout, INFO, msg);
#else
        Logger->PrintLine(LogFile, INFO, msg);
#endif
}

void LS::Log::TraceWarning([[maybe_unused]] std::wstring_view msg)
{
    using enum LOG_LEVEL;
    LOGGER_CHECK; 
    LEVEL_CHECK(WARN);
#if _DEBUG
        Logger->PrintLine(std::wcout, WARN, msg);
#else
        Logger->PrintLine(LogFile, WARN, msg);
#endif
}
void LS::Log::InitLog()
{
    Logger = std::make_unique<LSLogger>();
}

void LS::Log::InitLog(std::filesystem::path filepath)
{
    Logger = std::make_unique<LSLogger>();
    LogFile.open(filepath, std::ios::binary);
    if (!LogFile.is_open())
        throw std::runtime_error(std::format("Failed to init log file: {}", filepath.string()));
}
