module;
#include <cstdint>
#include <bitset>
export module Engine.EngineCodes;

export namespace LS
{
    /**
     * @brief Codes regarding engine level information at a top level overview.
     * Was this operation a success or failure. This isn't meant for detailed information.
     * That will be for an error message / exception system later. 
    */
    enum class ENGINE_CODE : uint64_t
    {
        UNKNOWN_ERROR = 0x0,
        LS_SUCCESS = 0x100000000,
        LS_ERROR = 0x100000001,
        DEVICE_CREATION_FAILURE = 0x100000011,
        DEVICE_CREATION_SUCCESS = 0x100000010,
        WINDOW_CREATION_FAIL = 0x100000021,
        WINDOW_CREATION_SUCCESS = 0x100000020,
        RESOURCE_CREATION_FAILED = 0x100000031,
        RESOURCE_CREATION_SUCCESS = 0x100000030,
        FILE_ERROR = 0x100000041,
        FILE_SUCCESS = 0x100000040,
        HEAP_ALLOC_FAILED = 0x100000051,
        HEAP_ALLOC_SUCCESS = 0x100000050,
        INVALID_ARGUMENTS = 0x1000000F1,
        NUMBER_OF_CODES,
    };

    bool IsSuccessCode(ENGINE_CODE code)
    {
        return !std::bitset<64>((uint64_t)(code)).test(0);
    }
}