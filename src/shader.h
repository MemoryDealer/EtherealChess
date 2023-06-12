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

#if !defined(SHADER_H)
#define SHADER_H

#include <string>
#include <map>
#include <utility>
#include "gl.h"

//-----------------------------------------------------------------------------
// Only supports up to OpenGL 3.3 GLSL functionality with respect to the
// glUniform() and glVertexAttrib() APIs.
//
// No support currently for geometry shaders. Just vertex and fragment shaders.
//
// How to use the Shader class:
//
// 1. Declare an instance of the class.
//	    Shader myShader;
//    Once a new instance of the Shader class is created the Shader class will
//    create a new GLSL program object and make it active for the Shader class
//    to attach shaders to.
//
// 2. Load the vertex shaders that make up your shader program by calling:
//      myShader.loadVertexShader("myVertexShader.vert");
//    or
//      myShader.loadVertexShader(MAKEINTRESOURCE(MY_VERTEX_SHADER_FILE_ID));
//
//    Multiple vertex shaders can be loaded into the Shader class if your
//    vertex shader is spread across multiple vertex shader source files.
//    But remember that your vertex shader source files must contain only one
//    main() function.
//
// 3. Load the fragment shaders that make up your shader program by calling:
//      myShader.loadFragmentShader("myFragmentShader.frag");
//    or
//      myShader.loadFragmentShaderFromResource(MAKEINTRESOURCE(MY_FRAGMENT_SHADER_FILE_ID));
//
//    Multiple fragment shaders can be loaded into the Shader class if your
//    fragment shader is spread across multiple fragment shader source files.
//    But remember that your fragment shader source files must contain only
//    one main() function.
//
// NOTE:
//	  The loadVertexShader(), loadVertexShader(), loadFragmentShader(), and
//    loadFragmentShaderFromResource() methods will load the shader source
//    files into GLSL shader objects, compile the shader source code, and
//    then attach the shader objects to the active program object.
//
// 4. Once all vertex and fragment shaders are loaded call:
//      myShader.link();
//	  to link the shaders into a shader program.
//
// 5. To use the shader program call:
//      myShader.enable();
//    This basically calls:
//      glUseProgram(myShader.program());
//
// 6. When you're finished using your shader program call:
//      myShader.disable();
//    This basically calls:
//      glUseProgram(0);
//
// 7. Set uniform variables by calling one of the Shader class' uniform()
//    methods. These are almost identical to the glUniform() API functions.
//    The Shader class' versions don't begin with the 'gl' prefix and the
//    first argument is the C-style string of your uniform variable instead
//    of the GLint location that it is bound to. The Shader class will call
//    glGetUniformLocation() to find the correct GLint location that the
//    uniform variable name has been bound to.
//
// 8. Similarly the Shader class contains its own versions of the
//    glVertexAttrib() API functions. The Shader class' versions don't begin
//    with the 'gl' prefix and the first argument is the C-style string name
//    of your generic vertex attribute instead of the GLint index that it is
//    bound to. The Shader class will call glGetAttribLocation() to find the
//    correct GLint index that the generic vertex attribute has been bound to.
//-----------------------------------------------------------------------------

class Shader
{
public:
	Shader();
	~Shader();
				
	void destroy();
	
	void disable();
	void enable();
	
	void enableVertexAttribArray(const char *pszName);
	
	const std::string &lastError() const;

	bool link();
	
	bool loadFragmentShader(const char *pszFilename);
	bool loadFragmentShaderFromResource(int resourceId);
	
	bool loadVertexShader(const char *pszFilename);
	bool loadVertexShaderFromResource(int resourceId);
	
	GLuint programObject() const;
	bool isEnabled(void) const;

	void uniform1(const char *pszName, GLfloat v0);
	void uniform1(const char *pszName, GLint v0);
	void uniform1(const char *pszName, GLuint v0);
	void uniform1(const char *pszName, GLsizei count, const GLfloat *value);
	void uniform1(const char *pszName, GLsizei count, const GLint *value);
	void uniform1(const char *pszName, GLsizei count, const GLuint *value);

