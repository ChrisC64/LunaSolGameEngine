module;
#include <cstdint>
#include <bitset>
#include <array>
#include <string>
#include <string_view>
export module Engine.EngineCodes;

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
        UNKNOWN_ERROR = 0x0,
        LS_SUCCESS = 0x1000'0000,
        LS_ERROR = 0x1000'0001,
        LS_ERROR_INVALID_ARGUMENTS = 0x1000'0002,
        LS_ERROR_PATH_NOT_FOUND = 0x1000'0003,
        LS_ERROR_FILE_NOT_FOUND = 0x1000'0004,
        DEVICE_CREATION_FAILURE = 0x1000'0011,
        DEVICE_CREATION_SUCCESS = 0x1000'0010,
        WINDOW_CREATION_FAIL = 0x1000'0021,
        WINDOW_CREATION_SUCCESS = 0x1000'0020,
        RESOURCE_CREATION_FAILED = 0x1000'0031,
        RESOURCE_CREATION_SUCCESS = 0x1000'0030,
        FILE_ERROR = 0x1000'0041,
        FILE_SUCCESS = 0x1000'0040,
        HEAP_ALLOC_FAILED = 0x1000'0051,
        HEAP_ALLOC_SUCCESS = 0x1000'0050,
        INVALID_ARGUMENTS = 0x1000'00F1,
        NUMBER_OF_CODES,
    };

    bool IsSuccessCode(ENGINE_CODE code)
    {
        return !std::bitset<32>((uint32_t)(code)).test(0);
    }

    export namespace System
    {
        enum class ErrorCategory
        {
            GENERAL = 0x0, //@brief A basic error code meant for any general category types
            GPU,//@brief Graphics Processing Unit (Device) issues
            WINDOW,//@brief Window/viewer errors 
            FILE, //@brief The file system 
            MEMORY,//@brief Memory on the system
            IO, //@brief Input/Output stream category
            OS, //@brief An operating system specific error
            NETWORK,//@brief A network error occurred
        };

        enum class ErrorStatus : uint8_t
        {
            SUCCESS = 0,
            ERROR
        };

        class ErrorCode
        {
        protected:
            const std::string ErrorMsg;
            ErrorStatus ErrStatus = ErrorStatus::ERROR;
            ErrorCategory ErrCategory = ErrorCategory::GENERAL;

        public:

            ErrorCode(LS::System::ErrorStatus status, LS::System::ErrorCategory category, std::string_view msg) : ErrStatus(status),
                ErrCategory(category),
                ErrorMsg(msg)
            {}
            
            /*explicit ErrorCode(const ErrorCode* sec) : ErrStatus(sec->ErrStatus),
                ErrCategory(sec->ErrCategory),
                ErrorMsg(sec->ErrorMsg)
            {}*/

            auto Message() const noexcept -> const std::string_view
            {
                return ErrorMsg.data();
            }

            auto Status() const noexcept -> ErrorStatus
            {
                return ErrStatus;
            }

            operator bool() const
            {
                return ErrStatus == ErrorStatus::SUCCESS;
            }
        };

        class SuccessErrorCode : public ErrorCode
        {
        public:
            SuccessErrorCode(ErrorCategory category = ErrorCategory::GENERAL, std::string_view msg = "") :
                ErrorCode(ErrorStatus::SUCCESS, category, msg)
            {}
        };

        class FailErrorCode : public ErrorCode
        {
        public:
            FailErrorCode(ErrorCategory category = ErrorCategory::GENERAL, std::string_view msg = "") :
                ErrorCode(ErrorStatus::ERROR, category, msg)
            {}
        };
    }// end namespace System
}// end namespace LS