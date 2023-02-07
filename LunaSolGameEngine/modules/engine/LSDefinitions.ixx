module;
#include <DirectXMath.h>
#include <wrl/client.h>
#include <memory>
#include <optional>
export module Engine.LSDefinitions;

export 
{
    template <class T, typename Deleter = std::default_delete<T>>
    using Ref = std::unique_ptr<T, Deleter>;
    template <class T, typename Deleter>
    using Ref = std::unique_ptr<T, Deleter>;

    template <class S>
    using SharedRef = std::shared_ptr<S>;

    template<class T>
    using LSOptional = std::optional<T>;
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