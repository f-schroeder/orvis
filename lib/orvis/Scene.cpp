#include "Scene.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/config.h>

#include <numeric>
#include <execution>

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
            m_meshes[i] = std::shared_ptr<Mesh>(new Mesh(assimpScene->mMeshes[i], assimpScene->mMaterials[assimpScene->mMeshes[i]->mMaterialIndex], path));
        }
    }

    // - - - M O D E L   M A T R I C E S - - -

    static_assert(sizeof(aiMatrix4x4) == sizeof(glm::mat4));

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
    std::thread modelMatThread([&]()
    {
        traverseChildren(root, glm::mat4(1.0f));
        calculateBoundingBox();
    });

    updateMultiDrawBuffers();

    modelMatThread.join();
    updateModelMatrices();
    updateBoundingBoxBuffer();

    std::cout << "Loading complete: " << filename.string() << std::endl;

    m_cullingProgram.attachNew(GL_COMPUTE_SHADER, ShaderFile::load("compute/viewFrustumCulling.comp"));
}

void Scene::render(const Program& program) const
{
    // CULLING
    m_cullingProgram.use();
    m_indirectDrawBuffer.bind(GL_SHADER_STORAGE_BUFFER, BufferBinding::indirectDraw);
    m_bBoxBuffer.bind(GL_SHADER_STORAGE_BUFFER, BufferBinding::boundingBoxes);
    m_modelMatBuffer.bind(GL_SHADER_STORAGE_BUFFER, BufferBinding::modelMatrices);
    m_camera->uploadToGpu();
    
    glDispatchCompute(static_cast<GLuint>(glm::ceil(m_indirectDrawBuffer.size() / 64.0f)), 1, 1);
    glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    // DRAW
    program.use();
    m_multiDrawVao.bind();
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectDrawBuffer.id());
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(m_indirectDrawBuffer.size()), 0);
}

const Bounds& Scene::calculateBoundingBox()
{
    struct Reduction
    {
        Bounds operator()(Bounds b, const std::shared_ptr<Mesh>& x) const {
            return b +
                Bounds(x->modelMatrix * glm::vec4(x->bounds.min, 1.0f), x->modelMatrix * glm::vec4(x->bounds.max, 1.0f));
        }

        Bounds operator()(const std::shared_ptr<Mesh>& x, Bounds b) const {
            return b +
                Bounds(x->modelMatrix * glm::vec4(x->bounds.min, 1.0f), x->modelMatrix * glm::vec4(x->bounds.max, 1.0f));
        }

        Bounds operator()(const std::shared_ptr<Mesh>& a, const std::shared_ptr<Mesh>& b) const
        {
            Bounds bounds;
            bounds = bounds + Bounds(a->modelMatrix * glm::vec4(a->bounds.min, 1.0f), a->modelMatrix * glm::vec4(a->bounds.max, 1.0f));
            return bounds + Bounds(b->modelMatrix * glm::vec4(b->bounds.min, 1.0f), b->modelMatrix * glm::vec4(b->bounds.max, 1.0f));
        }

        Bounds operator()(Bounds b, const Bounds& x) const { return b + x; }
    };

    bounds = std::reduce(std::execution::par_unseq, m_meshes.begin(), m_meshes.end(), Bounds(), Reduction());
    return bounds;
}

void Scene::updateModelMatrices()
{
    std::vector<glm::mat4> modelMatrices(m_meshes.size());

#pragma omp parallel for
    for (int i = 0; i < static_cast<int>(modelMatrices.size()); ++i)
        modelMatrices[i] = m_meshes[i]->modelMatrix;

    m_modelMatBuffer.resize(modelMatrices.size(), GL_DYNAMIC_STORAGE_BIT);
    m_modelMatBuffer.assign(modelMatrices);
}

void Scene::updateBoundingBoxBuffer()
{
    std::vector<Bounds> boundingBoxes(m_meshes.size());

#pragma omp parallel for
    for (int i = 0; i < static_cast<int>(boundingBoxes.size()); ++i)
        boundingBoxes[i] = m_meshes[i]->bounds;

    m_bBoxBuffer.resize(boundingBoxes.size(), GL_DYNAMIC_STORAGE_BIT);
    m_bBoxBuffer.assign(boundingBoxes);
}

void Scene::addMesh(const std::shared_ptr<Mesh>& mesh)
{
    m_meshes.push_back(mesh);
    updateModelMatrices();
    updateMultiDrawBuffers();
}

void Scene::setCamera(const std::shared_ptr<Camera>& camera)
{
    m_camera = camera;
}

void Scene::updateMultiDrawBuffers()
{
    std::vector<GLuint> allIndices;
    std::vector<glm::vec4> allVertices;
    std::vector<glm::vec4> allNormals;
    std::vector<glm::vec2> allUVs;

    std::vector<IndirectDrawCommand> indirectDrawParams;

    GLuint start = 0;
    GLuint baseVertexOffset = 0;
    for (const auto& mesh : m_meshes)
    {
        allIndices.insert(allIndices.end(), mesh->indices.begin(), mesh->indices.end());
        allVertices.insert(allVertices.end(), mesh->vertices.begin(), mesh->vertices.end());
        allNormals.insert(allNormals.end(), mesh->normals.begin(), mesh->normals.end());
        allUVs.insert(allUVs.end(), mesh->uvs.begin(), mesh->uvs.end());

        const auto count = static_cast<GLuint>(mesh->indices.size());

        indirectDrawParams.push_back({ count, 1U, start, baseVertexOffset, 0U });

        start += count;
        baseVertexOffset += static_cast<GLuint>(mesh->vertices.size());
    }

    m_multiDrawIndexBuffer.resize(allIndices.size(), GL_DYNAMIC_STORAGE_BIT);
    m_multiDrawIndexBuffer.assign(allIndices);
    m_multiDrawVertexBuffer.resize(allVertices.size(), GL_DYNAMIC_STORAGE_BIT);
    m_multiDrawVertexBuffer.assign(allVertices);
    m_multiDrawNormalBuffer.resize(allNormals.size(), GL_DYNAMIC_STORAGE_BIT);
    m_multiDrawNormalBuffer.assign(allNormals);
    m_multiDrawUVBuffer.resize(allUVs.size(), GL_DYNAMIC_STORAGE_BIT);
    m_multiDrawUVBuffer.assign(allUVs);
    m_indirectDrawBuffer.resize(indirectDrawParams.size(), GL_DYNAMIC_STORAGE_BIT);
    m_indirectDrawBuffer.assign(indirectDrawParams);

    m_multiDrawVao.setVertexBuffer(m_multiDrawVertexBuffer, VertexAttributeBinding::vertices);
    m_multiDrawVao.setVertexBuffer(m_multiDrawNormalBuffer, VertexAttributeBinding::normals);
    m_multiDrawVao.setVertexBuffer(m_multiDrawUVBuffer, VertexAttributeBinding::texCoords);
    m_multiDrawVao.setElementBuffer(m_multiDrawIndexBuffer);
}
