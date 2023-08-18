module;
#include <fstream>
#include <filesystem>
#include <memory>
#include <map>
#include <string>
#include <string_view>
#include <format>
#include <span>
#include <stdexcept>
#include <ranges>
#include <iostream>
#include <algorithm>

//TODO: Consider making Lse prepend for "Engine" stuff instead of just "Engine" like Engine.EngineCodes below.
export module LSE.Serialize.WavefrontObj;
import Data.LSMathTypes;
import Data.LSConcepts;
import Data.LSDataTypes;
import Engine.EngineCodes;

namespace LS::Serialize
{
    enum class TokenObj
    {
        Unknown,
        Vertex,
        Texture,
        Normal,
        Face,
        Group,
        ObjectName,
        UseMaterial,
        MaterialLib,
        Comment,
        Smooth,
    };

    using enum TokenObj;

    const std::map<TokenObj, std::string> ObjTokens
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
        { Smooth, "s"},
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

            auto GetIndices() const noexcept -> std::vector<int>
            {
                return Indices;
            }
            
            auto GetNormals() const noexcept -> std::vector<int>
            {
                return Normals;
            }
            
            auto GetTexcoords() const noexcept -> std::vector<int>
            {
                return Texcoords;
            }
        };

        [[nodiscard]] auto LoadFile(const std::filesystem::path file) noexcept -> LS::System::ErrorCode;
        [[nodiscard]] auto LoadObject() noexcept -> LS::System::ErrorCode;
        [[nodiscard]] auto ParseFace(std::span<std::string> line) noexcept -> LS::System::ErrorCode;

        auto GetFaces() const noexcept -> const std::vector<Face>
        {
            return m_faces;
        }
        
        auto GetVertices() const noexcept -> const std::vector<Vec3F>
        {
            return m_vertices;
        }
        
        auto GetNormals() const noexcept -> const std::vector<Vec3F>
        {
            return m_normals;
        }
        
        auto GetTexCoords() const noexcept -> const std::vector<Vec2F>
        {
            return m_uvs;
        }

        auto GetLines() const noexcept -> const std::vector<std::string>
        {
            return m_lines;
        }

        void SetClockwise(bool isCounterClockwise) noexcept
        {
            IsCounterClockwise = isCounterClockwise;
        }

    private:
        // @brief The lines of the file
        std::vector<std::string> m_lines;
        std::vector<Vec3F> m_vertices;
        std::vector<Vec3F> m_normals;
        std::vector<Vec2F> m_uvs;
        std::vector<Face> m_faces;
        bool IsCounterClockwise = false;
    };
}

module : private;

namespace fs = std::filesystem;
namespace LS::Serialize
{
    [[nodiscard]]
    auto Split(std::string_view line, std::string_view delim) -> std::vector<std::string>
    {
        std::vector<std::string> out;

        size_t offset = 0;
        auto pos = line.find(delim, offset);
        while (pos != std::string::npos)
        {
            auto substr = line.substr(offset, pos - offset);
            out.emplace_back(std::move(substr));
            offset = pos + 1;
            pos = line.find(delim, offset);
        }

        if (offset < line.size())
        {
            auto substr = line.substr(offset);
            out.emplace_back(std::move(substr));
        }

        return out;
    }

    [[nodiscard]]
    auto ParseVertex(std::span<std::string> splits, Vec3F& out) noexcept -> LS::System::ErrorCode
    {
        int count = 0;
        for (auto& s : splits)
        {
            if (s == "v")
                continue;

            try
            {
                auto value = std::stof(s);
                switch (count)
                {
                case 0:
                    out.x = value;
                    break;
                case 1:
                    out.y = value;
                    break;
                case 2:
                    out.z = value;
                    break;
                default:
                    break;
                }
            }
            catch (std::invalid_argument e)
            {
                return LS::System::CreateFailCode(e.what());
            }
            catch (std::out_of_range e)
            {
                return LS::System::CreateFailCode(e.what());
            }

            count++;
        }

        return System::CreateSuccessCode();
    }

    [[nodiscard]]
    auto ParseUvs(std::span<std::string> splits, Vec2F& out) noexcept -> LS::System::ErrorCode
    {
        int count = 0;
        for (auto& s : splits)
        {
            if (s == "vt")
                continue;

            try
            {
                auto value = std::stof(s);
                switch (count)
                {
                case 0:
                    out.x = value;
                    break;
                case 1:
                    out.y = value;
                    break;
                default:
                    break;
                }
            }
            catch (std::invalid_argument e)
            {
                return LS::System::CreateFailCode(e.what());
            }
            catch (std::out_of_range e)
            {
                return LS::System::CreateFailCode(e.what());
            }

            count++;
        }

        return LS::System::CreateSuccessCode();
    }


