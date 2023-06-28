module;
#include <filesystem>
#include <string>
#include <fstream>
#include <cstdint>
#include <vector>
#include <ranges>
#include <span>
#include <format>

export module Data.LSShader;

import Util.StdUtils;

export namespace LS
{
    enum class SHADER_TYPE
    {
        VERTEX,
        PIXEL,
        GEOMETRY,
        COMPUTE,
        HULL,
        DOM
    };

    /**
     * @brief Informs the type of data the shader element will be
     *
     * Replacing the VERTEX_DATA_TYPE element
     */
    enum class SHADER_DATA_TYPE
    {
        NONE = 0,
        FLOAT,
        FLOAT2,
        FLOAT3,
        FLOAT4,
        MAT3F,
        MAT4F,
        INT,
        INT2,
        INT3,
        INT4,
        UINT,
        UINT2,
        UINT3,
        UINT4,
        BOOL
    };

    /**
     * @brief Used to detail the type of data this element will be used for
     *
     * A vertex element may have instanced or vertex data. Vertex data is most common
     * for information relating to a Vertex buffer. When creating an Instance buffer,
     * you'll use Instance to identify the data in this element is used for instancing.
     */
    enum class INPUT_CLASS
    {
        VERTEX,
        INSTANCE
    };

    /***
     * @brief A vertex attribute or input layout is made up of elements that have some defined purpose.
     *
     * This struct can help define what each element is. For example, you may want m_Position and Color
     * for your vertex description, with m_Position havng 3 floats (xyz) and Color having RGBA (4 floats).
     * the @link VERTEX_DATA_DESC member desc is to tell us the use of the object, while the
     * count tells us how many (2D positions of x/y or 3D positon of x/y/z?) and then
     * VERTEX_DATA_TYPE tells us what type this data is. (byte, float, int, etc.)
     */
    struct ShaderElement
    {
        SHADER_DATA_TYPE ShaderData = SHADER_DATA_TYPE::NONE;
        std::string SemanticName; // The semantic name without the value (e.g. POSITION1 = POSITION)
        uint32_t SemanticIndex = 0; // The index value of the semantic (e.g. POSITION1 = 1)
        uint32_t OffsetAligned = 0; // The offset position for this data in the buffer
        uint32_t InputSlot = 0; // The input slot for this element (used for combined structures)
        INPUT_CLASS InputClass = INPUT_CLASS::VERTEX; // Purpose for this input element
        uint32_t InstanceStepRate = 0; // the Instance data step rate

        bool operator==(const ShaderElement& v) const
        {
            return SemanticName == v.SemanticName
                && OffsetAligned == v.OffsetAligned
                && InputSlot == v.InputSlot
                && InputClass == v.InputClass
                && InstanceStepRate == v.InstanceStepRate;
        }

        bool operator!=(const ShaderElement& v) const
        {
            return !(*this == v);
        }
    };

    /**
     * @brief Returns the number of BYTES that this object takes
     */
    uint32_t GetShaderDataByteSize(SHADER_DATA_TYPE type)
    {
        using SDT = SHADER_DATA_TYPE;
        switch (type)
        {
        case SDT::NONE:
            throw std::runtime_error("Cannot build layout stride for LSShaderInputSignature with a unspecified data type.\n");
            break;
        case SDT::FLOAT:  return 4;
        case SDT::FLOAT2: return 4 * 2;
        case SDT::FLOAT3: return 4 * 3;
        case SDT::FLOAT4: return 4 * 4;
        case SDT::INT:    return 4;
        case SDT::INT2:   return 4 * 2;
        case SDT::INT3:   return 4 * 3;
        case SDT::INT4:   return 4 * 4;
        case SDT::UINT:   return 4;
        case SDT::UINT2:  return 4 * 2;
        case SDT::UINT3:  return 4 * 3;
        case SDT::UINT4:  return 4 * 4;
        case SDT::BOOL:   return 4;
        default:
            throw std::runtime_error("Unknown SHADER DATA TYPE passed, cannot build layout stride.\n");
            break;
        }
    }

