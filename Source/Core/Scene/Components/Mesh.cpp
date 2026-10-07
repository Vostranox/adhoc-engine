#include "Mesh.hpp"
#include <Event/Event.hpp>
#include <Math/Math.hpp>
#include <Utility.hpp>
#include <Vulkan/Context.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cmath>

namespace adh {
    static Vector4D Tangent(const aiMesh& mesh, std::size_t i) {
        const aiVector3D& normal{ mesh.mNormals[i] };
        if (mesh.HasTangentsAndBitangents()) {
            const aiVector3D& tangent{ mesh.mTangents[i] };
            const aiVector3D& bitangent{ mesh.mBitangents[i] };
            const float length{ tangent.Length() };
            if (std::isfinite(length) && length > 0.0001f) {
                const float side{ (normal ^ tangent) * bitangent < 0.0f ? -1.0f : 1.0f };
                return Vector4D{ tangent.x, tangent.y, tangent.z, side };
            }
        }
        const aiVector3D axis{ std::abs(normal.x) < 0.9f ? aiVector3D{ 1.0f, 0.0f, 0.0f } : aiVector3D{ 0.0f, 1.0f, 0.0f } };
        const aiVector3D tangent{ (axis - normal * (axis * normal)).NormalizeSafe() };
        return Vector4D{ tangent.x, tangent.y, tangent.z, 1.0f };
    }

    void Mesh::Load(const std::string& meshPath) {
        Assimp::Importer imp;
        auto pModel = imp.ReadFile(
            meshPath.data(),
            aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

        const aiMesh* pMesh{ pModel && pModel->HasMeshes() ? pModel->mMeshes[0] : nullptr };
        if (pMesh && pMesh->HasNormals()) {
            bufferData = Mesh::meshes[meshPath];

            if (!bufferData) {
                bufferData             = MakeShared<MeshBufferData>();
                Mesh::meshes[meshPath] = bufferData;

                bufferData->vertices.Reserve(pMesh->mNumVertices);

                for (std::size_t i{}; i != pMesh->mNumVertices; ++i) {
                    const aiVector3D uv{ pMesh->HasTextureCoords(0) ? pMesh->mTextureCoords[0][i] : aiVector3D{} };
                    bufferData->vertices.EmplaceBack(
                        Vector3D{ pMesh->mVertices[i].x, pMesh->mVertices[i].y, pMesh->mVertices[i].z },
                        Vector3D{ pMesh->mNormals[i].x, pMesh->mNormals[i].y, pMesh->mNormals[i].z },
                        Vector2D{ uv.x, uv.y },
                        Tangent(*pMesh, i));
                    bufferData->vertices2.EmplaceBack(Vector3D{ pMesh->mVertices[i].x, pMesh->mVertices[i].y, pMesh->mVertices[i].z });
                }

                bufferData->indices.Reserve(pMesh->mNumFaces * 3);
                for (std::size_t i{}; i != pMesh->mNumFaces; ++i) {
                    const auto& face = pMesh->mFaces[i];
                    bufferData->indices.EmplaceBack(face.mIndices[0]);
                    bufferData->indices.EmplaceBack(face.mIndices[1]);
                    bufferData->indices.EmplaceBack(face.mIndices[2]);
                }
                bufferData->vertex.Create(bufferData->vertices);

                bufferData->index.Create(bufferData->indices);
            }
        } else {
            std::string s{ "[" + meshPath + "] Failed to load model: " + (pModel ? "no triangles" : imp.GetErrorString()) + "\n" };
            EventBus().publish<EditorLogEvent>(EditorLogEvent::Type::eError, s.data());

            if (bufferData) {
                return;
            }
        }

        name     = meshPath.substr(meshPath.find_last_of('/') + 1);
        filePath = meshPath;
    }

    void Mesh::Load2(const char* fileName) {
        Load(vk::Context::Get()->GetDataDirectory() + "Assets/Models/" + fileName);
    }
} // namespace adh
