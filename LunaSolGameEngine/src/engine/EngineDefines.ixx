export module Engine.Defines;
import <concepts>;
import <type_traits>;
import <optional>;
import <string>;
import <memory>;
import <vector>;
import <array>;

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

    using LSWindowHandle = void*  ;
    using LSAppInstance = void*;

    struct Rect
    {
        uint32_t TopX;
        uint32_t TopY;
        uint32_t Width;
        uint32_t Height;
    };

    template <class T, class C>
    constexpr auto FindOrNull(const C& container, const auto& query) -> LS::Nullable<T>
    {
        for (const auto& obj : container)
        {
            if (obj == query)
            {
                return obj.Resource;
            }
        }
        return {};
    }
}

export namespace LS::Colors
{
    struct RGBA
    {
        float R, G, B, A;
        RGBA() = default;
        RGBA(float r, float g, float b, float a) : R(r), G(g), B(b), A(a)
        {}

        RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : R(r / 255.0f), G(g / 255.0f), B(b / 255.0f), A(a / 255.0f)
        {}

        RGBA(const RGBA& other) : R(other.R), G(other.G), B(other.B), A(other.A)
        {}

        bool operator==(const RGBA& rhs) noexcept
        {
            return R == rhs.R && G == rhs.G && B == rhs.B && A == rhs.A;
        }
    };

    constexpr std::array<float, 4> RED = { 1.0f, 0.0f, 0.0f, 1.0f };
    constexpr std::array<float, 4> GREEN = { 0.0f, 1.0f, 0.0f, 1.0f };
    constexpr std::array<float, 4> BLUE = { 0.0f, 0.0f, 1.0f, 1.0f };
    constexpr std::array<float, 4> BLACK = { 0.0f, 0.0f, 0.0f, 1.0f };
    constexpr std::array<float, 4> WHITE = { 1.0f, 1.0f, 1.0f, 1.0f };
}