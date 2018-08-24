#include "Cubemap.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Cubemap::Cubemap() : m_skyboxShader(std::make_shared<Shader>(GL_FRAGMENT_SHADER, ShaderFile::load("fragment/skyBox.frag"))), m_screenFiller(m_skyboxShader)
{
}

void Cubemap::generateCubemap(const std::filesystem::path& path, const std::string& extension,
    const std::array<std::string, 6>& faces)
{
    generateCubemap(path, extension, faces[0], faces[1], faces[2], faces[3], faces[4], faces[5]);
}

void Cubemap::generateCubemap(const std::filesystem::path& cubeMapSourcePath,
    const std::string& extension, const std::string& posX,
    const std::string& negX, const std::string& posY,
    const std::string& negY, const std::string& posZ,
    const std::string& negZ)
{
    int width, height;

    assert(extension.length() > 1 && extension[0] == '.' && "Extension must start with a dot (.)");

    struct deleter
    {
        void operator()(float* f) const { stbi_image_free(f); }
        void operator()(unsigned char* f) const { stbi_image_free(f); }
    };

    if (stbi_is_hdr((cubeMapSourcePath / (posX + extension)).string().c_str()))
    {
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

        for (int i = 0; i < 6; ++i)
            m_texture.assign3D(0, { 0, 0, i }, { width, height, 1 }, GL_RGB, GL_FLOAT, data[i].get());
    }
    else
    {
        using Data = std::unique_ptr<unsigned char, deleter>;

        std::array<Data, 6> data{
            Data(stbi_load((cubeMapSourcePath / (posX + extension)).string().c_str(),
            &width,
            &height,
            nullptr,
            STBI_rgb)),
            Data(stbi_load((cubeMapSourcePath / (negX + extension)).string().c_str(),
            &width,
            &height,
            nullptr,
            STBI_rgb)),
            Data(stbi_load((cubeMapSourcePath / (posY + extension)).string().c_str(),
            &width,
            &height,
            nullptr,
            STBI_rgb)),
            Data(stbi_load((cubeMapSourcePath / (negY + extension)).string().c_str(),
            &width,
            &height,
            nullptr,
            STBI_rgb)),
            Data(stbi_load((cubeMapSourcePath / (posZ + extension)).string().c_str(),
            &width,
            &height,
            nullptr,
            STBI_rgb)),
            Data(stbi_load((cubeMapSourcePath / (negZ + extension)).string().c_str(),
            &width,
            &height,
            nullptr,
            STBI_rgb)),
        };

        m_texture.resize(GL_TEXTURE_CUBE_MAP, GL_R11F_G11F_B10F, glm::ivec2(width, height));

        for (int i = 0; i < 6; ++i)
            m_texture.assign3D(0, { 0, 0, i }, { width, height, 1 }, GL_RGB, GL_UNSIGNED_BYTE, data[i].get());
    }

    m_texture.generateMipmaps();
}

void Cubemap::renderAsSkybox(const std::shared_ptr<Camera>& camera)
{
    m_screenFiller.setCamera(camera);
    m_texture.bind(TextureBinding::skybox);
    m_screenFiller.render();
}

const Texture& Cubemap::getTexture() const { return m_texture; }
