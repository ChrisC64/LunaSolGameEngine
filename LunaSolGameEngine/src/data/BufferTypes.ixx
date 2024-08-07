module;
#include <cstdint>
#include <string>
#include <unordered_map>
#include <format>
#include <optional>
export module LSDataLib:BufferTypes;
import :DataTypes;
import Engine.EngineCodes;
import Engine.Defines;

export namespace LS
{
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
        COPY_ONLY // @brief Buffer can copy contents from GPU to CPU
    };

    /**
     * @brief Denotes where the buffer is expected to be bound to 
     */
    enum class BUFFER_BIND_TYPE : uint16_t
    {
        UNKNOWN = 0,
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

    enum class CPU_RESOURCE_ACCESS : uint8_t
    {
        UNDEFINED = 0,
        WRITE_ONLY = 1,
        READ_ONLY = 2,
        WRITE_AND_READ = 3
    };

    enum class RESOURCE_STATE : uint8_t
    {
        UNLOCKED,
        LOCKED
    };

    //TODO: Add constraints to this... because I don't know what but I'm sure I"ll think of some later!
    // 1. Should be the object type (not pointers)
    // 2. Should be constructible by default without params
    // 3. Should also allow for empty constructed objects (like an array)
    template<class TObject>
    class LSBuffer
    {
    protected:
        TObject                 m_bufferObject;
        std::string             m_bufferName{ "default" };
        size_t                  m_stride{ 0 };// @brief the number of bytes for an element of the array
        size_t                  m_width{ 0 };// @brief the number of bytes in a row (always set - 1D and greater arrays)
        size_t                  m_rows{ 0 };// @brief the number of rows in the 2D array (0 for 1D arrays))
        size_t                  m_depth{ 0 };// @brief the number of 2D arays for the 3D array (0 for 1D/2D arrays only)
        BUFFER_USAGE            m_usage{ BUFFER_USAGE::DEFAULT_RW }; // @brief details how the data will be used
        BUFFER_BIND_TYPE        m_bindType{ BUFFER_BIND_TYPE::UNKNOWN }; // @brief Information on what stage in the graphics pipeline this buffer is used
        CPU_RESOURCE_ACCESS     m_cpuAccess{ CPU_RESOURCE_ACCESS::UNDEFINED };
        RESOURCE_STATE          m_state{ RESOURCE_STATE::UNLOCKED };
    public:
        LSBuffer(TObject obj, std::string_view name, size_t stride = 0, size_t count = 0) :
            m_bufferObject(obj),
            m_bufferName(name.data()),
            m_stride(stride)
        {
        }

        ~LSBuffer() = default;

        auto GetData() const -> const std::byte*
        {
            return static_cast<const std::byte*>(&m_bufferObject);
        }

        auto GetTypePtr() const -> const TObject*
        {
            return &m_bufferObject;
        }

        auto GetBufferName() const -> std::string_view
        {
            return m_bufferName;
        }

        constexpr auto GetSizeOfType() const -> size_t
        {
            return sizeof(TObject);
        }

        constexpr auto GetSizeInBytes() const -> size_t
        {
            return sizeof(m_bufferObject);
        }

        constexpr auto GetStride() const -> size_t
        {
            return m_stride;
        }

        constexpr auto GetWidth() const -> size_t
        {
            return m_width;
        }

        constexpr auto GetRows() const -> size_t
        {
            return m_rows;
        }

        constexpr auto GetDepth() const -> size_t
        {
            return m_depth;
        }

        constexpr auto GetUsage() const -> BUFFER_USAGE
        {
            return m_usage;
        }

        constexpr auto GetBindStage() const -> RENDER_PIPELINE_STAGE
        {
            return m_bindType;
        }

        constexpr auto GetCpuAccess() const -> CPU_RESOURCE_ACCESS
        {
            return m_cpuAccess;
        }
    };

    //using LSBufferReserve = LSBuffer<void*>;

    //template <class Buffer>
    //class LSBufferCache
    //{
    //    using LSCacheSize = std::uint64_t;
    //    using LSCache = std::unordered_map<std::string, Buffer>;
    //public:
    //    LSBufferCache() = default;
    //    ~LSBufferCache() = default;

    //    LSBufferCache(const LSBufferCache&) = delete;
    //    LSBufferCache& operator=(const LSBufferCache&) = delete;

    //    LSBufferCache(LSBufferCache&&) = default;
    //    LSBufferCache& operator=(LSBufferCache&&) = default;

    //    /**
    //     * @brief Insert a buffer into the cache
    //     * @param key a unique ID
    //     * @param buffer The buffer to insert
    //     * @return A success error code means insertion took place, a fail error code means insertion did not
    //    */
    //    [[nodiscard]]
    //    auto Insert(std::string_view key, const Buffer& buffer) noexcept -> LS::System::ErrorCode
    //    {
    //        auto [_, status] = m_cache.emplace(key.data(), buffer);

    //        if (!status)
    //            return LS::System::CreateFailCode(std::format("Could not add key: {}", key));
    //        return LS::System::CreateSuccessCode();
    //    }

    //    [[nodiscard]]
    //    auto Get(std::string_view key) noexcept -> Nullable<Buffer>
    //    {
    //        if (!m_cache.contains(key.data()))
    //            return std::nullopt;

    //        return m_cache.at(key.data());
    //    }

    //    [[nodiscard]]
    //    auto Remove(std::string_view key) noexcept -> bool
    //    {
    //        if (!m_cache.contains(key.data()))
    //            return false;

    //        m_cache.erase(key.data());
    //        return true;
    //    }

    //    void SetCacheSize(LSCacheSize size) noexcept
    //    {
    //        m_limit = size;
    //    }

    //    [[nodiscard]]
    //    auto GetCacheSize() noexcept -> LSCacheSize
    //    {
    //        return m_limit;
    //    }

    //private:
    //    LSCacheSize m_limit = 1024u * 1024u * 1024u * 3;
    //    LSCache m_cache;
    //};
}