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
        POINT,
        LINE,
        TRIANGLE,
        POLYGON
    };

    /***
     * @enum RENDER_PIPELINE_STAGE
     * @brief The description of what pipeline stage this object will be used for in the graphics pipeline.     
     */
    enum class RENDER_PIPELINE_STAGE : uint16_t
    {
        UNDEFINED = 0,
        NO_PIPELINE,
        VERTEX, // @brief Buffer can be used for vertex pipeline
        INDEX, // @brief Buffer can be used for index pipeline
        HULL, // @brief Buffer is used for Hull-shader pipeline
        TESSELATOR, // @brief Buffer is used for Tesselator pipeline
        GEOMETRY, // @brief Buffer is used for Geometry pipeline
        SHADER_RESOURCE // @brief Buffer is a resource (texture or other type) that is used for shaders (not a constant buffer data)
    };

    /***
     * @enum BUFFER_USAGE
     * @brief Details how the buffer will be used which effects the GPU/CPU read/write access.
     */
    enum class BUFFER_USAGE : uint16_t
    {
        DEFAULT_RW = 0, // @brief Buffer can be read and written by the GPU. 
        CONSTANT,// @brief Buffer is meant to be used by shader's constant buffers
        IMMUTABLE, // @brief Buffer lives on GPU but can never be changed after initialization, and remains the same
        DYNAMIC, // @brief Buffer can be modified by CPU and read by the GPU only. 
        STAGING // @brief Buffer can copy contents from GPU to CPU
    };

    /**
     * @brief Denotes where the buffer is expected to be bound to
     */
    enum class BUFFER_BIND_TYPE : uint16_t
    {
        UNKNOWN = 0,
        VERTEX = 0x0001, // @brief A vertex buffer for geometry
        INDEX = 0x0002, // @brief An index buffer accompanying a vertex buffer
        CONSTANT_BUFFER = 0x0004, // @brief Shader constant buffer
        SHADER_RESOURCE = 0x0008, // @brief A shader resource 
        STREAM_OUTPUT = 0x0010,
        RENDER_TARGET = 0x0020,
        DEPTH_STENCIL = 0x0040,
        UNORDERED_ACCESS = 0x0100,
        DECODER = 0x0200,
        VIDEO_ENCODER = 0x0400
    };

    enum class CPU_RESOURCE_ACCESS : uint8_t
    {
        DEFAULT = 0,
        WRITE_ONLY = 1,
        READ_ONLY = 2,
        WRITE_AND_READ = 3
    };

    enum class RESOURCE_STATE : uint8_t
    {
        UNLOCKED,
        LOCKED
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