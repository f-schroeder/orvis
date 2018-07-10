#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "Material.hpp"
#include <assimp/scene.h>
#include "Scene.hpp"

class Mesh
{
public:
    friend Scene;

    Mesh() = default;

    std::vector<glm::vec4> vertices;
    std::vector<glm::vec4> normals;
    std::vector<glm::vec2> uvs;
    std::vector<unsigned int> indices;
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    Material material;

    Bounds bounds;

    Bounds& calculateBoundingBox();

private:
    explicit Mesh(aiMesh* assimpMesh, aiMaterial* assimpMat);

    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
};
