
template <typename T>
void VertexArray::setVertexBuffer(const Buffer<T>& buffer, GLuint binding,
    GLintptr offset, GLsizei stride)
{
    setVertexBuffer(buffer.id(), binding, offset, stride);
}

template <typename T>
void VertexArray::setElementBuffer(const Buffer<T>& buffer)
{
    setElementBuffer(buffer.id());
}
