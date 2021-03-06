#include "FrameBuffer.hpp"
#include <algorithm>
#include <iostream>

FrameBuffer::FrameBuffer() : m_fbo(glCreateFramebufferRAII())
{
}

FrameBuffer::FrameBuffer(glm::ivec2 depthTextureSize)
    : FrameBuffer()
{
    assert(glm::all(glm::greaterThan(depthTextureSize, glm::ivec2(0))) && "Size must be greater than 0!");

    m_size = depthTextureSize;

    setDepthAttachment(std::make_shared<Texture>(GL_TEXTURE_2D, GL_DEPTH_COMPONENT32F, depthTextureSize));
}

void FrameBuffer::setDepthAttachment(const std::shared_ptr<Texture>& texture)
{
    if (m_size != glm::ivec2(0))
        assert(glm::ivec2(texture->getSize()) == m_size && "Texture and Framebuffer sizes mismatch!");
    else
        m_size = glm::ivec2(texture->getSize());

    m_depthTexture = texture;

    glNamedFramebufferTexture(
        *m_fbo, GL_DEPTH_ATTACHMENT, *(texture->id()), 0);
    check();
}

void FrameBuffer::addColorAttachment(unsigned int attachmentIndex,
    const std::shared_ptr<Texture>& texture)
{
    if (m_size != glm::ivec2(0))
        assert(glm::ivec2(texture->getSize()) == m_size && "Texture and Framebuffer sizes mismatch!");
    else
        m_size = glm::ivec2(texture->getSize());

    m_colorAttachments[attachmentIndex] = texture;

    glNamedFramebufferTexture(
        *m_fbo, GL_COLOR_ATTACHMENT0 + attachmentIndex, *(texture->id()), 0);
    check();
}

void FrameBuffer::updateDrawBuffers()
{
    std::vector<GLenum> attachments;
    std::for_each(
        m_colorAttachments.begin(), m_colorAttachments.end(), [&attachments](auto& entry) {
        attachments.push_back(GL_COLOR_ATTACHMENT0 + entry.first);
    });

    std::sort(attachments.begin(), attachments.end());
    
    glNamedFramebufferDrawBuffers(
        *m_fbo, static_cast<GLsizei>(attachments.size()), attachments.data());
}

std::shared_ptr<Texture> FrameBuffer::getColorTexture(unsigned int attachmentIndex)
{
    return m_colorAttachments[attachmentIndex];
}

std::shared_ptr<Texture> FrameBuffer::getDepthTexture() const
{
    return m_depthTexture;
}

bool FrameBuffer::check()
{
    const auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "WARNING: Framebuffer is not complete! \n";
        return false;
    }
    return true;
}

void FrameBuffer::resize(glm::ivec2 size)
{
    assert(glm::all(glm::greaterThan(size, glm::ivec2(0))) && "Size must be greater than 0!");

    m_size = size;

	m_fbo.reset();
	m_fbo = glCreateFramebufferRAII();

    if(m_depthTexture)
    {
        m_depthTexture->resize(m_depthTexture->getTarget(), m_depthTexture->getFormat(), size);
        glNamedFramebufferTexture(
            *m_fbo, GL_DEPTH_ATTACHMENT, *(m_depthTexture->id()), 0);
    }

    std::for_each(m_colorAttachments.begin(), m_colorAttachments.end(), [&](auto& attachment) {
        attachment.second->resize(
            attachment.second->getTarget(), attachment.second->getFormat(), size);
        glNamedFramebufferTexture(
            *m_fbo, GL_COLOR_ATTACHMENT0 + attachment.first, *(attachment.second->id()), 0);
    });

    updateDrawBuffers();
    check();
}

void FrameBuffer::bind() const { glBindFramebuffer(GL_FRAMEBUFFER, *m_fbo); }

void FrameBuffer::unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

GLframebuffer FrameBuffer::id() const { return m_fbo; }

FrameBuffer::~FrameBuffer()
{
	// done by RAII
}

void FrameBuffer::blit(const std::shared_ptr<FrameBuffer>& other) const
{
    glBlitNamedFramebuffer(*m_fbo,
        *(other->id()),
        0,
        0,
        m_size.x,
        m_size.y,
        0,
        0,
        other->getSize().x,
        other->getSize().y,
        GL_COLOR_BUFFER_BIT,
        GL_LINEAR);
}

void FrameBuffer::blitToDefault() const
{
    glBlitNamedFramebuffer(*m_fbo,
        0,
        0,
        0,
        m_size.x,
        m_size.y,
        0,
        0,
        m_size.x,
        m_size.y,
        GL_COLOR_BUFFER_BIT,
        GL_LINEAR);
}

glm::ivec2 FrameBuffer::getSize() const { return m_size; }

PingPongBuffer::PingPongBuffer(glm::ivec2 size, bool useDepthTexture)
{
    m_textures[0] = std::make_shared<Texture>(GL_TEXTURE_2D, GL_RGB16F, size);
    m_textures[1] = std::make_shared<Texture>(GL_TEXTURE_2D, GL_RGB16F, size);

    m_fbos[0] = useDepthTexture ? std::make_shared<FrameBuffer>(size) : std::make_shared<FrameBuffer>();
    m_fbos[0]->addColorAttachment(0, m_textures[0]);
    m_fbos[0]->updateDrawBuffers();

    m_fbos[1] = useDepthTexture ? std::make_shared<FrameBuffer>(size) : std::make_shared<FrameBuffer>();
    m_fbos[1]->addColorAttachment(0, m_textures[1]);
    m_fbos[1]->updateDrawBuffers();
}

void PingPongBuffer::toggle() { m_activeTexIndex = m_activeTexIndex == 0 ? 1 : 0; }

void PingPongBuffer::bind() { m_fbos[m_activeTexIndex]->bind(); }

void PingPongBuffer::unbind() { m_fbos[m_activeTexIndex]->unbind(); }

std::shared_ptr<Texture> PingPongBuffer::getActiveTexture() const
{
    return m_textures[m_activeTexIndex];
}

std::shared_ptr<FrameBuffer> PingPongBuffer::getActiveFramebuffer() const
{
    return m_fbos[m_activeTexIndex];
}

void PingPongBuffer::resize(glm::ivec2 size)
{
    m_fbos[0]->resize(size);
    m_fbos[1]->resize(size);
}

void PingPongBuffer::clear()
{
    glm::vec3 clearVal(0.f);
    glClearNamedFramebufferfv(*(m_fbos[0]->id()), GL_COLOR, 0, glm::value_ptr(clearVal));
    glClearNamedFramebufferfv(*(m_fbos[1]->id()), GL_COLOR, 0, glm::value_ptr(clearVal));
}
