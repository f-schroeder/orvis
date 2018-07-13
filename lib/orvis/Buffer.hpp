#pragma once

#include <array>
#include <glbinding/gl/gl.h>
#include <vector>
#include <variant>
#include "Createable.hpp"
#include "Binding.hpp"

using namespace gl;

/**
 * @brief Wraps OpenGL buffers. Uses Direct State Access for its operations.
 * @tparam T The type of object stored in the buffer.
 */
template <typename T>
class Buffer : Createable<Buffer<T>>
{
public:
    /**
     * @brief Create a buffer without storage. It will remain invalid until calling Buffer::resize.
     */
    Buffer();

    /**
     * @brief Create a buffer from an std::array.
     * @tparam S Array size
     * @param data An array of data to be stored in the buffer.
     * @param flags Storage usage flags for glNamedBufferStorage.
     */
    template <size_t S>
    explicit Buffer(const std::array<T, S>& data, BufferStorageMask flags = GL_NONE_BIT);

    /**
     * @brief Create a buffer from an std::initializer_list.
     * @param data An initializer list to be stored in the buffer.
     * @param flags Storage usage flags for glNamedBufferStorage.
     */
    explicit Buffer(const std::initializer_list<T>& data, BufferStorageMask flags = GL_NONE_BIT);

    /**
     * @brief Create a buffer from an std::vector.
     * @param data A vector to be stored in the buffer.
     * @param flags Storage usage flags for glNamedBufferStorage.
     */
    explicit Buffer(const std::vector<T>& data, BufferStorageMask flags = GL_NONE_BIT);

    /**
     * @brief Creates a buffer for one object.
     * @param data Object to be stored in the buffer.
     * @param flags Storage usage flags for glNamedBufferStorage.
     */
    explicit Buffer(const T& data, BufferStorageMask flags);

    /**
     * @brief Creates a buffer from a pointer-size pair.
     * @param data Pointer to the data storage to be stored in the buffer.
     * @param count Number of elements contained in the data storage.
     * @param flags Storage usage flags for glNamedBufferStorage.
     */
    Buffer(const T* data, GLsizeiptr count, BufferStorageMask flags = GL_NONE_BIT);

    /**
     * @brief Destroys the OpenGL buffer if not set to 0 by move operations.
     */
    ~Buffer();

    /**
     * @brief Copies the buffer contents to a new buffer with the same flags and size, but different
     * ID.
     * @param other Buffer to copy.
     */
    Buffer(const Buffer& other);

    /**
     * @brief Move-constructing a buffer invalidates the moved buffer ID to 0 so it does not get
     * deleted.
     * @param other Buffer to move.
     */
    Buffer(Buffer&& other) noexcept;

    /**
     * @brief Copies the buffer contents to a new buffer with the same flags and size, but different
     * ID. The assignment target buffer needs to be recreated (probably with a new ID) because it
     * has immutable storage.
     * @param other Buffer to copy.
     */
    Buffer& operator=(const Buffer& other);

    /**
     * @brief Move-assigning a buffer invalidates the moved buffer ID to 0 so it does not get
     * deleted. The target buffer will take in the source buffer's ID after deleting its old one.
     * @param other Buffer to move.
     */
    Buffer& operator=(Buffer&& other) noexcept;

    /**
     * @brief Assigns the given data to the buffer at the given offset.
     * @tparam S The array size.
     * @param data An array to be assigned.
     * @param offset Buffer offset index where the assignment will start at.
     */
    template <size_t S>
    void assign(const std::array<T, S>& data, GLintptr offset = 0);

    /**
     * @brief Assigns the given data to the buffer at the given offset.
     * @param data An initializer_list to be assigned.
     * @param offset Buffer offset index where the assignment will start at.
     */
    void assign(const std::initializer_list<T>& data, GLintptr offset = 0);

    /**
     * @brief Assigns the given data to the buffer at the given offset.
     * @param data A vector to be assigned.
     * @param offset Buffer offset index where the assignment will start at.
     */
    void assign(const std::vector<T>& data, GLintptr offset = 0);

