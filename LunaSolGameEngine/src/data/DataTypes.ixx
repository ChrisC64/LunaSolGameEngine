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

    /***
     * @enum RENDER_PIPELINE_STAGE
     * @brief The description of what pipeline stage this buffer object will be used for in the graphics pipeline.
     */
    enum class RENDER_PIPELINE_STAGE : uint16_t
    {
        UNDEFINED = 0,
        VERTEX, // @brief Buffer can be used for vertex pipeline
        INDEX, // @brief Buffer can be used for index pipeline
        HULL, // @brief Buffer is used for Hull-shader pipeline
        TESSELATOR, // @brief Buffer is used for Tesselator pipeline
        GEOMETRY, // @brief Buffer is used for Geometry pipeline
        CONSTANT, // @brief Buffer is constant buffer data used by shader objects(not a pipeline stage)
        SHADER_RESOURCE // @brief Buffer is a resource (texture or other type) that is used for shaders (not a constant buffer data)
    };

    struct MeshData
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