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
        CPU_RESOURCE_ACCESS     m_cpuAccess{ CPU_RESOURCE_ACCESS::DEFAULT };
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
}