    /**
     * @brief Assigns the given data to the buffer at the given offset.
     * @param data A data pointer containing the elements to assign.
     * @param count The number of elements to assign.
     * @param offset Buffer offset index where the assignment will start at.
     */
    void assign(const T* data, GLsizeiptr count, GLintptr offset = 0);

    /**
     * @brief Assigns one element to the buffer at the given index.
     * @param data The element to assign.
     * @param index Buffer offset index to which the data will be assigned.
     */
    void assign(const T& data, GLintptr index = 0);

    /**
     * @brief Resize the buffer to the new size, using its current storage flags.
     * @param newSize The new element count in this buffer.
     */
    void resize(size_t newSize);

    /**
     * @brief Resize the buffer to the new size, using its current storage flags and assign a
     * default value to newly allocated positions.
     * @param newSize The new element count in this buffer.
     * @param element The element to assign to new positions.
     */
    void resize(size_t newSize, const T& element);

    /**
     * @brief Resize the buffer to the new size, using its new storage flags and assign a
     * default value to newly allocated positions.
     * @param newSize The new element count in this buffer.
     * @param flags Storage usage flags for glNamedBufferStorage.
     */
    void resize(size_t newSize, BufferStorageMask flags = GL_NONE_BIT);

    /**
     * @brief Resize the buffer to the new size, using new storage flags and assign a
     * default value to newly allocated positions.
     * @param newSize The new element count in this buffer.
     * @param element The element to assign to new positions.
     * @param flags Storage usage flags for glNamedBufferStorage.
     */
    void resize(size_t newSize, const T& element, BufferStorageMask flags = GL_NONE_BIT);

    /**
     * @brief Binds the buffer to an opengl target at a given binding index using glBindBufferRange.
     * @param target The OpenGL buffer binding target.
     * @param index The binding index to bind to.
     */
    void bind(GLenum target, std::variant<GLuint, BufferBinding> index) const;

    /**
     * @brief Binds the buffer to an opengl target at a given binding index using glBindBufferRange.
     * @param target The OpenGL buffer binding target.
     * @param index The binding index to bind to.
     * @param offset The element index offset into the buffer.
     * @param count The number of elements the binding will refer to.
     */
    void bind(GLenum target, std::variant<GLuint, BufferBinding> index, GLintptr offset, GLsizeiptr count) const;

    /**
     * @brief Maps the buffer to a CPU accessible data pointer.
     * @param access Buffer mapping access flags for glMapNamedBufferRange.
     * @return The mapped data.
     */
    T* map(BufferAccessMask access);

    /**
     * @brief Maps a sub-range of the buffer to a CPU accessible data pointer.
     * @param count Number of elements to map.
     * @param offset Offset into the buffer at which the mapping will start.
     * @param access Buffer mapping access flags for glMapNamedBufferRange.
     * @return The mapped data.
     */
    T* map(GLsizeiptr count, GLintptr offset, BufferAccessMask access);

    /**
     * @return The number of elements this buffer contains.
     */
    GLsizeiptr size() const;

    /**
     * @brief Retrieves the full buffer data via glGetNamedBufferSubData.
     * @return A vector containing the buffer data.
     */
    std::vector<T> data() const;

    /**
     * @brief Retrieves a sub-range of the buffer data via glGetNamedBufferSubData.
     * @param count Number of elements to retrieve.
     * @param offset Element offset at which to start.
     * @return A vector containing the buffer data.
     */
    std::vector<T> data(GLsizeiptr count, GLintptr offset = 0) const;

    /**
     * @return The OpenGL buffer name (ID).
     */
    GLuint id() const;

private:
    BufferStorageMask   m_storageFlags = GL_NONE_BIT;
    GLsizeiptr          m_size = 0;
    GLuint              m_buffer = GL_INVALID_INDEX;
};

#include "Buffer.inl"
