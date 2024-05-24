module;
#include <optional>
#include <string>
#include <memory>
#include <vector>
export module LSDataLib:DataTypes;
// A list of common data types to define here for use within the engine itself
import :MathTypes;

export namespace LS
{
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
    };
}