#pragma once
#include "Bounds.hpp"
#include "Light.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"

class Light; // forward declaration

class Scene
{
public:
    void render();
    const Bounds& getBoundingBox();

private:
    std::vector<std::shared_ptr<Light>> m_lights;
    std::vector<std::shared_ptr<Mesh>> m_meshes;
    std::shared_ptr<Camera> m_camera;
};
