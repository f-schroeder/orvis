#pragma once

#include <glbinding/gl/gl.h>
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

using namespace gl;

/**
 * @brief Sample counts for multisampling.
 */
enum class Samples
{
    X1  = 1 << 0,
    X2  = 1 << 1,
    X4  = 1 << 2,
    X8  = 1 << 3,
    X16 = 1 << 4
};

/**
 * @brief An OpenGL sampler object holding texture sampling data. This is to prohibit all the
 * glSetTextureParameter calls which may or may not work.
 */
class Sampler
{
public:
    Sampler();
    ~Sampler();
    Sampler(const Sampler& other);
    Sampler(Sampler&& other) noexcept;
    Sampler& operator=(const Sampler& other);
    Sampler& operator=(Sampler&& other) noexcept;

    /** @brief Sets a sampler parameter via glSamplerParameteri. */
    void set(GLenum texParam, int value);
    /** @brief Sets a sampler parameter via glSamplerParameterf. */
    void set(GLenum texParam, float value);
    /** @brief Sets a sampler parameter via glSamplerParametere. */
    void set(GLenum texParam, GLenum value);

    /** @return The sampler object ID. */
    GLuint id() const;

private:
    GLuint                                               m_samplerId;
    std::unordered_map<GLenum, std::variant<int, float, GLenum>> m_samplerParams;
};

/**
 * @brief An OpenGL texture object.
 * @details Every texture holds its own default sampler which can be accessed with
 * Texture::sampler(). It can be overridden using the overrideSampler function.
 */
class Texture
{
public:
    /**
     * @brief Textures are default constructible but invalid until reassignment or a call to
     * Texture::resize.
     */
    Texture() = default;

    /**
     * @brief Creates a 1D texture.
     * @param target The OpenGL texture target (e.g. GL_TEXTURE_1D)
     * @param format The internal texture format (e.g. GL_RGBA8, GL_RGBA32F, etc.)
     * @param size The 1D-size of the texture.
     * @param levels The number of mipmap levels for this texture. If set to -1, it will be set to
     * the maximum amount for the given size.
     */
    Texture(GLenum target, GLenum format, int size, int levels = -1);

    /**
     * @brief Creates a 2D texture.
     * @param target The OpenGL texture target (e.g. GL_TEXTURE_2D)
     * @param format The internal texture format (e.g. GL_RGBA8, GL_RGBA32F, etc.)
     * @param size The 2D-size of the texture.
     * @param levels The number of mipmap levels for this texture. If set to -1, it will be set to
     * the maximum amount for the given size.
     */
    Texture(GLenum target, GLenum format, glm::ivec2 size, int levels = -1);

    /**
     * @brief Creates a 3D texture.
     * @param target The OpenGL texture target (e.g. GL_TEXTURE_3D)
     * @param format The internal texture format (e.g. GL_RGBA8, GL_RGBA32F, etc.)
     * @param size The 3D-size of the texture.
     * @param levels The number of mipmap levels for this texture. If set to -1, it will be set to
     * the maximum amount for the given size.
     */
    Texture(GLenum target, GLenum format, glm::ivec3 size, int levels = -1);

    /**
     * @brief Creates a multisampled 2D texture.
     * @param target The OpenGL texture target (e.g. GL_TEXTURE_2D)
     * @param format The internal texture format (e.g. GL_RGBA8, GL_RGBA32F, etc.)
     * @param size The 2D-size of the texture.
     * @param samples The number of samples contained in the texture.
     * @param fixedSampleLocations Specifies whether the image will use identical sample locations
     * and the same number of samples for all texels in the image, and the sample locations will not
     * depend on the internal format or size of the image. Set to true if you want to use this
     * texture as a framebuffer attachment.
     */
    Texture(GLenum target, GLenum format, glm::ivec2 size, Samples samples,
            bool fixedSampleLocations = true);

