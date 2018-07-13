#pragma once

#include "Bounds.hpp"
#include "Light.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"
#include "VertexArray.hpp"

// forward declarations
class Light;
class Mesh;
class Light;

struct IndirectDrawCommand
{
    GLuint count;
    GLuint instanceCount;
    GLuint firstIndex;
    GLuint baseVertex;
    GLuint baseInstance;
};

class Scene
{
public:
    Scene(const std::experimental::filesystem::path& filename);

    /** @brief The bounding box around all _transformed_ meshes. */
    Bounds bounds;

    void render(const Program& program) const;

    /** @brief Calculates the bounding box around all transformed meshes.
    * Only has to be called if the bounds or the model-matrix of any mesh is changed.
    */
    const Bounds& calculateBoundingBox();

    /** @brief Fetches all model-matrices from all meshes and uploads them to the GPU. */
    void updateModelMatrices();

    /** @brief Fetches all bounding boxes from all meshes and uploads them to the GPU. */
    void updateBoundingBoxBuffer();

    void addMesh(const std::shared_ptr<Mesh>& mesh);

    void setCamera(const std::shared_ptr<Camera>& camera);

private:
    std::vector<std::shared_ptr<Light>> m_lights;
    std::vector<std::shared_ptr<Mesh>> m_meshes;
    std::shared_ptr<Camera> m_camera;

    Buffer<glm::mat4> m_modelMatBuffer;
    Buffer<Bounds> m_bBoxBuffer;

    Buffer<GLuint> m_multiDrawIndexBuffer;
    Buffer<glm::vec4> m_multiDrawVertexBuffer;
    Buffer<glm::vec4> m_multiDrawNormalBuffer;
    Buffer<glm::vec2> m_multiDrawUVBuffer;
    VertexArray m_multiDrawVao;

    Buffer<IndirectDrawCommand> m_indirectDrawBuffer;

    Program m_cullingProgram;

    void updateMultiDrawBuffers();
};