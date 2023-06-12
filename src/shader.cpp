//-----------------------------------------------------------------------------
// Copyright (c) 2011-2012 dhpoware. All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include <windows.h>
#include <string>
#include <sstream>
#include <fstream>
#include "shader.h"

//-----------------------------------------------------------------------------
// Public methods.
//-----------------------------------------------------------------------------

Shader::Shader() : m_program(0)
{
}

Shader::~Shader()
{
	destroy();
}

void Shader::destroy()
{
	if (!m_fileFragmentShaders.empty())
	{
		deleteShaders(m_fileFragmentShaders);
		m_fileFragmentShaders.clear();
	}
	
	if (!m_fileVertexShaders.empty())
	{
		deleteShaders(m_fileVertexShaders);
		m_fileVertexShaders.clear();
	}
	
	if (!m_resourceFragmentShaders.empty())
	{
		deleteShaders(m_resourceFragmentShaders);
		m_resourceFragmentShaders.clear();
	}
	
	if (!m_resourceVertexShaders.empty())
	{
		deleteShaders(m_resourceVertexShaders);
		m_resourceVertexShaders.clear();
	}
	
	if (m_program != 0)
	{
		glDeleteProgram(m_program);
		m_program = 0;
	}
}

void Shader::disable()
{
	glUseProgram(0);
	m_enabled = false;
}

void Shader::enable()
{
	glUseProgram(m_program);
	m_enabled = true;
}

void Shader::enableVertexAttribArray(const char *pszName)
{
	glEnableVertexAttribArray(getAttribLocation(pszName));
}

const std::string &Shader::lastError() const
{
	return m_lastError;
}

bool Shader::link()
{
	bool linked = false;
	GLint isLinked = GL_FALSE;

	glLinkProgram(m_program);
	glGetProgramiv(m_program, GL_LINK_STATUS, &isLinked);

	// Mark all attached shaders for deletion. These shaders aren't deleted
	// right now because they are already attached to a shader program.
	// When the shader program is deleted these shaders will be
	// automatically detached and deleted.

	for (std::map<std::string, GLuint>::iterator i = m_fileFragmentShaders.begin(); i != m_fileFragmentShaders.end(); ++i)
		glDeleteShader(i->second);

	for (std::map<std::string, GLuint>::iterator i = m_fileVertexShaders.begin(); i != m_fileVertexShaders.end(); ++i)
		glDeleteShader(i->second);

	for (std::map<int, GLuint>::iterator i = m_resourceFragmentShaders.begin(); i != m_resourceFragmentShaders.end(); ++i)
		glDeleteShader(i->second);

	for (std::map<int, GLuint>::iterator i = m_resourceVertexShaders.begin(); i != m_resourceVertexShaders.end(); ++i)
		glDeleteShader(i->second);

	if (isLinked == GL_FALSE)
	{
		std::ostringstream output;

		output << "Error linking GLSL shaders into a shader program." << std::endl;
		output << "Program info log:" << std::endl;
		output << getProgramInfoLog(m_program);

		error(output.str());

		glDeleteProgram(m_program);
		m_program = 0;

		linked = false;
	}
	else
	{
		m_lastError.clear();
		linked = true;
	}

	return linked;
}

bool Shader::loadFragmentShader(const char *pszFilename)
{
	return loadShader(pszFilename, GL_FRAGMENT_SHADER);
}

bool Shader::loadFragmentShaderFromResource(int resourceId)
{
	return loadShaderFromResource(resourceId, GL_FRAGMENT_SHADER);
}

bool Shader::loadVertexShader(const char *pszFilename)
{
	return loadShader(pszFilename, GL_VERTEX_SHADER);
}

bool Shader::loadVertexShaderFromResource(int resourceId)
{
	return loadShaderFromResource(resourceId, GL_VERTEX_SHADER);
}

GLuint Shader::programObject() const
{
	return m_program;
}

bool Shader::isEnabled(void) const
{
	return m_enabled;
}

