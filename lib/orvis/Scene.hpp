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

    /** @brief Performs GPU view frustum culling and afterwards draws the scene indirectly. 
     * @param program The Shader program that is used to render the scene.
     * @param overwriteCameraBuffer If true, overwites the camera buffer with data from the attached camera-object before rendering
     */
    void render(const Program& program, bool overwriteCameraBuffer = true) const;

    /** @brief Calculates the bounding box around all transformed meshes.
    * Only has to be called if the bounds or the model-matrix of any mesh is changed.
    * If a camera is set, also updates the camera speed accordingly.
    */
    const Bounds& calculateBoundingBox();

    /** @brief Fetches all model-matrices from all meshes and uploads them to the GPU. */
    void updateModelMatrices();

    /** @brief Fetches all bounding boxes from all meshes and uploads them to the GPU. */
    void updateBoundingBoxBuffer();

    /** @brief Fetches all materials from all meshes and uploads them to the GPU. */
    void updateMaterialBuffer();

    /** @brief Uploads all lights to the GPU. */
    void updateLightBuffer();

    /** @brief Uploads all lights to the GPU. */
    void updateShadowMaps();

    /** @brief Adds a mesh to the scene and updates all mesh related buffers */
    void addMesh(const std::shared_ptr<Mesh>& mesh);

    /** @brief Adds a light to the scene and updates the light buffer */
    void addLight(const std::shared_ptr<Light>& light);

    /** @brief Sets the camera and adjusts its speed to match the scene dimansions. */
    void setCamera(const std::shared_ptr<Camera>& camera);
    std::shared_ptr<Camera> getCamera() const;

    /** @return The list of all attached meshes. */
    const std::deque<std::shared_ptr<Mesh>>& getMeshes() const;

    /** @brief Reorders the meshes according to their transparencies (transparent objects are rendered last). 
     * Should be called if any opacity issues occur.
     */
    void reorderMeshes();

private:
    std::vector<std::shared_ptr<Light>> m_lights;
    std::deque<std::shared_ptr<Mesh>> m_meshes;
    std::shared_ptr<Camera> m_camera;

    Buffer<glm::mat4> m_modelMatBuffer;
    Buffer<Bounds> m_bBoxBuffer;
    Buffer<Material> m_materialBuffer;

    Buffer<GLuint> m_multiDrawIndexBuffer;
    Buffer<glm::vec4> m_multiDrawVertexBuffer;
    Buffer<glm::vec4> m_multiDrawNormalBuffer;
    Buffer<glm::vec2> m_multiDrawUVBuffer;
    VertexArray m_multiDrawVao;

    Buffer<IndirectDrawCommand> m_indirectDrawBuffer;

    Buffer<Light> m_lightBuffer;
    Buffer<int> m_lightIndexBuffer;

    Program m_cullingProgram;

    void updateMultiDrawBuffers();
};