module;
#include <cstdint>
#include <bitset>
#include <array>
#include <string>
#include <string_view>

export module Engine.EngineCodes;

constexpr size_t ERROR_MASK = 0x8000'0000;
export namespace LS
{
    /**
     * @brief Codes regarding engine level information at a top level overview.
     * Was this operation a success or failure. This isn't meant for detailed information.
     * That will be for an error message / exception system later. 
     * 
     * Representation of bytes is as follows: 
     * 0 byte - Error/Success code (1 for Error, 0 for success)
     * bytes 1-3 are not used and currently reserved
     * bytes 4-11 are the Category codes. This helps identify what these error codes may belong to. 
    */
    enum class ENGINE_CODE : uint32_t
    {
        LS_SUCCESS = 0x0,
        // General / Standard Errors //
        LS_ERROR                    = 0x8000'0000,
        LS_ERROR_INVALID_ARGUMENTS  = 0x8000'0002,
        LS_ERROR_PATH_NOT_FOUND     = 0x8000'0003,
        LS_ERROR_FILE_NOT_FOUND     = 0x8000'0004,
        // GPU / Hardware //
        GPU_SUCCESS                 = 0x0100'0000,
        GPU_FAILURE                 = 0x8100'0000,
        // Windowing //
        WINDOW_CREATION_SUCCESS     = 0x0200'0000,
        WINDOW_CREATION_FAIL        = 0x8200'0000,
        // File / Disk // 
        FILE_SUCCESS                = 0x0300'0000,
        FILE_ERROR                  = 0x8300'0000,
        // Memory //
        HEAP_ALLOC_SUCCESS          = 0x0400'0000,
        HEAP_ALLOC_FAILED           = 0x8400'0000,
        // Controller / Input Devices //
        INPUT_DEVICE_SUCCESS        = 0x0500'0000,
        INPUT_DEVICE_ERROR          = 0x8500'0000,
        // OS Level Error //
        OS_SUCCESS                  = 0x0600'0000,
        OS_ERROR                    = 0x8600'0000,
        // Network //
        NETWORK_SUCCESS             = 0x0700'0000,
        NETWORK_ERROR               = 0x8700'0000,
        // Input/Output Stream Issues //
        IO_SUCCESS                  = 0x0800'0000,
        IO_FAIL                     = 0x8800'0000
    };

    constexpr bool IsSuccessCode(ENGINE_CODE code)
    {
        return ((uint32_t)code & ERROR_MASK) == 0;
    }

    namespace System
    {
        enum class ErrorCategory : uint16_t
        {
            GENERAL = 0x0, //@brief A basic error code meant for any general category types
            GPU = 0x0100,//@brief Graphics Processing Unit (Device) issues
            WINDOW = 0x0200,//@brief Window/viewer errors 
            FILE = 0x0300, //@brief The file system 
            MEMORY = 0x0400,//@brief Memory on the system
            INPUT = 0x0500, //@brief An error with the input device (controller/keyboard) occurred. 
            OS = 0x0600, //@brief An operating system specific error
            NETWORK = 0x0700,//@brief A network error occurred
            IO = 0x0800, //@brief Input/Output stream category
        };

        constexpr auto FindCategory(ENGINE_CODE code) -> ErrorCategory
        {
            uint16_t val = ((uint32_t)code >> 16) & 0x0FFF;
            using enum ErrorCategory;
            switch (val)
            {
            case (size_t)GENERAL:
                return GENERAL;
            case (size_t)GPU:
                return GPU;
            case (size_t)WINDOW:
                return WINDOW;
            case (size_t)FILE:
                return FILE;
            case (size_t)MEMORY:
                return MEMORY;
            case (size_t)INPUT:
                return INPUT;
            case (size_t)OS:
                return OS;
            case (size_t)NETWORK:
                return NETWORK;
            case (size_t)IO:
                return IO;
            default:
                throw std::exception("Unknown ENGINE_CODE supplied.");
            }
        }
    }

    export namespace System
    {
        class ErrorCode
        {
        protected:
            std::string ErrorMsg;
            ENGINE_CODE Code = ENGINE_CODE::LS_SUCCESS;
            ErrorCategory ErrCategory = ErrorCategory::GENERAL;

        public:

            constexpr ErrorCode(ENGINE_CODE code, std::string_view msg) noexcept
                : ErrCategory(FindCategory(code)),
                Code(code),
                ErrorMsg(msg)
            {}

            constexpr ErrorCode(std::string_view msg, ENGINE_CODE code = ENGINE_CODE::LS_ERROR) noexcept
                : ErrCategory(FindCategory(code)),
                Code(code),
                ErrorMsg(msg)
            {
            }

            [[nodiscard]] ErrorCode& operator=(const ErrorCode&) noexcept = default;
            [[nodiscard]] ErrorCode& operator=(ErrorCode&&) noexcept = default;
            [[nodiscard]] ErrorCode(const ErrorCode&) noexcept = default;
            [[nodiscard]] ErrorCode(ErrorCode&&) noexcept = default;
            
            auto Message() const noexcept -> const std::string_view
            {
                return ErrorMsg;
            }

            constexpr operator bool() const
            {
                return IsSuccessCode(Code);
            }

            constexpr bool IsError() const
            {
                return !IsSuccessCode(Code);
            }
        };

        constexpr auto CreateFailCode(std::string_view message, ENGINE_CODE code = ENGINE_CODE::LS_ERROR) noexcept -> ErrorCode
        {
            return ErrorCode(code, message);
        }
        
        constexpr auto CreateFailCode() noexcept -> ErrorCode
        {
            return CreateFailCode("");
        }

        constexpr auto CreateSuccessCode(std::string_view message, ENGINE_CODE code = ENGINE_CODE::LS_SUCCESS) noexcept -> ErrorCode
        {
            return ErrorCode(code, message);
        }
        
        constexpr auto CreateSuccessCode() noexcept -> ErrorCode
        {
            return CreateSuccessCode("");
        }

    }// end namespace System
}// end namespace LS