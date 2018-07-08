#include "ScreenFiller.hpp"

ScreenFiller::ScreenFiller(const std::shared_ptr<Shader>& fragmentShader, const std::shared_ptr<Camera>& camera) : m_camera(camera)
{
    m_shaderProgram.attachNew(GL_VERTEX_SHADER, ShaderFile::load("vertex/screenFilling.vert"));

    assert(fragmentShader->type() == GL_FRAGMENT_SHADER && "fragmentShader must be of type GL_FRAGMENT_SHADER!");

    m_shaderProgram.attach(fragmentShader);
}

void ScreenFiller::render() const
{
    m_camera->uploadToGpu();
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    m_vao.bind();
    m_shaderProgram.use();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

void ScreenFiller::setCamera(const std::shared_ptr<Camera>& camera)
{
    m_camera = camera;
}