    /**
     * @brief Creates a multisampled 3D texture.
     * @param target The OpenGL texture target (e.g. GL_TEXTURE_3D)
     * @param format The internal texture format (e.g. GL_RGBA8, GL_RGBA32F, etc.)
     * @param size The 3D-size of the texture.
     * @param samples The number of samples contained in the texture.
     * @param fixedSampleLocations Specifies whether the image will use identical sample locations
     * and the same number of samples for all texels in the image, and the sample locations will not
     * depend on the internal format or size of the image. Set to true if you want to use this
     * texture as a framebuffer attachment.
     */
    Texture(GLenum target, GLenum format, glm::ivec3 size, Samples samples,
            bool fixedSampleLocations = true);
    ~Texture();

    Texture(Texture&& other) noexcept;
    Texture(const Texture& other);
    Texture& operator=(Texture&& other) noexcept;
    Texture& operator=(const Texture& other);

    /**
     * @brief Creates a 1D texture.
     * @param target The OpenGL texture target (e.g. GL_TEXTURE_2D)
     * @param format The internal texture format (e.g. GL_RGBA8, GL_RGBA32F, etc.)
     * @param size The 1D-size of the texture.
     * @param levels The number of mipmap levels for this texture. If set to -1, it will be set to
     * the maximum amount for the given size.
     */
    void resize(GLenum target, GLenum format, int size, int levels = -1);

    /**
     * @brief Creates a 2D texture.
     * @param target The OpenGL texture target (e.g. GL_TEXTURE_2D)
     * @param format The internal texture format (e.g. GL_RGBA8, GL_RGBA32F, etc.)
     * @param size The 2D-size of the texture.
     * @param levels The number of mipmap levels for this texture. If set to -1, it will be set to
     * the maximum amount for the given size.
     */
    void resize(GLenum target, GLenum format, glm::ivec2 size, int levels = -1);

    /**
     * @brief Creates a 3D texture.
     * @param target The OpenGL texture target (e.g. GL_TEXTURE_2D)
     * @param format The internal texture format (e.g. GL_RGBA8, GL_RGBA32F, etc.)
     * @param size The 3D-size of the texture.
     * @param levels The number of mipmap levels for this texture. If set to -1, it will be set to
     * the maximum amount for the given size.
     */
    void resize(GLenum target, GLenum format, glm::ivec3 size, int levels = -1);

    /**
     * @brief Creates a multisampled 2D texture.
     * @param target The OpenGL texture target (e.g. GL_TEXTURE_2D)
     * @param format The internal texture format (e.g. GL_RGBA8, GL_RGBA32F, etc.)
     * @param size The 2D-size of the texture.
     * @param samples The number of samples contained in the texture.
     * @param fixedSampleLocations Specifies whether the image will use identical sample locations
     * and the same number of samples for all texels in the image, and the sample locations will not
     * depend on the internal format or size of the image. Set to true if you want to use this
     * texture as a framebuffer attachment.
     */
    void resize(GLenum target, GLenum format, glm::ivec2 size, Samples samples,
                bool fixedSampleLocations = true);

    /**
     * @brief Creates a multisampled 3D texture.
     * @param target The OpenGL texture target (e.g. GL_TEXTURE_3D)
     * @param format The internal texture format (e.g. GL_RGBA8, GL_RGBA32F, etc.)
     * @param size The 3D-size of the texture.
     * @param samples The number of samples contained in the texture.
     * @param fixedSampleLocations Specifies whether the image will use identical sample locations
     * and the same number of samples for all texels in the image, and the sample locations will not
     * depend on the internal format or size of the image. Set to true if you want to use this
     * texture as a framebuffer attachment.
     */
    void resize(GLenum target, GLenum format, glm::ivec3 size, Samples samples,
                bool fixedSampleLocations = true);

    /**
     * @brief Overrides the texture default sampler.
     * @param sampler A new sampler.
     */
    void overrideSampler(const std::shared_ptr<Sampler>& sampler);

    /**
     * @brief Calls set on the currently active texture sampler of this texture. This function will
     * have to recreate the sampler.
     * @param texParam The texture parameter.
     * @param value its value.
     */
    void set(GLenum texParam, int value);

    /**
     * @brief Calls set on the currently active texture sampler of this texture. This function will
     * have to recreate the sampler.
     * @param texParam The texture parameter.
     * @param value its value.
     */
    void set(GLenum texParam, float value);

