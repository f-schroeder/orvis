#include "Texture.hpp"

#include <iostream>

Sampler::Sampler()
{
    glGenSamplers(1, &m_samplerId);

    // Set some default parameters for the sampler
    set(GL_TEXTURE_CUBE_MAP_SEAMLESS, true);
    set(GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
    set(GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    set(GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    set(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //set(GL_TEXTURE_MAX_ANISOTROPY, 16.f);
}
Sampler::~Sampler()
{
    if(m_samplerId)
        glDeleteSamplers(1, &m_samplerId);
}

Sampler::Sampler(const Sampler& other)
{
    glGenSamplers(1, &m_samplerId);
    for(const auto& var : other.m_samplerParams)
    {
        if(std::holds_alternative<int>(var.second))
            set(var.first, std::get<int>(var.second));
        else if(std::holds_alternative<float>(var.second))
            set(var.first, std::get<float>(var.second));
    }
}

Sampler::Sampler(Sampler&& other) noexcept
{
    m_samplerId       = other.m_samplerId;
    m_samplerParams   = std::move(other.m_samplerParams);
    other.m_samplerId = 0;
}

Sampler& Sampler::operator=(const Sampler& other)
{
    if(m_samplerId)
        glDeleteSamplers(1, &m_samplerId);
    glGenSamplers(1, &m_samplerId);

    for(const auto& var : other.m_samplerParams)
    {
        if(std::holds_alternative<int>(var.second))
            set(var.first, std::get<int>(var.second));
        else if(std::holds_alternative<float>(var.second))
            set(var.first, std::get<float>(var.second));
    }
    return *this;
}

Sampler& Sampler::operator=(Sampler&& other) noexcept
{
    if(m_samplerId)
        glDeleteSamplers(1, &m_samplerId);
    m_samplerId       = other.m_samplerId;
    m_samplerParams   = std::move(other.m_samplerParams);
    other.m_samplerId = 0;
    return *this;
}

void Sampler::set(GLenum texParam, int value)
{
    m_samplerParams[texParam] = value;
    glSamplerParameteri(m_samplerId, texParam, value);
}
void Sampler::set(GLenum texParam, float value)
{
    m_samplerParams[texParam] = value;
    glSamplerParameterf(m_samplerId, texParam, value);
}
void Sampler::set(GLenum texParam, GLenum value)
{
    m_samplerParams[texParam] = value;
    glSamplerParametere(m_samplerId, texParam, value);
}

Texture::Texture(GLenum target)
        : m_target(target)
{
    glCreateTextures(m_target, 1, &m_textureId);
}

Texture::Texture(GLenum target, GLenum format, int size, int levels)
        : Texture(target)
{
    m_format = format;
    m_size   = {size, 1, 1};
    m_levels = levels == -1 ? static_cast<int>(floor(log2(size)) + 1) : levels;
    glTextureStorage1D(m_textureId, m_levels, m_format, size);
    generateHandle();
}

Texture::Texture(GLenum target, GLenum format, glm::ivec2 size, int levels)
	: Texture(target)
{

	m_format = format;
	m_size = { size, 1 };
	m_levels = levels == -1 ? static_cast<int>(glm::floor(std::log2(glm::max(size.x, size.y))) + 1)
		: levels;

	glTextureStorage2D(m_textureId, m_levels, m_format, m_size.x, m_size.y);
	GLenum error = glGetError();
	//generateHandle();
}

Texture::Texture(GLenum target, GLenum format, glm::ivec3 size, int levels)
        : Texture(target)
{
    GLenum err;
    m_format = format;
    m_size   = size;
    m_levels = levels == -1
                       ? static_cast<int>(
                                 floor(std::log2(glm::max(glm::max(size.x, size.z), size.y))) + 1)
                       : levels;
    glTextureStorage3D(
            m_textureId, m_levels, m_format, m_size.x, m_size.y, m_size.z);
    while((err = glGetError()) != GL_NO_ERROR)
    {
        std::cout << "T0 OpenGL Error: " << err << std::endl;
    }

    generateHandle();
    while((err = glGetError()) != GL_NO_ERROR)
    {
        std::cout << "T1 OpenGL Error: " << err << std::endl;
    }
}

Texture::Texture(GLenum target, GLenum format, glm::ivec2 size, Samples samples,
                 bool fixedSampleLocations)
        : Texture(target)
{
    m_format               = format;
    m_size                 = {size, 1};
    m_levels               = 1;
    m_samples              = samples;
    m_fixedSampleLocations = fixedSampleLocations;
    glTextureStorage2DMultisample(m_textureId,
                                            static_cast<int>(m_samples),
                                            m_format,
                                            m_size.x,
                                            m_size.y,
                                            m_fixedSampleLocations);
    generateHandle();
}

Texture::Texture(GLenum target, GLenum format, glm::ivec3 size, Samples samples,
                 bool fixedSampleLocations)
        : Texture(target)
{
    m_format               = format;
    m_size                 = size;
    m_levels               = 1;
    m_samples              = samples;
    m_fixedSampleLocations = fixedSampleLocations;
    glTextureStorage3DMultisample(m_textureId,
                                            static_cast<int>(m_samples),
                                            m_format,
                                            m_size.x,
                                            m_size.y,
                                            m_size.z,
                                            m_fixedSampleLocations);
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

    if(m_textureId != 0)
    {
        /* if(glMakeTextureHandleNonResidentARB && glIsTextureHandleResidentARB(m_textureHandle))
             glMakeTextureHandleNonResidentARB(m_textureHandle);*/
        glDeleteTextures(1, &m_textureId);
    }
}

Texture::Texture(const Texture& other)
{
    m_size   = other.m_size;
    m_target = other.m_target;
    glCreateTextures(m_target, 1, &m_textureId);
    m_format               = other.m_format;
    m_defaultSampler       = other.m_defaultSampler;
    m_fixedSampleLocations = other.m_fixedSampleLocations;
    m_levels               = other.m_levels;
    m_samples              = other.m_samples;
    m_overrideSampler      = other.m_overrideSampler;

    switch(m_target)
    {
    case GL_TEXTURE_1D:
        glTextureStorage1D(m_textureId, m_levels, m_format, m_size.x);
        assign1D(GL_RGBA, GL_FLOAT, other.data<float>(GL_RGBA).data());
        break;
    case GL_TEXTURE_1D_ARRAY:
    case GL_TEXTURE_2D:
    case GL_TEXTURE_CUBE_MAP:
    case GL_TEXTURE_RECTANGLE:
        glTextureStorage2D(m_textureId, m_levels, m_format, m_size.x, m_size.y);
        assign2D(GL_RGBA, GL_FLOAT, other.data<float>(GL_RGBA).data());
        break;
    case GL_TEXTURE_CUBE_MAP_ARRAY:
    case GL_TEXTURE_2D_ARRAY:
    case GL_TEXTURE_3D:
        glTextureStorage3D(
                m_textureId, m_levels, m_format, m_size.x, m_size.y, m_size.z);
        assign3D(GL_RGBA, GL_FLOAT, other.data<float>(GL_RGBA).data());
        break;
    case GL_TEXTURE_2D_MULTISAMPLE:
        glTextureStorage2DMultisample(m_textureId,
                                                static_cast<int>(m_samples),
                                                m_format,
                                                m_size.x,
                                                m_size.y,
                                                m_fixedSampleLocations);
        break;
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
        glTextureStorage3DMultisample(m_textureId,
                                                static_cast<int>(m_samples),
                                                m_format,
                                                m_size.x,
                                                m_size.y,
                                                m_size.z,
                                                m_fixedSampleLocations);
    default:
        break;
    }

    if(other.m_hasMipmaps)
        generateMipmaps();
    generateHandle();
}

Texture::Texture(Texture&& other) noexcept
{
    m_size                 = other.m_size;
    m_target               = other.m_target;
    m_textureId            = other.m_textureId;
    m_format               = other.m_format;
    m_defaultSampler       = std::move(other.m_defaultSampler);
    m_fixedSampleLocations = other.m_fixedSampleLocations;
    m_levels               = other.m_levels;
    m_samples              = other.m_samples;
    m_overrideSampler      = std::move(other.m_overrideSampler);
    m_hasMipmaps           = other.m_hasMipmaps;
    m_imageHandleTree      = std::move(m_imageHandleTree);
    m_textureHandle        = other.m_textureHandle;
    other.m_textureId      = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if(glIsTexture(m_textureId))
        glDeleteTextures(1, &m_textureId);
    m_size                 = other.m_size;
    m_target               = other.m_target;
    m_textureId            = other.m_textureId;
    m_format               = other.m_format;
    m_defaultSampler       = std::move(other.m_defaultSampler);
    m_fixedSampleLocations = other.m_fixedSampleLocations;
    m_levels               = other.m_levels;
    m_samples              = other.m_samples;
    m_overrideSampler      = std::move(other.m_overrideSampler);
    m_hasMipmaps           = other.m_hasMipmaps;
    m_imageHandleTree      = std::move(m_imageHandleTree);
    m_textureHandle        = other.m_textureHandle;
    other.m_textureId      = 0;
    return *this;
}

Texture& Texture::operator=(const Texture& other)
{
    if(glIsTexture(m_textureId))
        glDeleteTextures(1, &m_textureId);
    m_size   = other.m_size;
    m_target = other.m_target;
    glCreateTextures(m_target, 1, &m_textureId);
    m_format               = other.m_format;
    m_defaultSampler       = other.m_defaultSampler;
    m_fixedSampleLocations = other.m_fixedSampleLocations;
    m_levels               = other.m_levels;
    m_samples              = other.m_samples;
    m_overrideSampler      = other.m_overrideSampler;

    switch(m_target)
    {
    case GL_TEXTURE_1D:
        glTextureStorage1D(m_textureId, m_levels, m_format, m_size.x);
        assign1D(GL_RGBA, GL_FLOAT, other.data<float>(GL_RGBA).data());
        break;
    case GL_TEXTURE_1D_ARRAY:
    case GL_TEXTURE_2D:
    case GL_TEXTURE_CUBE_MAP:
    case GL_TEXTURE_RECTANGLE:
        glTextureStorage2D(m_textureId, m_levels, m_format, m_size.x, m_size.y);
        assign2D(GL_RGBA, GL_FLOAT, other.data<float>(GL_RGBA).data());
        break;
    case GL_TEXTURE_CUBE_MAP_ARRAY:
    case GL_TEXTURE_2D_ARRAY:
    case GL_TEXTURE_3D:
        glTextureStorage3D(
                m_textureId, m_levels, m_format, m_size.x, m_size.y, m_size.z);
        assign3D(GL_RGBA, GL_FLOAT, other.data<float>(GL_RGBA).data());
        break;
    case GL_TEXTURE_2D_MULTISAMPLE:
        glTextureStorage2DMultisample(m_textureId,
                                                static_cast<int>(m_samples),
                                                m_format,
                                                m_size.x,
                                                m_size.y,
                                                m_fixedSampleLocations);
        break;
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
        glTextureStorage3DMultisample(m_textureId,
                                                static_cast<int>(m_samples),
                                                m_format,
                                                m_size.x,
                                                m_size.y,
                                                m_size.z,
                                                m_fixedSampleLocations);
    default:
        break;
    }

    if(other.m_hasMipmaps)
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
    if(m_overrideSampler)
        return;
    Sampler new_sampler = m_defaultSampler;
    new_sampler.set(texParam, value);
    m_defaultSampler = std::move(new_sampler);
    generateHandle();
}

void Texture::set(GLenum texParam, float value)
{
    if(m_overrideSampler)
        return;
    Sampler new_sampler = m_defaultSampler;
    new_sampler.set(texParam, value);
    m_defaultSampler = std::move(new_sampler);
    generateHandle();
}

void Texture::set(const TexParamMap& parameters)
{
    if(m_overrideSampler)
        return;

    struct ParamVisitor
    {
        ParamVisitor(GLenum tparam, Sampler& s)
                : m_sampler(s)
                , m_tparam(tparam)
        {
        }

        void operator()(float value) { m_sampler.set(m_tparam, value); }
        void operator()(int value) { m_sampler.set(m_tparam, value); }

    private:
        Sampler& m_sampler;
        GLenum   m_tparam;
    };
    Sampler new_sampler = m_defaultSampler;
    for(const auto& p : parameters)
        std::visit(ParamVisitor(p.first, new_sampler), p.second);
    m_defaultSampler = std::move(new_sampler);
    generateHandle();
}

void Texture::assign1D(int level, int offset, int size, GLenum format, GLenum type,
                       const void* pixels)
{
    glTextureSubImage1D(m_textureId, level, offset, size, format, type, pixels);
    m_hasMipmaps = false;
}

void Texture::assign1D(GLenum format, GLenum type, const void* pixels)
{
    assign1D(0, 0, m_size.x, format, type, pixels);
}

void Texture::assign2D(int level, glm::ivec2 offset, glm::ivec2 size, GLenum format, GLenum type,
                       const void* pixels)
{
    glTextureSubImage2D(
            m_textureId, level, offset.x, offset.y, size.x, size.y, format, type, pixels);
    m_hasMipmaps = false;
}

void Texture::generateMipmaps() const
{
    glGenerateTextureMipmap(m_textureId);
    m_hasMipmaps = true;
}

void Texture::assign2D(GLenum format, GLenum type, const void* pixels)
{
    assign2D(0, {0, 0}, glm::ivec2(m_size), format, type, pixels);
}

void Texture::assign3D(int level, glm::ivec3 offset, glm::ivec3 size, GLenum format, GLenum type,
                       const void* pixels)
{
    glTextureSubImage3D(m_textureId,
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
void Texture::assign3D(GLenum format, GLenum type, const void* pixels)
{
    assign3D(0, {0, 0, 0}, m_size, format, type, pixels);
}

void Texture::bind(GLuint binding) const
{
    glBindSampler(binding, sampler().id());
    glBindTextureUnit(binding, m_textureId);
}

void Texture::bindImage(GLuint binding) const { bindImage(binding, GL_READ_WRITE, m_format); }
void Texture::bindImage(GLuint binding, GLenum access, GLenum format) const
{
    bindImage(binding, 0, false, 0, access, format);
}
void Texture::bindImage(GLuint binding, int level, bool layered, int layer, GLenum access,
                        GLenum format) const
{
    glBindImageTexture(binding, m_textureId, level, layered, layer, access, format);
}

GLuint Texture::id() const { return m_textureId; }
GLuint Sampler::id() const { return m_samplerId; }
void   Texture::clear(GLint level, GLenum format, GLenum type, const void* data) const
{
    glClearTexImage(m_textureId, level, format, type, data);
}
void Texture::clear(GLenum format, GLenum type, const void* data) const
{
    clear(0, format, type, data);
}

void Texture::generateHandle()
{
    GLenum err;
    if(glGetTextureSamplerHandleARB)
    {
        if(m_textureHandle && glIsTextureHandleResidentARB(m_textureHandle))
            glMakeTextureHandleNonResidentARB(m_textureHandle);

        //if(glIsTextureHandleResidentARB(m_textureHandle))
        //    glMakeTextureHandleNonResidentARB(m_textureHandle);
        while((err = glGetError()) != GL_NO_ERROR)
        {
            std::cout << "TH2 OpenGL Error: " << err << std::endl;
        }

        m_textureHandle = glGetTextureSamplerHandleARB(m_textureId, sampler().id());
        while((err = glGetError()) != GL_NO_ERROR)
        {
            std::cout << "TH3 OpenGL Error: " << err << std::endl;
        }

        if(!glIsTextureHandleResidentARB(m_textureHandle))
            glMakeTextureHandleResidentARB(m_textureHandle);
        while((err = glGetError()) != GL_NO_ERROR)
        {
            std::cout << "TH4 OpenGL Error: " << err << std::endl;
        }
    }
    else
    {
        m_textureHandle = 0ull;
    }
}
GLuint64 Texture::handle() const { return m_textureHandle; }

GLuint64 Texture::imageHandle(int level, bool layered, int layer, GLenum access, GLenum format)
{
    if(glGetImageHandleARB)
    {
        GLuint64& handle = m_imageHandleTree[level][layered][layer][access][format];
        handle           = glGetImageHandleARB(m_textureId, level, layered, layer, format);
        if(!glIsImageHandleResidentARB(handle))
            glMakeImageHandleResidentARB(handle, access);
        return handle;
    }
    else
    {
        return 0;
    }
}

GLenum Texture::getTarget() const
{ return m_target; }

GLenum Texture::getFormat() const
{ return m_format; }

glm::ivec3 Texture::getSize() const
{ return m_size; }

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