	void uniform2(const char *pszName, GLfloat v0, GLfloat v1);
	void uniform2(const char *pszName, GLint v0, GLint v1);
	void uniform2(const char *pszName, GLuint v0, GLuint v1);
	void uniform2(const char *pszName, GLsizei count, const GLfloat *value);
	void uniform2(const char *pszName, GLsizei count, const GLint *value);
	void uniform2(const char *pszName, GLsizei count, const GLuint *value);
	
	void uniform3(const char *pszName, GLfloat v0, GLfloat v1, GLfloat v2);
	void uniform3(const char *pszName, GLint v0, GLint v1, GLint v2);
	void uniform3(const char *pszName, GLuint v0, GLuint v1, GLuint v2);
	void uniform3(const char *pszName, GLsizei count, const GLfloat *value);
	void uniform3(const char *pszName, GLsizei count, const GLint *value);
	void uniform3(const char *pszName, GLsizei count, const GLuint *value);
	
	void uniform4(const char *pszName, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	void uniform4(const char *pszName, GLint v0, GLint v1, GLint v2, GLint v3);
	void uniform4(const char *pszName, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
	void uniform4(const char *pszName, GLsizei count, const GLfloat *value);
	void uniform4(const char *pszName, GLsizei count, const GLint *value);
	void uniform4(const char *pszName, GLsizei count, const GLuint *value);
	
	void uniformMatrix2(const char *pszName, GLsizei count, GLboolean transpose, const GLfloat *value);
	void uniformMatrix2x3(const char *pszName, GLsizei count, GLboolean transpose, const GLfloat *value);
	void uniformMatrix2x4(const char *pszName, GLsizei count, GLboolean transpose, const GLfloat *value);
	
	void uniformMatrix3(const char *pszName, GLsizei count, GLboolean transpose, const GLfloat *value);
	void uniformMatrix3x2(const char *pszName, GLsizei count, GLboolean transpose, const GLfloat *value);
	void uniformMatrix3x4(const char *pszName, GLsizei count, GLboolean transpose, const GLfloat *value);
	
	void uniformMatrix4(const char *pszName, GLsizei count, GLboolean transpose, const GLfloat *value);
	void uniformMatrix4x2(const char *pszName, GLsizei count, GLboolean transpose, const GLfloat *value);
	void uniformMatrix4x3(const char *pszName, GLsizei count, GLboolean transpose, const GLfloat *value);

	void vertexAttrib1(const char *pszName, GLdouble x);
	void vertexAttrib1(const char *pszName, GLfloat x);
	void vertexAttrib1(const char *pszName, GLshort x);
	void vertexAttrib1(const char *pszName, const GLdouble *v);
	void vertexAttrib1(const char *pszName, const GLfloat *v);
	void vertexAttrib1(const char *pszName, const GLshort *v);
	
	void vertexAttrib2(const char *pszName, GLdouble x, GLdouble y);
	void vertexAttrib2(const char *pszName, GLfloat x, GLfloat y);
	void vertexAttrib2(const char *pszName, GLshort x, GLshort y);
	void vertexAttrib2(const char *pszName, const GLdouble *v);
	void vertexAttrib2(const char *pszName, const GLfloat *v);
	void vertexAttrib2(const char *pszName, const GLshort *v);
	
	void vertexAttrib3(const char *pszName, GLdouble x, GLdouble y, GLdouble z);
	void vertexAttrib3(const char *pszName, GLfloat x, GLfloat y, GLfloat z);
	void vertexAttrib3(const char *pszName, GLshort x, GLshort y, GLshort z);
	void vertexAttrib3(const char *pszName, const GLdouble *v);
	void vertexAttrib3(const char *pszName, const GLfloat *v);
	void vertexAttrib3(const char *pszName, const GLshort *v);
	
	void vertexAttrib4(const char *pszName, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	void vertexAttrib4(const char *pszName, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	void vertexAttrib4(const char *pszName, GLshort x, GLshort y, GLshort z, GLshort w);
	void vertexAttrib4(const char *pszName, const GLbyte *v);
	void vertexAttrib4(const char *pszName, const GLdouble *v);
	void vertexAttrib4(const char *pszName, const GLfloat *v);
	void vertexAttrib4(const char *pszName, const GLint *v);
	void vertexAttrib4(const char *pszName, const GLshort *v);
	void vertexAttrib4(const char *pszName, const GLubyte *v);
	void vertexAttrib4(const char *pszName, const GLuint *v);
	void vertexAttrib4(const char *pszName, const GLushort *v);
	
	void vertexAttrib4N(const char *pszName, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
	void vertexAttrib4N(const char *pszName, const GLbyte *v);
	void vertexAttrib4N(const char *pszName, const GLint *v);
	void vertexAttrib4N(const char *pszName, const GLshort *v);
	void vertexAttrib4N(const char *pszName, const GLubyte *v);
	void vertexAttrib4N(const char *pszName, const GLuint *v);
	void vertexAttrib4N(const char *pszName, const GLushort *v);
	
	void vertexAttribI1(const char *pszName, GLint x);
	void vertexAttribI1(const char *pszName, GLuint x);
	void vertexAttribI1(const char *pszName, const GLint *v);
	void vertexAttribI1(const char *pszName, const GLuint *v);
	
	void vertexAttribI2(const char *pszName, GLint x, GLint y);
	void vertexAttribI2(const char *pszName, GLuint x, GLuint y);
	void vertexAttribI2(const char *pszName, const GLint *v);
	void vertexAttribI2(const char *pszName, const GLuint *v);
	
	void vertexAttribI3(const char *pszName, GLint x, GLint y, GLint z);
	void vertexAttribI3(const char *pszName, GLuint x, GLuint y, GLuint z);
	void vertexAttribI3(const char *pszName, const GLint *v);
	void vertexAttribI3(const char *pszName, const GLuint *v);
	
	void vertexAttribI4(const char *pszName, GLint x, GLint y, GLint z, GLint w);
	void vertexAttribI4(const char *pszName, GLuint x, GLuint y, GLuint z, GLuint w);
	void vertexAttribI4(const char *pszName, const GLbyte *v);
	void vertexAttribI4(const char *pszName, const GLint *v);
	void vertexAttribI4(const char *pszName, const GLshort *v);
	void vertexAttribI4(const char *pszName, const GLubyte *v);
	void vertexAttribI4(const char *pszName, const GLuint *v);
	void vertexAttribI4(const char *pszName, const GLushort *v);
	void vertexAttribIPointer(const char *pszName, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
	
	void vertexAttribP1(const char *pszName, GLenum type, GLboolean normalized, GLuint value);
	void vertexAttribP1(const char *pszName, GLenum type, GLboolean normalized, const GLuint *value);
	void vertexAttribP2(const char *pszName, GLenum type, GLboolean normalized, GLuint value);
	void vertexAttribP2(const char *pszName, GLenum type, GLboolean normalized, const GLuint *value);
	void vertexAttribP3(const char *pszName, GLenum type, GLboolean normalized, GLuint value);
	void vertexAttribP3(const char *pszName, GLenum type, GLboolean normalized, const GLuint *value);
	void vertexAttribP4(const char *pszName, GLenum type, GLboolean normalized, GLuint value);
	void vertexAttribP4(const char *pszName, GLenum type, GLboolean normalized, const GLuint *value);
	void vertexAttribPointer(const char *pszName, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);

private:
	Shader(const Shader &);
	Shader &operator=(const Shader &);

	void compileError(const char *pszFilename, GLenum shaderType, GLuint shader);
	void compileError(int resourceId, GLenum shaderType, GLuint shader);
	void createProgram();
	void deleteShaders(const std::map<std::string, GLuint> &shaders) const;
	void deleteShaders(const std::map<int, GLuint> &shaders) const;
	void error(std::string msg);
	GLint getAttribLocation(const char *pszName);
	std::string getProgramInfoLog(GLuint program) const;
	std::string getShaderInfoLog(GLuint shader) const;
	GLint getUniformLocation(const char *pszName);
	bool loadShader(const char *pszFilename, GLenum shaderType);
	bool loadShaderFromResource(int resourceId, GLenum shaderType);

	GLuint m_program;
	bool m_enabled;
	std::string m_lastError;
	std::map<std::string, GLuint> m_fileVertexShaders;
	std::map<std::string, GLuint> m_fileFragmentShaders;
	std::map<int, GLuint> m_resourceVertexShaders;
	std::map<int, GLuint> m_resourceFragmentShaders;
};

#endif