#pragma once
#include <glbinding/gl/gl.h>
#include <GLFW/glfw3.h>
#include <memory>

using namespace gl;

template<void(*deleterFunc)(GLuint)>
struct GlPtr {
	GLuint id;
	GlPtr(std::nullptr_t = nullptr) : id(0) {}
	GlPtr(GLuint id) : id(id) {}
	operator GLuint() const { return id; }
	friend bool operator == (GlPtr x, GlPtr y) { return x.id == y.id; }
	friend bool operator != (GlPtr x, GlPtr y) { return x.id != y.id; }

	~GlPtr() { if (glfwGetCurrentContext() != nullptr) deleterFunc(id); }
};

// deleters
inline void deleteTexture(GLuint id) { if (id != 0) glDeleteTextures(1, &id); }
inline void deleteSampler(GLuint id) { if (id != 0) glDeleteSamplers(1, &id); }
inline void deleteShader(GLuint id) { if (glIsShader(id)) glDeleteShader(id); }
inline void deleteProgram(GLuint id) { if (glIsProgram(id)) glDeleteProgram(id); }
inline void deleteBuffer(GLuint id) { if (id != 0) glDeleteBuffers(1, &id); }
inline void deleteFramebuffer(GLuint id) { if (id != 0) glDeleteFramebuffers(1, &id); }
inline void deleteVertexArray(GLuint id) { if (glIsVertexArray(id)) glDeleteVertexArrays(1, &id); }
inline void deleteQuery(GLuint id) { if (id != 0) glDeleteQueries(1, &id); }

// types
typedef std::shared_ptr<GlPtr<deleteTexture>> GLtexture;
typedef std::shared_ptr<GlPtr<deleteSampler>> GLsampler;
typedef std::shared_ptr<GlPtr<deleteShader>> GLshader;
typedef std::shared_ptr<GlPtr<deleteProgram>> GLprogram;
typedef std::shared_ptr<GlPtr<deleteBuffer>> GLbuffer;
typedef std::shared_ptr<GlPtr<deleteFramebuffer>> GLframebuffer;
typedef std::shared_ptr<GlPtr<deleteVertexArray>> GLvertexArray;
typedef std::shared_ptr<GlPtr<deleteQuery>> GLquery;

// creation functions
inline GLtexture glCreateTexture(GLenum target) { GLuint ret; glCreateTextures(target, 1, &ret); return std::make_shared<GlPtr<deleteTexture>>(ret); }
inline GLsampler glCreateSampler() { GLuint ret; glCreateSamplers(1, &ret); return std::make_shared<GlPtr<deleteSampler>>(ret); }
inline GLshader glCreateShaderRAII(GLenum type) { GLuint ret = gl::glCreateShader(type); return std::make_shared<GlPtr<deleteShader>>(ret); }
inline GLprogram glCreateProgramRAII() { GLuint ret = gl::glCreateProgram(); return std::make_shared<GlPtr<deleteProgram>>(ret); }
inline GLbuffer glCreateBuffer() { GLuint ret; glCreateBuffers(1, &ret); return std::make_shared<GlPtr<deleteBuffer>>(ret); }
inline GLframebuffer glCreateFramebuffer() { GLuint ret; glCreateFramebuffers(1, &ret); return std::make_shared<GlPtr<deleteFramebuffer>>(ret); }
inline GLvertexArray glCreateVertexArray() { GLuint ret; glCreateVertexArrays(1, &ret); return std::make_shared<GlPtr<deleteVertexArray>>(ret); }
inline GLquery glCreateQuery(GLenum target) { GLuint ret; glCreateQueries(target, 1, &ret); return std::make_shared<GlPtr<deleteQuery>>(ret); }
