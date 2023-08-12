module;
#include <fstream>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <string>
#include <string_view>
#include <format>
//TODO: Consider making Lse prepend for "Engine" stuff instead of just "Engine" like Engine.EngineCodes below.
export module LSE.Serialize.WavefrontObj;
import Data.LSMathTypes;
import Data.LSConcepts;
import Data.LSDataTypes;
import Engine.EngineCodes;

namespace LS::Serialize
{
    enum class Token
    {
        Vertex,
        Texture,
        Normal,
        Face,
        Group,
        ObjectName,
        UseMaterial,
        MaterialLib,
        Comment,
    };

    using enum Token;

    constexpr std::unordered_map<Token, std::string> ObjTokens
    {
        { Vertex, "v"},
        { Texture, "vt"},
        { Normal, "vn"},
        { Face, "f"},
        { Group, "g"},
        { ObjectName, "o"},
        { UseMaterial, "usemtl"},
        { MaterialLib, "mtllib"},
        { Comment, "#"},
    };
}

export namespace LS::Serialize
{
    class WavefrontObj
    {
    public:
        WavefrontObj() = default;
        ~WavefrontObj() = default;

        struct Face
        {
            std::vector<int> Indices;
            std::vector<int> Normals;
            std::vector<int> Texcoords;
        };

         auto LoadFile(const std::filesystem::path file) noexcept -> Ref<LS::System::ErrorCode>;
         auto LoadObject() noexcept -> Ref<LS::System::ErrorCode>;
    private:
        std::vector<Vec3F> m_vertices;
        std::vector<Vec2F> m_normals;
        std::vector<Vec3F> m_uvs;
        std::vector<Face> m_faces;
    };
}

module : private;

namespace fs = std::filesystem;
namespace LS::Serialize
{
    auto ParseVertex(std::string_view line) noexcept -> Vec3F
    {
    }


    auto WavefrontObj::LoadFile(const std::filesystem::path file) noexcept -> Ref<LS::System::ErrorCode>
    {
        if (!fs::exists(file))
        {
            return std::make_unique<LS::System::FailErrorCode>(LS::System::ErrorCategory::FILE, 
                std::format("File was not found: {}", file.string()));
        }

        auto stream = std::fstream(file, std::fstream::in);

        std::string line;
        std::vector<std::string> lines;
        while (std::getline(stream, line) )
        {
            lines.emplace_back(std::move(line));
        }

        std::string delim = " ";
        for (auto& l : lines)
        {
            if (l.starts_with(ObjTokens[Comment]))
            {
                continue;
            }

            std::string token = l.substr(0, l.find(delim));

            if (token == ObjTokens[Vertex])
            {
                // parse vertex

            }
            else if (token == ObjTokens[Normal])
            {
                // parse normal
            }
            else if (token == ObjTokens[Texture])
            {
                // parse tex coords
            }
            else if (token == ObjTokens[Token::Face])
            {
                // parse face
            }

        }

        return std::make_unique<LS::System::SuccessErrorCode>();
    }
    
    auto WavefrontObj::LoadObject() noexcept -> Ref<LS::System::ErrorCode>
    {
        return std::make_unique<LS::System::SuccessErrorCode>();
    }
}