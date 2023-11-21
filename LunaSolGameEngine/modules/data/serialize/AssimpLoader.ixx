module;
#include <vector>
#include <filesystem>
#include <cstdint>
#include "assimp\Importer.hpp"
#include "assimp\postprocess.h"
#include "assimp\scene.h"

export module LSE.Serialize.AssimpLoader;
import LSData;
import Engine.EngineCodes;

namespace fs = std::filesystem;
export namespace LS::Serialize
{
    class AssimpLoader
    {
    public:
        AssimpLoader() = default;
        ~AssimpLoader() = default;

        AssimpLoader& operator=(const AssimpLoader&) = default;
        AssimpLoader& operator=(AssimpLoader&&) = default;
        AssimpLoader(const AssimpLoader&) = default;
        AssimpLoader(AssimpLoader&&) = default;

        auto Load(fs::path file, uint32_t flags = 0) noexcept -> LS::System::ErrorCode;
        auto GetMeshes() const noexcept -> const std::vector<LSMesh>&
        {
            return m_meshes;
        }

    private:
        Assimp::Importer m_importer;
        std::vector<LSMesh> m_meshes;

        void ProcessFace(const aiFace* face, LSMesh& lsMesh) noexcept;
        void ProcessMesh(const aiMesh* mesh, LSMesh& lsMesh) noexcept;
        void ProcessRootNode(const aiNode* node, const aiScene* scene) noexcept;
        void ProcessScene(const aiScene* scene) noexcept;
    };
}

module : private;

using namespace LS;

auto Serialize::AssimpLoader::Load(fs::path file, uint32_t flags) noexcept -> LS::System::ErrorCode
{
    Assimp::Importer importer;

    //aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_MakeLeftHanded
    const auto scene = importer.ReadFile(file.string(), flags);

    if (!scene)
    {
        return LS::System::CreateFailCode(importer.GetErrorString());
    }

    ProcessScene(scene);

    return LS::System::CreateSuccessCode();
}

void Serialize::AssimpLoader::ProcessFace(const aiFace* face, LSMesh& lsMesh) noexcept
{
    if (!face)
        return;

    std::vector<uint32_t> indices(face->mNumIndices);
    for (auto i = 0u; i < face->mNumIndices; i++)
    {
        indices.at(i) = face->mIndices[i];
    }
    lsMesh.Indices.insert(lsMesh.Indices.end(), indices.begin(), indices.end());
}

void Serialize::AssimpLoader::ProcessMesh(const aiMesh* mesh, LSMesh& lsMesh) noexcept
{
    if (!mesh || !mesh->HasPositions())
        return;

    // Find the Indices of the mesh's faces (a face can be X number of polygons that make up this mesh: i.e. triangles)
    for (auto i = 0u; i < mesh->mNumFaces; ++i)
    {
        ProcessFace(&mesh->mFaces[i], lsMesh);
    }

    // Obtain the mesh's positions (vertices) 
    // We should always assume there are vertices, otherwise what are we loading?
    lsMesh.Vertices.resize(mesh->mNumVertices);
    lsMesh.Normals.resize(mesh->mNumVertices);
    lsMesh.TexCoords.resize(mesh->mNumVertices);
    lsMesh.Colors.resize(mesh->mNumVertices);
    lsMesh.NumUvComponents = mesh->HasTextureCoords(0) ? mesh->mNumUVComponents[0] : 0;
    lsMesh.MaterialIndex = mesh->mMaterialIndex;

    const aiColor4D Zero4(0.0f, 0.0f, 0.0f, 0.0f);
    const aiVector3D Zero3(0.0f, 0.0f, 0.0f);
    
    for (auto i = 0u; i < mesh->mNumVertices; ++i)
    {
        const auto vert = mesh->mVertices[i];
        Vec3F vertex(vert.x, vert.y, vert.z);
        lsMesh.Vertices.at(i) = vertex;

        const auto norm = mesh->HasNormals() ? mesh->mNormals[i] : Zero3;
        Vec3F normal(norm.x, norm.y, norm.z);
        lsMesh.Normals.at(i) = normal;

        const auto uv = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : Zero3;
        Vec3F texcoord(uv.x, uv.y, uv.z);
        lsMesh.TexCoords.at(i) = texcoord;

        const auto color = mesh->HasVertexColors(0) ? mesh->mColors[0][i] : Zero4;
        Vec4F colors(color.r, color.g, color.b, color.a);
        lsMesh.Colors.at(i) = colors;

    }

}

void Serialize::AssimpLoader::ProcessRootNode(const aiNode* node, const aiScene* scene) noexcept
{
    if (!node)
        return;

    // Finds the meshes that are in this node 
    for (auto i = 0u; i < node->mNumMeshes; ++i)
    {
        auto index = node->mMeshes[i];
        LSMesh mesh;
        ProcessMesh(scene->mMeshes[index], mesh);
        m_meshes.at(i) = mesh;
    }
    
    // Process the Children 
    for (auto i = 0u; i < node->mNumChildren; ++i)
    {
        auto child = node->mChildren[i];
        ProcessRootNode(child, scene);
    }

}

void Serialize::AssimpLoader::ProcessScene(const aiScene* scene) noexcept
{
    m_meshes.resize(scene->mNumMeshes);
    ProcessRootNode(scene->mRootNode, scene);
}