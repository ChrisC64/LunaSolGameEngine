module;
#include <optional>
#include <string>
#include <memory>
#include <vector>
export module LSEDataLib:DataTypes;
// A list of common data types to define here for use within the engine itself
import :MathTypes;

export namespace LS
{
    struct ColorRGBA
    {
        float R, G, B, A;
        ColorRGBA() = default;
        ColorRGBA(float r, float g, float b, float a) : R(r), G(g), B(b), A(a)
        {}

        ColorRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : R(r / 255.0f), G(g / 255.0f), B(b / 255.0f), A(a / 255.0f)
        {}

        ColorRGBA(const ColorRGBA& other) : R(other.R), G(other.G), B(other.B), A(other.A)
        {}

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

    enum class PrimitiveType : uint32_t
    {
        UNKNOWN = 0,
        LINE,
        POINT,
        TRIANGLE,
        POLYGON
    };

    struct LSMesh
    {
        std::string Name;
        uint32_t NumUvComponents;
        uint32_t MaterialIndex;
        PrimitiveType PrimType;
        std::vector<uint32_t> Indices;
        std::vector<Vec3F> Vertices;
        std::vector<Vec3F> Normals;
        std::vector<Vec3F> TexCoords;
        std::vector<Vec4F> Colors;
        std::vector<Vec3F> Tangents;
        std::vector<Vec3F> BitTangents;
        //TODO: Add Texture objects
    };
}