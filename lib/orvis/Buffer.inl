#pragma once

#include <cassert>

template <typename T>
Buffer<T>::Buffer()
	: m_storageFlags(GL_NONE_BIT), m_size(0), m_buffer(glCreateBuffer())
{
}

template <typename T>
template <size_t S>
Buffer<T>::Buffer(const std::array<T, S>& data, BufferStorageMask flags)
        : Buffer(std::data(data), std::size(data), flags)
{
}
template <typename T>
Buffer<T>::Buffer(const std::initializer_list<T>& data, BufferStorageMask flags)
        : Buffer(std::data(data), std::size(data), flags)
{
}
template <typename T>
Buffer<T>::Buffer(const std::vector<T>& data, BufferStorageMask flags)
        : Buffer(std::data(data), std::size(data), flags)
{
}

template <typename T>
Buffer<T>::Buffer(const T& data, BufferStorageMask flags)
        : Buffer(&data, 1, flags)
{
}
template <typename T>
Buffer<T>::Buffer(const T* data, GLsizeiptr count, BufferStorageMask flags)
        : Buffer()
{
    m_size         = count;
    m_storageFlags = flags;
    glNamedBufferStorage(*m_buffer, sizeof(T) * count, data, flags);
}

template <typename T>
Buffer<T>::~Buffer()
{
	// done by RAII
}

template <typename T>
Buffer<T>::Buffer(const Buffer& other)
        : Buffer(other.data(), other.m_storageFlags)
{
}
template <typename T>
Buffer<T>::Buffer(Buffer&& other) noexcept
        : m_storageFlags(other.m_storageFlags)
        , m_size(other.m_size)
        , m_buffer(std::move(other.m_buffer))
{
}
template <typename T>
Buffer<T>& Buffer<T>::operator=(const Buffer& other)
{
	m_buffer.reset();
	m_buffer = glCreateBuffer();

    m_size         = other.m_size;
    m_storageFlags = other.m_storageFlags;
    glNamedBufferStorage(
            *m_buffer, sizeof(T) * m_size, other.data().data(), m_storageFlags);
    return *this;
}
template <typename T>
Buffer<T>& Buffer<T>::operator=(Buffer&& other) noexcept
{
	m_buffer.reset();
    m_size         = other.m_size;
    m_storageFlags = other.m_storageFlags;
    m_buffer       = std::move(other.m_buffer);
    return *this;
}

template <typename T>
template <size_t S>
void Buffer<T>::assign(const std::array<T, S>& data, GLintptr offset)
{
    assign(std::data(data), std::size(data), offset);
}
template <typename T>
void Buffer<T>::assign(const std::initializer_list<T>& data, GLintptr offset)
{
    assign(std::data(data), std::size(data), offset);
}
template <typename T>
void Buffer<T>::assign(const std::vector<T>& data, GLintptr offset)
{
    assign(std::data(data), std::size(data), offset);
}
template <typename T>
void Buffer<T>::assign(const T* data, GLsizeiptr count, GLintptr offset)
{
    assert(offset + count <= m_size && "Invalid Size and/or offset.");
    glNamedBufferSubData(*m_buffer, offset * sizeof(T), count * sizeof(T), data);
}

template <typename T>
void Buffer<T>::assign(const T& data, GLintptr index)
{
    assign(&data, 1, index);
}

template <typename T>
void Buffer<T>::resize(size_t newSize, BufferStorageMask flags)
{
    resize(newSize, T(), flags);
}

template <typename T>
void Buffer<T>::resize(size_t newSize, const T& element, BufferStorageMask flags)
{
    auto oldData = data();
    oldData.resize(newSize, element);
    *this = Buffer(oldData, flags);
}

template <typename T>
void Buffer<T>::resize(size_t newSize)
{
    resize(newSize, T());
}

template <typename T>
void Buffer<T>::resize(size_t newSize, const T& element)
{
    auto oldData = data();
    oldData.resize(newSize, element);
    *this = Buffer(oldData, m_storageFlags);
}

template <typename T>
void Buffer<T>::bind(GLenum target, std::variant<GLuint, BufferBinding> index) const
{
    bind(target, index, 0, m_size);
}
template <typename T>
void Buffer<T>::bind(GLenum target, std::variant<GLuint, BufferBinding> index, GLintptr offset, GLsizeiptr count) const
{
    assert(offset + count <= m_size && "Invalid Size and/or offset.");
    const GLuint i = std::holds_alternative<GLuint>(index) ? std::get<GLuint>(index) : static_cast<GLuint>(std::get<BufferBinding>(index));
    glBindBufferRange(target, i, *m_buffer, offset * sizeof(T), count * sizeof(T));
}

template <typename T>
T* Buffer<T>::map(BufferAccessMask access)
{
    return map(m_size, 0, access);
}
template <typename T>
T* Buffer<T>::map(GLsizeiptr count, GLintptr offset, BufferAccessMask access)
{
    return static_cast<T*>(
            glMapNamedBufferRange(*m_buffer, offset, count * sizeof(T), access));
}
template <typename T>
GLsizeiptr Buffer<T>::size() const
{
    return m_size;
}

template <typename T>
std::vector<T> Buffer<T>::data() const
{
    return data(m_size, 0);
}
template <typename T>
std::vector<T> Buffer<T>::data(GLsizeiptr count, GLintptr offset) const
{
    if(m_size == 0)
        return std::vector<T>();
    assert(offset + count <= m_size && "Invalid Size and/or offset.");
    std::vector<T> result(count);
    glGetNamedBufferSubData(
            *m_buffer, offset * sizeof(T), count * sizeof(T), result.data());
    return result;
}

template <typename T>
GLbuffer Buffer<T>::id() const
{
    return m_buffer;
}
