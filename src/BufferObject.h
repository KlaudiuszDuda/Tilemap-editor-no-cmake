#pragma once

#include "types.h"

#include <GL/glew.h>

struct DrawArraysIndirectCommand {
    unsigned int count;
    unsigned int instanceCount;
    unsigned int first;
    unsigned int baseInstance;

    DrawArraysIndirectCommand(unsigned int c, unsigned int i, unsigned int f, unsigned int b)
        : count(c), instanceCount(i), first(f), baseInstance(b) {
    }
};

template<u32 nRendererIds>
class VertexArray {
public:
    VertexArray() { glGenVertexArrays(nRendererIds, m_RendererId); }
    ~VertexArray() { glDeleteVertexArrays(nRendererIds, m_RendererId); }

    inline void bind(u32 i) const { glBindVertexArray(m_RendererId[i]); }
    inline static void unbind() { glBindVertexArray(0); }

    inline void VertexAttribDivisor(GLuint location, GLuint divisor)
    {
        glVertexAttribDivisor(location, divisor);
    }
    inline void EnableVertexAttribArray(GLuint location)
    {
        glEnableVertexAttribArray(location);
    }
    inline void DisableVertexAttribArray(GLuint location)
    {
        glDisableVertexAttribArray(location);
    }
    inline void VertexAttribPointer(GLuint location, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* offset)
    {
        glVertexAttribPointer(location, size, type, normalized, stride, offset);
    }
    inline void VertexAttribIPointer(GLuint location, GLint size, GLenum type, GLsizei stride, const GLvoid* offset)
    {
        glVertexAttribIPointer(location, size, type, stride, offset);
    }
    inline void VertexAttribLPointer(GLuint location, GLint size, GLenum type, GLsizei stride, const GLvoid* offset)
    {
        glVertexAttribLPointer(location, size, type, stride, offset);
    }
    inline void VertexAttribI1ui(GLuint location, GLuint v0)
    {
        glVertexAttribI1ui(location, v0);
    }
    inline void VertexAttribI1i(GLuint location, GLint v0)
    {
        glVertexAttribI1i(location, v0);
    }
    inline void DrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei primcount)
    {
        glDrawArraysInstanced(mode, first, count, primcount);
    }
    inline void DrawElementsInstanced(GLenum mode, GLint first, GLsizei count, const void* indices, GLsizei primcount)
    {
        glDrawElementsInstanced(mode, first, count, indices, primcount);
    }
    inline void DrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count, GLsizei primcount, GLuint baseinstance)
    {
        glDrawArraysInstancedBaseInstance(mode, first, count, primcount, baseinstance);
    }

    u32& getID(u32 i) { return m_RendererId[i]; }

    u32 m_RendererId[nRendererIds];
};

template<u32 nBuffers>
class Buffer
{
public:
    Buffer()
    {
        glGenBuffers(nBuffers, m_Buffer);
    }
    ~Buffer()
    {
        glDeleteBuffers(nBuffers, m_Buffer);
    }
    inline void BindBuffer(GLenum target, GLuint bufferIndex)
    {
        glBindBuffer(target, m_Buffer[bufferIndex]);
    }
    inline void BufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
    {
        glBufferData(target, size, data, usage);
    }
    inline void BindBufferBase(GLenum target, GLuint location, GLuint bufferIndex)
    {
        glBindBufferBase(target, location, m_Buffer[bufferIndex]);
    }
    inline void BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
    {
        glBufferSubData(target, offset, size, data);
    }
    inline void* MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
    {
        void* dst = glMapBufferRange(target, offset, length, access);
        return dst;
    }
    inline void UnmapBuffer(GLenum target)
    {
        glUnmapBuffer(target);
    }

    inline u32 getBuffer(u32 i) const { return m_Buffer[i]; }

private:
    u32 m_Buffer[nBuffers];
};