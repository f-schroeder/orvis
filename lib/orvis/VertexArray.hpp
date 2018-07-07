#pragma once

#include "Buffer.hpp"
#include <memory>
#include <unordered_map>

using namespace gl;

/**
 * @brief Wrapper class for OpenGL Vertex Array Objects.
 * @detail Those are used to index attributes in Vertex Buffers, as well as combining them with
 * Element Buffers for indexed rendering.
 */
class VertexArray : Createable<VertexArray>
{
public:
    VertexArray();
    ~VertexArray();

    VertexArray(const VertexArray& other);
    VertexArray(VertexArray&& other) noexcept;
    VertexArray& operator=(const VertexArray& other);
    VertexArray& operator=(VertexArray&& other) noexcept;

    /**
     * @brief Sets the data format for the given attribute.
     * @param attribute The attribute index.
     * @param components The number of components in this attribute (e.g. 3 for vec3)
     * @param type The component type (e.g. GL_UNSIGNED_INT for uvec4)
     * @param normalized Specifies whether data values should be normalized
     * @param offset The relative byte offset within the element stride. (e.g. the byte offset of a
     * Vertex::position attribute in the Vertex struct)
     */
    void format(GLuint attribute, GLint components, GLenum type, bool normalized, GLintptr offset);

    /**
     * @brief Specifies a vertex buffer binding for the given attribute. Afterwards, the attribute
     * will be read from the GL_ARRAY_BUFFER bound to the specified binding (e.g. with
     * VertexArray::setVertexBuffer).
     * @param attribute The attribute index.
     * @param binding The target buffer binding.
     */
    void binding(GLuint attribute, GLuint binding);

    /**
     * @brief Binds a vertex buffer at the given binding point which can then be read by all
     * attributes referencing it.
     * @tparam T The value type of the buffer.
     * @param buffer The referenced buffer.
     * @param binding The target buffer binding.
     * @param offset A byte offset into the buffer at which to start reading attributes.
     * @param stride The byte size of each vertex element in this buffer. (Or the byte distance
     * between the element start positions).
     */
    template <typename T>
    void setVertexBuffer(const Buffer<T>& buffer, GLuint binding, GLintptr offset = 0,
                         GLsizei stride = static_cast<GLsizei>(sizeof(T)));

    /**
     * @brief Binds a vertex buffer at the given binding point which can then be read by all
     * attributes referencing it.
     * @tparam T The value type of the buffer.
     * @param buffer The referenced buffer.
     * @param binding The target buffer binding.
     * @param offset A byte offset into the buffer at which to start reading attributes.
     * @param stride The byte size of each vertex element in this buffer. (Or the byte distance
     * between the element start positions).
     */
    void setVertexBuffer(const GLuint& buffer, GLuint binding, GLintptr offset, GLsizei stride);

    /**
     * @brief Binds an element buffer to this vertex array for indexed rendering.
     * @param buffer The element buffer containing the vertex indices.
     */
    template <typename T>
    void setElementBuffer(const Buffer<T>& buffer);

    /**
     * @brief Binds an element buffer to this vertex array for indexed rendering.
     * @param buffer The element buffer containing the vertex indices.
     */
    void setElementBuffer(const GLuint& buffer);

    /**
     * @return The opengl vertex array ID.
     */
    GLuint id() const;

    /**
     * @brief Activates the vertex array e.g. for drawing.
     */
    void bind() const;

private:
    /** @brief An attribute struct for caching attributes needed for copying VAOs. */
    struct Attribute
    {
        GLuint   attribute;
        GLint    components;
        GLenum   type;
        bool     normalized;
        GLintptr offset;
    };

    /** @brief A binding description for caching bindings needed for copying VAOs. */
    struct Bindings
    {
        GLuint                                         buffer;
        GLintptr                                       offset;
        GLsizei                                        stride;
        std::vector<std::reference_wrapper<Attribute>> attributes;
    };

    using AttribMap  = std::unordered_map<GLuint, std::unique_ptr<Attribute>>;
    using BindingMap = std::unordered_map<GLuint, Bindings>;

    GLuint     m_id;         //!< The VAO ID.
    AttribMap  m_attributes; //!< Maps attribute indices to their set attributes.
    BindingMap m_bindings;   //!< Maps buffer bindings to a vector of referencing attributes.
    GLuint     m_elementArrayBuffer;
};

#include "VertexArray.inl"
