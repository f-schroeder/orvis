#include "VertexArray.hpp"

VertexArray::VertexArray() { glCreateVertexArrays(1, &m_id); }

VertexArray::~VertexArray()
{
    if(glIsVertexArray(m_id))
        glDeleteVertexArrays(1, &m_id);
}

void VertexArray::format(GLuint attribute, GLint components, GLenum type, bool normalized,
                         GLintptr offset)
{
    auto&& attr = m_attributes[attribute];
    if(!attr)
        attr = std::make_unique<Attribute>();
    attr->attribute  = attribute;
    attr->components = components;
    attr->type       = type;
    attr->normalized = normalized;
    attr->offset     = offset;

    glEnableVertexArrayAttrib(m_id, attribute);
    glVertexArrayAttribFormat(
            m_id, attribute, components, type, normalized, static_cast<GLuint>(offset));
}

void VertexArray::binding(GLuint attribute, GLuint binding)
{
    m_bindings[binding].attributes.emplace_back(std::ref(*m_attributes[attribute]));
    glVertexArrayAttribBinding(m_id, attribute, binding);
}

GLuint VertexArray::id() const { return m_id; }
void   VertexArray::bind() const { glBindVertexArray(m_id); }

VertexArray::VertexArray(const VertexArray& other)
        : VertexArray()
{
    for(const auto& attr : other.m_attributes)
    {
        format(attr.second->attribute,
               attr.second->components,
               attr.second->type,
               attr.second->normalized,
               attr.second->offset);
    }

    for(const auto& bin : other.m_bindings)
    {
        for(const auto& attrRef : bin.second.attributes)
            binding(bin.first, attrRef.get().attribute);
        setVertexBuffer(bin.second.buffer, bin.first, bin.second.offset, bin.second.stride);
    }
    setElementBuffer(other.m_elementArrayBuffer);
}

VertexArray::VertexArray(VertexArray&& other) noexcept
{
    m_id         = other.m_id;
    m_attributes = std::move(other.m_attributes);
    m_bindings   = std::move(other.m_bindings);
    other.m_id   = 0;
}

VertexArray& VertexArray::operator=(const VertexArray& other)
{
    if(glIsVertexArray(m_id))
        glDeleteVertexArrays(1, &m_id);

    glCreateVertexArrays(1, &m_id);
    for(const auto& attr : other.m_attributes)
    {
        format(attr.second->attribute,
               attr.second->components,
               attr.second->type,
               attr.second->normalized,
               attr.second->offset);
    }
    for(const auto& bin : other.m_bindings)
    {
        for(const auto& attrRef : bin.second.attributes)
            binding(bin.first, attrRef.get().attribute);
        setVertexBuffer(bin.second.buffer, bin.first, bin.second.offset, bin.second.stride);
    }
    setElementBuffer(other.m_elementArrayBuffer);
    return *this;
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
{
    if(glIsVertexArray(m_id))
        glDeleteVertexArrays(1, &m_id);
    m_id                 = other.m_id;
    m_attributes         = std::move(other.m_attributes);
    m_bindings           = std::move(other.m_bindings);
    m_elementArrayBuffer = other.m_elementArrayBuffer;
    other.m_id           = 0;
    return *this;
}

void VertexArray::setElementBuffer(const GLuint& buffer)
{
    glVertexArrayElementBuffer(m_id, buffer);
    m_elementArrayBuffer = buffer;
}

void VertexArray::setVertexBuffer(const GLuint& buffer, GLuint binding, GLintptr offset,
                                  GLsizei stride)
{
    glVertexArrayVertexBuffer(m_id, binding, buffer, offset, stride);
    auto& buf  = m_bindings[binding];
    buf.buffer = buffer;
    buf.offset = offset;
    buf.stride = stride;
}
