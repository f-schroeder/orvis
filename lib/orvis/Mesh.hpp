#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "Material.hpp"
#include <assimp/scene.h>
#include "Scene.hpp"

class Mesh
{
public:
    friend class Scene;

    Mesh() = default;

    std::vector<glm::vec4> vertices;
    std::vector<glm::vec4> normals;
    std::vector<glm::vec2> uvs;
    std::vector<unsigned int> indices;

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    Material material;

    /** @brief The _untransformed_ bounding box. */
    Bounds bounds;

    /** @brief Calculates the untransformed bounding box. 
     * Only has to be called if the vertices are changed.
     */
    Bounds& calculateBoundingBox();

    /** @return True if the mesh has a (partially) transparent material */
    bool isTransparent() const;

private:
    explicit Mesh(aiMesh* assimpMesh, aiMaterial* assimpMat, const std::filesystem::path& rootPath);

    std::unordered_map<aiTextureType, std::shared_ptr<Texture>> m_textures;

    void copyToAlpha(const std::filesystem::path& src, const std::shared_ptr<Texture>& dst) const;
    std::shared_ptr<Texture> generateNormalFromHeight(const std::filesystem::path& src) const;

    bool m_transparent = false;
};
