#pragma once

#include <cassert>

template <typename T>
Buffer<T>::Buffer()
        : m_storageFlags(0), m_size(0)
{
    BufferCompat::get().createBuffers(1, &m_buffer);
}

template <typename T>
template <size_t S>
Buffer<T>::Buffer(const std::array<T, S>& data, GLbitfield flags)
        : Buffer(std::data(data), std::size(data), flags)
{
}
template <typename T>
Buffer<T>::Buffer(const std::initializer_list<T>& data, GLbitfield flags)
        : Buffer(std::data(data), std::size(data), flags)
{
}
template <typename T>
Buffer<T>::Buffer(const std::vector<T>& data, GLbitfield flags)
        : Buffer(std::data(data), std::size(data), flags)
{
}

template <typename T>
Buffer<T>::Buffer(const T& data, GLbitfield flags)
        : Buffer(&data, 1, flags)
{
}
template <typename T>
Buffer<T>::Buffer(const T* data, GLsizeiptr count, GLbitfield flags)
        : Buffer()
{
    m_size         = count;
    m_storageFlags = flags;
    BufferCompat::get().namedBufferStorage(m_buffer, sizeof(T) * count, data, flags);
}

template <typename T>
Buffer<T>::~Buffer()
{
    if(m_buffer != 0)
        glDeleteBuffers(1, &m_buffer);
}

template <typename T>
Buffer<T>::Buffer(const Buffer& other)
        : Buffer(other.data(), other.m_storageFlags)
{
}
template <typename T>
Buffer<T>::Buffer(Buffer&& other) noexcept
        : m_size(other.m_size)
        , m_storageFlags(other.m_storageFlags)
        , m_buffer(other.m_buffer)
{
    other.m_buffer = 0;
}
template <typename T>
Buffer<T>& Buffer<T>::operator=(const Buffer& other)
{
    if(m_buffer != 0)
        glDeleteBuffers(1, &m_buffer);
    BufferCompat::get().createBuffers(1, &m_buffer);
    m_size         = other.m_size;
    m_storageFlags = other.m_storageFlags;
    BufferCompat::get().namedBufferStorage(
            m_buffer, sizeof(T) * m_size, other.data().data(), m_storageFlags);
    return *this;
}
template <typename T>
Buffer<T>& Buffer<T>::operator=(Buffer&& other) noexcept
{
    if(m_buffer != 0)
        glDeleteBuffers(1, &m_buffer);
    m_size         = other.m_size;
    m_storageFlags = other.m_storageFlags;
    m_buffer       = other.m_buffer;
    other.m_buffer = 0;
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
    BufferCompat::get().namedBufferSubData(m_buffer, offset * sizeof(T), count * sizeof(T), data);
}

template <typename T>
void Buffer<T>::assign(const T& data, GLintptr index)
{
    assign(&data, 1, index);
}

template <typename T>
void Buffer<T>::resize(size_t newSize, GLbitfield flags)
{
    resize(newSize, T(), flags);
}

template <typename T>
void Buffer<T>::resize(size_t newSize, const T& element, GLbitfield flags)
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
void Buffer<T>::bind(GLenum target, GLuint index) const
{
    bind(target, index, 0, m_size);
}
template <typename T>
void Buffer<T>::bind(GLenum target, GLuint index, GLintptr offset, GLsizeiptr count) const
{
    assert(offset + count <= m_size && "Invalid Size and/or offset.");
    glBindBufferRange(target, index, m_buffer, offset * sizeof(T), count * sizeof(T));
}

template <typename T>
T* Buffer<T>::map(GLbitfield access)
{
    return map(m_size, 0, access);
}
template <typename T>
T* Buffer<T>::map(GLsizeiptr count, GLintptr offset, GLbitfield access)
{
    return static_cast<T*>(
            BufferCompat::get().mapNamedBufferRange(m_buffer, offset, count * sizeof(T), access));
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
    BufferCompat::get().getNamedBufferSubData(
            m_buffer, offset * sizeof(T), count * sizeof(T), result.data());
    return result;
}

template <typename T>
GLuint Buffer<T>::id() const
{
    return m_buffer;
}
