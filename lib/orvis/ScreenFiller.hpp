#pragma once

#include <glbinding/gl/gl.h>
#include "Createable.hpp"
#include "Shader.hpp"
#include "VertexArray.hpp"
#include "Camera.hpp"
using namespace gl;

/**
 * @brief Class for rendering a screen filling triangle (SFT). 
 */
class ScreenFiller : Createable<ScreenFiller>
{
public:
    /**
     * @brief Constructs a new ScreenFiller object
     * @param fragmentShader The fragment shader that is used to render the SFT
     * @param camera The camera that is used to render the SFT. Needed for calculation of rayDirection in vertex shader
     */
    ScreenFiller(const std::shared_ptr<Shader>& fragmentShader, const std::shared_ptr<Camera>& camera = nullptr);

    /**
     * @brief Renders the SFT using the attached fragment shader
     */
    void render() const;

    /**
     * @brief Sets the camera used for rendering
     */
    void setCamera(const std::shared_ptr<Camera>& camera);

private:
    Program m_shaderProgram;
    VertexArray m_vao;
    std::shared_ptr<Camera> m_camera = nullptr;
};
