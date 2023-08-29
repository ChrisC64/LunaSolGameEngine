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

    namespace LS
    {
        struct ColorRGBA;

        struct ColorRGB
        {
            float R, G, B;
            ColorRGB() = default;
            ColorRGB(float r, float g, float b);
            ColorRGB(uint8_t r, uint8_t g, uint8_t b);
            ColorRGB(const ColorRGBA& rgba);

            bool operator==(const ColorRGB& rhs) noexcept
            {
                return R == rhs.R && G == rhs.G && B == rhs.B;
            }
        };

        struct ColorRGBA
        {
            float R, G, B, A;
            ColorRGBA() = default;
            ColorRGBA(float r, float g, float b, float a);
            ColorRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
            ColorRGBA(const ColorRGB& rgb, float a);

            bool operator==(const ColorRGBA& rhs) noexcept
            {
                return R == rhs.R && G == rhs.G && B == rhs.B && A == rhs.A;
            }
        };

        struct Rect
        {
            uint32_t TopX;
            uint32_t TopY;
            uint32_t Width;
            uint32_t Height;
        };
    }
}

module : private;

LS::ColorRGB::ColorRGB(float r, float g, float b) : R(r), G(g), B(b)
{}

LS::ColorRGB::ColorRGB(uint8_t r, uint8_t g, uint8_t b) : R(r / 255.0f),
G(g / 255.0f), B(b / 255.0f)
{}

LS::ColorRGB::ColorRGB(const ColorRGBA& rgba) : R(rgba.R), G(rgba.G), B(rgba.B)
{}

LS::ColorRGBA::ColorRGBA(float r, float g, float b, float a) : R(r), G(g), B(b), A(a)
{}

LS::ColorRGBA::ColorRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : R(r / 255.0f),
G(g / 255.0f), B(b / 255.0f), A(a / 255.0f)
{}

LS::ColorRGBA::ColorRGBA(const ColorRGB& rgb, float a) : R(rgb.R), G(rgb.G), B(rgb.B), A(a)
{}