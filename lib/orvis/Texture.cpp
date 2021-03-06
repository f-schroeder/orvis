#include "Texture.hpp"

#include "Util.hpp"
#include "stb/stb_image.h"

Sampler::Sampler() : m_samplerId(glCreateSamplerRAII())
{
    // Set some default parameters for the sampler
    set(GL_TEXTURE_CUBE_MAP_SEAMLESS, true);
    set(GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
    set(GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    set(GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    set(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    set(GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.f);
}
Sampler::~Sampler()
{
	// done by RAII
}

Sampler::Sampler(const Sampler& other) : m_samplerId(glCreateSamplerRAII())
{
    for (const auto& var : other.m_samplerParams)
    {
        if (std::holds_alternative<int>(var.second))
            set(var.first, std::get<int>(var.second));
        else if (std::holds_alternative<float>(var.second))
            set(var.first, std::get<float>(var.second));
        else if (std::holds_alternative<GLenum>(var.second))
            set(var.first, std::get<GLenum>(var.second));
    }
}

Sampler::Sampler(Sampler&& other) noexcept
{
    m_samplerId = std::move(other.m_samplerId);
    m_samplerParams = std::move(other.m_samplerParams);
}

Sampler& Sampler::operator=(const Sampler& other)
{
	m_samplerId.reset();
	m_samplerId = glCreateSamplerRAII();

    for (const auto& var : other.m_samplerParams)
    {
        if (std::holds_alternative<int>(var.second))
            set(var.first, std::get<int>(var.second));
        else if (std::holds_alternative<float>(var.second))
            set(var.first, std::get<float>(var.second));
        else if (std::holds_alternative<GLenum>(var.second))
            set(var.first, std::get<GLenum>(var.second));
    }
    return *this;
}

Sampler& Sampler::operator=(Sampler&& other) noexcept
{
	m_samplerId.reset();
    m_samplerId = std::move(other.m_samplerId);
    m_samplerParams = std::move(other.m_samplerParams);
    return *this;
}

void Sampler::set(GLenum texParam, int value)
{
    m_samplerParams[texParam] = value;
    glSamplerParameteri(*m_samplerId, texParam, value);
}
void Sampler::set(GLenum texParam, float value)
{
    m_samplerParams[texParam] = value;
    glSamplerParameterf(*m_samplerId, texParam, value);
}
void Sampler::set(GLenum texParam, GLenum value)
{
    m_samplerParams[texParam] = value;
    glSamplerParametere(*m_samplerId, texParam, value);
}

Texture::Texture(GLenum target)
    : m_textureId(glCreateTextureRAII(target)), m_target(target)
{
}

Texture::Texture(GLenum target, GLenum format, int size, int levels)
    : Texture(target)
{
    m_format = format;
    m_size = { size, 1, 1 };
    m_levels = levels == -1 ? static_cast<int>(floor(log2(size)) + 1) : levels;
    glTextureStorage1D(*m_textureId, m_levels, m_format, size);

    util::getGlError(__LINE__, __FUNCTION__);

    generateHandle();
}

Texture::Texture(GLenum target, GLenum format, glm::ivec2 size, int levels)
    : Texture(target)
{

    m_format = format;
    m_size = { size, 1 };
    m_levels = levels == -1 ? static_cast<int>(glm::floor(std::log2(glm::max(size.x, size.y))) + 1)
        : levels;

    glTextureStorage2D(*m_textureId, m_levels, m_format, m_size.x, m_size.y);

    util::getGlError(__LINE__, __FUNCTION__);

    generateHandle();
}

Texture::Texture(const std::filesystem::path& filename, unsigned int channels, int levels)
    :Texture(GL_TEXTURE_2D)
{
    const bool isHdr = stbi_is_hdr(filename.string().c_str());

    GLenum format, internalFormat;
    switch (channels)
    {
    case 1:
        format = GL_RED;
        internalFormat = GL_R16F;// isHdr ? GL_R16F : GL_R8;
        break;
    case 2:
        format = GL_RG;
        internalFormat = GL_RG16F;// isHdr ? GL_RG16F : GL_RG8;
        break;
    case 3:
        format = GL_RGB;
        internalFormat = GL_RGB16F;// isHdr ? GL_RGB16F : GL_RGB8;
        break;
    case 4:
        format = GL_RGBA;
        internalFormat = GL_RGBA16F;// isHdr ? GL_RGBA16F : GL_RGBA8;
        break;
    default:
        throw std::runtime_error("Tried to load texture with invalid number of channels (" + std::to_string(channels) + ")");
    }

    int imageWidth, imageHeight, numChannels;

    stbi_set_flip_vertically_on_load(true);

    stbi_info(filename.string().c_str(), &imageWidth, &imageHeight, &numChannels);

    resize(GL_TEXTURE_2D, internalFormat, { imageWidth, imageHeight });

    util::getGlError(__LINE__, __FUNCTION__);

    if (isHdr)
    {
        const auto img = stbi_loadf(filename.string().c_str(), &imageWidth, &imageHeight, &numChannels, channels);
        assign2D(format, GL_FLOAT, img);
        stbi_image_free(img);
    }
    else
    {
        const auto img = stbi_load(filename.string().c_str(), &imageWidth, &imageHeight, &numChannels, channels);
        assign2D(format, GL_UNSIGNED_BYTE, img);
        stbi_image_free(img);
    }

    util::getGlError(__LINE__, __FUNCTION__);

    generateMipmaps();
    generateHandle();

    util::getGlError(__LINE__, __FUNCTION__);
}

Texture::Texture(GLenum target, GLenum format, glm::ivec3 size, int levels)
    : Texture(target)
{
    m_format = format;
    m_size = size;
    m_levels = levels == -1
        ? static_cast<int>(
            floor(std::log2(glm::max(glm::max(size.x, size.z), size.y))) + 1)
        : levels;
    glTextureStorage3D(
        *m_textureId, m_levels, m_format, m_size.x, m_size.y, m_size.z);

    util::getGlError(__LINE__, __FUNCTION__);

    generateHandle();
}

Texture::Texture(GLenum target, GLenum format, glm::ivec2 size, Samples samples,
    bool fixedSampleLocations)
    : Texture(target)
{
    m_format = format;
    m_size = { size, 1 };
    m_levels = 1;
    m_samples = samples;
    m_fixedSampleLocations = fixedSampleLocations;
    glTextureStorage2DMultisample(*m_textureId,
        static_cast<int>(m_samples),
        m_format,
        m_size.x,
        m_size.y,
        m_fixedSampleLocations);

    util::getGlError(__LINE__, __FUNCTION__);

    generateHandle();
}

Texture::Texture(GLenum target, GLenum format, glm::ivec3 size, Samples samples,
    bool fixedSampleLocations)
    : Texture(target)
{
    m_format = format;
    m_size = size;
    m_levels = 1;
    m_samples = samples;
    m_fixedSampleLocations = fixedSampleLocations;
    glTextureStorage3DMultisample(*m_textureId,
        static_cast<int>(m_samples),
        m_format,
        m_size.x,
        m_size.y,
        m_size.z,
        m_fixedSampleLocations);

    util::getGlError(__LINE__, __FUNCTION__);

    generateHandle();
}

Texture::~Texture()
{
    // if(glMakeImageHandleNonResidentARB)
    //    for(auto&& m0 : m_imageHandleTree)
    //        for(auto&& m1 : m0.second)
    //            for(auto&& m2 : m1.second)
    //                for(auto&& m3 : m2.second)
    //                    for(auto&& m4 : m3.second)
    //                        if(glIsImageHandleResidentARB(m4.second))
    //                            glMakeImageHandleNonResidentARB(m4.second);

	// done by RAII
    //if (m_textureId != GL_INVALID_INDEX)
    //{
    //    /* if(glMakeTextureHandleNonResidentARB && glIsTextureHandleResidentARB(m_textureHandle))
    //         glMakeTextureHandleNonResidentARB(m_textureHandle);*/
    //    glDeleteTextures(1, &m_textureId);
    //}
}

Texture::Texture(const Texture& other)
{
    m_size = other.m_size;
    m_target = other.m_target;
	m_textureId = glCreateTextureRAII(m_target);
    m_format = other.m_format;
    m_defaultSampler = other.m_defaultSampler;
    m_fixedSampleLocations = other.m_fixedSampleLocations;
    m_levels = other.m_levels;
    m_samples = other.m_samples;
    m_overrideSampler = other.m_overrideSampler;

    switch (m_target)
    {
    case GL_TEXTURE_1D:
        glTextureStorage1D(*m_textureId, m_levels, m_format, m_size.x);
        assign1D(GL_RGBA, GL_FLOAT, other.data<float>(GL_RGBA).data());
        break;
    case GL_TEXTURE_1D_ARRAY:
    case GL_TEXTURE_2D:
    case GL_TEXTURE_CUBE_MAP:
    case GL_TEXTURE_RECTANGLE:
        glTextureStorage2D(*m_textureId, m_levels, m_format, m_size.x, m_size.y);
        assign2D(GL_RGBA, GL_FLOAT, other.data<float>(GL_RGBA).data());
        break;
    case GL_TEXTURE_CUBE_MAP_ARRAY:
    case GL_TEXTURE_2D_ARRAY:
    case GL_TEXTURE_3D:
        glTextureStorage3D(
            *m_textureId, m_levels, m_format, m_size.x, m_size.y, m_size.z);
        assign3D(GL_RGBA, GL_FLOAT, other.data<float>(GL_RGBA).data());
        break;
    case GL_TEXTURE_2D_MULTISAMPLE:
        glTextureStorage2DMultisample(*m_textureId,
            static_cast<int>(m_samples),
            m_format,
            m_size.x,
            m_size.y,
            m_fixedSampleLocations);
        break;
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
        glTextureStorage3DMultisample(*m_textureId,
            static_cast<int>(m_samples),
            m_format,
            m_size.x,
            m_size.y,
            m_size.z,
            m_fixedSampleLocations);
    default:
        break;
    }

    if (other.m_hasMipmaps)
        generateMipmaps();
    generateHandle();
}

Texture::Texture(Texture&& other) noexcept
{
    m_size = other.m_size;
    m_target = other.m_target;
    m_textureId = std::move(other.m_textureId);
    m_format = other.m_format;
    m_defaultSampler = std::move(other.m_defaultSampler);
    m_fixedSampleLocations = other.m_fixedSampleLocations;
    m_levels = other.m_levels;
    m_samples = other.m_samples;
    m_overrideSampler = std::move(other.m_overrideSampler);
    m_hasMipmaps = other.m_hasMipmaps;
    m_imageHandleTree = std::move(other.m_imageHandleTree);
    m_textureHandle = other.m_textureHandle;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
	m_textureId.reset();
    m_size = other.m_size;
    m_target = other.m_target;
    m_textureId = std::move(other.m_textureId);
    m_format = other.m_format;
    m_defaultSampler = std::move(other.m_defaultSampler);
    m_fixedSampleLocations = other.m_fixedSampleLocations;
    m_levels = other.m_levels;
    m_samples = other.m_samples;
    m_overrideSampler = std::move(other.m_overrideSampler);
    m_hasMipmaps = other.m_hasMipmaps;
    m_imageHandleTree = std::move(other.m_imageHandleTree);
    m_textureHandle = other.m_textureHandle;
    return *this;
}

Texture& Texture::operator=(const Texture& other)
{
	m_textureId.reset();
    m_size = other.m_size;
    m_target = other.m_target;
	m_textureId = glCreateTextureRAII(m_target);
    m_format = other.m_format;
    m_defaultSampler = other.m_defaultSampler;
    m_fixedSampleLocations = other.m_fixedSampleLocations;
    m_levels = other.m_levels;
    m_samples = other.m_samples;
    m_overrideSampler = other.m_overrideSampler;

    switch (m_target)
    {
    case GL_TEXTURE_1D:
        glTextureStorage1D(*m_textureId, m_levels, m_format, m_size.x);
        assign1D(GL_RGBA, GL_FLOAT, other.data<float>(GL_RGBA).data());
        break;
    case GL_TEXTURE_1D_ARRAY:
    case GL_TEXTURE_2D:
    case GL_TEXTURE_CUBE_MAP:
    case GL_TEXTURE_RECTANGLE:
        glTextureStorage2D(*m_textureId, m_levels, m_format, m_size.x, m_size.y);
        assign2D(GL_RGBA, GL_FLOAT, other.data<float>(GL_RGBA).data());
        break;
    case GL_TEXTURE_CUBE_MAP_ARRAY:
    case GL_TEXTURE_2D_ARRAY:
    case GL_TEXTURE_3D:
        glTextureStorage3D(
            *m_textureId, m_levels, m_format, m_size.x, m_size.y, m_size.z);
        assign3D(GL_RGBA, GL_FLOAT, other.data<float>(GL_RGBA).data());
        break;
    case GL_TEXTURE_2D_MULTISAMPLE:
        glTextureStorage2DMultisample(*m_textureId,
            static_cast<int>(m_samples),
            m_format,
            m_size.x,
            m_size.y,
            m_fixedSampleLocations);
        break;
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
        glTextureStorage3DMultisample(*m_textureId,
            static_cast<int>(m_samples),
            m_format,
            m_size.x,
            m_size.y,
            m_size.z,
            m_fixedSampleLocations);
    default:
        break;
    }

    if (other.m_hasMipmaps)
        generateMipmaps();
    generateHandle();
    return *this;
}

Sampler& Texture::sampler() { return m_overrideSampler ? *m_overrideSampler : m_defaultSampler; }
const Sampler& Texture::sampler() const
{
    return m_overrideSampler ? *m_overrideSampler : m_defaultSampler;
}
void Texture::overrideSampler(const std::shared_ptr<Sampler>& sampler)
{
    m_overrideSampler = sampler;
    generateHandle();
}

void Texture::set(GLenum texParam, int value)
{
    if (m_overrideSampler)
        return;
    Sampler new_sampler = m_defaultSampler;
    new_sampler.set(texParam, value);
    m_defaultSampler = std::move(new_sampler);
    generateHandle();
}

void Texture::set(GLenum texParam, float value)
{
    if (m_overrideSampler)
        return;
    Sampler new_sampler = m_defaultSampler;
    new_sampler.set(texParam, value);
    m_defaultSampler = std::move(new_sampler);
    generateHandle();
}

void Texture::set(GLenum texParam, GLenum value)
{
    if (m_overrideSampler)
        return;
    Sampler new_sampler = m_defaultSampler;
    new_sampler.set(texParam, value);
    m_defaultSampler = std::move(new_sampler);
    generateHandle();
}

void Texture::set(const TexParamMap& parameters)
{
    if (m_overrideSampler)
        return;

    struct ParamVisitor
    {
        ParamVisitor(GLenum tparam, Sampler& s)
            : m_sampler(s)
            , m_tparam(tparam)
        {
        }

        void operator()(float value) const { m_sampler.set(m_tparam, value); }
        void operator()(int value) const { m_sampler.set(m_tparam, value); }
        void operator()(GLenum value) const { m_sampler.set(m_tparam, value); }

    private:
        Sampler & m_sampler;
        GLenum   m_tparam;
    };
    Sampler new_sampler = m_defaultSampler;
    for (const auto& p : parameters)
        std::visit(ParamVisitor(p.first, new_sampler), p.second);
    m_defaultSampler = std::move(new_sampler);
    generateHandle();
}

void Texture::assign1D(int level, int offset, int size, GLenum format, GLenum type,
    const void* pixels) const
{
    glTextureSubImage1D(*m_textureId, level, offset, size, format, type, pixels);
    m_hasMipmaps = false;
}

void Texture::assign1D(GLenum format, GLenum type, const void* pixels) const
{
    assign1D(0, 0, m_size.x, format, type, pixels);
}

void Texture::assign2D(int level, glm::ivec2 offset, glm::ivec2 size, GLenum format, GLenum type,
    const void* pixels) const
{
    glTextureSubImage2D(
        *m_textureId, level, offset.x, offset.y, size.x, size.y, format, type, pixels);
    m_hasMipmaps = false;
}

void Texture::assign2D(GLenum format, GLenum type, const void* pixels) const
{
    assign2D(0, { 0, 0 }, glm::ivec2(m_size), format, type, pixels);
}

void Texture::assign3D(int level, glm::ivec3 offset, glm::ivec3 size, GLenum format, GLenum type,
    const void* pixels) const
{
    glTextureSubImage3D(*m_textureId,
        level,
        offset.x,
        offset.y,
        offset.z,
        size.x,
        size.y,
        size.z,
        format,
        type,
        pixels);
    m_hasMipmaps = false;
}
void Texture::assign3D(GLenum format, GLenum type, const void* pixels) const
{
    assign3D(0, { 0, 0, 0 }, m_size, format, type, pixels);
}

void Texture::generateMipmaps() const
{
    glGenerateTextureMipmap(*m_textureId);
    m_hasMipmaps = true;
}

void Texture::bind(std::variant<GLuint, TextureBinding> binding) const
{
    const GLuint b = std::holds_alternative<GLuint>(binding) ? std::get<GLuint>(binding) : static_cast<GLuint>(std::get<TextureBinding>(binding));
    glBindSampler(b, *(sampler().id()));
    glBindTextureUnit(b, *m_textureId);
}

void Texture::bindImage(std::variant<GLuint, TextureBinding> binding) const { bindImage(binding, GL_READ_WRITE, m_format); }
void Texture::bindImage(std::variant<GLuint, TextureBinding> binding, GLenum access, GLenum format) const
{
    bindImage(binding, 0, true, 0, access, format);
}
void Texture::bindImage(std::variant<GLuint, TextureBinding> binding, int level, bool layered, int layer, GLenum access,
    GLenum format) const
{
    const GLuint b = std::holds_alternative<GLuint>(binding) ? std::get<GLuint>(binding) : static_cast<GLuint>(std::get<TextureBinding>(binding));
    glBindImageTexture(b, *m_textureId, level, layered, layer, access, format);
}

GLtexture Texture::id() const { return m_textureId; }
GLsampler Sampler::id() const { return m_samplerId; }
void   Texture::clear(GLint level, GLenum format, GLenum type, const void* data) const
{
    glClearTexImage(*m_textureId, level, format, type, data);
}
void Texture::clear(GLenum format, GLenum type, const void* data) const
{
    clear(0, format, type, data);
}

void Texture::generateHandle()
{
    if (m_textureHandle && glIsTextureHandleResidentARB(m_textureHandle))
        glMakeTextureHandleNonResidentARB(m_textureHandle);

    //if(glIsTextureHandleResidentARB(m_textureHandle))
    //    glMakeTextureHandleNonResidentARB(m_textureHandle);

    util::getGlError(__LINE__, __FUNCTION__);

    m_textureHandle = glGetTextureSamplerHandleARB(*m_textureId, *(sampler().id()));

    util::getGlError(__LINE__, __FUNCTION__);

    if (!glIsTextureHandleResidentARB(m_textureHandle))
        glMakeTextureHandleResidentARB(m_textureHandle);

    util::getGlError(__LINE__, __FUNCTION__);
}
GLuint64 Texture::handle() const { return m_textureHandle; }

GLuint64 Texture::imageHandle(int level, bool layered, int layer, GLenum access, GLenum format)
{

    GLuint64& handle = m_imageHandleTree[level][layered][layer][access][format];
    handle = glGetImageHandleARB(*m_textureId, level, layered, layer, format);

    util::getGlError(__LINE__, __FUNCTION__);

    if (!glIsImageHandleResidentARB(handle))
        glMakeImageHandleResidentARB(handle, access);

    util::getGlError(__LINE__, __FUNCTION__);

    return handle;
}

GLenum Texture::getTarget() const
{
    return m_target;
}

GLenum Texture::getFormat() const
{
    return m_format;
}

glm::ivec3 Texture::getSize() const
{
    return m_size;
}

void Texture::resize(GLenum target, GLenum format, glm::ivec2 size, Samples samples,
    bool fixedSampleLocations)
{
    *this = Texture(target, format, size, samples, fixedSampleLocations);
}

void Texture::resize(GLenum target, GLenum format, glm::ivec3 size, Samples samples,
    bool fixedSampleLocations)
{
    *this = Texture(target, format, size, samples, fixedSampleLocations);
}

void Texture::resize(GLenum target, GLenum format, int size, int levels)
{
    *this = Texture(target, format, size, levels);
}

void Texture::resize(GLenum target, GLenum format, glm::ivec2 size, int levels)
{
    *this = Texture(target, format, size, levels);
}

void Texture::resize(GLenum target, GLenum format, glm::ivec3 size, int levels)
{
    *this = Texture(target, format, size, levels);
}
