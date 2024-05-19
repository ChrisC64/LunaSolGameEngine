module;
#include <concepts>
#include <type_traits>
#include <optional>
#include <string>
#include <memory>
#include <vector>

export module Engine.Defines;

export namespace LS
{
    //////////////
    // Typedefs //
    //////////////
    
    // Reference Pointers //
    template <class T, typename Deleter = std::default_delete<T>>
    using Ref = std::unique_ptr<T, Deleter>;
    template <class T, typename Deleter>
    using Ref = std::unique_ptr<T, Deleter>;

    template <class S>
    using SharedRef = std::shared_ptr<S>;
    template <class W>
    using WeakRef = std::weak_ptr<W>;
    /**
     * @brief Typedef of a Nullable Object (std::optional wrapper)
     * @tparam T type to wrap in optional object
    */
    template<class T>
    using Nullable = std::optional<T>;
    using GuidStr = std::string;
    using GuidUL = uint64_t;

    template <class T>
    concept IsEnum = std::is_enum_v<T>;

    template <class T>
    concept IsIntegral = std::is_integral_v<T>;
    template <class T>
    concept IsSigned = IsIntegral<T> && std::is_signed_v<T>;
    template <class T>
    concept IsUnsigned = IsIntegral<T> && std::is_unsigned_v<T>;
    template <class T>
    concept IsFloatingPoint = std::is_floating_point_v<T>;

    template <class T>
    concept IsNumerical = std::is_integral_v<T> || std::is_floating_point_v<T>;

    template <class T>
    concept Addable = requires (T x) { x + x; };
    template <class T>
    concept Subtractable = requires (T x) { x - x; };
    template <class T>
    concept Multiplicative = requires (T x) { x* x; };
    template <class T>
    concept Divisible = requires (T x) { x / x; };
    template <class T>
    concept BasicMathOperators = requires {
        { Addable<T> };
        { Subtractable<T> };
        { Multiplicative<T> };
        { Divisible<T> };
    };

    template <class T>
    concept DestructibleNoThrow = std::is_nothrow_destructible_v<T>;
}