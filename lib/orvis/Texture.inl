#pragma once

#include <glm/ext.hpp>

template <typename T>
std::vector<T> Texture::data(GLenum format, int level) const
{
    GLenum type;
    if constexpr(std::is_same_v<T, float>)
        type = GL_FLOAT;
    else if constexpr(std::is_same_v<T, int32_t>)
        type = GL_INT;
    else if constexpr(std::is_same_v<T, int16_t>)
        type = GL_SHORT;
    else if constexpr(std::is_same_v<T, int8_t>)
        type = GL_BYTE;
    else if constexpr(std::is_same_v<T, uint32_t>)
        type = GL_UNSIGNED_INT;
    else if constexpr(std::is_same_v<T, uint16_t>)
        type = GL_UNSIGNED_SHORT;
    else if constexpr(std::is_same_v<T, uint8_t>)
        type = GL_UNSIGNED_BYTE;

    size_t components = 1;
    switch(format)
    {
    case GL_RED:
        components = 1;
        break;
    case GL_RG:
        components = 2;
        break;
    case GL_RGB:
        components = 3;
        break;
    case GL_RGBA:
        components = 4;
        break;
    }

    const glm::ivec3 dim = m_size;
    std::vector<T>   pixels(dim.x * dim.y * dim.z * components);
    if(glGetTextureImage)
        glGetTextureImage(m_textureId,
                          level,
                          format,
                          type,
                          static_cast<GLsizei>(pixels.size() * sizeof(T)),
                          pixels.data());
    else
    {
        glBindTexture(m_target, m_textureId);
        glGetTexImage(m_target, level, format, type, pixels.data());
        glBindTexture(m_target, 0);
    }
    return pixels;
}

template <glm::length_t L, typename T, glm::qualifier Q>
void Texture::clear(const glm::vec<L, T, Q>& data) const
{
    clear(0, data);
};

template <glm::length_t L, typename T, glm::qualifier Q>
void Texture::clear(GLint level, const glm::vec<L, T, Q>& data) const
{
    GLenum format;
    if constexpr(L == 1)
        format = GL_RED;
    else if constexpr(L == 2)
        format = GL_RG;
    else if constexpr(L == 3)
        format = GL_RGB;
    else if constexpr(L == 4)
        format = GL_RGBA;

    GLenum type;
    if constexpr(std::is_same_v<T, glm::f32>)
        type = GL_FLOAT;
    else if constexpr(std::is_same_v<T, glm::u32>)
        type = GL_UNSIGNED_INT;
    else if constexpr(std::is_same_v<T, glm::u16>)
        type = GL_UNSIGNED_SHORT;
    else if constexpr(std::is_same_v<T, glm::u8>)
        type = GL_UNSIGNED_BYTE;
    else if constexpr(std::is_same_v<T, glm::i32>)
        type = GL_INT;
    else if constexpr(std::is_same_v<T, glm::i16>)
        type = GL_SHORT;
    else if constexpr(std::is_same_v<T, glm::i8>)
        type = GL_BYTE;

    clear(level, format, type, &data[0]);
};
