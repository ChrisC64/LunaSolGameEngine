#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include "LSTypeDefs.h"

namespace LS
{
    /***
     * @enum BUFFER_USAGE
     * @brief Details how the buffer will be used which effects the GPU/CPU read/write access.
     */
    enum class BUFFER_USAGE
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
    enum class BUFFER_PIPELINE_STAGES
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

    enum class BUFFER_BIND_TYPE
    {
        UNKNOWN,
        VERTEX, // @brief A vertex buffer for geometry
        INDEX, // @brief An index buffer accompanying a vertex buffer
        CONSTANT_BUFFER, // @brief Shader constant buffer
        SHADER_RESOURCE // @brief A shader resource (textures
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

#ifdef LS_WINDOWS_BUILD
    /**
     * @brief Finds the D3D11_USAGE type for the given LS::BUFFER_USAGE enum
     * @param bufferUsage 
     * @return the value that corresponds to the given D3D11_USAGE enum from the given LS::BUFFER_USAGE enum
    */
    [[nodiscard]]
    inline D3D11_USAGE FindUsageFromLSBufferUsage(LS::BUFFER_USAGE bufferUsage)
    {
        using enum LS::BUFFER_USAGE;

        switch (bufferUsage)
        {
        case DEFAULT_RW: return D3D11_USAGE::D3D11_USAGE_DEFAULT;
        case CONSTANT: return D3D11_USAGE::D3D11_USAGE_DEFAULT;
        case IMMUTABLE: return D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
        case DYNAMIC: return D3D11_USAGE::D3D11_USAGE_DYNAMIC;
        case COPY_ONLY: return D3D11_USAGE::D3D11_USAGE_STAGING;
        default:
            throw std::runtime_error("Failed to find a suitable buffer usage type");
        }
    }

    [[nodiscard]]
    inline D3D11_BIND_FLAG FindBindFlagFromBindType(LS::BUFFER_BIND_TYPE bindType)
    {
        using enum LS::BUFFER_BIND_TYPE;
        switch (bindType)
        {
        case UNKNOWN: 
            throw std::runtime_error("Failed to find a corresponding enum to the given UNKNOWN bindType");
        case VERTEX: return D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
        case INDEX: return D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
        case CONSTANT_BUFFER: return D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
        case SHADER_RESOURCE: return D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
        default: 
            throw std::runtime_error("Unsupported or unknown enum passed for LS::BUFFER_BIND_TYPE to D3D11_BIND_FLAG conversion");
        }
    }

    /**
     * @brief Creates a buffer desc based on the given LS::LSBuffer template class
     * @tparam TObject 
     * @param buffer 
     * @return 
    */
    template<class TObject>
    inline D3D11_BUFFER_DESC CreateBufferDesc(const LS::LSBuffer<TObject>& buffer)
    {
        D3D11_BUFFER_DESC out{};
        out.ByteWidth = buffer.GetSizeInBytes();
        out.Usage = FindUsageFromLSBufferUsage(buffer.m_usage);
        out.BindFlags = FindBindFlagFromBindType(buffer.m_bindType);
        out.CPUAccessFlags = 0;
        return out;
    }
#endif
}