//-----------------------------------------------------------------------------
// glUniform API helper methods.
//-----------------------------------------------------------------------------

void Shader::uniform1(const char *pszName, GLfloat v0)
{
	glUniform1f(getUniformLocation(pszName), v0);
}

void Shader::uniform1(const char *pszName, GLint v0)
{
	glUniform1i(getUniformLocation(pszName), v0);
}

void Shader::uniform1(const char *pszName, GLuint v0)
{
	glUniform1ui(getUniformLocation(pszName), v0);
}

void Shader::uniform1(const char *pszName, GLsizei count, const GLfloat *value)
{
	glUniform1fv(getUniformLocation(pszName), count, value);
}

void Shader::uniform1(const char *pszName, GLsizei count, const GLint *value)
{
	glUniform1iv(getUniformLocation(pszName), count, value);
}

void Shader::uniform1(const char *pszName, GLsizei count, const GLuint *value)
{
	glUniform1uiv(getUniformLocation(pszName), count, value);
}

void Shader::uniform2(const char *pszName, GLfloat v0, GLfloat v1)
{
	glUniform2f(getUniformLocation(pszName), v0, v1);
}

void Shader::uniform2(const char *pszName, GLint v0, GLint v1)
{
	glUniform2i(getUniformLocation(pszName), v0, v1);
}

void Shader::uniform2(const char *pszName, GLuint v0, GLuint v1)
{
	glUniform2ui(getUniformLocation(pszName), v0, v1);
}

void Shader::uniform2(const char *pszName, GLsizei count, const GLfloat *value)
{
	glUniform2fv(getUniformLocation(pszName), count, value);
}

void Shader::uniform2(const char *pszName, GLsizei count, const GLint *value)
{
	glUniform2iv(getUniformLocation(pszName), count, value);
}

void Shader::uniform2(const char *pszName, GLsizei count, const GLuint *value)
{
	glUniform2uiv(getUniformLocation(pszName), count, value);
}

void Shader::uniform3(const char *pszName, GLfloat v0, GLfloat v1, GLfloat v2)
{
	glUniform3f(getUniformLocation(pszName), v0, v1, v2);
}

void Shader::uniform3(const char *pszName, GLint v0, GLint v1, GLint v2)
{
	glUniform3i(getUniformLocation(pszName), v0, v1, v2);
}

void Shader::uniform3(const char *pszName, GLuint v0, GLuint v1, GLuint v2)
{
	glUniform3ui(getUniformLocation(pszName), v0, v1, v2);
}

void Shader::uniform3(const char *pszName, GLsizei count, const GLfloat *value)
{
	glUniform3fv(getUniformLocation(pszName), count, value);
}

void Shader::uniform3(const char *pszName, GLsizei count, const GLint *value)
{
	glUniform3iv(getUniformLocation(pszName), count, value);
}

void Shader::uniform3(const char *pszName, GLsizei count, const GLuint *value)
{
	glUniform3uiv(getUniformLocation(pszName), count, value);
}

void Shader::uniform4(const char *pszName, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	glUniform4f(getUniformLocation(pszName), v0, v1, v2, v3);
}

void Shader::uniform4(const char *pszName, GLint v0, GLint v1, GLint v2, GLint v3)
{
	glUniform4i(getUniformLocation(pszName), v0, v1, v2, v3);
}

void Shader::uniform4(const char *pszName, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	glUniform4ui(getUniformLocation(pszName), v0, v1, v2, v3);
}

void Shader::uniform4(const char *pszName, GLsizei count, const GLfloat *value)
{
	glUniform4fv(getUniformLocation(pszName), count, value);
}

void Shader::uniform4(const char *pszName, GLsizei count, const GLint *value)
{
	glUniform4iv(getUniformLocation(pszName), count, value);
}

void Shader::uniform4(const char *pszName, GLsizei count, const GLuint *value)
{
	glUniform4uiv(getUniformLocation(pszName), count, value);
}

