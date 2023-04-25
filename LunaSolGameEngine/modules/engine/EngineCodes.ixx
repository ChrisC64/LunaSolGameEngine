module;
#include <cstdint>
export module Engine.EngineCodes;

export namespace LS::EC
{
    /**
     * @brief Codes regarding engine level information
    */
    enum class ENGINE_CODES : uint64_t
    {
        UNKNOWN,
        APPLICATION_EXIT_ERROR,
        APPLICATION_EXIT_SUCCESS,
        DEVICE_CREATION_FAILURE,
        DEVICE_CREATION_SUCCESS,
        WINDOW_CREATION_FAIL,
        WINDOW_CREATION_SUCCESS,
        MEMORY_ALLOCATION_ERROR,
        INVALID_ARGUMENTS
    };

}