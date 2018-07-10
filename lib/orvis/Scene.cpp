#include "Scene.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/config.h>

#include "Util.hpp"
#include "stb/stb_image.h"

Scene::Scene(const std::experimental::filesystem::path& filename)
{
    const auto path = util::resourcesPath / filename;
    const auto pathString = path.string();

    Assimp::Importer importer;

    std::cout << "Loading model from " << filename.string() << std::endl;

    const aiScene * assimpScene = importer.ReadFile(pathString.c_str(), aiProcess_GenSmoothNormals | 
        aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_JoinIdenticalVertices | 
        aiProcess_RemoveComponent | aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS | 
        aiComponent_CAMERAS | aiComponent_LIGHTS | aiComponent_TANGENTS_AND_BITANGENTS | aiComponent_COLORS |
        aiProcess_SplitLargeMeshes | aiProcess_ImproveCacheLocality | aiProcess_RemoveRedundantMaterials |
        aiProcess_OptimizeMeshes);

    if (!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        const std::string err = importer.GetErrorString();
        throw std::runtime_error("Assimp import failed: " + err);
    }

    std::cout << "Assimp import complete. Processing Model..." << std::endl;

    // - - - M E S H E S - - -

    if (assimpScene->HasMeshes())
    {
        const auto numMeshes = assimpScene->mNumMeshes;
        m_meshes.resize(numMeshes);
        for (int i = 0; i < static_cast<int>(numMeshes); ++i)
        {
            m_meshes[i] = std::make_shared<Mesh>(assimpScene->mMeshes[i], assimpScene->mMaterials[assimpScene->mMeshes[i]->mMaterialIndex]);
        }
    }

    // - - - M O D E L   M A T R I C E S - - -

    static_assert(alignof(aiMatrix4x4) == alignof(glm::mat4) && sizeof(aiMatrix4x4) == sizeof(glm::mat4));

    std::function<void(aiNode* node, glm::mat4 trans)> traverseChildren = [this, &traverseChildren](aiNode* node, glm::mat4 trans)
    {
        // check if transformation exists
        if (std::none_of(&node->mTransformation.a1, (&node->mTransformation.d4) + 1,
            [](float f) { return std::isnan(f) || std::isinf(f); }))
        {
            // accumulate transform
            const glm::mat4 transform = reinterpret_cast<glm::mat4&>(node->mTransformation);
            trans *= transform;
        }

        // assign transformation to meshes
#pragma omp parallel for
        for (int i = 0; i < static_cast<int>(node->mNumMeshes); ++i)
        {
            m_meshes[node->mMeshes[i]]->modelMatrix = trans;
        }

        // recursively work on the child nodes
#pragma omp parallel for
        for (int i = 0; i < static_cast<int>(node->mNumChildren); ++i)
        {
            traverseChildren(node->mChildren[i], trans);
        }
    };

    const auto root = assimpScene->mRootNode;
    std::thread modelMatThread(traverseChildren, root, glm::mat4(1.0f));


    modelMatThread.join();
}