    using TexParamMap = std::unordered_map<GLenum, std::variant<int, float>>;
    /**
     * @brief Sets all given parameters at once if the sampler is not overridden.
     * @param parameters A map of parameter-value pairs.
     */
    void set(const TexParamMap& parameters);

    /**
     * @return The default sampler or the overridden one, if there is one.
     */
    Sampler& sampler();

    /**
     * @return The default sampler or the overridden one, if there is one.
     */
    const Sampler& sampler() const;

    /**
     * @brief Assign pixel data to a sub range of the texture.
     * @param level The level to assign into.
     * @param offset The starting position of the assignment.
     * @param size The amount of pixels to set.
     * @param format The format of the given pixel data. (e.g. GL_RGBA)
     * @param type The pixel data type. (e.g. GL_FLOAT)
     * @param pixels The pixel data.
     */
    void assign1D(int level, int offset, int size, GLenum format, GLenum type, const void* pixels);

    /**
     * @brief Assign pixel data to the whole texture.
     * @param format The format of the given pixel data. (e.g. GL_RGBA)
     * @param type The pixel data type. (e.g. GL_FLOAT)
     * @param pixels The pixel data.
     */
    void assign1D(GLenum format, GLenum type, const void* pixels);

    /**
     * @brief Assign pixel data to a sub range of the texture.
     * @param level The level to assign into.
     * @param offset The starting position of the assignment.
     * @param size The amount of pixels to set.
     * @param format The format of the given pixel data. (e.g. GL_RGBA)
     * @param type The pixel data type. (e.g. GL_FLOAT)
     * @param pixels The pixel data.
     */
    void assign2D(int level, glm::ivec2 offset, glm::ivec2 size, GLenum format, GLenum type,
                  const void* pixels);

    /**
     * @brief Assign pixel data to the whole texture.
     * @param format The format of the given pixel data. (e.g. GL_RGBA)
     * @param type The pixel data type. (e.g. GL_FLOAT)
     * @param pixels The pixel data.
     */
    void assign2D(GLenum format, GLenum type, const void* pixels);

    /**
     * @brief Assign pixel data to a sub range of the texture.
     * @param level The level to assign into.
     * @param offset The starting position of the assignment.
     * @param size The amount of pixels to set.
     * @param format The format of the given pixel data. (e.g. GL_RGBA)
     * @param type The pixel data type. (e.g. GL_FLOAT)
     * @param pixels The pixel data.
     */
    void assign3D(int level, glm::ivec3 offset, glm::ivec3 size, GLenum format, GLenum type,
                  const void* pixels);

    /**
     * @brief Assign pixel data to the whole texture.
     * @param format The format of the given pixel data. (e.g. GL_RGBA)
     * @param type The pixel data type. (e.g. GL_FLOAT)
     * @param pixels The pixel data.
     */
    void assign3D(GLenum format, GLenum type, const void* pixels);

    /**
     * @brief Loads the data from the texture memory into a vector.
     * @tparam T The texture data type which will be stored in the vector.
     * @param format The data format (e.g. GL_RGBA)
     * @return A vector containing the pixel data.
     */
    template <typename T>
    std::vector<T> data(GLenum format, int level = 0) const;

    /**
     * @brief Generates texture mipmaps. (glGenerateTextureMipmaps)
     */
    void generateMipmaps() const;

    /**
     * @brief Binds the texture to the given binding point.
     * @param binding The binding point which can then be accessed by a GLSL layout binding
     * declaration.
     */
    void bind(GLuint binding) const;

    /**
     * @brief Binds the first mipmap level of the texture as a read-write enabled non-layered
     * image with its preset internal format.
     * @param binding The image binding.
     */
    void bindImage(GLuint binding) const;

    /**
     * @brief Binds the first mipmap level of the texture to as a non-layered image.
     * @param binding The image binding.
     * @param access The access flag for the image.
     * @param format The binding format (e.g. GL_RGBA32F for rgba32f)
     */
    void bindImage(GLuint binding, GLenum access, GLenum format) const;