    /**
     * @brief Returns the number of components per data type. For matrices,
     * this will return the row component count. (e.g. MAT4F = 4 rows with 4 components, so the value
     * returned is 4 to indicate a 4 component row.
     */
    uint32_t GetShaderDataTypeCount(SHADER_DATA_TYPE type)
    {
        using SDT = SHADER_DATA_TYPE;
        switch (type)
        {
        case SDT::FLOAT:  return 1;
        case SDT::FLOAT2: return 2;
        case SDT::FLOAT3: return 3;
        case SDT::FLOAT4: return 4;
        case SDT::MAT3F:  return 3;//Returns the row count
        case SDT::MAT4F:  return 4;//Returns the row count
        case SDT::INT:	  return 1;
        case SDT::INT2:	  return 2;
        case SDT::INT3:	  return 3;
        case SDT::INT4:	  return 4;
        case SDT::UINT:	  return 1;
        case SDT::UINT2:  return 2;
        case SDT::UINT3:  return 3;
        case SDT::UINT4:  return 4;
        case SDT::BOOL:	  return 1;
        default:
            throw std::runtime_error("Unknown SHADER DATA TYPE passed, cannot build component size\n");
            break;
        }
    }

    inline size_t HashShaderElement(ShaderElement const& elem) noexcept
    {
        std::size_t h1 = Utils::HashEnum(elem.ShaderData);
        std::size_t h2 = std::hash<uint32_t>{}(elem.OffsetAligned);
        std::size_t h3 = Utils::HashEnum(elem.InputClass);
        std::size_t h4 = std::hash<std::string_view>{}(elem.SemanticName);
        h3 ^= (h4 << 1);
        h2 ^= (h3 << 1);
        h1 ^= (h2 << 1);
        std::size_t out = h1 >> 1;
        return out;
    }

    //TODO: Implement Shader parser and setup (Input Layout) to make things easier when handling Shaders (HLSL for now)
    struct LSShaderFile
    {
        SHADER_TYPE				m_shaderType;//@brief Shader type
        std::filesystem::path	m_filePath{};//@brief The file's path
        std::filesystem::path	m_includePath{ "." };//@brief The directory to look at for include (default to current directory of file)
        std::string				m_entryPoint{ "main" }; //@brief The main entry point into the shader (if not main)
        std::string				m_shaderTarget{};//@brief The target profile to compile in
        std::fstream			m_file;//@brief The file
    };

    /**
     * @brief An object that represents the signature of the shader input
     */
    struct LSShaderInputSignature
    {
    public:
        LSShaderInputSignature() = default;
        LSShaderInputSignature(std::span<ShaderElement> elems)
        {
            SetInputLayout(elems);
        }

        ~LSShaderInputSignature() = default;

        const std::vector<ShaderElement>& GetInputLayout() const
        {
            return Elements;
        }

        void SetInputLayout(std::span<ShaderElement> elems)
        {
            Elements.resize(elems.size());
            std::ranges::copy(elems, Elements.data());
        }

        /**
        * @brief Adds the vertex element to this vertex description
        */
        void AddElement(ShaderElement desc)
        {
            Elements.emplace_back(desc);
        }

        void AddElement(SHADER_DATA_TYPE dataType, std::string_view semanticName,
            uint32_t offsetAligned = 0, uint32_t semanticIndex = 0,
            INPUT_CLASS inputClass = INPUT_CLASS::VERTEX,
            uint32_t inputSlot = 0,
            uint32_t instanceStepRate = 0)
        {
            ShaderElement elem{ .ShaderData = dataType,
            .SemanticName = semanticName.data(),
            .SemanticIndex = semanticIndex,
            .OffsetAligned = offsetAligned,
            .InputSlot = inputSlot,
            .InputClass = inputClass,
            .InstanceStepRate = instanceStepRate
            };
            Elements.emplace_back(elem);
        }
        
        bool operator==(const LSShaderInputSignature& rhs) const
        {
            return this->Elements == rhs.Elements;
        }
        bool operator!=(const LSShaderInputSignature& rhs) const
        {
            return !(*this == rhs);
        }

        std::vector<ShaderElement> Elements;
    };
}