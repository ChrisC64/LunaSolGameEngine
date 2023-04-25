module;
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
    /**
     * @brief Typedef of a Nullable Object (std::optional wrapper)
     * @tparam T type to wrap in optional object
    */
    template<class T>
    using Nullable = std::optional<T>;
    using GuidStr = std::string;
    using GuidLL = uint64_t;
    using Id = uint64_t;

    namespace LS
    {
        struct ColorRGBA
        {
            float R, G, B, A;

            ColorRGBA(float r, float g, float b, float a) : R(r), G(g), B(b), A(a)
            {}

            ColorRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : R(r / 255.0f),
                G(g / 255.0f), B(b / 255.0f), A(a / 255.0f)
            {}

            bool operator==(const ColorRGBA& rhs) noexcept
            {
                return R == rhs.R && G == rhs.G && B == rhs.B && A == rhs.A;
            }
        };

        struct ColorRGB
        {
            float R, G, B;
            ColorRGB(float r, float g, float b) : R(r), G(g), B(b)
            {}

            ColorRGB(uint8_t r, uint8_t g, uint8_t b) : R(r / 255.0f),
                G(g / 255.0f), B(b / 255.0f)
            {}

            bool operator==(const ColorRGB& rhs) noexcept
            {
                return R == rhs.R && G == rhs.G && B == rhs.B;
            }
        };
    }
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