    /**
     * @brief Binds the texture as an image.
     * @param binding The image binding.
     * @param level The mipmap level to bind.
     * @param layered true, if the texture is layered.
     * @param layer The texture layer to bind if the texture is layered.
     * @param access The access flag for the image.
     * @param format The binding format (e.g. GL_RGBA32F for rgba32f)
     */
    void bindImage(GLuint binding, int level, bool layered, int layer, GLenum access,
                   GLenum format) const;

    /** @return The texture id */
    GLuint id() const;

    /**
     * @brief Clears the whole texture to the given data value.
     * @param level The texture mipmap level to clear.
     * @param format The format of the given data (e.g. GL_RGBA for 4 components).
     * @param type The type of the given data (e.g. GL_FLOAT for float data).
     * @param data The clear data (a pointer to a 1 to 4 component vector.
     */
    void clear(GLint level, GLenum format, GLenum type, const void* data) const;

    /**
     * @brief Clears the whole texture to the given data value.
     * @tparam L The glm::vec type length.
     * @tparam T The glm::vec value type.
     * @tparam Q The glm::vec qualifier.
     * @param level The texture mipmap level to clear.
     * @param data The data vector.
     */
    template <glm::length_t L, typename T, glm::qualifier Q>
    void clear(GLint level, const glm::vec<L, T, Q>& data) const;

    /**
     * @brief Clears the whole first level of the texture to the given data value.
     * @param level The texture mipmap level to clear.
     * @param format The format of the given data (e.g. GL_RGBA for 4 components).
     * @param type The type of the given data (e.g. GL_FLOAT for float data).
     * @param data The clear data (a pointer to a 1 to 4 component vector.
     */
    void clear(GLenum format, GLenum type, const void* data) const;

    /**
     * @brief Clears the whole texture to the given data value.
     * @tparam L The glm::vec type length.
     * @tparam T The glm::vec value type.
     * @tparam Q The glm::vec qualifier.
     * @param data The data vector.
     */
    template <glm::length_t L, typename T, glm::qualifier Q>
    void clear(const glm::vec<L, T, Q>& data) const;

    /**
     * @return The generated bindless texture handle or 0 if bindless textures are not supported.
     */
    GLuint64 handle() const;

    /**
     * @brief Retrieves and stores the image handle for the texture part referenced by the given
     * parameters.
     * @param level The texture mipmap level.
     * @param layered True for layered (or 3D-) texture types.
     * @param layer The texture layer (ignored if the texture is not layered).
     * @param access The image access.
     * @param format The image access format.
     * @return The bindless image handle or 0 if bindless images are not supported.
     */
    GLuint64 imageHandle(int level, bool layered, int layer, GLenum access, GLenum format);

    /**
     * @brief Gets the OpenGL target of the texture.
     * @return The target (e.g. GL_TEXTURE_2D).
     */
    GLenum getTarget() const;

    /**
     * @brief Gets the OpenGL format of the texture.
     * @return The format (e.g. GL_RGBA32F).
     */
    GLenum getFormat() const;

    /**
     * @brief Gets the size of the texture in pixels.
     * @return The size. For 1D and 2D textures only x or xy are populated.
     */
    glm::ivec3 getSize() const;

private:
    template <typename T, typename N>
    using UMap       = std::unordered_map<T, N>;
    using HandleTree = UMap<int, UMap<bool, UMap<int, UMap<GLenum, UMap<GLenum, GLuint64>>>>>;

    explicit Texture(GLenum target);
    void generateHandle();

    GLuint                   m_textureId     = GL_INVALID_INDEX;
    GLenum                   m_target        = GL_INVALID_ENUM;
    GLenum                   m_format        = GL_INVALID_ENUM;
    GLuint64                 m_textureHandle = 0;
    HandleTree               m_imageHandleTree;
    glm::ivec3               m_size                 = {0, 0, 0};
    int                      m_levels               = 0;
    Samples                  m_samples              = Samples(0);
    bool                     m_fixedSampleLocations = false;
    mutable bool             m_hasMipmaps           = false;
    std::shared_ptr<Sampler> m_overrideSampler      = nullptr;
    Sampler                  m_defaultSampler;
};

#include "Texture.inl"
