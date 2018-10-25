#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "Material.hpp"
#include <assimp/scene.h>
#include "Scene.hpp"
#include "Vertex.hpp"
#include <tiny_obj_loader.h>

class Mesh
{
public:
    friend class Scene;

    Mesh() = default;

	unsigned int startIndex = 0;
	unsigned int endIndex = 0;

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    Material material;

    /** @brief The _untransformed_ bounding box. */
    Bounds bounds;

    /** @return True if the mesh has a (partially) transparent material */
    bool isTransparent() const;

private:
    explicit Mesh(unsigned int startIndex, unsigned int endIndex/*, tinyobj::material_t& material*/);

	void setBoundingBox(const Bounds& bounds);

    std::unordered_map<aiTextureType, std::shared_ptr<Texture>> m_textures;

    void copyToAlpha(const std::filesystem::path& src, const std::shared_ptr<Texture>& dst) const;
    std::shared_ptr<Texture> generateNormalFromHeight(const std::filesystem::path& src) const;

    bool m_transparent = false;
};
