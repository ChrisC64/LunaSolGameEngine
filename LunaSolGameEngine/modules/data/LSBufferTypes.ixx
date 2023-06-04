module;
#include <cstdint>
#include <string>

export module Data.LSBufferTypes;
import Data.LSDataTypes;

export namespace LS
{
    /***
     * @enum BUFFER_USAGE
     * @brief Details how the buffer will be used which effects the GPU/CPU read/write access.
     */
    enum class BUFFER_USAGE : uint16_t
    {
        DEFAULT_RW, // @brief Buffer can be read and written by the GPU. 
        CONSTANT,// @brief Buffer is meant to be used by shader's constant buffers
        IMMUTABLE, // @brief Buffer lives on GPU but can never be changed after initialization, and remains the same
        DYNAMIC, // @brief Buffer can be modified by CPU and read by the GPU only. 
        COPY_ONLY // @brief Buffer can copy contents from GPU to CPU
    };

    /***
     * @enum BUFFER_PIPELINE_STAGES
     * @brief The description of what pipeline stage this buffer object will be used for in the graphics pipeline.
     */
    enum class BUFFER_PIPELINE_STAGES : uint16_t
    {
        UNDEFINED,
        VERTEX, // @brief Buffer can be used for vertex pipeline
        INDEX, // @brief Buffer can be used for index pipeline
        HULL, // @brief Buffer is used for Hull-shader pipeline
        TESSELATOR, // @brief Buffer is used for Tesselator pipeline
        GEOMETRY, // @brief Buffer is used for Geometry pipeline
        CONSTANT, // @brief Buffer is constant buffer data used by shader objects(not a pipeline stage)
        SHADER_RESOURCE // @brief Buffer is a resource (texture or other type) that is used for shaders (not a constant buffer data)
    };

    enum class BUFFER_BIND_TYPE : uint16_t
    {
        UNKNOWN,
        VERTEX, // @brief A vertex buffer for geometry
        INDEX, // @brief An index buffer accompanying a vertex buffer
        CONSTANT_BUFFER, // @brief Shader constant buffer
        SHADER_RESOURCE, // @brief A shader resource (textures
        STREAM_OUTPUT,
        RENDER_TARGET,
        DEPTH_STENCIL,
        UNORDERED_ACCESS,
        DECODER,
        VIDEO_ENCODER
    };

    //TODO: Add constraints to this... because I don't know what but I'm sure I"ll think of some later!
    template<class TObject>
    class LSBuffer
    {
    protected:
        Ref<TObject>            m_bufferObject{ nullptr };
        std::string             m_bufferName{ "default" };
        size_t                  m_stride{ 0 };
        size_t                  m_count{ 0 };
        BUFFER_USAGE            m_usage{ BUFFER_USAGE::DEFAULT_RW }; // @brief details how the data will be used
        BUFFER_BIND_TYPE        m_bindType{ BUFFER_BIND_TYPE::UNKNOWN }; // @brief Information on what stage in the graphics pipeline this buffer is used

    public:
        LSBuffer(Ref<TObject> obj, std::string_view name, size_t stride = 0, size_t count = 0) :
            m_bufferObject(std::move(obj)),
            m_bufferName(name.data()),
            m_stride(stride),
            m_count(count)
        {
        }

        ~LSBuffer()
        {
            m_bufferObject.release();
        }

        const std::byte* GetData() const
        {
            return reinterpret_cast<const std::byte*>(m_bufferObject.get());
        }

        const TObject* GetTypePtr() const
        {
            return m_bufferObject.get();
        }

        std::string_view GetBufferName() const
        {
            return m_bufferName;
        }

        constexpr size_t GetSizeOfType() const
        {
            return sizeof(TObject);
        }

        constexpr size_t GetSizeInBytes() const
        {
            return sizeof(*m_bufferObject);
        }

        constexpr size_t GetStride() const
        {
            return m_stride;
        }

        constexpr size_t GetCount() const
        {
            return m_count;
        }

        constexpr BUFFER_USAGE GetUsage() const
        {
            return m_usage;
        }

        constexpr BUFFER_PIPELINE_STAGES GetPipelineStage() const
        {
            return m_bindType;
        }
    };
}