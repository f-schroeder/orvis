#include "Cubemap.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

SkyBox::SkyBox()
{
    m_shaderProgram.attachNew(GL_VERTEX_SHADER, ShaderFile::load("skyBox.vert"));
    m_shaderProgram.attachNew(GL_FRAGMENT_SHADER, ShaderFile::load("skyBox.frag"));
    m_shaderProgram.link();
    m_shaderProgram.use();
    m_viewProjLoc = glGetUniformLocation(m_shaderProgram.id(), "invProjView");
}

void SkyBox::generateCubeMap(const std::experimental::filesystem::path& path, const std::string& extension,
                             const std::array<std::string, 6>& faces)
{
    generateCubeMap(path, extension, faces[0], faces[1], faces[2], faces[3], faces[4], faces[5]);
}

void SkyBox::generateCubeMap(const std::experimental::filesystem::path& cubeMapSourcePath,
                             const std::string& extension, const std::string& posX,
                             const std::string& negX, const std::string& posY,
                             const std::string& negY, const std::string& posZ,
                             const std::string& negZ)
{
    int width, height;

    assert(extension.length() > 1 && extension[0] == '.' && "Extension must start with a dot (.)");

    struct deleter
    {
        void operator()(float* f) { stbi_image_free(f); }
    };
    using Data = std::unique_ptr<float, deleter>;

    std::array<Data, 6> data{
            Data(stbi_loadf((cubeMapSourcePath / (posX + extension)).string().c_str(),
                            &width,
                            &height,
                            nullptr,
                            STBI_rgb)),
            Data(stbi_loadf((cubeMapSourcePath / (negX + extension)).string().c_str(),
                            &width,
                            &height,
                            nullptr,
                            STBI_rgb)),
            Data(stbi_loadf((cubeMapSourcePath / (posY + extension)).string().c_str(),
                            &width,
                            &height,
                            nullptr,
                            STBI_rgb)),
            Data(stbi_loadf((cubeMapSourcePath / (negY + extension)).string().c_str(),
                            &width,
                            &height,
                            nullptr,
                            STBI_rgb)),
            Data(stbi_loadf((cubeMapSourcePath / (posZ + extension)).string().c_str(),
                            &width,
                            &height,
                            nullptr,
                            STBI_rgb)),
            Data(stbi_loadf((cubeMapSourcePath / (negZ + extension)).string().c_str(),
                            &width,
                            &height,
                            nullptr,
                            STBI_rgb)),
    };

    m_texture.resize(GL_TEXTURE_CUBE_MAP, GL_R11F_G11F_B10F, glm::ivec2(width, height));

    int version;
    glGetIntegerv(GL_MINOR_VERSION, &version);
    if(version >= 5)
    {
        for(int i = 0; i < 6; ++i)
            m_texture.assign3D(0, {0, 0, i}, {width, height, 1}, GL_RGB, GL_FLOAT, data[i].get());
    }
    else
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture.id());

        for(int i = 0; i < 6; ++i)
            glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                            0,
                            0,
                            0,
                            width,
                            height,
                            GL_RGB,
                            GL_FLOAT,
                            data[i].get());
    }

    m_texture.generateMipmaps();
}

void SkyBox::render(const glm::mat4& viewMatrix, const glm::mat4& projMatrix)
{
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    m_vao.bind();
    m_shaderProgram.use();
    glm::mat4 cameraRotMat = glm::mat4(glm::mat3(viewMatrix));
    glProgramUniformMatrix4fv(m_shaderProgram.id(),
                              m_viewProjLoc,
                              1,
                              GL_FALSE,
                              glm::value_ptr(glm::inverse(projMatrix * cameraRotMat)));
    m_texture.bind(0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
}

const Texture& SkyBox::getTexture() const { return m_texture; }
