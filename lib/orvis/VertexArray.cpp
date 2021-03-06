#include "VertexArray.hpp"

VertexArray::VertexArray() : m_id(glCreateVertexArrayRAII())
{
}

VertexArray::~VertexArray()
{
	// done by RAII
}

void VertexArray::format(std::variant<GLuint, VertexAttributeBinding> attribute, GLint components, GLenum type, bool normalized,
                         GLintptr offset)
{
    const GLuint att = std::holds_alternative<GLuint>(attribute) ? std::get<GLuint>(attribute) : static_cast<GLuint>(std::get<VertexAttributeBinding>(attribute));
    auto&& attr = m_attributes[att];
    if(!attr)
        attr = std::make_unique<Attribute>();
    attr->attribute  = att;
    attr->components = components;
    attr->type       = type;
    attr->normalized = normalized;
    attr->offset     = offset;

    glEnableVertexArrayAttrib(*m_id, att);
    glVertexArrayAttribFormat(
            *m_id, att, components, type, normalized, static_cast<GLuint>(offset));
}

void VertexArray::binding(std::variant<GLuint, VertexAttributeBinding> attribute, std::variant<GLuint, VertexAttributeBinding> binding)
{
    const GLuint att = std::holds_alternative<GLuint>(attribute) ? std::get<GLuint>(attribute) : static_cast<GLuint>(std::get<VertexAttributeBinding>(attribute));
    const GLuint bnd = std::holds_alternative<GLuint>(binding) ? std::get<GLuint>(binding) : static_cast<GLuint>(std::get<VertexAttributeBinding>(binding));
    m_bindings[bnd].attributes.emplace_back(std::ref(*m_attributes[att]));
    glVertexArrayAttribBinding(*m_id, att, bnd == GL_INVALID_INDEX ? att : bnd);
}

GLvertexArray VertexArray::id() const { return m_id; }
void   VertexArray::bind() const { glBindVertexArray(*m_id); }

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
    m_id         = std::move(other.m_id);
    m_attributes = std::move(other.m_attributes);
    m_bindings   = std::move(other.m_bindings);
}

VertexArray& VertexArray::operator=(const VertexArray& other)
{
	m_id.reset();
	m_id = glCreateVertexArrayRAII();

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
	m_id.reset();
    m_id                 = std::move(other.m_id);
    m_attributes         = std::move(other.m_attributes);
    m_bindings           = std::move(other.m_bindings);
    m_elementArrayBuffer = std::move(other.m_elementArrayBuffer);
    return *this;
}

void VertexArray::setElementBuffer(const GLbuffer& buffer)
{
    glVertexArrayElementBuffer(*m_id, *buffer);
    m_elementArrayBuffer = buffer;
}

void VertexArray::setVertexBuffer(const GLbuffer& buffer, std::variant<GLuint, VertexAttributeBinding> binding, GLintptr offset,
                                  GLsizei stride)
{
    const GLuint bnd = std::holds_alternative<GLuint>(binding) ? std::get<GLuint>(binding) : static_cast<GLuint>(std::get<VertexAttributeBinding>(binding));
    glVertexArrayVertexBuffer(*m_id, bnd, *buffer, offset, stride);
    auto& buf  = m_bindings[bnd];
    buf.buffer = buffer;
    buf.offset = offset;
    buf.stride = stride;
}
