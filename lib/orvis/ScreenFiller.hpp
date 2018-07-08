#pragma once

#include <glbinding/gl/gl.h>
#include "Createable.hpp"
#include "Shader.hpp"
#include "VertexArray.hpp"
#include <glm/glm.hpp>
#include "Camera.hpp"
using namespace gl;

class ScreenFiller : Createable<ScreenFiller>
{
public:
    ScreenFiller(const std::shared_ptr<Shader>& fragmentShader, const std::shared_ptr<Camera>& camera = nullptr);

    void render() const;

    void setCamera(const std::shared_ptr<Camera>& camera);

private:
    Program m_shaderProgram;
    VertexArray m_vao;
    std::shared_ptr<Camera> m_camera = nullptr;
};