void Shader::uniformMatrix2(const char *pszName, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	glUniformMatrix2fv(getUniformLocation(pszName), count, transpose, value);
}

void Shader::uniformMatrix2x3(const char *pszName, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	glUniformMatrix2x3fv(getUniformLocation(pszName), count, transpose, value);
}

void Shader::uniformMatrix2x4(const char *pszName, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	glUniformMatrix2x4fv(getUniformLocation(pszName), count, transpose, value);
}

void Shader::uniformMatrix3(const char *pszName, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	glUniformMatrix3fv(getUniformLocation(pszName), count, transpose, value);
}

void Shader::uniformMatrix3x2(const char *pszName, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	glUniformMatrix3x2fv(getUniformLocation(pszName), count, transpose, value);
}

void Shader::uniformMatrix3x4(const char *pszName, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	glUniformMatrix3x4fv(getUniformLocation(pszName), count, transpose, value);
}

void Shader::uniformMatrix4(const char *pszName, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	glUniformMatrix4fv(getUniformLocation(pszName), count, transpose, value);
}

void Shader::uniformMatrix4x2(const char *pszName, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	glUniformMatrix4x2fv(getUniformLocation(pszName), count, transpose, value);
}

void Shader::uniformMatrix4x3(const char *pszName, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	glUniformMatrix4x3fv(getUniformLocation(pszName), count, transpose, value);
}

//-----------------------------------------------------------------------------
// glVertexAttrib API helper methods.
//-----------------------------------------------------------------------------

void Shader::vertexAttrib1(const char *pszName, GLdouble x)
{
	glVertexAttrib1d(getAttribLocation(pszName), x);
}

void Shader::vertexAttrib1(const char *pszName, GLfloat x)
{
	glVertexAttrib1f(getAttribLocation(pszName), x);
}

void Shader::vertexAttrib1(const char *pszName, GLshort x)
{
	glVertexAttrib1s(getAttribLocation(pszName), x);
}

