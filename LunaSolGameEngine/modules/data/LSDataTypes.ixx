module;
#ifdef LS_WINDOWS_BUILD
// DirectX Libraries //
#include <DirectXMath.h>
#endif

#include <optional>
#include <string>
#include <memory>
export module Data.LSDataTypes;
// A list of common data types to define here for use within the engine itself

export
{
    // Typedefs //
// Reference Pointers //
    template <class T, typename Deleter = std::default_delete<T>>
    using Ref = std::unique_ptr<T, Deleter>;
    template <class T, typename Deleter>
    using Ref = std::unique_ptr<T, Deleter>;

    template <class S>
    using SharedRef = std::shared_ptr<S>;
    template<class T>
    using Nullable = std::optional<T>;
    using Guid = std::string;
    using Id = uint64_t;
#ifdef LS_WINDOWS_BUILD
    using mat4 = DirectX::XMFLOAT4X4;
#endif
}

export namespace LS
{
    enum class ERROR_LEVEL
    {
        INFO,
        WARN,
        FAIL,
        CRITICAL
    };
}