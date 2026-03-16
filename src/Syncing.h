#pragma once
#include <GL/glew.h>

class Syncing
{
private:
	GLsync fence;

public:
	inline GLenum ClientWaitSync(GLbitfield flags, GLuint64 timeout)
	{
		return glClientWaitSync(fence, flags, timeout);
	}
	~Syncing()
	{
		glDeleteSync(fence);
	}
	void FenceSync(GLenum condition, GLbitfield flags)
	{
		fence = glFenceSync(condition, flags);
	}
	void GetSynciv(GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values)
	{
		glGetSynciv(fence, pname, bufSize, length, values);
	}
	GLboolean IsSync()
	{
		return glIsSync(fence);
	}
	void WaitSync(GLbitfield flags, GLuint64 timeout)
	{
		glWaitSync(fence, flags, timeout);
	}
};