    [[nodiscard]]
    auto ParseNormal(std::span<std::string> splits, Vec3F& out) noexcept -> LS::System::ErrorCode
    {
        int count = 0;
        for (auto& s : splits)
        {
            if (s == "vn")
                continue;

            try
            {
                auto value = std::stof(s);
                switch (count)
                {
                case 0:
                    out.x = value;
                    break;
                case 1:
                    out.y = value;
                    break;
                case 2:
                    out.z = value;
                    break;
                default:
                    break;
                }
            }
            catch (std::invalid_argument e)
            {
                return LS::System::CreateFailCode(e.what());
            }
            catch (std::out_of_range e)
            {
                return LS::System::CreateFailCode(e.what());
            }

            count++;
        }

        return System::CreateSuccessCode();
    }

    [[nodiscard]]
    auto WavefrontObj::LoadFile(const std::filesystem::path file) noexcept -> LS::System::ErrorCode
    {
        if (!fs::exists(file))
        {
            return LS::System::CreateFailCode(std::format("File was not found: {}", file.string()), LS::System::ErrorCategory::FILE);
        }

        auto stream = std::fstream(file, std::fstream::in);

        std::string line;
        while (std::getline(stream, line))
        {
            m_lines.emplace_back(std::move(line));
        }

        return LS::System::CreateSuccessCode();
    }

    [[nodiscard]]
    auto WavefrontObj::LoadObject() noexcept -> LS::System::ErrorCode
    {
        std::string delim = " ";
        for (auto& l : m_lines)
        {
            if (l.starts_with(ObjTokens.at(Comment)) || l.starts_with(ObjTokens.at(Smooth)) 
                || l.starts_with(ObjTokens.at(MaterialLib)) || l.starts_with(ObjTokens.at(UseMaterial)) 
                || l.starts_with(ObjTokens.at(ObjectName)) )
            {
                continue; // Skip comments 
            }

            std::string token = l.substr(0, l.find(delim));
            auto splits = Split(l, delim);

            if (token == ObjTokens.at(Vertex))
            {
                // parse vertex
                Vec3F vertex;
                if (auto vResult = ParseVertex(splits, vertex); !vResult)
                {
                    return vResult;
                }

                m_vertices.emplace_back(vertex);
            }
            else if (token == ObjTokens.at(Normal))
            {
                // parse normal
                Vec3F normal;

                if (auto vnResult = ParseNormal(splits, normal); !vnResult)
                {
                    return vnResult;
                }

                m_normals.emplace_back(normal);
            }
            else if (token == ObjTokens.at(Texture))
            {
                // parse tex coords
                Vec2F uv;
                if (auto vtResult = ParseUvs(splits, uv); !vtResult)
                {
                    return vtResult;
                }

                m_uvs.emplace_back(uv);
            }
            else if (token == ObjTokens.at(TokenObj::Face))
            {
                // parse face
                if (auto fResult = ParseFace(splits); !fResult)
                {
                    return fResult;
                }
            }

        }

        return System::CreateSuccessCode();
    }

    [[nodiscard]]
    auto WavefrontObj::ParseFace(std::span<std::string> line) noexcept -> LS::System::ErrorCode
    {
        Face face;

        // f will be in this line, so we want 1 less 
        auto faceCount = line.size() - 1;

        // Skip F and parse each section
        for (auto p = 1u; p < faceCount + 1; ++p)
        {
            auto split = Split(line[p], "/");
            for (auto i = 0u; i < split.size(); ++i)
            {
                if (split.at(i).empty())
                    continue;

                auto value = std::stoi(split.at(i)) - 1;

                if (i == 0) // Vertex Index
                {
                    face.Indices.emplace_back(value);
                }
                else if (i == 1) // Vertex Texture (UV)
                {
                    face.Texcoords.emplace_back(value);
                }
                else if (i == 2)// Vertex Normal
                {
                    face.Normals.emplace_back(value);
                }
            }
        }

        // Reverse Index Order //
        if (IsCounterClockwise)
        {
            std::reverse(face.Indices.begin(), face.Indices.end());
        }

        m_faces.emplace_back(face);

        return LS::System::CreateSuccessCode();
    }
}