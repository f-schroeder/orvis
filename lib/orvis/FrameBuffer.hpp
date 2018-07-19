#pragma once

#include "Texture.hpp"
#include <array>
#include <unordered_map>

using namespace gl;

/**
 * @brief An OpenGL framebuffer object.
 * @details Every framebuffer holds its own framebuffer-handle and in case the depth texture is used
 * a renderbuffer-handle. It also manages the color-attachments and the corresponding drawbuffers.
 */
class FrameBuffer
{
public:
    /**
     * @brief Default constructor. Only generates an OpenGL framebuffer object.
     */
    FrameBuffer();

    /**
     * @brief Constructs a framebuffer object and attaches a new depth texture with the given size.
     * @param depthTextureSize The size of the depth texture attached to the framebuffer (should be greater than 0 in all dimensions).
     */
    explicit FrameBuffer(glm::ivec2 depthTextureSize);

    /**
    * @brief Sets a Texture as the framebuffers depth attachment.
    * @param texture The Texture that is attached to the depth-attachment.
    */
    void setDepthAttachment(const std::shared_ptr<Texture>& texture);

    /**
     * @brief Adds a Texture to the framebuffer at the given attachment index.
     * @param attachmentIndex The number of the color attachment that should be used for this
     * texture.
     * @param texture The Texture that is attached to the given color-attachment.
     */
    void addColorAttachment(unsigned int attachmentIndex, const std::shared_ptr<Texture>& texture);

    /**
     * @brief Updates the drawbuffers to match the color-attachments of the framebuffer.
     *        Should be called after all desired color-attachments have been added.
     */
    void updateDrawBuffers();

    /**
     * @brief Gets the Texture attached to the given color-attachment.
     * @param attachmentIndex The index of the color-attachment.
     * @return The Texture attached to that index.
     */
    std::shared_ptr<Texture> getColorTexture(unsigned int attachmentIndex);

    /**
    * @return  the Texture attached to the depth-attachment.
    */
    std::shared_ptr<Texture> getDepthTexture() const;

    /**
     * @brief Checks the current state of the framebuffer (i.e. looks for OpenGL errors).
     */
    static bool check();

    /**
     * @brief Resizes the framebuffer and all its currently attached Textures.
     *        Also updates the drawbuffers afterwards.
     * @param size The new size of the framebuffer.
     */
    void resize(glm::ivec2 size);

    /**
     * @brief Binds the framebuffer.
     */
    void bind() const;

    /**
     * @brief Unbinds the framebuffer (i.e. binds the default one).
     */
    static void unbind();

    /**
     * @brief Gets the OpenGL-ID of the framebuffer object.
     */
    GLuint id() const;

    /**
     * @brief Destructor. Destroys all allocated GPU-resources.
     */
    ~FrameBuffer();

    /**
     * @brief Blits (i.e. copies) this framebuffer to another
     */
    void blit(const std::shared_ptr<FrameBuffer>& other) const;

    /**
     * @brief Blits (i.e. copies) this framebuffer to the default framebuffer
     */
    void blitToDefault() const;

    /**
     * @brief Returns the size of the framebuffer
     */
    glm::ivec2 getSize() const;

private:
    std::unordered_map<unsigned int, std::shared_ptr<Texture>> m_colorAttachments;
    std::shared_ptr<Texture> m_depthTexture;

    glm::ivec2 m_size     = glm::ivec2(0, 0);

    GLuint m_fbo = GL_INVALID_INDEX;
};

/**
 * @brief This class wraps two framebuffers with one color attachment texture each
 * to toggle between them
 */
class PingPongBuffer
{
public:
    /**
     * @brief Constructor. Created the PingPongBuffer object and initializes its FBOs and textures
     * @param size The size of the Textures attached to the FBOs
     * @param useDepthTexture Specifies whether the depth buffer should be used (false by default)
     */
    explicit PingPongBuffer(glm::ivec2 size, bool useDepthTexture = false);

    /**
     * @brief Toggles the active FBO (i.e. switches the active FBO)
     * This does not bind or unbind anything
     */
    void toggle();

    /**
     * @brief Bind the currently active FBO
     */
    void bind();

    /**
     * @brief Unbind the currently active FBO (i.e. bind the default FBO)
     */
    void unbind();

    /**
     * @brief Resizes both framebuffers and all attached Textures.
     *        Also updates the drawbuffers afterwards.
     * @param size The new size of the framebuffers.
     */
    void resize(glm::ivec2 size);

    /**
     * @brief Clears both framebuffers to [0,0,0]
     */
    void clear();

    /**
     * @brief Returns the currently active texture
     */
    std::shared_ptr<Texture> getActiveTexture() const;

    /**
     * @brief Returns the currently active framebuffer
     */
    std::shared_ptr<FrameBuffer> getActiveFramebuffer() const;

private:
    int m_activeTexIndex = 0;

    std::array<std::shared_ptr<Texture>, 2>     m_textures;
    std::array<std::shared_ptr<FrameBuffer>, 2> m_fbos;
};