void Shader::vertexAttrib1(const char *pszName, const GLdouble *v)
{
	glVertexAttrib1dv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib1(const char *pszName, const GLfloat *v)
{
	glVertexAttrib1fv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib1(const char *pszName, const GLshort *v)
{
	glVertexAttrib1sv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib2(const char *pszName, GLdouble x, GLdouble y)
{
	glVertexAttrib2d(getAttribLocation(pszName), x, y);
}

void Shader::vertexAttrib2(const char *pszName, GLfloat x, GLfloat y)
{
	glVertexAttrib2f(getAttribLocation(pszName), x, y);
}

void Shader::vertexAttrib2(const char *pszName, GLshort x, GLshort y)
{
	glVertexAttrib2s(getAttribLocation(pszName), x, y);
}

void Shader::vertexAttrib2(const char *pszName, const GLdouble *v)
{
	glVertexAttrib2dv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib2(const char *pszName, const GLfloat *v)
{
	glVertexAttrib2fv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib2(const char *pszName, const GLshort *v)
{
	glVertexAttrib2sv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib3(const char *pszName, GLdouble x, GLdouble y, GLdouble z)
{
	glVertexAttrib3d(getAttribLocation(pszName), x, y, z);
}

void Shader::vertexAttrib3(const char *pszName, GLfloat x, GLfloat y, GLfloat z)
{
	glVertexAttrib3f(getAttribLocation(pszName), x, y, z);
}

void Shader::vertexAttrib3(const char *pszName, GLshort x, GLshort y, GLshort z)
{
	glVertexAttrib3s(getAttribLocation(pszName), x, y, z);
}

void Shader::vertexAttrib3(const char *pszName, const GLdouble *v)
{
	glVertexAttrib3dv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib3(const char *pszName, const GLfloat *v)
{
	glVertexAttrib3fv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib3(const char *pszName, const GLshort *v)
{
	glVertexAttrib3sv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib4(const char *pszName, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	glVertexAttrib4d(getAttribLocation(pszName), x, y, z, w);
}

void Shader::vertexAttrib4(const char *pszName, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	glVertexAttrib4f(getAttribLocation(pszName), x, y, z, w);
}

void Shader::vertexAttrib4(const char *pszName, GLshort x, GLshort y, GLshort z, GLshort w)
{
	glVertexAttrib4s(getAttribLocation(pszName), x, y, z, w);
}

void Shader::vertexAttrib4(const char *pszName, const GLbyte *v)
{
	glVertexAttrib4bv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib4(const char *pszName, const GLdouble *v)
{
	glVertexAttrib4dv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib4(const char *pszName, const GLfloat *v)
{
	glVertexAttrib4fv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib4(const char *pszName, const GLint *v)
{
	glVertexAttrib4iv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib4(const char *pszName, const GLshort *v)
{
	glVertexAttrib4sv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib4(const char *pszName, const GLubyte *v)
{
	glVertexAttrib4ubv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib4(const char *pszName, const GLuint *v)
{
	glVertexAttrib4uiv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib4(const char *pszName, const GLushort *v)
{
	glVertexAttrib4usv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib4N(const char *pszName, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
	glVertexAttrib4Nub(getAttribLocation(pszName), x, y, z, w);
}

void Shader::vertexAttrib4N(const char *pszName, const GLbyte *v)
{
	glVertexAttrib4Nbv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib4N(const char *pszName, const GLint *v)
{
	glVertexAttrib4Niv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib4N(const char *pszName, const GLshort *v)
{
	glVertexAttrib4Nsv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib4N(const char *pszName, const GLubyte *v)
{
	glVertexAttrib4Nubv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib4N(const char *pszName, const GLuint *v)
{
	glVertexAttrib4Nuiv(getAttribLocation(pszName), v);
}

void Shader::vertexAttrib4N(const char *pszName, const GLushort *v)
{
	glVertexAttrib4Nusv(getAttribLocation(pszName), v);
}

void Shader::vertexAttribI1(const char *pszName, GLint x)
{
	glVertexAttribI1i(getAttribLocation(pszName), x);
}

void Shader::vertexAttribI1(const char *pszName, GLuint x)
{
	glVertexAttribI1ui(getAttribLocation(pszName), x);
}

void Shader::vertexAttribI1(const char *pszName, const GLint *v)
{
	glVertexAttribI1iv(getAttribLocation(pszName), v);
}

void Shader::vertexAttribI1(const char *pszName, const GLuint *v)
{
	glVertexAttribI1uiv(getAttribLocation(pszName), v);
}

void Shader::vertexAttribI2(const char *pszName, GLint x, GLint y)
{
	glVertexAttribI2i(getAttribLocation(pszName), x, y);
}

void Shader::vertexAttribI2(const char *pszName, GLuint x, GLuint y)
{
	glVertexAttribI2ui(getAttribLocation(pszName), x, y);
}

void Shader::vertexAttribI2(const char *pszName, const GLint *v)
{
	glVertexAttribI2iv(getAttribLocation(pszName), v);
}

void Shader::vertexAttribI2(const char *pszName, const GLuint *v)
{
	glVertexAttribI2uiv(getAttribLocation(pszName), v);
}

void Shader::vertexAttribI3(const char *pszName, GLint x, GLint y, GLint z)
{
	glVertexAttribI3i(getAttribLocation(pszName), x, y, z);
}

void Shader::vertexAttribI3(const char *pszName, GLuint x, GLuint y, GLuint z)
{
	glVertexAttribI3ui(getAttribLocation(pszName), x, y, z);
}

void Shader::vertexAttribI3(const char *pszName, const GLint *v)
{
	glVertexAttribI3iv(getAttribLocation(pszName), v);
}

void Shader::vertexAttribI3(const char *pszName, const GLuint *v)
{
	glVertexAttribI3uiv(getAttribLocation(pszName), v);
}

void Shader::vertexAttribI4(const char *pszName, GLint x, GLint y, GLint z, GLint w)
{
	glVertexAttribI4i(getAttribLocation(pszName), x, y, z, w);
}

void Shader::vertexAttribI4(const char *pszName, GLuint x, GLuint y, GLuint z, GLuint w)
{
	glVertexAttribI4ui(getAttribLocation(pszName), x, y, z, w);
}

void Shader::vertexAttribI4(const char *pszName, const GLbyte *v)
{
	glVertexAttribI4bv(getAttribLocation(pszName), v);
}

void Shader::vertexAttribI4(const char *pszName, const GLint *v)
{
	glVertexAttribI4iv(getAttribLocation(pszName), v);
}

void Shader::vertexAttribI4(const char *pszName, const GLshort *v)
{
	glVertexAttribI4sv(getAttribLocation(pszName), v);
}

void Shader::vertexAttribI4(const char *pszName, const GLubyte *v)
{
	glVertexAttribI4ubv(getAttribLocation(pszName), v);
}

void Shader::vertexAttribI4(const char *pszName, const GLuint *v)
{
	glVertexAttribI4uiv(getAttribLocation(pszName), v);
}

void Shader::vertexAttribI4(const char *pszName, const GLushort *v)
{
	glVertexAttribI4usv(getAttribLocation(pszName), v);
}

void Shader::vertexAttribIPointer(const char *pszName, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	glVertexAttribIPointer(getAttribLocation(pszName), size, type, stride, pointer);
}

void Shader::vertexAttribP1(const char *pszName, GLenum type, GLboolean normalized, GLuint value)
{
	glVertexAttribP1ui(getAttribLocation(pszName), type, normalized, value);
}

void Shader::vertexAttribP1(const char *pszName, GLenum type, GLboolean normalized, const GLuint *value)
{
	glVertexAttribP1uiv(getAttribLocation(pszName), type, normalized, value);
}

void Shader::vertexAttribP2(const char *pszName, GLenum type, GLboolean normalized, GLuint value)
{
	glVertexAttribP2ui(getAttribLocation(pszName), type, normalized, value);
}

void Shader::vertexAttribP2(const char *pszName, GLenum type, GLboolean normalized, const GLuint *value)
{
	glVertexAttribP2uiv(getAttribLocation(pszName), type, normalized, value);
}

void Shader::vertexAttribP3(const char *pszName, GLenum type, GLboolean normalized, GLuint value)
{
	glVertexAttribP3ui(getAttribLocation(pszName), type, normalized, value);
}

void Shader::vertexAttribP3(const char *pszName, GLenum type, GLboolean normalized, const GLuint *value)
{
	glVertexAttribP3uiv(getAttribLocation(pszName), type, normalized, value);
}

void Shader::vertexAttribP4(const char *pszName, GLenum type, GLboolean normalized, GLuint value)
{
	glVertexAttribP4ui(getAttribLocation(pszName), type, normalized, value);
}

void Shader::vertexAttribP4(const char *pszName, GLenum type, GLboolean normalized, const GLuint *value)
{
	glVertexAttribP4uiv(getAttribLocation(pszName), type, normalized, value);
}

void Shader::vertexAttribPointer(const char *pszName, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer)
{
	glVertexAttribPointer(getAttribLocation(pszName), size, type, normalized, stride, pointer);
}

//-----------------------------------------------------------------------------
// Private methods.
//-----------------------------------------------------------------------------

void Shader::compileError(int resourceId, GLenum shaderType, GLuint shader)
{
	std::ostringstream name;
	name << "Resource ID " << resourceId;

	compileError(name.str().c_str(), shaderType, shader);
}

void Shader::compileError(const char *pszFilename, GLenum shaderType, GLuint shader)
{
	std::ostringstream output;

	output << "Error compiling ";

	switch (shaderType)
	{
	case GL_FRAGMENT_SHADER:
		output << "fragment shader: ";
		break;

	case GL_VERTEX_SHADER:
		output << "vertex shader: ";
		break;

	default:
		output << ": ";
		break;
	}

	output << "\"" << pszFilename << "\".";
	output << std::endl << std::endl;
	output << "Shader info log:" << std::endl;
	output << getShaderInfoLog(shader);

	error(output.str());
}

void Shader::createProgram()
{
	if (!m_program)
	{
		GLuint program = glCreateProgram();

		if (glIsProgram(program) == GL_TRUE)
			m_program = program;
		else
			error("A fatal error occurred creating the GLSL program object.");
	}
}

void Shader::deleteShaders(const std::map<std::string, GLuint> &shaders) const
{
	for (std::map<std::string, GLuint>::const_iterator i = shaders.begin(); i != shaders.end(); ++i)
	{
		GLuint shader = i->second;
		GLint isFlaggedForDeletion = GL_FALSE;

		glGetShaderiv(i->second, GL_DELETE_STATUS, &isFlaggedForDeletion);

		if (isFlaggedForDeletion == GL_FALSE)
			glDeleteShader(shader);
	}
}

void Shader::deleteShaders(const std::map<int, GLuint> &shaders) const
{
	for (std::map<int, GLuint>::const_iterator i = shaders.begin(); i != shaders.end(); ++i)
	{
		GLuint shader = i->second;
		GLint isFlaggedForDeletion = GL_FALSE;

		glGetShaderiv(i->second, GL_DELETE_STATUS, &isFlaggedForDeletion);

		if (isFlaggedForDeletion == GL_FALSE)
			glDeleteShader(shader);
	}
}

void Shader::error(std::string msg)
{
	if (m_lastError.empty())
	{
		m_lastError = msg;

	}
	else
	{
		std::ostringstream output;

		output << msg;
		output << std::endl << std::endl;
		output << m_lastError;

		m_lastError = output.str();
	}
}

GLint Shader::getAttribLocation(const char *pszName)
{
	GLint location = glGetAttribLocation(m_program, pszName);

	if (location == -1)
	{
		std::string name = pszName;
		std::string::size_type index = name.find("gl_");
		std::ostringstream output;

		output << pszName;

		std::string::npos;

		switch (index)
		{
		default:
			output << " does not correspond to an active attribute.";
			break;

		case 0:
			output << " starts with the reserved prefix \"gl_\".";
			break;
		}

		error(output.str());
	}

	return location;
}

std::string Shader::getProgramInfoLog(GLuint program) const
{
	GLsizei infoLogSize = 0;
	std::string infoLog;

	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogSize);
	
	if (infoLogSize > 0)
	{
		infoLog.resize(infoLogSize);
		glGetProgramInfoLog(program, infoLogSize, &infoLogSize, &infoLog[0]);
	}

	return infoLog;
}

std::string Shader::getShaderInfoLog(GLuint shader) const
{
	GLsizei infoLogSize = 0;
	std::string infoLog;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogSize);

	if (infoLogSize > 0)
	{
		infoLog.resize(infoLogSize);
		glGetShaderInfoLog(shader, infoLogSize, &infoLogSize, &infoLog[0]);
	}

	return infoLog;
}

GLint Shader::getUniformLocation(const char *pszName)
{
	GLint location = glGetUniformLocation(m_program, pszName);

	if (location == -1)
	{
		std::string name = pszName;
		std::string::size_type index = name.find("gl_");
		std::ostringstream output;

		output << pszName;

		std::string::npos;

		switch (index)
		{
		default:
			output << " does not correspond to an active uniform variable.";
			break;

		case 0:
			output << " starts with the reserved prefix \"gl_\".";
			break;
		}

		error(output.str());
	}

	return location;
}

bool Shader::loadShader(const char *pszFilename, GLenum shaderType)
{
	bool loaded = false;
	std::ifstream file(pszFilename, std::ios::binary);

	if (file.is_open())
	{
		file.seekg(0, std::ios::end);

		unsigned int fileSize = static_cast<unsigned int>(file.tellg());
		std::string source;

		source.resize(fileSize);
		file.seekg(0, std::ios::beg);
		file.read(&source[0], fileSize);

		GLuint shader = glCreateShader(shaderType);

		if (glIsShader(shader) == GL_TRUE)
		{
			const GLchar *pszSource = reinterpret_cast<const GLchar *>(source.c_str());
			GLint length = 0;

			glShaderSource(shader, 1, &pszSource, 0);
			glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &length);

			if (length > 0)
			{
				GLint isCompiled = GL_FALSE;

				glCompileShader(shader);
				glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

				if (isCompiled == GL_FALSE)
				{
					compileError(pszFilename, shaderType, shader);
					glDeleteShader(shader);
					shader = 0;
				}
				else
				{
					createProgram();
					glAttachShader(m_program, shader);

					switch (shaderType)
					{
					case GL_FRAGMENT_SHADER:
						m_fileFragmentShaders[pszFilename] = shader;
						break;

					case GL_VERTEX_SHADER:
						m_fileVertexShaders[pszFilename] = shader;
						break;

					default:
						break;
					}

					loaded = true;
				}
			}
		}
	}

	return loaded;
}

bool Shader::loadShaderFromResource(int resourceId, GLenum shaderType)
{
	// Embedding Your Shader Source Code In Your C/C++ application
	// -----------------------------------------------------------
	//	1. Add a define to your resource.h file.
	//		#define MY_VERTEX_SHADER_FILE 1000
	//		#define MY_FRAGMENT_SHADER_FILE 1001
	//
	//	2. Add an entry in your resource.rc file.
	//		MY_VERTEX_SHADER_FILE RCDATA DISCARDABLE "myShader.vert"
	//		MY_FRAGMENT_SHADER_FILE RCDATA DISCARDABLE "myShader.frag"
	//
	//	3. Call loadShaderFromResource().
	//		loadShaderFromResource(MAKEINTRESOURCE(MY_VERTEX_SHADER_FILE), GL_VERTEX_SHADER)
	//		loadShaderFromResource(MAKEINTRESOURCE(MY_FRAGMENT_SHADER_FILE), GL_FRAGMENT_SHADER)

	bool loaded = false;
	HMODULE hModule = GetModuleHandle(0);
	HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(resourceId), RT_RCDATA);

	if (hResource)
	{
		DWORD dwSize = SizeofResource(hModule, hResource);
		HGLOBAL hGlobal = LoadResource(hModule, hResource);

		if (hGlobal)
		{
			if (LPVOID pData = LockResource(hGlobal))
			{
				std::string buffer;

				buffer.assign(reinterpret_cast<const char *>(pData), dwSize);
				UnlockResource(hGlobal);

				GLuint shader = glCreateShader(shaderType);

				if (glIsShader(shader) == GL_TRUE)
				{
					const GLchar *pszSource = reinterpret_cast<const GLchar *>(buffer.c_str());
					GLint length = 0;

					glShaderSource(shader, 1, &pszSource, 0);
					glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &length);

					if (length > 0)
					{
						GLint isCompiled = GL_FALSE;

						glCompileShader(shader);
						glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

						if (isCompiled == GL_FALSE)
						{
							compileError(resourceId, shaderType, shader);
							glDeleteShader(shader);
							shader = 0;
						}
						else
						{
							createProgram();
							glAttachShader(m_program, shader);

							switch (shaderType)
							{
							case GL_FRAGMENT_SHADER:
								m_resourceFragmentShaders[resourceId] = shader;
								break;

							case GL_VERTEX_SHADER:
								m_resourceVertexShaders[resourceId] = shader;
								break;

							default:
								break;
							}

							loaded = true;
						}
					}
				}
			}
		}
	}

	return loaded;
}