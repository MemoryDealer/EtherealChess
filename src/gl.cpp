//-----------------------------------------------------------------------------
// Copyright (c) 2007-2012 dhpoware. All Rights Reserved.
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

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <set>
#include <sstream>
#include <string>
#include "gl.h"

namespace
{
	WNDCLASSEX g_wcl;
	HWND g_hWnd;
	HDC g_hDC;
	HGLRC g_hRC;

	LRESULT CALLBACK DummyGLWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_CREATE:
			if (!(g_hDC = GetDC(hWnd)))
				return -1;
			break;

		case WM_DESTROY:
			if (g_hDC)
			{
				if (g_hRC)
				{
					wglMakeCurrent(g_hDC, 0);
					wglDeleteContext(g_hRC);
					g_hRC = 0;
				}

				ReleaseDC(hWnd, g_hDC);
				g_hDC = 0;
			}

			PostQuitMessage(0);
			return 0;

		default:
			break;
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	bool CreateDummyGLWindow()
	{
		g_wcl.cbSize = sizeof(g_wcl);
		g_wcl.style = CS_OWNDC;
		g_wcl.lpfnWndProc = DummyGLWndProc;
		g_wcl.hInstance = reinterpret_cast<HINSTANCE>(GetModuleHandle(0));
		g_wcl.lpszClassName = "DummyGLWindowClass";

		if (!RegisterClassEx(&g_wcl))
			return false;

		g_hWnd = CreateWindow(g_wcl.lpszClassName, "", WS_OVERLAPPEDWINDOW,
					0, 0, 0, 0, 0, 0, g_wcl.hInstance, 0);

		if (!g_hWnd)
			return false;

		PIXELFORMATDESCRIPTOR pfd = {0};

		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 24;
		pfd.cDepthBits = 16;
		pfd.iLayerType = PFD_MAIN_PLANE;

		int pf = ChoosePixelFormat(g_hDC, &pfd);

		if (!SetPixelFormat(g_hDC, pf, &pfd))
			return false;

		if (!(g_hRC = wglCreateContext(g_hDC)))
			return false;

		if (!wglMakeCurrent(g_hDC, g_hRC))
			return false;

		return true;
	}

	void DestroyDummyGLWindow()
	{
		if (g_hWnd)
		{
			PostMessage(g_hWnd, WM_CLOSE, 0, 0);

			BOOL bRet;
			MSG msg;

			while ((bRet = GetMessage(&msg, 0, 0, 0)) != 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		UnregisterClass(g_wcl.lpszClassName, g_wcl.hInstance);
	}
}

namespace gl
{
    bool ExtensionSupported(const char *pszExtensionName)
    {
        static std::set<std::string> extensions;

        if (extensions.empty())
		{
			std::vector<std::string> supportedExtensions = GetExtensions();

			if (!supportedExtensions.empty())
				extensions.insert(supportedExtensions.begin(), supportedExtensions.end());
		}

        if (extensions.find(pszExtensionName) == extensions.end())
            return false;

        return true;
    }

	std::vector<std::string> GetExtensions()
	{
		std::vector<std::string> extensions;

		if (CreateDummyGLWindow())
		{
			// The dummy window will only be at most an OpenGL 2.1 rendering
			// context so we can still use the deprecated GL_EXTENSIONS flag
			// to glGetString().

			if (const char *pszExtensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)))
			{
				std::string str(pszExtensions);
				std::istringstream is(str);

				while (is >> str)
					extensions.push_back(str);
			}

			// WGL_ARB_extensions_string.

			typedef const char *(WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC)(HDC);

			PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB =
				reinterpret_cast<PFNWGLGETEXTENSIONSSTRINGARBPROC>(
				wglGetProcAddress("wglGetExtensionsStringARB"));

			if (wglGetExtensionsStringARB)
			{
				std::string str(wglGetExtensionsStringARB(wglGetCurrentDC()));
				std::istringstream is(str);

				while (is >> str)
					extensions.push_back(str);
			}

			// Sort all the supported extensions.

			std::sort(extensions.begin(), extensions.end());

			DestroyDummyGLWindow();
		}

		return extensions;
	}

    void GetGLVersion(int &major, int &minor)
    {
        static int majorGL = 0;
        static int minorGL = 0;

        if (!majorGL && !minorGL)
        {
            const char *pszVersion = reinterpret_cast<const char *>(glGetString(GL_VERSION));

            if (pszVersion)
                sscanf(pszVersion, "%d.%d", &majorGL, &minorGL);
        }

        major = majorGL;
        minor = minorGL;
    }

    void GetGLSLVersion(int &major, int &minor)
    {
        static int majorGLSL = 0;
        static int minorGLSL = 0;

        if (!majorGLSL && !minorGLSL)
        {
            int majorGL = 0;
            int minorGL = 0;

            GetGLVersion(majorGL, minorGL);

            if (majorGL >= 2)
            {
                const char *pszShaderVersion = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));

                if (pszShaderVersion)
                    sscanf(pszShaderVersion, "%d.%d", &majorGLSL, &minorGLSL);
            }
            else
            {
                const char *pszExtension = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));

                if (pszExtension)
                {
                    if (strstr(pszExtension, "GL_ARB_shading_language_100"))
                    {
                        majorGLSL = 1;
                        minorGLSL = 0;
                    }
                }
            }
        }

        major = majorGLSL;
        minor = minorGLSL;
    }

    bool SupportsGLVersion(int major, int minor)
    {
        static int majorGL = 0;
        static int minorGL = 0;

        if (!majorGL && !minorGL)
            GetGLVersion(majorGL, minorGL);

        if (majorGL > major)
            return true;

        if (majorGL == major && minorGL >= minor)
            return true;

        return false;
    }

    bool SupportsGLSLVersion(int major, int minor)
    {
        static int majorGLSL = 0;
        static int minorGLSL = 0;

        if (!majorGLSL && !minorGLSL)
            GetGLSLVersion(majorGLSL, minorGLSL);

        if (majorGLSL > major)
            return true;

        if (majorGLSL == major && minorGLSL >= minor)
            return true;

        return false;
    }
}

#define LOAD_ENTRYPOINT(name, var, type) \
    if (!var) \
    { \
        var = reinterpret_cast<type>(wglGetProcAddress(name)); \
        assert(var != 0); \
    }

//
// OpenGL 1.2
//

void glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    typedef void (APIENTRY * PFNGLBLENDCOLORPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
    static PFNGLBLENDCOLORPROC pfnBlendColor = 0;
    LOAD_ENTRYPOINT("glBlendColor", pfnBlendColor, PFNGLBLENDCOLORPROC);
    pfnBlendColor(red, green, blue, alpha);
}

void glBlendEquation(GLenum mode)
{
    typedef void (APIENTRY * PFNGLBLENDEQUATIONPROC) (GLenum mode);
    static PFNGLBLENDEQUATIONPROC pfnBlendEquation = 0;
    LOAD_ENTRYPOINT("glBlendEquation", pfnBlendEquation, PFNGLBLENDEQUATIONPROC);
    pfnBlendEquation(mode);
}

void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
    typedef void (APIENTRY * PFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
    static PFNGLDRAWRANGEELEMENTSPROC pfnDrawRangeElements = 0;
    LOAD_ENTRYPOINT("glDrawRangeElements", pfnDrawRangeElements, PFNGLDRAWRANGEELEMENTSPROC);
    pfnDrawRangeElements(mode, start, end, count, type, indices);
}

void glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    typedef void (APIENTRY * PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    static PFNGLTEXIMAGE3DPROC pfnTexImage3D = 0;
    LOAD_ENTRYPOINT("glTexImage3D", pfnTexImage3D, PFNGLTEXIMAGE3DPROC);
    pfnTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
}

void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels)
{
    typedef void (APIENTRY * PFNGLTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
    static PFNGLTEXSUBIMAGE3DPROC pfnTexSubImage3D = 0;
    LOAD_ENTRYPOINT("glTexSubImage3D", pfnTexSubImage3D, PFNGLTEXSUBIMAGE3DPROC);
    pfnTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

void glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    typedef void (APIENTRY * PFNGLCOPYTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    static PFNGLCOPYTEXSUBIMAGE3DPROC pfnCopyTexSubImage3D = 0;
    LOAD_ENTRYPOINT("glCopyTexSubImage3D", pfnCopyTexSubImage3D, PFNGLCOPYTEXSUBIMAGE3DPROC);
    pfnCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

//
// OpenGL 1.3
//

void glActiveTexture(GLenum texture)
{
    typedef void (APIENTRY * PFNGLACTIVETEXTUREPROC) (GLenum texture);
    static PFNGLACTIVETEXTUREPROC pfnActiveTexture = 0;
    LOAD_ENTRYPOINT("glActiveTexture", pfnActiveTexture, PFNGLACTIVETEXTUREPROC);
    pfnActiveTexture(texture);
}

void glSampleCoverage(GLclampf value, GLboolean invert)
{
    typedef void (APIENTRY * PFNGLSAMPLECOVERAGEPROC) (GLclampf value, GLboolean invert);
    static PFNGLSAMPLECOVERAGEPROC pfnSampleCoverage = 0;
    LOAD_ENTRYPOINT("glSampleCoverage", pfnSampleCoverage, PFNGLSAMPLECOVERAGEPROC);
    pfnSampleCoverage(value, invert);
}

void glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data)
{
    typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE3DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
    static PFNGLCOMPRESSEDTEXIMAGE3DPROC pfnCompressedTexImage3D = 0;
    LOAD_ENTRYPOINT("glCompressedTexImage3D", pfnCompressedTexImage3D, PFNGLCOMPRESSEDTEXIMAGE3DPROC);
    pfnCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
}

void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data)
{
    typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
    static PFNGLCOMPRESSEDTEXIMAGE2DPROC pfnCompressedTexImage2D = 0;
    LOAD_ENTRYPOINT("glCompressedTexImage2D", pfnCompressedTexImage2D, PFNGLCOMPRESSEDTEXIMAGE2DPROC);
    pfnCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}

void glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data)
{
    typedef void (APIENTRY * PFNGLCOMPRESSEDTEXIMAGE1DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
    static PFNGLCOMPRESSEDTEXIMAGE1DPROC pfnCompressedTexImage1D = 0;
    LOAD_ENTRYPOINT("glCompressedTexImage1D", pfnCompressedTexImage1D, PFNGLCOMPRESSEDTEXIMAGE1DPROC);
    pfnCompressedTexImage1D(target, level, internalformat, width, border, imageSize, data);
}

void glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
    static PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC pfnCompressedTexSubImage3D = 0;
    LOAD_ENTRYPOINT("glCompressedTexSubImage3D", pfnCompressedTexSubImage3D, PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC);
    pfnCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
    static PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC pfnCompressedTexSubImage2D = 0;
    LOAD_ENTRYPOINT("glCompressedTexSubImage2D", pfnCompressedTexSubImage2D, PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC);
    pfnCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

void glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    typedef void (APIENTRY * PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
    static PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC pfnCompressedTexSubImage1D = 0;
    LOAD_ENTRYPOINT("glCompressedTexSubImage1D", pfnCompressedTexSubImage1D, PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC);
    pfnCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data);
}

void glGetCompressedTexImage(GLenum target, GLint level, GLvoid *img)
{
    typedef void (APIENTRY * PFNGLGETCOMPRESSEDTEXIMAGEPROC) (GLenum target, GLint level, GLvoid *img);
    static PFNGLGETCOMPRESSEDTEXIMAGEPROC pfnGetCompressedTexImage = 0;
    LOAD_ENTRYPOINT("glGetCompressedTexImage", pfnGetCompressedTexImage, PFNGLGETCOMPRESSEDTEXIMAGEPROC);
    pfnGetCompressedTexImage(target, level, img);
}

//
// OpenGl 1.4
//

void glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    typedef void (APIENTRY * PFNGLBLENDFUNCSEPARATEPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
    static PFNGLBLENDFUNCSEPARATEPROC pfnBlendFuncSeparate = 0;
    LOAD_ENTRYPOINT("glBlendFuncSeparate", pfnBlendFuncSeparate, PFNGLBLENDFUNCSEPARATEPROC);
    pfnBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}

void glMultiDrawArrays(GLenum mode, GLint *first, GLsizei *count, GLsizei primcount)
{
    typedef void (APIENTRY * PFNGLMULTIDRAWARRAYSPROC) (GLenum mode, GLint *first, GLsizei *count, GLsizei primcount);
    static PFNGLMULTIDRAWARRAYSPROC pfnMultiDrawArrays = 0;
    LOAD_ENTRYPOINT("glMultiDrawArrays", pfnMultiDrawArrays, PFNGLMULTIDRAWARRAYSPROC);
    pfnMultiDrawArrays(mode, first, count, primcount);
}

void glMultiDrawElements(GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount)
{
    typedef void (APIENTRY * PFNGLMULTIDRAWELEMENTSPROC) (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount);
    static PFNGLMULTIDRAWELEMENTSPROC pfnMultiDrawElements = 0;
    LOAD_ENTRYPOINT("glMultiDrawElements", pfnMultiDrawElements, PFNGLMULTIDRAWELEMENTSPROC);
    pfnMultiDrawElements(mode, count, type, indices, primcount);
}

void glPointParameterf(GLenum pname, GLfloat param)
{
    typedef void (APIENTRY * PFNGLPOINTPARAMETERFPROC) (GLenum pname, GLfloat param);
    static PFNGLPOINTPARAMETERFPROC pfnPointParameterf = 0;
    LOAD_ENTRYPOINT("glPointParameterf", pfnPointParameterf, PFNGLPOINTPARAMETERFPROC);
    pfnPointParameterf(pname, param);
}

void glPointParameterfv(GLenum pname, const GLfloat *params)
{
    typedef void (APIENTRY * PFNGLPOINTPARAMETERFVPROC) (GLenum pname, const GLfloat *params);
    static PFNGLPOINTPARAMETERFVPROC pfnPointParameterfv = 0;
    LOAD_ENTRYPOINT("glPointParameterfv", pfnPointParameterfv, PFNGLPOINTPARAMETERFVPROC);
    pfnPointParameterfv(pname, params);
}

void glPointParameteri(GLenum pname, GLint param)
{
    typedef void (APIENTRY * PFNGLPOINTPARAMETERIPROC) (GLenum pname, GLint param);
    static PFNGLPOINTPARAMETERIPROC pfnPointParameteri = 0;
    LOAD_ENTRYPOINT("glPointParameteri", pfnPointParameteri, PFNGLPOINTPARAMETERIPROC);
    pfnPointParameteri(pname, param);
}

void glPointParameteriv(GLenum pname, const GLint *params)
{
    typedef void (APIENTRY * PFNGLPOINTPARAMETERIVPROC) (GLenum pname, const GLint *params);
    static PFNGLPOINTPARAMETERIVPROC pfnPointParameteriv = 0;
    LOAD_ENTRYPOINT("glPointParameteriv", pfnPointParameteriv, PFNGLPOINTPARAMETERIVPROC);
    pfnPointParameteriv(pname, params);
}

//
// OpenGL 1.5
//

void glGenQueries(GLsizei n, GLuint *ids)
{
    typedef void (APIENTRY * PFNGLGENQUERIESPROC) (GLsizei n, GLuint *ids);
    static PFNGLGENQUERIESPROC pfnGenQueries = 0;
    LOAD_ENTRYPOINT("glGenQueries", pfnGenQueries, PFNGLGENQUERIESPROC);
    pfnGenQueries(n, ids);
}

void glDeleteQueries(GLsizei n, const GLuint *ids)
{
    typedef void (APIENTRY * PFNGLDELETEQUERIESPROC) (GLsizei n, const GLuint *ids);
    static PFNGLDELETEQUERIESPROC pfnDeleteQueries = 0;
    LOAD_ENTRYPOINT("glDeleteQueries", pfnDeleteQueries, PFNGLDELETEQUERIESPROC);
    pfnDeleteQueries(n, ids);
}

GLboolean glIsQuery(GLuint id)
{
    typedef GLboolean (APIENTRY * PFNGLISQUERYPROC) (GLuint id);
    static PFNGLISQUERYPROC pfnIsQuery = 0;
    LOAD_ENTRYPOINT("glIsQuery", pfnIsQuery, PFNGLISQUERYPROC);
    return pfnIsQuery(id);
}

void glBeginQuery(GLenum target, GLuint id)
{
    typedef void (APIENTRY * PFNGLBEGINQUERYPROC) (GLenum target, GLuint id);
    static PFNGLBEGINQUERYPROC pfnBeginQuery = 0;
    LOAD_ENTRYPOINT("glBeginQuery", pfnBeginQuery, PFNGLBEGINQUERYPROC);
    pfnBeginQuery(target, id);
}

void glEndQuery(GLenum target)
{
    typedef void (APIENTRY * PFNGLENDQUERYPROC) (GLenum target);
    static PFNGLENDQUERYPROC pfnEndQuery = 0;
    LOAD_ENTRYPOINT("glEndQuery", pfnEndQuery, PFNGLENDQUERYPROC);
    pfnEndQuery(target);
}

void glGetQueryiv(GLenum target, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETQUERYIVPROC) (GLenum target, GLenum pname, GLint *params);
    static PFNGLGETQUERYIVPROC pfnGetQueryiv = 0;
    LOAD_ENTRYPOINT("glGetQueryiv", pfnGetQueryiv, PFNGLGETQUERYIVPROC);
    pfnGetQueryiv(target, pname, params);
}

void glGetQueryObjectiv(GLuint id, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETQUERYOBJECTIVPROC) (GLuint id, GLenum pname, GLint *params);
    static PFNGLGETQUERYOBJECTIVPROC pfnGetQueryObjectiv = 0;
    LOAD_ENTRYPOINT("glGetQueryObjectiv", pfnGetQueryObjectiv, PFNGLGETQUERYOBJECTIVPROC);
    pfnGetQueryObjectiv(id, pname, params);
}

void glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params)
{
    typedef void (APIENTRY * PFNGLGETQUERYOBJECTUIVPROC) (GLuint id, GLenum pname, GLuint *params);
    static PFNGLGETQUERYOBJECTUIVPROC pfnGetQueryObjectuiv = 0;
    LOAD_ENTRYPOINT("glGetQueryObjectuiv", pfnGetQueryObjectuiv, PFNGLGETQUERYOBJECTUIVPROC);
    pfnGetQueryObjectuiv(id, pname, params);
}

void glBindBuffer(GLenum target, GLuint buffer)
{
    typedef void (APIENTRY * PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
    static PFNGLBINDBUFFERPROC pfnBindBuffer = 0;
    LOAD_ENTRYPOINT("glBindBuffer", pfnBindBuffer, PFNGLBINDBUFFERPROC);
    pfnBindBuffer(target, buffer);
}

void glDeleteBuffers(GLsizei n, const GLuint *buffers)
{
    typedef void (APIENTRY * PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
    static PFNGLDELETEBUFFERSPROC pfnDeleteBuffers = 0;
    LOAD_ENTRYPOINT("glDeleteBuffers", pfnDeleteBuffers, PFNGLDELETEBUFFERSPROC);
    pfnDeleteBuffers(n, buffers);
}

void glGenBuffers(GLsizei n, GLuint *buffers)
{
    typedef void (APIENTRY * PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
    static PFNGLGENBUFFERSPROC pfnGenBuffers = 0;
    LOAD_ENTRYPOINT("glGenBuffers", pfnGenBuffers, PFNGLGENBUFFERSPROC);
    pfnGenBuffers(n, buffers);
}

GLboolean glIsBuffer(GLuint buffer)
{
    typedef GLboolean (APIENTRY * PFNGLISBUFFERPROC) (GLuint buffer);
    static PFNGLISBUFFERPROC pfnIsBuffer = 0;
    LOAD_ENTRYPOINT("glIsBuffer", pfnIsBuffer, PFNGLISBUFFERPROC);
    return pfnIsBuffer(buffer);
}

void glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
    typedef void (APIENTRY * PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
    static PFNGLBUFFERDATAPROC pfnBufferData = 0;
    LOAD_ENTRYPOINT("glBufferData", pfnBufferData, PFNGLBUFFERDATAPROC);
    pfnBufferData(target, size, data, usage);
}

void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data)
{
    typedef void (APIENTRY * PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
    static PFNGLBUFFERSUBDATAPROC pfnBufferSubData = 0;
    LOAD_ENTRYPOINT("glBufferSubData", pfnBufferSubData, PFNGLBUFFERSUBDATAPROC);
    pfnBufferSubData(target, offset, size, data);
}

void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data)
{
    typedef void (APIENTRY * PFNGLGETBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data);
    static PFNGLGETBUFFERSUBDATAPROC pfnGetBufferSubData = 0;
    LOAD_ENTRYPOINT("glGetBufferSubData", pfnGetBufferSubData, PFNGLGETBUFFERSUBDATAPROC);
    pfnGetBufferSubData(target, offset, size, data);
}

GLvoid* glMapBuffer(GLenum target, GLenum access)
{
    typedef GLvoid* (APIENTRY * PFNGLMAPBUFFERPROC) (GLenum target, GLenum access);
    static PFNGLMAPBUFFERPROC pfnMapBuffer = 0;
    LOAD_ENTRYPOINT("glMapBuffer", pfnMapBuffer, PFNGLMAPBUFFERPROC);
    return pfnMapBuffer(target, access);
}

GLboolean glUnmapBuffer(GLenum target)
{
    typedef GLboolean (APIENTRY * PFNGLUNMAPBUFFERPROC) (GLenum target);
    static PFNGLUNMAPBUFFERPROC pfnUnmapBuffer = 0;
    LOAD_ENTRYPOINT("glUnmapBuffer", pfnUnmapBuffer, PFNGLUNMAPBUFFERPROC);
    return pfnUnmapBuffer(target);
}

void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
    static PFNGLGETBUFFERPARAMETERIVPROC pfnGetBufferParameteriv = 0;
    LOAD_ENTRYPOINT("glGetBufferParameteriv", pfnGetBufferParameteriv, PFNGLGETBUFFERPARAMETERIVPROC);
    pfnGetBufferParameteriv(target, pname, params);
}

void glGetBufferPointerv(GLenum target, GLenum pname, GLvoid* *params)
{
    typedef void (APIENTRY * PFNGLGETBUFFERPOINTERVPROC) (GLenum target, GLenum pname, GLvoid* *params);
    static PFNGLGETBUFFERPOINTERVPROC pfnGetBufferPointerv = 0;
    LOAD_ENTRYPOINT("glGetBufferPointerv", pfnGetBufferPointerv, PFNGLGETBUFFERPOINTERVPROC);
    pfnGetBufferPointerv(target, pname, params);
}

//
// OpenGL 2.0
//

void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
    typedef void (APIENTRY * PFNGLBLENDEQUATIONSEPARATEPROC) (GLenum modeRGB, GLenum modeAlpha);
    static PFNGLBLENDEQUATIONSEPARATEPROC pfnBlendEquationSeparate = 0;
    LOAD_ENTRYPOINT("glBlendEquationSeparate", pfnBlendEquationSeparate, PFNGLBLENDEQUATIONSEPARATEPROC);
    pfnBlendEquationSeparate(modeRGB, modeAlpha);
}

void glDrawBuffers(GLsizei n, const GLenum *bufs)
{
    typedef void (APIENTRY * PFNGLDRAWBUFFERSPROC) (GLsizei n, const GLenum *bufs);
    static PFNGLDRAWBUFFERSPROC pfnDrawBuffers = 0;
    LOAD_ENTRYPOINT("glDrawBuffers", pfnDrawBuffers, PFNGLDRAWBUFFERSPROC);
    pfnDrawBuffers(n, bufs);
}

void glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
    typedef void (APIENTRY * PFNGLSTENCILOPSEPARATEPROC) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
    static PFNGLSTENCILOPSEPARATEPROC pfnStencilOpSeparate = 0;
    LOAD_ENTRYPOINT("glStencilOpSeparate", pfnStencilOpSeparate, PFNGLSTENCILOPSEPARATEPROC);
    pfnStencilOpSeparate(face, sfail, dpfail, dppass);
}

void glStencilFuncSeparate(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask)
{
    typedef void (APIENTRY * PFNGLSTENCILFUNCSEPARATEPROC) (GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
    static PFNGLSTENCILFUNCSEPARATEPROC pfnStencilFuncSeparate = 0;
    LOAD_ENTRYPOINT("glStencilFuncSeparate", pfnStencilFuncSeparate, PFNGLSTENCILFUNCSEPARATEPROC);
    pfnStencilFuncSeparate(frontfunc, backfunc, ref, mask);
}

void glStencilMaskSeparate(GLenum face, GLuint mask)
{
    typedef void (APIENTRY * PFNGLSTENCILMASKSEPARATEPROC) (GLenum face, GLuint mask);
    static PFNGLSTENCILMASKSEPARATEPROC pfnStencilMaskSeparate = 0;
    LOAD_ENTRYPOINT("glStencilMaskSeparate", pfnStencilMaskSeparate, PFNGLSTENCILMASKSEPARATEPROC);
    pfnStencilMaskSeparate(face, mask);
}

void glAttachShader(GLuint program, GLuint shader)
{
    typedef void (APIENTRY * PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
    static PFNGLATTACHSHADERPROC pfnAttachShader = 0;
    LOAD_ENTRYPOINT("glAttachShader", pfnAttachShader, PFNGLATTACHSHADERPROC);
    pfnAttachShader(program, shader);
}

void glBindAttribLocation(GLuint program, GLuint index, const GLchar *name)
{
    typedef void (APIENTRY * PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar *name);
    static PFNGLBINDATTRIBLOCATIONPROC pfnBindAttribLocation = 0;
    LOAD_ENTRYPOINT("glBindAttribLocation", pfnBindAttribLocation, PFNGLBINDATTRIBLOCATIONPROC);
    pfnBindAttribLocation(program, index, name);
}

void glCompileShader(GLuint shader)
{
    typedef void (APIENTRY * PFNGLCOMPILESHADERPROC) (GLuint shader);
    static PFNGLCOMPILESHADERPROC pfnCompileShader = 0;
    LOAD_ENTRYPOINT("glCompileShader", pfnCompileShader, PFNGLCOMPILESHADERPROC);
    pfnCompileShader(shader);
}

GLuint glCreateProgram()
{
    typedef GLuint (APIENTRY * PFNGLCREATEPROGRAMPROC) ();
    static PFNGLCREATEPROGRAMPROC pfnCreateProgram = 0;
    LOAD_ENTRYPOINT("glCreateProgram", pfnCreateProgram, PFNGLCREATEPROGRAMPROC);
    return pfnCreateProgram();
}

GLuint glCreateShader(GLenum type)
{
    typedef GLuint (APIENTRY * PFNGLCREATESHADERPROC) (GLenum type);
    static PFNGLCREATESHADERPROC pfnCreateShader = 0;
    LOAD_ENTRYPOINT("glCreateShader", pfnCreateShader, PFNGLCREATESHADERPROC);
    return pfnCreateShader(type);
}

void glDeleteProgram(GLuint program)
{
    typedef void (APIENTRY * PFNGLDELETEPROGRAMPROC) (GLuint program);
    static PFNGLDELETEPROGRAMPROC pfnDeleteProgram = 0;
    LOAD_ENTRYPOINT("glDeleteProgram", pfnDeleteProgram, PFNGLDELETEPROGRAMPROC);
    pfnDeleteProgram(program);
}

void glDeleteShader(GLuint shader)
{
    typedef void (APIENTRY * PFNGLDELETESHADERPROC) (GLuint shader);
    static PFNGLDELETESHADERPROC pfnDeleteShader = 0;
    LOAD_ENTRYPOINT("glDeleteShader", pfnDeleteShader, PFNGLDELETESHADERPROC);
    pfnDeleteShader(shader);
}

void glDetachShader(GLuint program, GLuint shader)
{
    typedef void (APIENTRY * PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
    static PFNGLDETACHSHADERPROC pfnDetachShader = 0;
    LOAD_ENTRYPOINT("glDetachShader", pfnDetachShader, PFNGLDETACHSHADERPROC);
    pfnDetachShader(program, shader);
}

void glDisableVertexAttribArray(GLuint index)
{
    typedef void (APIENTRY * PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
    static PFNGLDISABLEVERTEXATTRIBARRAYPROC pfnDisableVertexAttribArray = 0;
    LOAD_ENTRYPOINT("glDisableVertexAttribArray", pfnDisableVertexAttribArray, PFNGLDISABLEVERTEXATTRIBARRAYPROC);
    pfnDisableVertexAttribArray(index);
}

void glEnableVertexAttribArray(GLuint index)
{
    typedef void (APIENTRY * PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
    static PFNGLENABLEVERTEXATTRIBARRAYPROC pfnEnableVertexAttribArray = 0;
    LOAD_ENTRYPOINT("glEnableVertexAttribArray", pfnEnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC);
    pfnEnableVertexAttribArray(index);
}

void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    typedef void (APIENTRY * PFNGLGETACTIVEATTRIBPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
    static PFNGLGETACTIVEATTRIBPROC pfnGetActiveAttrib = 0;
    LOAD_ENTRYPOINT("glGetActiveAttrib", pfnGetActiveAttrib, PFNGLGETACTIVEATTRIBPROC);
    pfnGetActiveAttrib(program, index, bufSize, length, size, type, name);
}

void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    typedef void (APIENTRY * PFNGLGETACTIVEUNIFORMPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
    static PFNGLGETACTIVEUNIFORMPROC pfnGetActiveUniform = 0;
    LOAD_ENTRYPOINT("glGetActiveUniform", pfnGetActiveUniform, PFNGLGETACTIVEUNIFORMPROC);
    pfnGetActiveUniform(program, index, bufSize, length, size, type, name);
}

void glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *obj)
{
    typedef void (APIENTRY * PFNGLGETATTACHEDSHADERSPROC) (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *obj);
    static PFNGLGETATTACHEDSHADERSPROC pfnGetAttachedShaders = 0;
    LOAD_ENTRYPOINT("glGetAttachedShaders", pfnGetAttachedShaders, PFNGLGETATTACHEDSHADERSPROC);
    pfnGetAttachedShaders(program, maxCount, count, obj);
}

GLint glGetAttribLocation(GLuint program, const GLchar *name)
{
    typedef GLint (APIENTRY * PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const GLchar *name);
    static PFNGLGETATTRIBLOCATIONPROC pfnGetAttribLocation = 0;
    LOAD_ENTRYPOINT("glGetAttribLocation", pfnGetAttribLocation, PFNGLGETATTRIBLOCATIONPROC);
    return pfnGetAttribLocation(program, name);
}

void glGetProgramiv(GLuint program, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
    static PFNGLGETPROGRAMIVPROC pfnGetProgramiv = 0;
    LOAD_ENTRYPOINT("glGetProgramiv", pfnGetProgramiv, PFNGLGETPROGRAMIVPROC);
    pfnGetProgramiv(program, pname, params);
}

void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    typedef void (APIENTRY * PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    static PFNGLGETPROGRAMINFOLOGPROC pfnGetProgramInfoLog = 0;
    LOAD_ENTRYPOINT("glGetProgramInfoLog", pfnGetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC);
    pfnGetProgramInfoLog(program, bufSize, length, infoLog);
}

void glGetShaderiv(GLuint shader, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
    static PFNGLGETSHADERIVPROC pfnGetShaderiv = 0;
    LOAD_ENTRYPOINT("glGetShaderiv", pfnGetShaderiv, PFNGLGETSHADERIVPROC);
    pfnGetShaderiv(shader, pname, params);
}

void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    typedef void (APIENTRY * PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    static PFNGLGETSHADERINFOLOGPROC pfnGetShaderInfoLog = 0;
    LOAD_ENTRYPOINT("glGetShaderInfoLog", pfnGetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC);
    pfnGetShaderInfoLog(shader, bufSize, length, infoLog);
}

void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)
{
    typedef void (APIENTRY * PFNGLGETSHADERSOURCEPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
    static PFNGLGETSHADERSOURCEPROC pfnGetShaderSource = 0;
    LOAD_ENTRYPOINT("glGetShaderSource", pfnGetShaderSource, PFNGLGETSHADERSOURCEPROC);
    pfnGetShaderSource(shader, bufSize, length, source);
}

GLint glGetUniformLocation(GLuint program, const GLchar *name)
{
    typedef GLint (APIENTRY * PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
    static PFNGLGETUNIFORMLOCATIONPROC pfnGetUniformLocation = 0;
    LOAD_ENTRYPOINT("glGetUniformLocation", pfnGetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC);
    return pfnGetUniformLocation(program, name);
}

void glGetUniformfv(GLuint program, GLint location, GLfloat *params)
{
    typedef void (APIENTRY * PFNGLGETUNIFORMFVPROC) (GLuint program, GLint location, GLfloat *params);
    static PFNGLGETUNIFORMFVPROC pfnGetUniformfv = 0;
    LOAD_ENTRYPOINT("glGetUniformfv", pfnGetUniformfv, PFNGLGETUNIFORMFVPROC);
    pfnGetUniformfv(program, location, params);
}

void glGetUniformiv(GLuint program, GLint location, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETUNIFORMIVPROC) (GLuint program, GLint location, GLint *params);
    static PFNGLGETUNIFORMIVPROC pfnGetUniformiv = 0;
    LOAD_ENTRYPOINT("glGetUniformiv", pfnGetUniformiv, PFNGLGETUNIFORMIVPROC);
    pfnGetUniformiv(program, location, params);
}

void glGetVertexAttribdv(GLuint index, GLenum pname, GLdouble *params)
{
    typedef void (APIENTRY * PFNGLGETVERTEXATTRIBDVPROC) (GLuint index, GLenum pname, GLdouble *params);
    static PFNGLGETVERTEXATTRIBDVPROC pfnGetVertexAttribdv = 0;
    LOAD_ENTRYPOINT("glGetVertexAttribdv", pfnGetVertexAttribdv, PFNGLGETVERTEXATTRIBDVPROC);
    pfnGetVertexAttribdv(index, pname, params);
}

void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params)
{
    typedef void (APIENTRY * PFNGLGETVERTEXATTRIBFVPROC) (GLuint index, GLenum pname, GLfloat *params);
    static PFNGLGETVERTEXATTRIBFVPROC pfnGetVertexAttribfv = 0;
    LOAD_ENTRYPOINT("glGetVertexAttribfv", pfnGetVertexAttribfv, PFNGLGETVERTEXATTRIBFVPROC);
    pfnGetVertexAttribfv(index, pname, params);
}

void glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETVERTEXATTRIBIVPROC) (GLuint index, GLenum pname, GLint *params);
    static PFNGLGETVERTEXATTRIBIVPROC pfnGetVertexAttribiv = 0;
    LOAD_ENTRYPOINT("glGetVertexAttribiv", pfnGetVertexAttribiv, PFNGLGETVERTEXATTRIBIVPROC);
    pfnGetVertexAttribiv(index, pname, params);
}

void glGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid* *pointer)
{
    typedef void (APIENTRY * PFNGLGETVERTEXATTRIBPOINTERVPROC) (GLuint index, GLenum pname, GLvoid* *pointer);
    static PFNGLGETVERTEXATTRIBPOINTERVPROC pfnGetVertexAttribPointerv = 0;
    LOAD_ENTRYPOINT("glGetVertexAttribPointerv", pfnGetVertexAttribPointerv, PFNGLGETVERTEXATTRIBPOINTERVPROC);
    pfnGetVertexAttribPointerv(index, pname, pointer);
}

GLboolean glIsProgram(GLuint program)
{
    typedef GLboolean (APIENTRY * PFNGLISPROGRAMPROC) (GLuint program);
    static PFNGLISPROGRAMPROC pfnIsProgram = 0;
    LOAD_ENTRYPOINT("glIsProgram", pfnIsProgram, PFNGLISPROGRAMPROC);
    return pfnIsProgram(program);
}

GLboolean glIsShader(GLuint shader)
{
    typedef GLboolean (APIENTRY * PFNGLISSHADERPROC) (GLuint shader);
    static PFNGLISSHADERPROC pfnIsShader = 0;
    LOAD_ENTRYPOINT("glIsShader", pfnIsShader, PFNGLISSHADERPROC);
    return pfnIsShader(shader);
}

void glLinkProgram(GLuint program)
{
    typedef void (APIENTRY * PFNGLLINKPROGRAMPROC) (GLuint program);
    static PFNGLLINKPROGRAMPROC pfnLinkProgram = 0;
    LOAD_ENTRYPOINT("glLinkProgram", pfnLinkProgram, PFNGLLINKPROGRAMPROC);
    pfnLinkProgram(program);
}

void glShaderSource(GLuint shader, GLsizei count, const GLchar* *string, const GLint *length)
{
    typedef void (APIENTRY * PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
    static PFNGLSHADERSOURCEPROC pfnShaderSource = 0;
    LOAD_ENTRYPOINT("glShaderSource", pfnShaderSource, PFNGLSHADERSOURCEPROC);
    pfnShaderSource(shader, count, string, length);
}

void glUseProgram(GLuint program)
{
    typedef void (APIENTRY * PFNGLUSEPROGRAMPROC) (GLuint program);
    static PFNGLUSEPROGRAMPROC pfnUseProgram = 0;
    LOAD_ENTRYPOINT("glUseProgram", pfnUseProgram, PFNGLUSEPROGRAMPROC);
    pfnUseProgram(program);
}

void glUniform1f(GLint location, GLfloat v0)
{
    typedef void (APIENTRY * PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
    static PFNGLUNIFORM1FPROC pfnUniform1f = 0;
    LOAD_ENTRYPOINT("glUniform1f", pfnUniform1f, PFNGLUNIFORM1FPROC);
    pfnUniform1f(location, v0);
}

void glUniform2f(GLint location, GLfloat v0, GLfloat v1)
{
    typedef void (APIENTRY * PFNGLUNIFORM2FPROC) (GLint location, GLfloat v0, GLfloat v1);
    static PFNGLUNIFORM2FPROC pfnUniform2f = 0;
    LOAD_ENTRYPOINT("glUniform2f", pfnUniform2f, PFNGLUNIFORM2FPROC);
    pfnUniform2f(location, v0, v1);
}

void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    typedef void (APIENTRY * PFNGLUNIFORM3FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    static PFNGLUNIFORM3FPROC pfnUniform3f = 0;
    LOAD_ENTRYPOINT("glUniform3f", pfnUniform3f, PFNGLUNIFORM3FPROC);
    pfnUniform3f(location, v0, v1, v2);
}

void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    typedef void (APIENTRY * PFNGLUNIFORM4FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    static PFNGLUNIFORM4FPROC pfnUniform4f = 0;
    LOAD_ENTRYPOINT("glUniform4f", pfnUniform4f, PFNGLUNIFORM4FPROC);
    pfnUniform4f(location, v0, v1, v2, v3);
}

void glUniform1i(GLint location, GLint v0)
{
    typedef void (APIENTRY * PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
    static PFNGLUNIFORM1IPROC pfnUniform1i = 0;
    LOAD_ENTRYPOINT("glUniform1i", pfnUniform1i, PFNGLUNIFORM1IPROC);
    pfnUniform1i(location, v0);
}

void glUniform2i(GLint location, GLint v0, GLint v1)
{
    typedef void (APIENTRY * PFNGLUNIFORM2IPROC) (GLint location, GLint v0, GLint v1);
    static PFNGLUNIFORM2IPROC pfnUniform2i = 0;
    LOAD_ENTRYPOINT("glUniform2i", pfnUniform2i, PFNGLUNIFORM2IPROC);
    pfnUniform2i(location, v0, v1);
}

void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2)
{
    typedef void (APIENTRY * PFNGLUNIFORM3IPROC) (GLint location, GLint v0, GLint v1, GLint v2);
    static PFNGLUNIFORM3IPROC pfnUniform3i = 0;
    LOAD_ENTRYPOINT("glUniform3i", pfnUniform3i, PFNGLUNIFORM3IPROC);
    pfnUniform3i(location, v0, v1, v2);
}

void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    typedef void (APIENTRY * PFNGLUNIFORM4IPROC) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    static PFNGLUNIFORM4IPROC pfnUniform4i = 0;
    LOAD_ENTRYPOINT("glUniform4i", pfnUniform4i, PFNGLUNIFORM4IPROC);
    pfnUniform4i(location, v0, v1, v2, v3);
}

void glUniform1fv(GLint location, GLsizei count, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORM1FVPROC) (GLint location, GLsizei count, const GLfloat *value);
    static PFNGLUNIFORM1FVPROC pfnUniform1fv = 0;
    LOAD_ENTRYPOINT("glUniform1fv", pfnUniform1fv, PFNGLUNIFORM1FVPROC);
    pfnUniform1fv(location, count, value);
}

void glUniform2fv(GLint location, GLsizei count, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORM2FVPROC) (GLint location, GLsizei count, const GLfloat *value);
    static PFNGLUNIFORM2FVPROC pfnUniform2fv = 0;
    LOAD_ENTRYPOINT("glUniform2fv", pfnUniform2fv, PFNGLUNIFORM2FVPROC);
    pfnUniform2fv(location, count, value);
}

void glUniform3fv(GLint location, GLsizei count, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat *value);
    static PFNGLUNIFORM3FVPROC pfnUniform3fv = 0;
    LOAD_ENTRYPOINT("glUniform3fv", pfnUniform3fv, PFNGLUNIFORM3FVPROC);
    pfnUniform3fv(location, count, value);
}

void glUniform4fv(GLint location, GLsizei count, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat *value);
    static PFNGLUNIFORM4FVPROC pfnUniform4fv = 0;
    LOAD_ENTRYPOINT("glUniform4fv", pfnUniform4fv, PFNGLUNIFORM4FVPROC);
    pfnUniform4fv(location, count, value);
}

void glUniform1iv(GLint location, GLsizei count, const GLint *value)
{
    typedef void (APIENTRY * PFNGLUNIFORM1IVPROC) (GLint location, GLsizei count, const GLint *value);
    static PFNGLUNIFORM1IVPROC pfnUniform1iv = 0;
    LOAD_ENTRYPOINT("glUniform1iv", pfnUniform1iv, PFNGLUNIFORM1IVPROC);
    pfnUniform1iv(location, count, value);
}

void glUniform2iv(GLint location, GLsizei count, const GLint *value)
{
    typedef void (APIENTRY * PFNGLUNIFORM2IVPROC) (GLint location, GLsizei count, const GLint *value);
    static PFNGLUNIFORM2IVPROC pfnUniform2iv = 0;
    LOAD_ENTRYPOINT("glUniform2iv", pfnUniform2iv, PFNGLUNIFORM2IVPROC);
    pfnUniform2iv(location, count, value);
}

void glUniform3iv(GLint location, GLsizei count, const GLint *value)
{
    typedef void (APIENTRY * PFNGLUNIFORM3IVPROC) (GLint location, GLsizei count, const GLint *value);
    static PFNGLUNIFORM3IVPROC pfnUniform3iv = 0;
    LOAD_ENTRYPOINT("glUniform3iv", pfnUniform3iv, PFNGLUNIFORM3IVPROC);
    pfnUniform3iv(location, count, value);
}

void glUniform4iv(GLint location, GLsizei count, const GLint *value)
{
    typedef void (APIENTRY * PFNGLUNIFORM4IVPROC) (GLint location, GLsizei count, const GLint *value);
    static PFNGLUNIFORM4IVPROC pfnUniform4iv = 0;
    LOAD_ENTRYPOINT("glUniform4iv", pfnUniform4iv, PFNGLUNIFORM4IVPROC);
    pfnUniform4iv(location, count, value);
}

void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORMMATRIX2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    static PFNGLUNIFORMMATRIX2FVPROC pfnUniformMatrix2fv = 0;
    LOAD_ENTRYPOINT("glUniformMatrix2fv", pfnUniformMatrix2fv, PFNGLUNIFORMMATRIX2FVPROC);
    pfnUniformMatrix2fv(location, count, transpose, value);
}

void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORMMATRIX3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    static PFNGLUNIFORMMATRIX3FVPROC pfnUniformMatrix3fv = 0;
    LOAD_ENTRYPOINT("glUniformMatrix3fv", pfnUniformMatrix3fv, PFNGLUNIFORMMATRIX3FVPROC);
    pfnUniformMatrix3fv(location, count, transpose, value);
}

void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    static PFNGLUNIFORMMATRIX4FVPROC pfnUniformMatrix4fv = 0;
    LOAD_ENTRYPOINT("glUniformMatrix4fv", pfnUniformMatrix4fv, PFNGLUNIFORMMATRIX4FVPROC);
    pfnUniformMatrix4fv(location, count, transpose, value);
}

void glValidateProgram(GLuint program)
{
    typedef void (APIENTRY * PFNGLVALIDATEPROGRAMPROC) (GLuint program);
    static PFNGLVALIDATEPROGRAMPROC pfnValidateProgram = 0;
    LOAD_ENTRYPOINT("glValidateProgram", pfnValidateProgram, PFNGLVALIDATEPROGRAMPROC);
    pfnValidateProgram(program);
}

void glVertexAttrib1d(GLuint index, GLdouble x)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB1DPROC) (GLuint index, GLdouble x);
    static PFNGLVERTEXATTRIB1DPROC pfnVertexAttrib1d = 0;
    LOAD_ENTRYPOINT("glVertexAttrib1d", pfnVertexAttrib1d, PFNGLVERTEXATTRIB1DPROC);
    pfnVertexAttrib1d(index, x);
}

void glVertexAttrib1dv(GLuint index, const GLdouble *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB1DVPROC) (GLuint index, const GLdouble *v);
    static PFNGLVERTEXATTRIB1DVPROC pfnVertexAttrib1dv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib1dv", pfnVertexAttrib1dv, PFNGLVERTEXATTRIB1DVPROC);
    pfnVertexAttrib1dv(index, v);
}

void glVertexAttrib1f(GLuint index, GLfloat x)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB1FPROC) (GLuint index, GLfloat x);
    static PFNGLVERTEXATTRIB1FPROC pfnVertexAttrib1f = 0;
    LOAD_ENTRYPOINT("glVertexAttrib1f", pfnVertexAttrib1f, PFNGLVERTEXATTRIB1FPROC);
    pfnVertexAttrib1f(index, x);
}

void glVertexAttrib1fv(GLuint index, const GLfloat *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB1FVPROC) (GLuint index, const GLfloat *v);
    static PFNGLVERTEXATTRIB1FVPROC pfnVertexAttrib1fv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib1fv", pfnVertexAttrib1fv, PFNGLVERTEXATTRIB1FVPROC);
    pfnVertexAttrib1fv(index, v);
}

void glVertexAttrib1s(GLuint index, GLshort x)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB1SPROC) (GLuint index, GLshort x);
    static PFNGLVERTEXATTRIB1SPROC pfnVertexAttrib1s = 0;
    LOAD_ENTRYPOINT("glVertexAttrib1s", pfnVertexAttrib1s, PFNGLVERTEXATTRIB1SPROC);
    pfnVertexAttrib1s(index, x);
}

void glVertexAttrib1sv(GLuint index, const GLshort *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB1SVPROC) (GLuint index, const GLshort *v);
    static PFNGLVERTEXATTRIB1SVPROC pfnVertexAttrib1sv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib1sv", pfnVertexAttrib1sv, PFNGLVERTEXATTRIB1SVPROC);
    pfnVertexAttrib1sv(index, v);
}

void glVertexAttrib2d(GLuint index, GLdouble x, GLdouble y)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB2DPROC) (GLuint index, GLdouble x, GLdouble y);
    static PFNGLVERTEXATTRIB2DPROC pfnVertexAttrib2d = 0;
    LOAD_ENTRYPOINT("glVertexAttrib2d", pfnVertexAttrib2d, PFNGLVERTEXATTRIB2DPROC);
    pfnVertexAttrib2d(index, x, y);
}

void glVertexAttrib2dv(GLuint index, const GLdouble *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB2DVPROC) (GLuint index, const GLdouble *v);
    static PFNGLVERTEXATTRIB2DVPROC pfnVertexAttrib2dv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib2dv", pfnVertexAttrib2dv, PFNGLVERTEXATTRIB2DVPROC);
    pfnVertexAttrib2dv(index, v);
}

void glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB2FPROC) (GLuint index, GLfloat x, GLfloat y);
    static PFNGLVERTEXATTRIB2FPROC pfnVertexAttrib2f = 0;
    LOAD_ENTRYPOINT("glVertexAttrib2f", pfnVertexAttrib2f, PFNGLVERTEXATTRIB2FPROC);
    pfnVertexAttrib2f(index, x, y);
}

void glVertexAttrib2fv(GLuint index, const GLfloat *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB2FVPROC) (GLuint index, const GLfloat *v);
    static PFNGLVERTEXATTRIB2FVPROC pfnVertexAttrib2fv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib2fv", pfnVertexAttrib2fv, PFNGLVERTEXATTRIB2FVPROC);
    pfnVertexAttrib2fv(index, v);
}

void glVertexAttrib2s(GLuint index, GLshort x, GLshort y)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB2SPROC) (GLuint index, GLshort x, GLshort y);
    static PFNGLVERTEXATTRIB2SPROC pfnVertexAttrib2s = 0;
    LOAD_ENTRYPOINT("glVertexAttrib2s", pfnVertexAttrib2s, PFNGLVERTEXATTRIB2SPROC);
    pfnVertexAttrib2s(index, x, y);
}

void glVertexAttrib2sv(GLuint index, const GLshort *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB2SVPROC) (GLuint index, const GLshort *v);
    static PFNGLVERTEXATTRIB2SVPROC pfnVertexAttrib2sv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib2sv", pfnVertexAttrib2sv, PFNGLVERTEXATTRIB2SVPROC);
    pfnVertexAttrib2sv(index, v);
}

void glVertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB3DPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
    static PFNGLVERTEXATTRIB3DPROC pfnVertexAttrib3d = 0;
    LOAD_ENTRYPOINT("glVertexAttrib3d", pfnVertexAttrib3d, PFNGLVERTEXATTRIB3DPROC);
    pfnVertexAttrib3d(index, x, y, z);
}

void glVertexAttrib3dv(GLuint index, const GLdouble *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB3DVPROC) (GLuint index, const GLdouble *v);
    static PFNGLVERTEXATTRIB3DVPROC pfnVertexAttrib3dv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib3dv", pfnVertexAttrib3dv, PFNGLVERTEXATTRIB3DVPROC);
    pfnVertexAttrib3dv(index, v);
}

void glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB3FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
    static PFNGLVERTEXATTRIB3FPROC pfnVertexAttrib3f = 0;
    LOAD_ENTRYPOINT("glVertexAttrib3f", pfnVertexAttrib3f, PFNGLVERTEXATTRIB3FPROC);
    pfnVertexAttrib3f(index, x, y, z);
}

void glVertexAttrib3fv(GLuint index, const GLfloat *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB3FVPROC) (GLuint index, const GLfloat *v);
    static PFNGLVERTEXATTRIB3FVPROC pfnVertexAttrib3fv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib3fv", pfnVertexAttrib3fv, PFNGLVERTEXATTRIB3FVPROC);
    pfnVertexAttrib3fv(index, v);
}

void glVertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB3SPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
    static PFNGLVERTEXATTRIB3SPROC pfnVertexAttrib3s = 0;
    LOAD_ENTRYPOINT("glVertexAttrib3s", pfnVertexAttrib3s, PFNGLVERTEXATTRIB3SPROC);
    pfnVertexAttrib3s(index, x, y, z);
}

void glVertexAttrib3sv(GLuint index, const GLshort *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB3SVPROC) (GLuint index, const GLshort *v);
    static PFNGLVERTEXATTRIB3SVPROC pfnVertexAttrib3sv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib3sv", pfnVertexAttrib3sv, PFNGLVERTEXATTRIB3SVPROC);
    pfnVertexAttrib3sv(index, v);
}

void glVertexAttrib4Nbv(GLuint index, const GLbyte *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4NBVPROC) (GLuint index, const GLbyte *v);
    static PFNGLVERTEXATTRIB4NBVPROC pfnVertexAttrib4Nbv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4Nbv", pfnVertexAttrib4Nbv, PFNGLVERTEXATTRIB4NBVPROC);
    pfnVertexAttrib4Nbv(index, v);
}

void glVertexAttrib4Niv(GLuint index, const GLint *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4NIVPROC) (GLuint index, const GLint *v);
    static PFNGLVERTEXATTRIB4NIVPROC pfnVertexAttrib4Niv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4Niv", pfnVertexAttrib4Niv, PFNGLVERTEXATTRIB4NIVPROC);
    pfnVertexAttrib4Niv(index, v);
}

void glVertexAttrib4Nsv(GLuint index, const GLshort *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4NSVPROC) (GLuint index, const GLshort *v);
    static PFNGLVERTEXATTRIB4NSVPROC pfnVertexAttrib4Nsv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4Nsv", pfnVertexAttrib4Nsv, PFNGLVERTEXATTRIB4NSVPROC);
    pfnVertexAttrib4Nsv(index, v);
}

void glVertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4NUBPROC) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
    static PFNGLVERTEXATTRIB4NUBPROC pfnVertexAttrib4Nub = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4Nub", pfnVertexAttrib4Nub, PFNGLVERTEXATTRIB4NUBPROC);
    pfnVertexAttrib4Nub(index, x, y, z, w);
}

void glVertexAttrib4Nubv(GLuint index, const GLubyte *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4NUBVPROC) (GLuint index, const GLubyte *v);
    static PFNGLVERTEXATTRIB4NUBVPROC pfnVertexAttrib4Nubv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4Nubv", pfnVertexAttrib4Nubv, PFNGLVERTEXATTRIB4NUBVPROC);
    pfnVertexAttrib4Nubv(index, v);
}

void glVertexAttrib4Nuiv(GLuint index, const GLuint *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4NUIVPROC) (GLuint index, const GLuint *v);
    static PFNGLVERTEXATTRIB4NUIVPROC pfnVertexAttrib4Nuiv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4Nuiv", pfnVertexAttrib4Nuiv, PFNGLVERTEXATTRIB4NUIVPROC);
    pfnVertexAttrib4Nuiv(index, v);
}

void glVertexAttrib4Nusv(GLuint index, const GLushort *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4NUSVPROC) (GLuint index, const GLushort *v);
    static PFNGLVERTEXATTRIB4NUSVPROC pfnVertexAttrib4Nusv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4Nusv", pfnVertexAttrib4Nusv, PFNGLVERTEXATTRIB4NUSVPROC);
    pfnVertexAttrib4Nusv(index, v);
}

void glVertexAttrib4bv(GLuint index, const GLbyte *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4BVPROC) (GLuint index, const GLbyte *v);
    static PFNGLVERTEXATTRIB4BVPROC pfnVertexAttrib4bv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4bv", pfnVertexAttrib4bv, PFNGLVERTEXATTRIB4BVPROC);
    pfnVertexAttrib4bv(index, v);
}

void glVertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4DPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    static PFNGLVERTEXATTRIB4DPROC pfnVertexAttrib4d = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4d", pfnVertexAttrib4d, PFNGLVERTEXATTRIB4DPROC);
    pfnVertexAttrib4d(index, x, y, z, w);
}

void glVertexAttrib4dv(GLuint index, const GLdouble *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4DVPROC) (GLuint index, const GLdouble *v);
    static PFNGLVERTEXATTRIB4DVPROC pfnVertexAttrib4dv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4dv", pfnVertexAttrib4dv, PFNGLVERTEXATTRIB4DVPROC);
    pfnVertexAttrib4dv(index, v);
}

void glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    static PFNGLVERTEXATTRIB4FPROC pfnVertexAttrib4f = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4f", pfnVertexAttrib4f, PFNGLVERTEXATTRIB4FPROC);
    pfnVertexAttrib4f(index, x, y, z, w);
}

void glVertexAttrib4fv(GLuint index, const GLfloat *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4FVPROC) (GLuint index, const GLfloat *v);
    static PFNGLVERTEXATTRIB4FVPROC pfnVertexAttrib4fv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4fv", pfnVertexAttrib4fv, PFNGLVERTEXATTRIB4FVPROC);
    pfnVertexAttrib4fv(index, v);
}

void glVertexAttrib4iv(GLuint index, const GLint *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4IVPROC) (GLuint index, const GLint *v);
    static PFNGLVERTEXATTRIB4IVPROC pfnVertexAttrib4iv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4iv", pfnVertexAttrib4iv, PFNGLVERTEXATTRIB4IVPROC);
    pfnVertexAttrib4iv(index, v);
}

void glVertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4SPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
    static PFNGLVERTEXATTRIB4SPROC pfnVertexAttrib4s = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4s", pfnVertexAttrib4s, PFNGLVERTEXATTRIB4SPROC);
    pfnVertexAttrib4s(index, x, y, z, w);
}

void glVertexAttrib4sv(GLuint index, const GLshort *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4SVPROC) (GLuint index, const GLshort *v);
    static PFNGLVERTEXATTRIB4SVPROC pfnVertexAttrib4sv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4sv", pfnVertexAttrib4sv, PFNGLVERTEXATTRIB4SVPROC);
    pfnVertexAttrib4sv(index, v);
}

void glVertexAttrib4ubv(GLuint index, const GLubyte *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4UBVPROC) (GLuint index, const GLubyte *v);
    static PFNGLVERTEXATTRIB4UBVPROC pfnVertexAttrib4ubv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4ubv", pfnVertexAttrib4ubv, PFNGLVERTEXATTRIB4UBVPROC);
    pfnVertexAttrib4ubv(index, v);
}

void glVertexAttrib4uiv(GLuint index, const GLuint *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4UIVPROC) (GLuint index, const GLuint *v);
    static PFNGLVERTEXATTRIB4UIVPROC pfnVertexAttrib4uiv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4uiv", pfnVertexAttrib4uiv, PFNGLVERTEXATTRIB4UIVPROC);
    pfnVertexAttrib4uiv(index, v);
}

void glVertexAttrib4usv(GLuint index, const GLushort *v)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIB4USVPROC) (GLuint index, const GLushort *v);
    static PFNGLVERTEXATTRIB4USVPROC pfnVertexAttrib4usv = 0;
    LOAD_ENTRYPOINT("glVertexAttrib4usv", pfnVertexAttrib4usv, PFNGLVERTEXATTRIB4USVPROC);
    pfnVertexAttrib4usv(index, v);
}

void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer)
{
    typedef void (APIENTRY * PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
    static PFNGLVERTEXATTRIBPOINTERPROC pfnVertexAttribPointer = 0;
    LOAD_ENTRYPOINT("glVertexAttribPointer", pfnVertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC);
    pfnVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

//
// OpenGL 2.1
//

void glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORMMATRIX2X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    static PFNGLUNIFORMMATRIX2X3FVPROC pfnUniformMatrix2x3fv = 0;
    LOAD_ENTRYPOINT("glUniformMatrix2x3fv", pfnUniformMatrix2x3fv, PFNGLUNIFORMMATRIX2X3FVPROC);
    pfnUniformMatrix2x3fv(location, count, transpose, value);
}

void glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORMMATRIX3X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    static PFNGLUNIFORMMATRIX3X2FVPROC pfnUniformMatrix3x2fv = 0;
    LOAD_ENTRYPOINT("glUniformMatrix3x2fv", pfnUniformMatrix3x2fv, PFNGLUNIFORMMATRIX3X2FVPROC);
    pfnUniformMatrix3x2fv(location, count, transpose, value);
}

void glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORMMATRIX2X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    static PFNGLUNIFORMMATRIX2X4FVPROC pfnUniformMatrix2x4fv = 0;
    LOAD_ENTRYPOINT("glUniformMatrix2x4fv", pfnUniformMatrix2x4fv, PFNGLUNIFORMMATRIX2X4FVPROC);
    pfnUniformMatrix2x4fv(location, count, transpose, value);
}

void glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORMMATRIX4X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    static PFNGLUNIFORMMATRIX4X2FVPROC pfnUniformMatrix4x2fv = 0;
    LOAD_ENTRYPOINT("glUniformMatrix4x2fv", pfnUniformMatrix4x2fv, PFNGLUNIFORMMATRIX4X2FVPROC);
    pfnUniformMatrix4x2fv(location, count, transpose, value);
}

void glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORMMATRIX3X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    static PFNGLUNIFORMMATRIX3X4FVPROC pfnUniformMatrix3x4fv = 0;
    LOAD_ENTRYPOINT("glUniformMatrix3x4fv", pfnUniformMatrix3x4fv, PFNGLUNIFORMMATRIX3X4FVPROC);
    pfnUniformMatrix3x4fv(location, count, transpose, value);
}

void glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    typedef void (APIENTRY * PFNGLUNIFORMMATRIX4X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    static PFNGLUNIFORMMATRIX4X3FVPROC pfnUniformMatrix4x3fv = 0;
    LOAD_ENTRYPOINT("glUniformMatrix4x3fv", pfnUniformMatrix4x3fv, PFNGLUNIFORMMATRIX4X3FVPROC);
    pfnUniformMatrix4x3fv(location, count, transpose, value);
}

//
// OpenGL 3.0
//

void glColorMaski(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
	typedef void (APIENTRY * PFNGLCOLORMASKIPROC)(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
	static PFNGLCOLORMASKIPROC pfnColorMaski = 0;
	LOAD_ENTRYPOINT("glColorMaski", pfnColorMaski, PFNGLCOLORMASKIPROC);
	pfnColorMaski(index, r, g, b, a);
}

void glGetBooleani_v(GLenum target, GLuint index, GLboolean *data)
{
	typedef void (APIENTRY * PFNGLGETBOOLEANI_VPROC)(GLenum target, GLuint index, GLboolean *data);
	static PFNGLGETBOOLEANI_VPROC pfnGetBooleani_v = 0;
	LOAD_ENTRYPOINT("glGetBooleani_v", pfnGetBooleani_v, PFNGLGETBOOLEANI_VPROC);
	pfnGetBooleani_v(target, index, data);
}

void glGetIntegeri_v(GLenum target, GLuint index, GLint *data)
{
	typedef void (APIENTRY * PFNGLGETINTEGERI_VPROC)(GLenum target, GLuint index, GLint *data);
	static PFNGLGETINTEGERI_VPROC pfnGetIntegeri_v = 0;
	LOAD_ENTRYPOINT("glGetIntegeri_v", pfnGetIntegeri_v, PFNGLGETINTEGERI_VPROC);
	pfnGetIntegeri_v(target, index, data);
}

void glEnablei(GLenum target, GLuint index)
{
	typedef void (APIENTRY * PFNGLENABLEIPROC)(GLenum target, GLuint index);
	static PFNGLENABLEIPROC pfnEnablei = 0;
	LOAD_ENTRYPOINT("glEnablei", pfnEnablei, PFNGLENABLEIPROC);
	pfnEnablei(target, index);
}

void glDisablei(GLenum target, GLuint index)
{
	typedef void (APIENTRY * PFNGLDISABLEIPROC)(GLenum target, GLuint index);
	static PFNGLDISABLEIPROC pfnDisablei = 0;
	LOAD_ENTRYPOINT("glDisablei", pfnDisablei, PFNGLDISABLEIPROC);
	pfnDisablei(target, index);
}

GLboolean glIsEnabledi(GLenum target, GLuint index)
{
	typedef GLboolean (APIENTRY * PFNGLISENABLEDIPROC)(GLenum target, GLuint index);
	static PFNGLISENABLEDIPROC pfnIsEnabledi = 0;
	LOAD_ENTRYPOINT("glIsEnabledi", pfnIsEnabledi, PFNGLISENABLEDIPROC);
	return pfnIsEnabledi(target, index);
}

void glBeginTransformFeedback(GLenum primitiveMode)
{
	typedef void (APIENTRY * PFNGLBEGINTRANSFORMFEEDBACKPROC)(GLenum primitiveMode);
	static PFNGLBEGINTRANSFORMFEEDBACKPROC pfnBeginTransformFeedback = 0;
	LOAD_ENTRYPOINT("glBeginTransformFeedback", pfnBeginTransformFeedback, PFNGLBEGINTRANSFORMFEEDBACKPROC);
	pfnBeginTransformFeedback(primitiveMode);
}

void glEndTransformFeedback(void)
{
	typedef void (APIENTRY * PFNGLENDTRANSFORMFEEDBACKPROC)(void);
	static PFNGLENDTRANSFORMFEEDBACKPROC pfnEndTransformFeedback = 0;
	LOAD_ENTRYPOINT("glEndTransformFeedback", pfnEndTransformFeedback, PFNGLENDTRANSFORMFEEDBACKPROC);
	pfnEndTransformFeedback();
}

void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
	typedef void (APIENTRY * PFNGLBINDBUFFERRANGEPROC)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
	static PFNGLBINDBUFFERRANGEPROC pfnBindBufferRange = 0;
	LOAD_ENTRYPOINT("glBindBufferRange", pfnBindBufferRange, PFNGLBINDBUFFERRANGEPROC);
	pfnBindBufferRange(target, index, buffer, offset, size);
}

void glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
	typedef void (APIENTRY * PFNGLBINDBUFFERBASEPROC)(GLenum target, GLuint index, GLuint buffer);
	static PFNGLBINDBUFFERBASEPROC pfnBindBufferBase = 0;
	LOAD_ENTRYPOINT("glBindBufferBase", pfnBindBufferBase, PFNGLBINDBUFFERBASEPROC);
	pfnBindBufferBase(target, index, buffer);
}

void glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar **varyings, GLenum bufferMode)
{
	typedef void (APIENTRY * PFNGLTRANSFORMFEEDBACKVARYINGSPROC)(GLuint program, GLsizei count, const GLchar **varyings, GLenum bufferMode);
	static PFNGLTRANSFORMFEEDBACKVARYINGSPROC pfnTransformFeedbackVaryings = 0;
	LOAD_ENTRYPOINT("glTransformFeedbackVaryings", pfnTransformFeedbackVaryings, PFNGLTRANSFORMFEEDBACKVARYINGSPROC);
	pfnTransformFeedbackVaryings(program, count, varyings, bufferMode);
}

void glGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name)
{
	typedef void (APIENTRY * PFNGLGETTRANSFORMFEEDBACKVARYINGPROC)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
	static PFNGLGETTRANSFORMFEEDBACKVARYINGPROC pfnGetTransformFeedbackVarying = 0;
	LOAD_ENTRYPOINT("glGetTransformFeedbackVarying", pfnGetTransformFeedbackVarying, PFNGLGETTRANSFORMFEEDBACKVARYINGPROC);
	pfnGetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
}

void glClampColor(GLenum target, GLenum clamp)
{
	typedef void (APIENTRY * PFNGLCLAMPCOLORPROC)(GLenum target, GLenum clamp);
	static PFNGLCLAMPCOLORPROC pfnClampColor = 0;
	LOAD_ENTRYPOINT("glClampColor", pfnClampColor, PFNGLCLAMPCOLORPROC);
	pfnClampColor(target, clamp);
}

void glBeginConditionalRender(GLuint id, GLenum mode)
{
	typedef void (APIENTRY * PFNGLBEGINCONDITIONALRENDERPROC)(GLuint id, GLenum mode);
	static PFNGLBEGINCONDITIONALRENDERPROC pfnBeginConditionalRender = 0;
	LOAD_ENTRYPOINT("glBeginConditionalRender", pfnBeginConditionalRender, PFNGLBEGINCONDITIONALRENDERPROC);
	pfnBeginConditionalRender(id, mode);
}

void glEndConditionalRender(void)
{
	typedef void (APIENTRY * PFNGLENDCONDITIONALRENDERPROC)(void);
	static PFNGLENDCONDITIONALRENDERPROC pfnEndConditionalRender = 0;
	LOAD_ENTRYPOINT("glEndConditionalRender", pfnEndConditionalRender, PFNGLENDCONDITIONALRENDERPROC);
	pfnEndConditionalRender();
}

void glVertexAttribI1i(GLuint index, GLint x)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI1IPROC)(GLuint index, GLint x);
	static PFNGLVERTEXATTRIBI1IPROC pfnVertexAttribI1i = 0;
	LOAD_ENTRYPOINT("glVertexAttribI1i", pfnVertexAttribI1i, PFNGLVERTEXATTRIBI1IPROC);
	pfnVertexAttribI1i(index, x);
}

void glVertexAttribI2i(GLuint index, GLint x, GLint y)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI2IPROC)(GLuint index, GLint x, GLint y);
	static PFNGLVERTEXATTRIBI2IPROC pfnVertexAttribI2i = 0;
	LOAD_ENTRYPOINT("glVertexAttribI2i", pfnVertexAttribI2i, PFNGLVERTEXATTRIBI2IPROC);
	pfnVertexAttribI2i(index, x, y);
}

void glVertexAttribI3i(GLuint index, GLint x, GLint y, GLint z)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI3IPROC)(GLuint index, GLint x, GLint y, GLint z);
	static PFNGLVERTEXATTRIBI3IPROC pfnVertexAttribI3i = 0;
	LOAD_ENTRYPOINT("glVertexAttribI3i", pfnVertexAttribI3i, PFNGLVERTEXATTRIBI3IPROC);
	pfnVertexAttribI3i(index, x, y, z);
}

void glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI4IPROC)(GLuint index, GLint x, GLint y, GLint z, GLint w);
	static PFNGLVERTEXATTRIBI4IPROC pfnVertexAttribI4i = 0;
	LOAD_ENTRYPOINT("glVertexAttribI4i", pfnVertexAttribI4i, PFNGLVERTEXATTRIBI4IPROC);
	pfnVertexAttribI4i(index, x, y, z, w);
}

void glVertexAttribI1ui(GLuint index, GLuint x)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI1UIPROC)(GLuint index, GLuint x);
	static PFNGLVERTEXATTRIBI1UIPROC pfnVertexAttribI1ui = 0;
	LOAD_ENTRYPOINT("glVertexAttribI1ui", pfnVertexAttribI1ui, PFNGLVERTEXATTRIBI1UIPROC);
	pfnVertexAttribI1ui(index, x);
}

void glVertexAttribI2ui(GLuint index, GLuint x, GLuint y)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI2UIPROC)(GLuint index, GLuint x, GLuint y);
	static PFNGLVERTEXATTRIBI2UIPROC pfnVertexAttribI2ui = 0;
	LOAD_ENTRYPOINT("glVertexAttribI2ui", pfnVertexAttribI2ui, PFNGLVERTEXATTRIBI2UIPROC);
	pfnVertexAttribI2ui(index, x, y);
}

void glVertexAttribI3ui(GLuint index, GLuint x, GLuint y, GLuint z)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI3UIPROC)(GLuint index, GLuint x, GLuint y, GLuint z);
	static PFNGLVERTEXATTRIBI3UIPROC pfnVertexAttribI3ui = 0;
	LOAD_ENTRYPOINT("glVertexAttribI3ui", pfnVertexAttribI3ui, PFNGLVERTEXATTRIBI3UIPROC);
	pfnVertexAttribI3ui(index, x, y, z);
}

void glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI4UIPROC)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
	static PFNGLVERTEXATTRIBI4UIPROC pfnVertexAttribI4ui = 0;
	LOAD_ENTRYPOINT("glVertexAttribI4ui", pfnVertexAttribI4ui, PFNGLVERTEXATTRIBI4UIPROC);
	pfnVertexAttribI4ui(index, x, y, z, w);
}

void glVertexAttribI1iv(GLuint index, const GLint *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI1IVPROC)(GLuint index, const GLint *v);
	static PFNGLVERTEXATTRIBI1IVPROC pfnVertexAttribI1iv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI1iv", pfnVertexAttribI1iv, PFNGLVERTEXATTRIBI1IVPROC);
	pfnVertexAttribI1iv(index, v);
}

void glVertexAttribI2iv(GLuint index, const GLint *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI2IVPROC)(GLuint index, const GLint *v);
	static PFNGLVERTEXATTRIBI2IVPROC pfnVertexAttribI2iv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI2iv", pfnVertexAttribI2iv, PFNGLVERTEXATTRIBI2IVPROC);
	pfnVertexAttribI2iv(index, v);
}

void glVertexAttribI3iv(GLuint index, const GLint *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI3IVPROC)(GLuint index, const GLint *v);
	static PFNGLVERTEXATTRIBI3IVPROC pfnVertexAttribI3iv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI3iv", pfnVertexAttribI3iv, PFNGLVERTEXATTRIBI3IVPROC);
	pfnVertexAttribI3iv(index, v);
}

void glVertexAttribI4iv(GLuint index, const GLint *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI4IVPROC)(GLuint index, const GLint *v);
	static PFNGLVERTEXATTRIBI4IVPROC pfnVertexAttribI4iv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI4iv", pfnVertexAttribI4iv, PFNGLVERTEXATTRIBI4IVPROC);
	pfnVertexAttribI4iv(index, v);
}

void glVertexAttribI1uiv(GLuint index, const GLuint *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI1UIVPROC)(GLuint index, const GLuint *v);
	static PFNGLVERTEXATTRIBI1UIVPROC pfnVertexAttribI1uiv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI1uiv", pfnVertexAttribI1uiv, PFNGLVERTEXATTRIBI1UIVPROC);
	pfnVertexAttribI1uiv(index, v);
}

void glVertexAttribI2uiv(GLuint index, const GLuint *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI2UIVPROC)(GLuint index, const GLuint *v);
	static PFNGLVERTEXATTRIBI2UIVPROC pfnVertexAttribI2uiv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI2uiv", pfnVertexAttribI2uiv, PFNGLVERTEXATTRIBI2UIVPROC);
	pfnVertexAttribI2uiv(index, v);
}

void glVertexAttribI3uiv(GLuint index, const GLuint *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI3UIVPROC)(GLuint index, const GLuint *v);
	static PFNGLVERTEXATTRIBI3UIVPROC pfnVertexAttribI3uiv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI3uiv", pfnVertexAttribI3uiv, PFNGLVERTEXATTRIBI3UIVPROC);
	pfnVertexAttribI3uiv(index, v);
}

void glVertexAttribI4uiv(GLuint index, const GLuint *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI4UIVPROC)(GLuint index, const GLuint *v);
	static PFNGLVERTEXATTRIBI4UIVPROC pfnVertexAttribI4uiv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI4uiv", pfnVertexAttribI4uiv, PFNGLVERTEXATTRIBI4UIVPROC);
	pfnVertexAttribI4uiv(index, v);
}

void glVertexAttribI4bv(GLuint index, const GLbyte *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI4BVPROC)(GLuint index, const GLbyte *v);
	static PFNGLVERTEXATTRIBI4BVPROC pfnVertexAttribI4bv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI4bv", pfnVertexAttribI4bv, PFNGLVERTEXATTRIBI4BVPROC);
	pfnVertexAttribI4bv(index, v);
}

void glVertexAttribI4sv(GLuint index, const GLshort *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI4SVPROC)(GLuint index, const GLshort *v);
	static PFNGLVERTEXATTRIBI4SVPROC pfnVertexAttribI4sv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI4sv", pfnVertexAttribI4sv, PFNGLVERTEXATTRIBI4SVPROC);
	pfnVertexAttribI4sv(index, v);
}

void glVertexAttribI4ubv(GLuint index, const GLubyte *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI4UBVPROC)(GLuint index, const GLubyte *v);
	static PFNGLVERTEXATTRIBI4UBVPROC pfnVertexAttribI4ubv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI4ubv", pfnVertexAttribI4ubv, PFNGLVERTEXATTRIBI4UBVPROC);
	pfnVertexAttribI4ubv(index, v);
}

void glVertexAttribI4usv(GLuint index, const GLushort *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBI4USVPROC)(GLuint index, const GLushort *v);
	static PFNGLVERTEXATTRIBI4USVPROC pfnVertexAttribI4usv = 0;
	LOAD_ENTRYPOINT("glVertexAttribI4usv", pfnVertexAttribI4usv, PFNGLVERTEXATTRIBI4USVPROC);
	pfnVertexAttribI4usv(index, v);
}

void glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBIPOINTERPROC)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
	static PFNGLVERTEXATTRIBIPOINTERPROC pfnVertexAttribIPointer = 0;
	LOAD_ENTRYPOINT("glVertexAttribIPointer", pfnVertexAttribIPointer, PFNGLVERTEXATTRIBIPOINTERPROC);
	pfnVertexAttribIPointer(index, size, type, stride, pointer);
}

void glGetVertexAttribIiv(GLuint index, GLenum pname, GLint *params)
{
	typedef void (APIENTRY * PFNGLGETVERTEXATTRIBIIVPROC)(GLuint index, GLenum pname, GLint *params);
	static PFNGLGETVERTEXATTRIBIIVPROC pfnGetVertexAttribIiv = 0;
	LOAD_ENTRYPOINT("glGetVertexAttribIiv", pfnGetVertexAttribIiv, PFNGLGETVERTEXATTRIBIIVPROC);
	pfnGetVertexAttribIiv(index, pname, params);
}

void glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint *params)
{
	typedef void (APIENTRY * PFNGLGETVERTEXATTRIBIUIVPROC)(GLuint index, GLenum pname, GLuint *params);
	static PFNGLGETVERTEXATTRIBIUIVPROC pfnGetVertexAttribIuiv = 0;
	LOAD_ENTRYPOINT("glGetVertexAttribIuiv", pfnGetVertexAttribIuiv, PFNGLGETVERTEXATTRIBIUIVPROC);
	pfnGetVertexAttribIuiv(index, pname, params);
}

void glGetUniformuiv(GLuint program, GLint location, GLuint *params)
{
	typedef void (APIENTRY * PFNGLGETUNIFORMUIVPROC)(GLuint program, GLint location, GLuint *params);
	static PFNGLGETUNIFORMUIVPROC pfnGetUniformuiv = 0;
	LOAD_ENTRYPOINT("glGetUniformuiv", pfnGetUniformuiv, PFNGLGETUNIFORMUIVPROC);
	pfnGetUniformuiv(program, location, params);
}

void glBindFragDataLocation(GLuint program, GLuint color, const GLchar *name)
{
	typedef void (APIENTRY * PFNGLBINDFRAGDATALOCATIONPROC)(GLuint program, GLuint color, const GLchar *name);
	static PFNGLBINDFRAGDATALOCATIONPROC pfnBindFragDataLocation = 0;
	LOAD_ENTRYPOINT("glBindFragDataLocation", pfnBindFragDataLocation, PFNGLBINDFRAGDATALOCATIONPROC);
	pfnBindFragDataLocation(program, color, name);
}

GLint glGetFragDataLocation(GLuint program, const GLchar *name)
{
	typedef GLint (APIENTRY * PFNGLGETFRAGDATALOCATIONPROC)(GLuint program, const GLchar *name);
	static PFNGLGETFRAGDATALOCATIONPROC pfnGetFragDataLocation = 0;
	LOAD_ENTRYPOINT("glGetFragDataLocation", pfnGetFragDataLocation, PFNGLGETFRAGDATALOCATIONPROC);
	return pfnGetFragDataLocation(program, name);
}

void glUniform1ui(GLint location, GLuint v0)
{
	typedef void (APIENTRY * PFNGLUNIFORM1UIPROC)(GLint location, GLuint v0);
	static PFNGLUNIFORM1UIPROC pfnUniform1ui = 0;
	LOAD_ENTRYPOINT("glUniform1ui", pfnUniform1ui, PFNGLUNIFORM1UIPROC);
	pfnUniform1ui(location, v0);
}

void glUniform2ui(GLint location, GLuint v0, GLuint v1)
{
	typedef void (APIENTRY * PFNGLUNIFORM2UIPROC)(GLint location, GLuint v0, GLuint v1);
	static PFNGLUNIFORM2UIPROC pfnUniform2ui = 0;
	LOAD_ENTRYPOINT("glUniform2ui", pfnUniform2ui, PFNGLUNIFORM2UIPROC);
	pfnUniform2ui(location, v0, v1);
}

void glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	typedef void (APIENTRY * PFNGLUNIFORM3UIPROC)(GLint location, GLuint v0, GLuint v1, GLuint v2);
	static PFNGLUNIFORM3UIPROC pfnUniform3ui = 0;
	LOAD_ENTRYPOINT("glUniform3ui", pfnUniform3ui, PFNGLUNIFORM3UIPROC);
	pfnUniform3ui(location, v0, v1, v2);
}

void glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	typedef void (APIENTRY * PFNGLUNIFORM4UIPROC)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
	static PFNGLUNIFORM4UIPROC pfnUniform4ui = 0;
	LOAD_ENTRYPOINT("glUniform4ui", pfnUniform4ui, PFNGLUNIFORM4UIPROC);
	pfnUniform4ui(location, v0, v1, v2, v3);
}

void glUniform1uiv(GLint location, GLsizei count, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLUNIFORM1UIVPROC)(GLint location, GLsizei count, const GLuint *value);
	static PFNGLUNIFORM1UIVPROC pfnUniform1uiv = 0;
	LOAD_ENTRYPOINT("glUniform1uiv", pfnUniform1uiv, PFNGLUNIFORM1UIVPROC);
	pfnUniform1uiv(location, count, value);
}

void glUniform2uiv(GLint location, GLsizei count, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLUNIFORM2UIVPROC)(GLint location, GLsizei count, const GLuint *value);
	static PFNGLUNIFORM2UIVPROC pfnUniform2uiv = 0;
	LOAD_ENTRYPOINT("glUniform2uiv", pfnUniform2uiv, PFNGLUNIFORM2UIVPROC);
	pfnUniform2uiv(location, count, value);
}

void glUniform3uiv(GLint location, GLsizei count, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLUNIFORM3UIVPROC)(GLint location, GLsizei count, const GLuint *value);
	static PFNGLUNIFORM3UIVPROC pfnUniform3uiv = 0;
	LOAD_ENTRYPOINT("glUniform3uiv", pfnUniform3uiv, PFNGLUNIFORM3UIVPROC);
	pfnUniform3uiv(location, count, value);
}

void glUniform4uiv(GLint location, GLsizei count, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLUNIFORM4UIVPROC)(GLint location, GLsizei count, const GLuint *value);
	static PFNGLUNIFORM4UIVPROC pfnUniform4uiv = 0;
	LOAD_ENTRYPOINT("glUniform4uiv", pfnUniform4uiv, PFNGLUNIFORM4UIVPROC);
	pfnUniform4uiv(location, count, value);
}

void glTexParameterIiv(GLenum target, GLenum pname, const GLint *params)
{
	typedef void (APIENTRY * PFNGLTEXPARAMETERIIVPROC)(GLenum target, GLenum pname, const GLint *params);
	static PFNGLTEXPARAMETERIIVPROC pfnTexParameterIiv = 0;
	LOAD_ENTRYPOINT("glTexParameterIiv", pfnTexParameterIiv, PFNGLTEXPARAMETERIIVPROC);
	pfnTexParameterIiv(target, pname, params);
}

void glTexParameterIuiv(GLenum target, GLenum pname, const GLuint *params)
{
	typedef void (APIENTRY * PFNGLTEXPARAMETERIUIVPROC)(GLenum target, GLenum pname, const GLuint *params);
	static PFNGLTEXPARAMETERIUIVPROC pfnTexParameterIuiv = 0;
	LOAD_ENTRYPOINT("glTexParameterIuiv", pfnTexParameterIuiv, PFNGLTEXPARAMETERIUIVPROC);
	pfnTexParameterIuiv(target, pname, params);
}

void glGetTexParameterIiv(GLenum target, GLenum pname, GLint *params)
{
	typedef void (APIENTRY * PFNGLGETTEXPARAMETERIIVPROC)(GLenum target, GLenum pname, GLint *params);
	static PFNGLGETTEXPARAMETERIIVPROC pfnGetTexParameterIiv = 0;
	LOAD_ENTRYPOINT("glGetTexParameterIiv", pfnGetTexParameterIiv, PFNGLGETTEXPARAMETERIIVPROC);
	pfnGetTexParameterIiv(target, pname, params);
}

void glGetTexParameterIuiv(GLenum target, GLenum pname, GLuint *params)
{
	typedef void (APIENTRY * PFNGLGETTEXPARAMETERIUIVPROC)(GLenum target, GLenum pname, GLuint *params);
	static PFNGLGETTEXPARAMETERIUIVPROC pfnGetTexParameterIuiv = 0;
	LOAD_ENTRYPOINT("glGetTexParameterIuiv", pfnGetTexParameterIuiv, PFNGLGETTEXPARAMETERIUIVPROC);
	pfnGetTexParameterIuiv(target, pname, params);
}

void glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint *value)
{
	typedef void (APIENTRY * PFNGLCLEARBUFFERIVPROC)(GLenum buffer, GLint drawbuffer, const GLint *value);
	static PFNGLCLEARBUFFERIVPROC pfnClearBufferiv = 0;
	LOAD_ENTRYPOINT("glClearBufferiv", pfnClearBufferiv, PFNGLCLEARBUFFERIVPROC);
	pfnClearBufferiv(buffer, drawbuffer, value);
}

void glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLCLEARBUFFERUIVPROC)(GLenum buffer, GLint drawbuffer, const GLuint *value);
	static PFNGLCLEARBUFFERUIVPROC pfnClearBufferuiv = 0;
	LOAD_ENTRYPOINT("glClearBufferuiv", pfnClearBufferuiv, PFNGLCLEARBUFFERUIVPROC);
	pfnClearBufferuiv(buffer, drawbuffer, value);
}

void glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat *value)
{
	typedef void (APIENTRY * PFNGLCLEARBUFFERFVPROC)(GLenum buffer, GLint drawbuffer, const GLfloat *value);
	static PFNGLCLEARBUFFERFVPROC pfnClearBufferfv = 0;
	LOAD_ENTRYPOINT("glClearBufferfv", pfnClearBufferfv, PFNGLCLEARBUFFERFVPROC);
	pfnClearBufferfv(buffer, drawbuffer, value);
}

void glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
	typedef void (APIENTRY * PFNGLCLEARBUFFERFIPROC)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
	static PFNGLCLEARBUFFERFIPROC pfnClearBufferfi = 0;
	LOAD_ENTRYPOINT("glClearBufferfi", pfnClearBufferfi, PFNGLCLEARBUFFERFIPROC);
	pfnClearBufferfi(buffer, drawbuffer, depth, stencil);
}

const GLubyte *glGetStringi(GLenum name, GLuint index)
{
	typedef const GLubyte *(APIENTRY * PFNGLGETSTRINGIPROC)(GLenum name, GLuint index);
	static PFNGLGETSTRINGIPROC pfnGetStringi = 0;
	LOAD_ENTRYPOINT("glGetStringi", pfnGetStringi, PFNGLGETSTRINGIPROC);
	return pfnGetStringi(name, index);
}

GLboolean glIsRenderbuffer(GLuint renderbuffer)
{
	typedef GLboolean (APIENTRY * PFNGLISRENDERBUFFERPROC)(GLuint renderbuffer);
	static PFNGLISRENDERBUFFERPROC pfnIsRenderbuffer = 0;
	LOAD_ENTRYPOINT("glIsRenderbuffer", pfnIsRenderbuffer, PFNGLISRENDERBUFFERPROC);
	return pfnIsRenderbuffer(renderbuffer);
}

void glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
	typedef void (APIENTRY * PFNGLBINDRENDERBUFFERPROC)(GLenum target, GLuint renderbuffer);
	static PFNGLBINDRENDERBUFFERPROC pfnBindRenderbuffer = 0;
	LOAD_ENTRYPOINT("glBindRenderbuffer", pfnBindRenderbuffer, PFNGLBINDRENDERBUFFERPROC);
	pfnBindRenderbuffer(target, renderbuffer);
}

void glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers)
{
	typedef void (APIENTRY * PFNGLDELETERENDERBUFFERSPROC)(GLsizei n, const GLuint *renderbuffers);
	static PFNGLDELETERENDERBUFFERSPROC pfnDeleteRenderbuffers = 0;
	LOAD_ENTRYPOINT("glDeleteRenderbuffers", pfnDeleteRenderbuffers, PFNGLDELETERENDERBUFFERSPROC);
	pfnDeleteRenderbuffers(n, renderbuffers);
}

void glGenRenderbuffers(GLsizei n, GLuint *renderbuffers)
{
	typedef void (APIENTRY * PFNGLGENRENDERBUFFERSPROC)(GLsizei n, GLuint *renderbuffers);
	static PFNGLGENRENDERBUFFERSPROC pfnGenRenderbuffers = 0;
	LOAD_ENTRYPOINT("glGenRenderbuffers", pfnGenRenderbuffers, PFNGLGENRENDERBUFFERSPROC);
	pfnGenRenderbuffers(n, renderbuffers);
}

void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
	typedef void (APIENTRY * PFNGLRENDERBUFFERSTORAGEPROC)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
	static PFNGLRENDERBUFFERSTORAGEPROC pfnRenderbufferStorage = 0;
	LOAD_ENTRYPOINT("glRenderbufferStorage", pfnRenderbufferStorage, PFNGLRENDERBUFFERSTORAGEPROC);
	pfnRenderbufferStorage(target, internalformat, width, height);
}

void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
	typedef void (APIENTRY * PFNGLGETRENDERBUFFERPARAMETERIVPROC)(GLenum target, GLenum pname, GLint *params);
	static PFNGLGETRENDERBUFFERPARAMETERIVPROC pfnGetRenderbufferParameteriv = 0;
	LOAD_ENTRYPOINT("glGetRenderbufferParameteriv", pfnGetRenderbufferParameteriv, PFNGLGETRENDERBUFFERPARAMETERIVPROC);
	pfnGetRenderbufferParameteriv(target, pname, params);
}

GLboolean glIsFramebuffer(GLuint framebuffer)
{
	typedef GLboolean (APIENTRY * PFNGLISFRAMEBUFFERPROC)(GLuint framebuffer);
	static PFNGLISFRAMEBUFFERPROC pfnIsFramebuffer = 0;
	LOAD_ENTRYPOINT("glIsFramebuffer", pfnIsFramebuffer, PFNGLISFRAMEBUFFERPROC);
	return pfnIsFramebuffer(framebuffer);
}

void glBindFramebuffer(GLenum target, GLuint framebuffer)
{
	typedef void (APIENTRY * PFNGLBINDFRAMEBUFFERPROC)(GLenum target, GLuint framebuffer);
	static PFNGLBINDFRAMEBUFFERPROC pfnBindFramebuffer = 0;
	LOAD_ENTRYPOINT("glBindFramebuffer", pfnBindFramebuffer, PFNGLBINDFRAMEBUFFERPROC);
	pfnBindFramebuffer(target, framebuffer);
}

void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers)
{
	typedef void (APIENTRY * PFNGLDELETEFRAMEBUFFERSPROC)(GLsizei n, const GLuint *framebuffers);
	static PFNGLDELETEFRAMEBUFFERSPROC pfnDeleteFramebuffers = 0;
	LOAD_ENTRYPOINT("glDeleteFramebuffers", pfnDeleteFramebuffers, PFNGLDELETEFRAMEBUFFERSPROC);
	pfnDeleteFramebuffers(n, framebuffers);
}

void glGenFramebuffers(GLsizei n, GLuint *framebuffers)
{
	typedef void (APIENTRY * PFNGLGENFRAMEBUFFERSPROC)(GLsizei n, GLuint *framebuffers);
	static PFNGLGENFRAMEBUFFERSPROC pfnGenFramebuffers = 0;
	LOAD_ENTRYPOINT("glGenFramebuffers", pfnGenFramebuffers, PFNGLGENFRAMEBUFFERSPROC);
	pfnGenFramebuffers(n, framebuffers);
}

GLenum glCheckFramebufferStatus(GLenum target)
{
	typedef GLenum (APIENTRY * PFNGLCHECKFRAMEBUFFERSTATUSPROC)(GLenum target);
	static PFNGLCHECKFRAMEBUFFERSTATUSPROC pfnCheckFramebufferStatus = 0;
	LOAD_ENTRYPOINT("glCheckFramebufferStatus", pfnCheckFramebufferStatus, PFNGLCHECKFRAMEBUFFERSTATUSPROC);
	return pfnCheckFramebufferStatus(target);
}

void glFramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTURE1DPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	static PFNGLFRAMEBUFFERTEXTURE1DPROC pfnFramebufferTexture1D = 0;
	LOAD_ENTRYPOINT("glFramebufferTexture1D", pfnFramebufferTexture1D, PFNGLFRAMEBUFFERTEXTURE1DPROC);
	pfnFramebufferTexture1D(target, attachment, textarget, texture, level);
}

void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTURE2DPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	static PFNGLFRAMEBUFFERTEXTURE2DPROC pfnFramebufferTexture2D = 0;
	LOAD_ENTRYPOINT("glFramebufferTexture2D", pfnFramebufferTexture2D, PFNGLFRAMEBUFFERTEXTURE2DPROC);
	pfnFramebufferTexture2D(target, attachment, textarget, texture, level);
}

void glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
	typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTURE3DPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
	static PFNGLFRAMEBUFFERTEXTURE3DPROC pfnFramebufferTexture3D = 0;
	LOAD_ENTRYPOINT("glFramebufferTexture3D", pfnFramebufferTexture3D, PFNGLFRAMEBUFFERTEXTURE3DPROC);
	pfnFramebufferTexture3D(target, attachment, textarget, texture, level, zoffset);
}

void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	typedef void (APIENTRY * PFNGLFRAMEBUFFERRENDERBUFFERPROC)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
	static PFNGLFRAMEBUFFERRENDERBUFFERPROC pfnFramebufferRenderbuffer = 0;
	LOAD_ENTRYPOINT("glFramebufferRenderbuffer", pfnFramebufferRenderbuffer, PFNGLFRAMEBUFFERRENDERBUFFERPROC);
	pfnFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}

void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
	typedef void (APIENTRY * PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)(GLenum target, GLenum attachment, GLenum pname, GLint *params);
	static PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC pfnGetFramebufferAttachmentParameteriv = 0;
	LOAD_ENTRYPOINT("glGetFramebufferAttachmentParameteriv", pfnGetFramebufferAttachmentParameteriv, PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC);
	pfnGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
}

void glGenerateMipmap(GLenum target)
{
	typedef void (APIENTRY * PFNGLGENERATEMIPMAPPROC)(GLenum target);
	static PFNGLGENERATEMIPMAPPROC pfnGenerateMipmap = 0;
	LOAD_ENTRYPOINT("glGenerateMipmap", pfnGenerateMipmap, PFNGLGENERATEMIPMAPPROC);
	pfnGenerateMipmap(target);
}

void glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
	typedef void (APIENTRY * PFNGLBLITFRAMEBUFFERPROC)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
	static PFNGLBLITFRAMEBUFFERPROC pfnBlitFramebuffer = 0;
	LOAD_ENTRYPOINT("glBlitFramebuffer", pfnBlitFramebuffer, PFNGLBLITFRAMEBUFFERPROC);
	pfnBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

void glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
	typedef void (APIENTRY * PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
	static PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC pfnRenderbufferStorageMultisample = 0;
	LOAD_ENTRYPOINT("glRenderbufferStorageMultisample", pfnRenderbufferStorageMultisample, PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC);
	pfnRenderbufferStorageMultisample(target, samples, internalformat, width, height);
}

void glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
	typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTURELAYERPROC)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
	static PFNGLFRAMEBUFFERTEXTURELAYERPROC pfnFramebufferTextureLayer = 0;
	LOAD_ENTRYPOINT("glFramebufferTextureLayer", pfnFramebufferTextureLayer, PFNGLFRAMEBUFFERTEXTURELAYERPROC);
	pfnFramebufferTextureLayer(target, attachment, texture, level, layer);
}

GLvoid *glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
	typedef GLvoid* (APIENTRY * PFNGLMAPBUFFERRANGEPROC)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
	static PFNGLMAPBUFFERRANGEPROC pfnMapBufferRange = 0;
	LOAD_ENTRYPOINT("glMapBufferRange", pfnMapBufferRange, PFNGLMAPBUFFERRANGEPROC);
	return pfnMapBufferRange(target, offset, length, access);
}

void glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
	typedef void (APIENTRY * PFNGLFLUSHMAPPEDBUFFERRANGEPROC)(GLenum target, GLintptr offset, GLsizeiptr length);
	static PFNGLFLUSHMAPPEDBUFFERRANGEPROC pfnFlushMappedBufferRange = 0;
	LOAD_ENTRYPOINT("glFlushMappedBufferRange", pfnFlushMappedBufferRange, PFNGLFLUSHMAPPEDBUFFERRANGEPROC);
	pfnFlushMappedBufferRange(target, offset, length);
}

void glBindVertexArray(GLuint array)
{
	typedef void (APIENTRY * PFNGLBINDVERTEXARRAYPROC)(GLuint array);
	static PFNGLBINDVERTEXARRAYPROC pfnBindVertexArray = 0;
	LOAD_ENTRYPOINT("glBindVertexArray", pfnBindVertexArray, PFNGLBINDVERTEXARRAYPROC);
	pfnBindVertexArray(array);
}

void glDeleteVertexArrays(GLsizei n, const GLuint *arrays)
{
	typedef void (APIENTRY * PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint *arrays);
	static PFNGLDELETEVERTEXARRAYSPROC pfnDeleteVertexArrays = 0;
	LOAD_ENTRYPOINT("glDeleteVertexArrays", pfnDeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC);
	pfnDeleteVertexArrays(n, arrays);
}

void glGenVertexArrays(GLsizei n, GLuint *arrays)
{
	typedef void (APIENTRY * PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint *arrays);
	static PFNGLGENVERTEXARRAYSPROC pfnGenVertexArrays = 0;
	LOAD_ENTRYPOINT("glGenVertexArrays", pfnGenVertexArrays, PFNGLGENVERTEXARRAYSPROC);
	pfnGenVertexArrays(n, arrays);
}

GLboolean glIsVertexArray(GLuint array)
{
	typedef GLboolean (APIENTRY * PFNGLISVERTEXARRAYPROC)(GLuint array);
	static PFNGLISVERTEXARRAYPROC pfnIsVertexArray = 0;
	LOAD_ENTRYPOINT("glIsVertexArray", pfnIsVertexArray, PFNGLISVERTEXARRAYPROC);
	return pfnIsVertexArray(array);
}

//
// OpenGL 3.1
//

void glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    typedef void (APIENTRY * PFNGLCOPYBUFFERSUBDATAPROC)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
    static PFNGLCOPYBUFFERSUBDATAPROC pfnCopyBufferSubData = 0;
    LOAD_ENTRYPOINT("glCopyBufferSubData", pfnCopyBufferSubData, PFNGLCOPYBUFFERSUBDATAPROC);
    pfnCopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
}

void glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
    typedef void (APIENTRY * PFNGLDRAWARRAYSINSTANCEDPROC)(GLenum mode, GLint first, GLsizei count, GLsizei primcount);
    static PFNGLDRAWARRAYSINSTANCEDPROC pfnDrawArraysInstanced = 0;
    LOAD_ENTRYPOINT("glDrawArraysInstanced", pfnDrawArraysInstanced, PFNGLDRAWARRAYSINSTANCEDPROC);
    pfnDrawArraysInstanced(mode, first, count, primcount);
}

void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount)
{
    typedef void (APIENTRY * PFNGLDRAWELEMENTSINSTANCEDPROC)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount);
    static PFNGLDRAWELEMENTSINSTANCEDPROC pfnDrawElementsInstanced = 0;
    LOAD_ENTRYPOINT("glDrawElementsInstanced", pfnDrawElementsInstanced, PFNGLDRAWELEMENTSINSTANCEDPROC);
    pfnDrawElementsInstanced(mode, count, type, indices, primcount);
}

void glGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName)
{
    typedef void (APIENTRY * PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
    static PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC pfnGetActiveUniformBlockName = 0;
    LOAD_ENTRYPOINT("glGetActiveUniformBlockName", pfnGetActiveUniformBlockName, PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC);
    pfnGetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
}

void glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETACTIVEUNIFORMBLOCKIVPROC)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
    static PFNGLGETACTIVEUNIFORMBLOCKIVPROC pfnGetActiveUniformBlockiv = 0;
    LOAD_ENTRYPOINT("glGetActiveUniformBlockiv", pfnGetActiveUniformBlockiv, PFNGLGETACTIVEUNIFORMBLOCKIVPROC);
    pfnGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
}

void glGetActiveUniformName(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName)
{
    typedef void (APIENTRY * PFNGLGETACTIVEUNIFORMNAMEPROC)(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName);
    static PFNGLGETACTIVEUNIFORMNAMEPROC pfnGetActiveUniformName = 0;
    LOAD_ENTRYPOINT("glGetActiveUniformName", pfnGetActiveUniformName, PFNGLGETACTIVEUNIFORMNAMEPROC);
    pfnGetActiveUniformName(program, uniformIndex, bufSize, length, uniformName);
}

void glGetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params)
{
    typedef void (APIENTRY * PFNGLGETACTIVEUNIFORMSIVPROC)(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
    static PFNGLGETACTIVEUNIFORMSIVPROC pfnGetActiveUniformsiv = 0;
    LOAD_ENTRYPOINT("glGetActiveUniformsiv", pfnGetActiveUniformsiv, PFNGLGETACTIVEUNIFORMSIVPROC);
    pfnGetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
}

GLuint glGetUniformBlockIndex(GLuint program, const GLchar *uniformBlockName)
{
    typedef GLuint (APIENTRY * PFNGLGETUNIFORMBLOCKINDEXPROC)(GLuint program, const GLchar *uniformBlockName);
    static PFNGLGETUNIFORMBLOCKINDEXPROC pfnGetUniformBlockIndex = 0;
    LOAD_ENTRYPOINT("glGetUniformBlockIndex", pfnGetUniformBlockIndex, PFNGLGETUNIFORMBLOCKINDEXPROC);
    return pfnGetUniformBlockIndex(program, uniformBlockName);
}

void glGetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar **uniformNames, GLuint *uniformIndices)
{
    typedef void (APIENTRY * PFNGLGETUNIFORMINDICESPROC)(GLuint program, GLsizei uniformCount, const GLchar **uniformNames, GLuint *uniformIndices);
    static PFNGLGETUNIFORMINDICESPROC pfnGetUniformIndices = 0;
    LOAD_ENTRYPOINT("glGetUniformIndices", pfnGetUniformIndices, PFNGLGETUNIFORMINDICESPROC);
    pfnGetUniformIndices(program, uniformCount, uniformNames, uniformIndices);
}

void glPrimitiveRestartIndex(GLuint index)
{
    typedef void (APIENTRY * PFNGLPRIMITIVERESTARTINDEXPROC)(GLuint index);
    static PFNGLPRIMITIVERESTARTINDEXPROC pfnPrimitiveRestartIndex = 0;
    LOAD_ENTRYPOINT("glPrimitiveRestartIndex", pfnPrimitiveRestartIndex, PFNGLPRIMITIVERESTARTINDEXPROC);
    pfnPrimitiveRestartIndex(index);
}

void glTexBuffer(GLenum target, GLenum internalformat, GLuint buffer)
{
    typedef void (APIENTRY * PFNGLTEXBUFFERPROC)(GLenum target, GLenum internalformat, GLuint buffer);
    static PFNGLTEXBUFFERPROC pfnTexBuffer = 0;
    LOAD_ENTRYPOINT("glTexBuffer", pfnTexBuffer, PFNGLTEXBUFFERPROC);
    pfnTexBuffer(target, internalformat, buffer);
}

void glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    typedef void (APIENTRY * PFNGLUNIFORMBLOCKBINDINGPROC)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
    static PFNGLUNIFORMBLOCKBINDINGPROC pfnUniformBlockBinding = 0;
    LOAD_ENTRYPOINT("glUniformBlockBinding", pfnUniformBlockBinding, PFNGLUNIFORMBLOCKBINDINGPROC);
    pfnUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
}

//
// OpenGL 3.2
//

void glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex)
{
	typedef void (APIENTRY * PFNGLDRAWELEMENTSBASEVERTEXPROC)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex);
	static PFNGLDRAWELEMENTSBASEVERTEXPROC pfnDrawElementsBaseVertex = 0;
	LOAD_ENTRYPOINT("glDrawElementsBaseVertex", pfnDrawElementsBaseVertex, PFNGLDRAWELEMENTSBASEVERTEXPROC);
	pfnDrawElementsBaseVertex(mode, count, type, indices, basevertex);
}

void glDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex)
{
	typedef void (APIENTRY * PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex);
	static PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC pfnDrawRangeElementsBaseVertex = 0;
	LOAD_ENTRYPOINT("glDrawRangeElementsBaseVertex", pfnDrawRangeElementsBaseVertex, PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC);
	pfnDrawRangeElementsBaseVertex(mode, start, end, count, type, indices, basevertex);
}

void glDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount, GLint basevertex)
{
	typedef void (APIENTRY * PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount, GLint basevertex);
	static PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC pfnDrawElementsInstancedBaseVertex = 0;
	LOAD_ENTRYPOINT("glDrawElementsInstancedBaseVertex", pfnDrawElementsInstancedBaseVertex, PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC);
	pfnDrawElementsInstancedBaseVertex(mode, count, type, indices, primcount, basevertex);
}

void glMultiDrawElementsBaseVertex(GLenum mode, const GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount, const GLint *basevertex)
{
	typedef void (APIENTRY * PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC)(GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount, const GLint *basevertex);
	static PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC pfnMultiDrawElementsBaseVertex = 0;
	LOAD_ENTRYPOINT("glMultiDrawElementsBaseVertex", pfnMultiDrawElementsBaseVertex, PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC);
	pfnMultiDrawElementsBaseVertex(mode, count, type, indices, primcount, basevertex);
}

void glProvokingVertex(GLenum mode)
{
	typedef void (APIENTRY * PFNGLPROVOKINGVERTEXPROC)(GLenum mode);
	static PFNGLPROVOKINGVERTEXPROC pfnProvokingVertex = 0;
	LOAD_ENTRYPOINT("glProvokingVertex", pfnProvokingVertex, PFNGLPROVOKINGVERTEXPROC);
	pfnProvokingVertex(mode);
}

GLsync glFenceSync(GLenum condition, GLbitfield flags)
{
	typedef GLsync (APIENTRY * PFNGLFENCESYNCPROC)(GLenum condition, GLbitfield flags);
	static PFNGLFENCESYNCPROC pfnFenceSync = 0;
	LOAD_ENTRYPOINT("glFenceSync", pfnFenceSync, PFNGLFENCESYNCPROC);
	return pfnFenceSync(condition, flags);
}

GLboolean glIsSync(GLsync sync)
{
	typedef GLboolean (APIENTRY * PFNGLISSYNCPROC)(GLsync sync);
	static PFNGLISSYNCPROC pfnIsSync = 0;
	LOAD_ENTRYPOINT("glIsSync", pfnIsSync, PFNGLISSYNCPROC);
	return pfnIsSync(sync);
}

void glDeleteSync(GLsync sync)
{
	typedef void (APIENTRY * PFNGLDELETESYNCPROC)(GLsync sync);
	static PFNGLDELETESYNCPROC pfnDeleteSync = 0;
	LOAD_ENTRYPOINT("glDeleteSync", pfnDeleteSync, PFNGLDELETESYNCPROC);
	pfnDeleteSync(sync);
}

GLenum glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
	typedef GLenum (APIENTRY * PFNGLCLIENTWAITSYNCPROC)(GLsync sync, GLbitfield flags, GLuint64 timeout);
	static PFNGLCLIENTWAITSYNCPROC pfnClientWaitSync = 0;
	LOAD_ENTRYPOINT("glClientWaitSync", pfnClientWaitSync, PFNGLCLIENTWAITSYNCPROC);
	return pfnClientWaitSync(sync, flags, timeout);
}

void glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
	typedef void (APIENTRY * PFNGLWAITSYNCPROC)(GLsync sync, GLbitfield flags, GLuint64 timeout);
	static PFNGLWAITSYNCPROC pfnWaitSync = 0;
	LOAD_ENTRYPOINT("glWaitSync", pfnWaitSync, PFNGLWAITSYNCPROC);
	pfnWaitSync(sync, flags, timeout);
}

void glGetInteger64v(GLenum pname, GLint64 *params)
{
	typedef void (APIENTRY * PFNGLGETINTEGER64VPROC)(GLenum pname, GLint64 *params);
	static PFNGLGETINTEGER64VPROC pfnGetInteger64v = 0;
	LOAD_ENTRYPOINT("glGetInteger64v", pfnGetInteger64v, PFNGLGETINTEGER64VPROC);
	pfnGetInteger64v(pname, params);
}

void glGetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values)
{
	typedef void (APIENTRY * PFNGLGETSYNCIVPROC)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
	static PFNGLGETSYNCIVPROC pfnGetSynciv = 0;
	LOAD_ENTRYPOINT("glGetSynciv", pfnGetSynciv, PFNGLGETSYNCIVPROC);
	pfnGetSynciv(sync, pname, bufSize, length, values);
}

void glTexImage2DMultisample(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
	typedef void (APIENTRY * PFNGLTEXIMAGE2DMULTISAMPLEPROC)(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
	static PFNGLTEXIMAGE2DMULTISAMPLEPROC pfnTexImage2DMultisample = 0;
	LOAD_ENTRYPOINT("glTexImage2DMultisample", pfnTexImage2DMultisample, PFNGLTEXIMAGE2DMULTISAMPLEPROC);
	pfnTexImage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
}

void glTexImage3DMultisample(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
	typedef void (APIENTRY * PFNGLTEXIMAGE3DMULTISAMPLEPROC)(GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
	static PFNGLTEXIMAGE3DMULTISAMPLEPROC pfnTexImage3DMultisample = 0;
	LOAD_ENTRYPOINT("glTexImage3DMultisample", pfnTexImage3DMultisample, PFNGLTEXIMAGE3DMULTISAMPLEPROC);
	pfnTexImage3DMultisample(target, samples, internalformat, width, height, depth, fixedsamplelocations);
}

void glGetMultisamplefv(GLenum pname, GLuint index, GLfloat *val)
{
	typedef void (APIENTRY * PFNGLGETMULTISAMPLEFVPROC)(GLenum pname, GLuint index, GLfloat *val);
	static PFNGLGETMULTISAMPLEFVPROC pfnGetMultisamplefv = 0;
	LOAD_ENTRYPOINT("glGetMultisamplefv", pfnGetMultisamplefv, PFNGLGETMULTISAMPLEFVPROC);
	pfnGetMultisamplefv(pname, index, val);
}

void glSampleMaski(GLuint index, GLbitfield mask)
{
	typedef void (APIENTRY * PFNGLSAMPLEMASKIPROC)(GLuint index, GLbitfield mask);
	static PFNGLSAMPLEMASKIPROC pfnSampleMaski = 0;
	LOAD_ENTRYPOINT("glSampleMaski", pfnSampleMaski, PFNGLSAMPLEMASKIPROC);
	pfnSampleMaski(index, mask);
}

void glGetInteger64i_v(GLenum target, GLuint index, GLint64 *data)
{
	typedef void (APIENTRY * PFNGLGETINTEGER64I_VPROC)(GLenum target, GLuint index, GLint64 *data);
	static PFNGLGETINTEGER64I_VPROC pfnGetInteger64i_v = 0;
	LOAD_ENTRYPOINT("glGetInteger64i_v", pfnGetInteger64i_v, PFNGLGETINTEGER64I_VPROC);
	pfnGetInteger64i_v(target, index, data);
}

void glGetBufferParameteri64v(GLenum target, GLenum pname, GLint64 *params)
{
	typedef void (APIENTRY * PFNGLGETBUFFERPARAMETERI64VPROC)(GLenum target, GLenum pname, GLint64 *params);
	static PFNGLGETBUFFERPARAMETERI64VPROC pfnGetBufferParameteri64v = 0;
	LOAD_ENTRYPOINT("glGetBufferParameteri64v", pfnGetBufferParameteri64v, PFNGLGETBUFFERPARAMETERI64VPROC);
	pfnGetBufferParameteri64v(target, pname, params);
}

void glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
	typedef void (APIENTRY * PFNGLFRAMEBUFFERTEXTUREPROC)(GLenum target, GLenum attachment, GLuint texture, GLint level);
	static PFNGLFRAMEBUFFERTEXTUREPROC pfnFramebufferTexture = 0;
	LOAD_ENTRYPOINT("glFramebufferTexture", pfnFramebufferTexture, PFNGLFRAMEBUFFERTEXTUREPROC);
	pfnFramebufferTexture(target, attachment, texture, level);
}

//
// OpenGL 3.3
//

void glBindFragDataLocationIndexed(GLuint program, GLuint colorNumber, GLuint index, const GLchar *name)
{
	typedef void (APIENTRY * PFNGLBINDFRAGDATALOCATIONINDEXEDPROC)(GLuint program, GLuint colorNumber, GLuint index, const GLchar *name);
	static PFNGLBINDFRAGDATALOCATIONINDEXEDPROC pfnBindFragDataLocationIndexed = 0;
	LOAD_ENTRYPOINT("glBindFragDataLocationIndexed", pfnBindFragDataLocationIndexed, PFNGLBINDFRAGDATALOCATIONINDEXEDPROC);
	pfnBindFragDataLocationIndexed(program, colorNumber, index, name);
}

GLint glGetFragDataIndex(GLuint program, const GLchar *name)
{
	typedef GLint (APIENTRY * PFNGLGETFRAGDATAINDEXPROC)(GLuint program, const GLchar *name);
	static PFNGLGETFRAGDATAINDEXPROC pfnGetFragDataIndex = 0;
	LOAD_ENTRYPOINT("glGetFragDataIndex", pfnGetFragDataIndex, PFNGLGETFRAGDATAINDEXPROC);
	return pfnGetFragDataIndex(program, name);
}

void glGenSamplers(GLsizei count, GLuint *samplers)
{
	typedef void (APIENTRY * PFNGLGENSAMPLERSPROC)(GLsizei count, GLuint *samplers);
	static PFNGLGENSAMPLERSPROC pfnGenSamplers = 0;
	LOAD_ENTRYPOINT("glGenSamplers", pfnGenSamplers, PFNGLGENSAMPLERSPROC);
	pfnGenSamplers(count, samplers);
}

void glDeleteSamplers(GLsizei count, const GLuint *samplers)
{
	typedef void (APIENTRY * PFNGLDELETESAMPLERSPROC)(GLsizei count, const GLuint *samplers);
	static PFNGLDELETESAMPLERSPROC pfnDeleteSamplers = 0;
	LOAD_ENTRYPOINT("glDeleteSamplers", pfnDeleteSamplers, PFNGLDELETESAMPLERSPROC);
	pfnDeleteSamplers(count, samplers);
}

GLboolean glIsSampler(GLuint sampler)
{
	typedef GLboolean (APIENTRY * PFNGLISSAMPLERPROC)(GLuint sampler);
	static PFNGLISSAMPLERPROC pfnIsSampler = 0;
	LOAD_ENTRYPOINT("glIsSampler", pfnIsSampler, PFNGLISSAMPLERPROC);
	return pfnIsSampler(sampler);
}

void glBindSampler(GLenum unit, GLuint sampler)
{
	typedef void (APIENTRY * PFNGLBINDSAMPLERPROC)(GLenum unit, GLuint sampler);
	static PFNGLBINDSAMPLERPROC pfnBindSampler = 0;
	LOAD_ENTRYPOINT("glBindSampler", pfnBindSampler, PFNGLBINDSAMPLERPROC);
	pfnBindSampler(unit, sampler);
}

void glSamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
	typedef void (APIENTRY * PFNGLSAMPLERPARAMETERIPROC)(GLuint sampler, GLenum pname, GLint param);
	static PFNGLSAMPLERPARAMETERIPROC pfnSamplerParameteri = 0;
	LOAD_ENTRYPOINT("glSamplerParameteri", pfnSamplerParameteri, PFNGLSAMPLERPARAMETERIPROC);
	pfnSamplerParameteri(sampler, pname, param);
}

void glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint *param)
{
	typedef void (APIENTRY * PFNGLSAMPLERPARAMETERIVPROC)(GLuint sampler, GLenum pname, const GLint *param);
	static PFNGLSAMPLERPARAMETERIVPROC pfnSamplerParameteriv = 0;
	LOAD_ENTRYPOINT("glSamplerParameteriv", pfnSamplerParameteriv, PFNGLSAMPLERPARAMETERIVPROC);
	pfnSamplerParameteriv(sampler, pname, param);
}

void glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
	typedef void (APIENTRY * PFNGLSAMPLERPARAMETERFPROC)(GLuint sampler, GLenum pname, GLfloat param);
	static PFNGLSAMPLERPARAMETERFPROC pfnSamplerParameterf = 0;
	LOAD_ENTRYPOINT("glSamplerParameterf", pfnSamplerParameterf, PFNGLSAMPLERPARAMETERFPROC);
	pfnSamplerParameterf(sampler, pname, param);
}

void glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat *param)
{
	typedef void (APIENTRY * PFNGLSAMPLERPARAMETERFVPROC)(GLuint sampler, GLenum pname, const GLfloat *param);
	static PFNGLSAMPLERPARAMETERFVPROC pfnSamplerParameterfv = 0;
	LOAD_ENTRYPOINT("glSamplerParameterfv", pfnSamplerParameterfv, PFNGLSAMPLERPARAMETERFVPROC);
	pfnSamplerParameterfv(sampler, pname, param);
}

void glSamplerParameterIiv(GLuint sampler, GLenum pname, const GLint *param)
{
	typedef void (APIENTRY * PFNGLSAMPLERPARAMETERIIVPROC)(GLuint sampler, GLenum pname, const GLint *param);
	static PFNGLSAMPLERPARAMETERIIVPROC pfnSamplerParameterIiv = 0;
	LOAD_ENTRYPOINT("glSamplerParameterIiv", pfnSamplerParameterIiv, PFNGLSAMPLERPARAMETERIIVPROC);
	pfnSamplerParameterIiv(sampler, pname, param);
}

void glSamplerParameterIuiv(GLuint sampler, GLenum pname, const GLuint *param)
{
	typedef void (APIENTRY * PFNGLSAMPLERPARAMETERIUIVPROC)(GLuint sampler, GLenum pname, const GLuint *param);
	static PFNGLSAMPLERPARAMETERIUIVPROC pfnSamplerParameterIuiv = 0;
	LOAD_ENTRYPOINT("glSamplerParameterIuiv", pfnSamplerParameterIuiv, PFNGLSAMPLERPARAMETERIUIVPROC);
	pfnSamplerParameterIuiv(sampler, pname, param);
}

void glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint *params)
{
	typedef void (APIENTRY * PFNGLGETSAMPLERPARAMETERIVPROC)(GLuint sampler, GLenum pname, GLint *params);
	static PFNGLGETSAMPLERPARAMETERIVPROC pfnGetSamplerParameteriv = 0;
	LOAD_ENTRYPOINT("glGetSamplerParameteriv", pfnGetSamplerParameteriv, PFNGLGETSAMPLERPARAMETERIVPROC);
	pfnGetSamplerParameteriv(sampler, pname, params);
}

void glGetSamplerParameterIiv(GLuint sampler, GLenum pname, GLint *params)
{
	typedef void (APIENTRY * PFNGLGETSAMPLERPARAMETERIIVPROC)(GLuint sampler, GLenum pname, GLint *params);
	static PFNGLGETSAMPLERPARAMETERIIVPROC pfnGetSamplerParameterIiv = 0;
	LOAD_ENTRYPOINT("glGetSamplerParameterIiv", pfnGetSamplerParameterIiv, PFNGLGETSAMPLERPARAMETERIIVPROC);
	pfnGetSamplerParameterIiv(sampler, pname, params);
}

void glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat *params)
{
	typedef void (APIENTRY * PFNGLGETSAMPLERPARAMETERFVPROC)(GLuint sampler, GLenum pname, GLfloat *params);
	static PFNGLGETSAMPLERPARAMETERFVPROC pfnGetSamplerParameterfv = 0;
	LOAD_ENTRYPOINT("glGetSamplerParameterfv", pfnGetSamplerParameterfv, PFNGLGETSAMPLERPARAMETERFVPROC);
	pfnGetSamplerParameterfv(sampler, pname, params);
}

void glGetSamplerParameterIfv(GLuint sampler, GLenum pname, GLfloat *params)
{
	typedef void (APIENTRY * PFNGLGETSAMPLERPARAMETERIFVPROC)(GLuint sampler, GLenum pname, GLfloat *params);
	static PFNGLGETSAMPLERPARAMETERIFVPROC pfnGetSamplerParameterIfv = 0;
	LOAD_ENTRYPOINT("glGetSamplerParameterIfv", pfnGetSamplerParameterIfv, PFNGLGETSAMPLERPARAMETERIFVPROC);
	pfnGetSamplerParameterIfv(sampler, pname, params);
}

void glQueryCounter(GLuint id, GLenum target)
{
	typedef void (APIENTRY * PFNGLQUERYCOUNTERPROC)(GLuint id, GLenum target);
	static PFNGLQUERYCOUNTERPROC pfnQueryCounter = 0;
	LOAD_ENTRYPOINT("glQueryCounter", pfnQueryCounter, PFNGLQUERYCOUNTERPROC);
	pfnQueryCounter(id, target);
}

void glGetQueryObjecti64v(GLuint id, GLenum pname, GLint64 *params)
{
	typedef void (APIENTRY * PFNGLGETQUERYOBJECTI64VPROC)(GLuint id, GLenum pname, GLint64 *params);
	static PFNGLGETQUERYOBJECTI64VPROC pfnGetQueryObjecti64v = 0;
	LOAD_ENTRYPOINT("glGetQueryObjecti64v", pfnGetQueryObjecti64v, PFNGLGETQUERYOBJECTI64VPROC);
	pfnGetQueryObjecti64v(id, pname, params);
}

void glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64 *params)
{
	typedef void (APIENTRY * PFNGLGETQUERYOBJECTUI64VPROC)(GLuint id, GLenum pname, GLuint64 *params);
	static PFNGLGETQUERYOBJECTUI64VPROC pfnGetQueryObjectui64v = 0;
	LOAD_ENTRYPOINT("glGetQueryObjectui64v", pfnGetQueryObjectui64v, PFNGLGETQUERYOBJECTUI64VPROC);
	pfnGetQueryObjectui64v(id, pname, params);
}

void glVertexP2ui(GLenum type, GLuint value)
{
	typedef void (APIENTRY * PFNGLVERTEXP2UIPROC)(GLenum type, GLuint value);
	static PFNGLVERTEXP2UIPROC pfnVertexP2ui = 0;
	LOAD_ENTRYPOINT("glVertexP2ui", pfnVertexP2ui, PFNGLVERTEXP2UIPROC);
	pfnVertexP2ui(type, value);
}

void glVertexP2uiv(GLenum type, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLVERTEXP2UIVPROC)(GLenum type, const GLuint *value);
	static PFNGLVERTEXP2UIVPROC pfnVertexP2uiv = 0;
	LOAD_ENTRYPOINT("glVertexP2uiv", pfnVertexP2uiv, PFNGLVERTEXP2UIVPROC);
	pfnVertexP2uiv(type, value);
}

void glVertexP3ui(GLenum type, GLuint value)
{
	typedef void (APIENTRY * PFNGLVERTEXP3UIPROC)(GLenum type, GLuint value);
	static PFNGLVERTEXP3UIPROC pfnVertexP3ui = 0;
	LOAD_ENTRYPOINT("glVertexP3ui", pfnVertexP3ui, PFNGLVERTEXP3UIPROC);
	pfnVertexP3ui(type, value);
}

void glVertexP3uiv(GLenum type, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLVERTEXP3UIVPROC)(GLenum type, const GLuint *value);
	static PFNGLVERTEXP3UIVPROC pfnVertexP3uiv = 0;
	LOAD_ENTRYPOINT("glVertexP3uiv", pfnVertexP3uiv, PFNGLVERTEXP3UIVPROC);
	pfnVertexP3uiv(type, value);
}

void glVertexP4ui(GLenum type, GLuint value)
{
	typedef void (APIENTRY * PFNGLVERTEXP4UIPROC)(GLenum type, GLuint value);
	static PFNGLVERTEXP4UIPROC pfnVertexP4ui = 0;
	LOAD_ENTRYPOINT("glVertexP4ui", pfnVertexP4ui, PFNGLVERTEXP4UIPROC);
	pfnVertexP4ui(type, value);
}

void glVertexP4uiv(GLenum type, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLVERTEXP4UIVPROC)(GLenum type, const GLuint *value);
	static PFNGLVERTEXP4UIVPROC pfnVertexP4uiv = 0;
	LOAD_ENTRYPOINT("glVertexP4uiv", pfnVertexP4uiv, PFNGLVERTEXP4UIVPROC);
	pfnVertexP4uiv(type, value);
}

void glTexCoordP1ui(GLenum type, GLuint coords)
{
	typedef void (APIENTRY * PFNGLTEXCOORDP1UIPROC)(GLenum type, GLuint coords);
	static PFNGLTEXCOORDP1UIPROC pfnTexCoordP1ui = 0;
	LOAD_ENTRYPOINT("glTexCoordP1ui", pfnTexCoordP1ui, PFNGLTEXCOORDP1UIPROC);
	pfnTexCoordP1ui(type, coords);
}

void glTexCoordP1uiv(GLenum type, const GLuint *coords)
{
	typedef void (APIENTRY * PFNGLTEXCOORDP1UIVPROC)(GLenum type, const GLuint *coords);
	static PFNGLTEXCOORDP1UIVPROC pfnTexCoordP1uiv = 0;
	LOAD_ENTRYPOINT("glTexCoordP1uiv", pfnTexCoordP1uiv, PFNGLTEXCOORDP1UIVPROC);
	pfnTexCoordP1uiv(type, coords);
}

void glTexCoordP2ui(GLenum type, GLuint coords)
{
	typedef void (APIENTRY * PFNGLTEXCOORDP2UIPROC)(GLenum type, GLuint coords);
	static PFNGLTEXCOORDP2UIPROC pfnTexCoordP2ui = 0;
	LOAD_ENTRYPOINT("glTexCoordP2ui", pfnTexCoordP2ui, PFNGLTEXCOORDP2UIPROC);
	pfnTexCoordP2ui(type, coords);
}

void glTexCoordP2uiv(GLenum type, const GLuint *coords)
{
	typedef void (APIENTRY * PFNGLTEXCOORDP2UIVPROC)(GLenum type, const GLuint *coords);
	static PFNGLTEXCOORDP2UIVPROC pfnTexCoordP2uiv = 0;
	LOAD_ENTRYPOINT("glTexCoordP2uiv", pfnTexCoordP2uiv, PFNGLTEXCOORDP2UIVPROC);
	pfnTexCoordP2uiv(type, coords);
}

void glTexCoordP3ui(GLenum type, GLuint coords)
{
	typedef void (APIENTRY * PFNGLTEXCOORDP3UIPROC)(GLenum type, GLuint coords);
	static PFNGLTEXCOORDP3UIPROC pfnTexCoordP3ui = 0;
	LOAD_ENTRYPOINT("glTexCoordP3ui", pfnTexCoordP3ui, PFNGLTEXCOORDP3UIPROC);
	pfnTexCoordP3ui(type, coords);
}

void glTexCoordP3uiv(GLenum type, const GLuint *coords)
{
	typedef void (APIENTRY * PFNGLTEXCOORDP3UIVPROC)(GLenum type, const GLuint *coords);
	static PFNGLTEXCOORDP3UIVPROC pfnTexCoordP3uiv = 0;
	LOAD_ENTRYPOINT("glTexCoordP3uiv", pfnTexCoordP3uiv, PFNGLTEXCOORDP3UIVPROC);
	pfnTexCoordP3uiv(type, coords);
}

void glTexCoordP4ui(GLenum type, GLuint coords)
{
	typedef void (APIENTRY * PFNGLTEXCOORDP4UIPROC)(GLenum type, GLuint coords);
	static PFNGLTEXCOORDP4UIPROC pfnTexCoordP4ui = 0;
	LOAD_ENTRYPOINT("glTexCoordP4ui", pfnTexCoordP4ui, PFNGLTEXCOORDP4UIPROC);
	pfnTexCoordP4ui(type, coords);
}

void glTexCoordP4uiv(GLenum type, const GLuint *coords)
{
	typedef void (APIENTRY * PFNGLTEXCOORDP4UIVPROC)(GLenum type, const GLuint *coords);
	static PFNGLTEXCOORDP4UIVPROC pfnTexCoordP4uiv = 0;
	LOAD_ENTRYPOINT("glTexCoordP4uiv", pfnTexCoordP4uiv, PFNGLTEXCOORDP4UIVPROC);
	pfnTexCoordP4uiv(type, coords);
}

void glMultiTexCoordP1ui(GLenum texture, GLenum type, GLuint coords)
{
	typedef void (APIENTRY * PFNGLMULTITEXCOORDP1UIPROC)(GLenum texture, GLenum type, GLuint coords);
	static PFNGLMULTITEXCOORDP1UIPROC pfnMultiTexCoordP1ui = 0;
	LOAD_ENTRYPOINT("glMultiTexCoordP1ui", pfnMultiTexCoordP1ui, PFNGLMULTITEXCOORDP1UIPROC);
	pfnMultiTexCoordP1ui(texture, type, coords);
}

void glMultiTexCoordP1uiv(GLenum texture, GLenum type, const GLuint *coords)
{
	typedef void (APIENTRY * PFNGLMULTITEXCOORDP1UIVPROC)(GLenum texture, GLenum type, const GLuint *coords);
	static PFNGLMULTITEXCOORDP1UIVPROC pfnMultiTexCoordP1uiv = 0;
	LOAD_ENTRYPOINT("glMultiTexCoordP1uiv", pfnMultiTexCoordP1uiv, PFNGLMULTITEXCOORDP1UIVPROC);
	pfnMultiTexCoordP1uiv(texture, type, coords);
}

void glMultiTexCoordP2ui(GLenum texture, GLenum type, GLuint coords)
{
	typedef void (APIENTRY * PFNGLMULTITEXCOORDP2UIPROC)(GLenum texture, GLenum type, GLuint coords);
	static PFNGLMULTITEXCOORDP2UIPROC pfnMultiTexCoordP2ui = 0;
	LOAD_ENTRYPOINT("glMultiTexCoordP2ui", pfnMultiTexCoordP2ui, PFNGLMULTITEXCOORDP2UIPROC);
	pfnMultiTexCoordP2ui(texture, type, coords);
}

void glMultiTexCoordP2uiv(GLenum texture, GLenum type, const GLuint *coords)
{
	typedef void (APIENTRY * PFNGLMULTITEXCOORDP2UIVPROC)(GLenum texture, GLenum type, const GLuint *coords);
	static PFNGLMULTITEXCOORDP2UIVPROC pfnMultiTexCoordP2uiv = 0;
	LOAD_ENTRYPOINT("glMultiTexCoordP2uiv", pfnMultiTexCoordP2uiv, PFNGLMULTITEXCOORDP2UIVPROC);
	pfnMultiTexCoordP2uiv(texture, type, coords);
}

void glMultiTexCoordP3ui(GLenum texture, GLenum type, GLuint coords)
{
	typedef void (APIENTRY * PFNGLMULTITEXCOORDP3UIPROC)(GLenum texture, GLenum type, GLuint coords);
	static PFNGLMULTITEXCOORDP3UIPROC pfnMultiTexCoordP3ui = 0;
	LOAD_ENTRYPOINT("glMultiTexCoordP3ui", pfnMultiTexCoordP3ui, PFNGLMULTITEXCOORDP3UIPROC);
	pfnMultiTexCoordP3ui(texture, type, coords);
}

void glMultiTexCoordP3uiv(GLenum texture, GLenum type, const GLuint *coords)
{
	typedef void (APIENTRY * PFNGLMULTITEXCOORDP3UIVPROC)(GLenum texture, GLenum type, const GLuint *coords);
	static PFNGLMULTITEXCOORDP3UIVPROC pfnMultiTexCoordP3uiv = 0;
	LOAD_ENTRYPOINT("glMultiTexCoordP3uiv", pfnMultiTexCoordP3uiv, PFNGLMULTITEXCOORDP3UIVPROC);
	pfnMultiTexCoordP3uiv(texture, type, coords);
}

void glMultiTexCoordP4ui(GLenum texture, GLenum type, GLuint coords)
{
	typedef void (APIENTRY * PFNGLMULTITEXCOORDP4UIPROC)(GLenum texture, GLenum type, GLuint coords);
	static PFNGLMULTITEXCOORDP4UIPROC pfnMultiTexCoordP4ui = 0;
	LOAD_ENTRYPOINT("glMultiTexCoordP4ui", pfnMultiTexCoordP4ui, PFNGLMULTITEXCOORDP4UIPROC);
	pfnMultiTexCoordP4ui(texture, type, coords);
}

void glMultiTexCoordP4uiv(GLenum texture, GLenum type, const GLuint *coords)
{
	typedef void (APIENTRY * PFNGLMULTITEXCOORDP4UIVPROC)(GLenum texture, GLenum type, const GLuint *coords);
	static PFNGLMULTITEXCOORDP4UIVPROC pfnMultiTexCoordP4uiv = 0;
	LOAD_ENTRYPOINT("glMultiTexCoordP4uiv", pfnMultiTexCoordP4uiv, PFNGLMULTITEXCOORDP4UIVPROC);
	pfnMultiTexCoordP4uiv(texture, type, coords);
}

void glNormalP3ui(GLenum type, GLuint coords)
{
	typedef void (APIENTRY * PFNGLNORMALP3UIPROC)(GLenum type, GLuint coords);
	static PFNGLNORMALP3UIPROC pfnNormalP3ui = 0;
	LOAD_ENTRYPOINT("glNormalP3ui", pfnNormalP3ui, PFNGLNORMALP3UIPROC);
	pfnNormalP3ui(type, coords);
}

void glNormalP3uiv(GLenum type, const GLuint *coords)
{
	typedef void (APIENTRY * PFNGLNORMALP3UIVPROC)(GLenum type, const GLuint *coords);
	static PFNGLNORMALP3UIVPROC pfnNormalP3uiv = 0;
	LOAD_ENTRYPOINT("glNormalP3uiv", pfnNormalP3uiv, PFNGLNORMALP3UIVPROC);
	pfnNormalP3uiv(type, coords);
}

void glColorP3ui(GLenum type, GLuint color)
{
	typedef void (APIENTRY * PFNGLCOLORP3UIPROC)(GLenum type, GLuint color);
	static PFNGLCOLORP3UIPROC pfnColorP3ui = 0;
	LOAD_ENTRYPOINT("glColorP3ui", pfnColorP3ui, PFNGLCOLORP3UIPROC);
	pfnColorP3ui(type, color);
}

void glColorP3uiv(GLenum type, const GLuint *color)
{
	typedef void (APIENTRY * PFNGLCOLORP3UIVPROC)(GLenum type, const GLuint *color);
	static PFNGLCOLORP3UIVPROC pfnColorP3uiv = 0;
	LOAD_ENTRYPOINT("glColorP3uiv", pfnColorP3uiv, PFNGLCOLORP3UIVPROC);
	pfnColorP3uiv(type, color);
}

void glColorP4ui(GLenum type, GLuint color)
{
	typedef void (APIENTRY * PFNGLCOLORP4UIPROC)(GLenum type, GLuint color);
	static PFNGLCOLORP4UIPROC pfnColorP4ui = 0;
	LOAD_ENTRYPOINT("glColorP4ui", pfnColorP4ui, PFNGLCOLORP4UIPROC);
	pfnColorP4ui(type, color);
}

void glColorP4uiv(GLenum type, const GLuint *color)
{
	typedef void (APIENTRY * PFNGLCOLORP4UIVPROC)(GLenum type, const GLuint *color);
	static PFNGLCOLORP4UIVPROC pfnColorP4uiv = 0;
	LOAD_ENTRYPOINT("glColorP4uiv", pfnColorP4uiv, PFNGLCOLORP4UIVPROC);
	pfnColorP4uiv(type, color);
}

void glSecondaryColorP3ui(GLenum type, GLuint color)
{
	typedef void (APIENTRY * PFNGLSECONDARYCOLORP3UIPROC)(GLenum type, GLuint color);
	static PFNGLSECONDARYCOLORP3UIPROC pfnSecondaryColorP3ui = 0;
	LOAD_ENTRYPOINT("glSecondaryColorP3ui", pfnSecondaryColorP3ui, PFNGLSECONDARYCOLORP3UIPROC);
	pfnSecondaryColorP3ui(type, color);
}

void glSecondaryColorP3uiv(GLenum type, const GLuint *color)
{
	typedef void (APIENTRY * PFNGLSECONDARYCOLORP3UIVPROC)(GLenum type, const GLuint *color);
	static PFNGLSECONDARYCOLORP3UIVPROC pfnSecondaryColorP3uiv = 0;
	LOAD_ENTRYPOINT("glSecondaryColorP3uiv", pfnSecondaryColorP3uiv, PFNGLSECONDARYCOLORP3UIVPROC);
	pfnSecondaryColorP3uiv(type, color);
}

void glVertexAttribP1ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBP1UIPROC)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
	static PFNGLVERTEXATTRIBP1UIPROC pfnVertexAttribP1ui = 0;
	LOAD_ENTRYPOINT("glVertexAttribP1ui", pfnVertexAttribP1ui, PFNGLVERTEXATTRIBP1UIPROC);
	pfnVertexAttribP1ui(index, type, normalized, value);
}

void glVertexAttribP1uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBP1UIVPROC)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
	static PFNGLVERTEXATTRIBP1UIVPROC pfnVertexAttribP1uiv = 0;
	LOAD_ENTRYPOINT("glVertexAttribP1uiv", pfnVertexAttribP1uiv, PFNGLVERTEXATTRIBP1UIVPROC);
	pfnVertexAttribP1uiv(index, type, normalized, value);
}

void glVertexAttribP2ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBP2UIPROC)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
	static PFNGLVERTEXATTRIBP2UIPROC pfnVertexAttribP2ui = 0;
	LOAD_ENTRYPOINT("glVertexAttribP2ui", pfnVertexAttribP2ui, PFNGLVERTEXATTRIBP2UIPROC);
	pfnVertexAttribP2ui(index, type, normalized, value);
}

void glVertexAttribP2uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBP2UIVPROC)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
	static PFNGLVERTEXATTRIBP2UIVPROC pfnVertexAttribP2uiv = 0;
	LOAD_ENTRYPOINT("glVertexAttribP2uiv", pfnVertexAttribP2uiv, PFNGLVERTEXATTRIBP2UIVPROC);
	pfnVertexAttribP2uiv(index, type, normalized, value);
}

void glVertexAttribP3ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBP3UIPROC)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
	static PFNGLVERTEXATTRIBP3UIPROC pfnVertexAttribP3ui = 0;
	LOAD_ENTRYPOINT("glVertexAttribP3ui", pfnVertexAttribP3ui, PFNGLVERTEXATTRIBP3UIPROC);
	pfnVertexAttribP3ui(index, type, normalized, value);
}

void glVertexAttribP3uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBP3UIVPROC)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
	static PFNGLVERTEXATTRIBP3UIVPROC pfnVertexAttribP3uiv = 0;
	LOAD_ENTRYPOINT("glVertexAttribP3uiv", pfnVertexAttribP3uiv, PFNGLVERTEXATTRIBP3UIVPROC);
	pfnVertexAttribP3uiv(index, type, normalized, value);
}

void glVertexAttribP4ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBP4UIPROC)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
	static PFNGLVERTEXATTRIBP4UIPROC pfnVertexAttribP4ui = 0;
	LOAD_ENTRYPOINT("glVertexAttribP4ui", pfnVertexAttribP4ui, PFNGLVERTEXATTRIBP4UIPROC);
	pfnVertexAttribP4ui(index, type, normalized, value);
}

void glVertexAttribP4uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBP4UIVPROC)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
	static PFNGLVERTEXATTRIBP4UIVPROC pfnVertexAttribP4uiv = 0;
	LOAD_ENTRYPOINT("glVertexAttribP4uiv", pfnVertexAttribP4uiv, PFNGLVERTEXATTRIBP4UIVPROC);
	pfnVertexAttribP4uiv(index, type, normalized, value);
}

//
// OpenGL 4.0
//

void glUniform1d(GLint location, GLdouble x)
{
	typedef void (APIENTRY * PFNGLUNIFORM1DPROC)(GLint location, GLdouble x);
	static PFNGLUNIFORM1DPROC pfnUniform1d = 0;
	LOAD_ENTRYPOINT("glUniform1d", pfnUniform1d, PFNGLUNIFORM1DPROC);
	pfnUniform1d(location, x);
}

void glUniform2d(GLint location, GLdouble x, GLdouble y)
{
	typedef void (APIENTRY * PFNGLUNIFORM2DPROC)(GLint location, GLdouble x, GLdouble y);
	static PFNGLUNIFORM2DPROC pfnUniform2d = 0;
	LOAD_ENTRYPOINT("glUniform2d", pfnUniform2d, PFNGLUNIFORM2DPROC);
	pfnUniform2d(location, x, y);
}

void glUniform3d(GLint location, GLdouble x, GLdouble y, GLdouble z)
{
	typedef void (APIENTRY * PFNGLUNIFORM3DPROC)(GLint location, GLdouble x, GLdouble y, GLdouble z);
	static PFNGLUNIFORM3DPROC pfnUniform3d = 0;
	LOAD_ENTRYPOINT("glUniform3d", pfnUniform3d, PFNGLUNIFORM3DPROC);
	pfnUniform3d(location, x, y, z);
}

void glUniform4d(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	typedef void (APIENTRY * PFNGLUNIFORM4DPROC)(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	static PFNGLUNIFORM4DPROC pfnUniform4d = 0;
	LOAD_ENTRYPOINT("glUniform4d", pfnUniform4d, PFNGLUNIFORM4DPROC);
	pfnUniform4d(location, x, y, z, w);
}

void glUniform1dv(GLint location, GLsizei count, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLUNIFORM1DVPROC)(GLint location, GLsizei count, const GLdouble *value);
	static PFNGLUNIFORM1DVPROC pfnUniform1dv = 0;
	LOAD_ENTRYPOINT("glUniform1dv", pfnUniform1dv, PFNGLUNIFORM1DVPROC);
	pfnUniform1dv(location, count, value);
}

void glUniform2dv(GLint location, GLsizei count, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLUNIFORM2DVPROC)(GLint location, GLsizei count, const GLdouble *value);
	static PFNGLUNIFORM2DVPROC pfnUniform2dv = 0;
	LOAD_ENTRYPOINT("glUniform2dv", pfnUniform2dv, PFNGLUNIFORM2DVPROC);
	pfnUniform2dv(location, count, value);
}

void glUniform3dv(GLint location, GLsizei count, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLUNIFORM3DVPROC)(GLint location, GLsizei count, const GLdouble *value);
	static PFNGLUNIFORM3DVPROC pfnUniform3dv = 0;
	LOAD_ENTRYPOINT("glUniform3dv", pfnUniform3dv, PFNGLUNIFORM3DVPROC);
	pfnUniform3dv(location, count, value);
}

void glUniform4dv(GLint location, GLsizei count, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLUNIFORM4DVPROC)(GLint location, GLsizei count, const GLdouble *value);
	static PFNGLUNIFORM4DVPROC pfnUniform4dv = 0;
	LOAD_ENTRYPOINT("glUniform4dv", pfnUniform4dv, PFNGLUNIFORM4DVPROC);
	pfnUniform4dv(location, count, value);
}

void glUniformMatrix2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLUNIFORMMATRIX2DVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLUNIFORMMATRIX2DVPROC pfnUniformMatrix2dv = 0;
	LOAD_ENTRYPOINT("glUniformMatrix2dv", pfnUniformMatrix2dv, PFNGLUNIFORMMATRIX2DVPROC);
	pfnUniformMatrix2dv(location, count, transpose, value);
}

void glUniformMatrix3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLUNIFORMMATRIX3DVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLUNIFORMMATRIX3DVPROC pfnUniformMatrix3dv = 0;
	LOAD_ENTRYPOINT("glUniformMatrix3dv", pfnUniformMatrix3dv, PFNGLUNIFORMMATRIX3DVPROC);
	pfnUniformMatrix3dv(location, count, transpose, value);
}

void glUniformMatrix4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLUNIFORMMATRIX4DVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLUNIFORMMATRIX4DVPROC pfnUniformMatrix4dv = 0;
	LOAD_ENTRYPOINT("glUniformMatrix4dv", pfnUniformMatrix4dv, PFNGLUNIFORMMATRIX4DVPROC);
	pfnUniformMatrix4dv(location, count, transpose, value);
}

void glUniformMatrix2x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLUNIFORMMATRIX2X3DVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLUNIFORMMATRIX2X3DVPROC pfnUniformMatrix2x3dv = 0;
	LOAD_ENTRYPOINT("glUniformMatrix2x3dv", pfnUniformMatrix2x3dv, PFNGLUNIFORMMATRIX2X3DVPROC);
	pfnUniformMatrix2x3dv(location, count, transpose, value);
}

void glUniformMatrix2x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLUNIFORMMATRIX2X4DVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLUNIFORMMATRIX2X4DVPROC pfnUniformMatrix2x4dv = 0;
	LOAD_ENTRYPOINT("glUniformMatrix2x4dv", pfnUniformMatrix2x4dv, PFNGLUNIFORMMATRIX2X4DVPROC);
	pfnUniformMatrix2x4dv(location, count, transpose, value);
}

void glUniformMatrix3x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLUNIFORMMATRIX3X2DVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLUNIFORMMATRIX3X2DVPROC pfnUniformMatrix3x2dv = 0;
	LOAD_ENTRYPOINT("glUniformMatrix3x2dv", pfnUniformMatrix3x2dv, PFNGLUNIFORMMATRIX3X2DVPROC);
	pfnUniformMatrix3x2dv(location, count, transpose, value);
}

void glUniformMatrix3x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLUNIFORMMATRIX3X4DVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLUNIFORMMATRIX3X4DVPROC pfnUniformMatrix3x4dv = 0;
	LOAD_ENTRYPOINT("glUniformMatrix3x4dv", pfnUniformMatrix3x4dv, PFNGLUNIFORMMATRIX3X4DVPROC);
	pfnUniformMatrix3x4dv(location, count, transpose, value);
}

void glUniformMatrix4x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLUNIFORMMATRIX4X2DVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLUNIFORMMATRIX4X2DVPROC pfnUniformMatrix4x2dv = 0;
	LOAD_ENTRYPOINT("glUniformMatrix4x2dv", pfnUniformMatrix4x2dv, PFNGLUNIFORMMATRIX4X2DVPROC);
	pfnUniformMatrix4x2dv(location, count, transpose, value);
}

void glUniformMatrix4x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLUNIFORMMATRIX4X3DVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLUNIFORMMATRIX4X3DVPROC pfnUniformMatrix4x3dv = 0;
	LOAD_ENTRYPOINT("glUniformMatrix4x3dv", pfnUniformMatrix4x3dv, PFNGLUNIFORMMATRIX4X3DVPROC);
	pfnUniformMatrix4x3dv(location, count, transpose, value);
}

void glGetUniformdv(GLuint program, GLint location, GLdouble *params)
{
	typedef void (APIENTRY * PFNGLGETUNIFORMDVPROC)(GLuint program, GLint location, GLdouble *params);
	static PFNGLGETUNIFORMDVPROC pfnGetUniformdv = 0;
	LOAD_ENTRYPOINT("glGetUniformdv", pfnGetUniformdv, PFNGLGETUNIFORMDVPROC);
	pfnGetUniformdv(program, location, params);
}

void glProgramUniform1dEXT(GLuint program, GLint location, GLdouble x)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM1DEXTPROC)(GLuint program, GLint location, GLdouble x);
	static PFNGLPROGRAMUNIFORM1DEXTPROC pfnProgramUniform1dEXT = 0;
	LOAD_ENTRYPOINT("glProgramUniform1dEXT", pfnProgramUniform1dEXT, PFNGLPROGRAMUNIFORM1DEXTPROC);
	pfnProgramUniform1dEXT(program, location, x);
}

void glProgramUniform2dEXT(GLuint program, GLint location, GLdouble x, GLdouble y)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM2DEXTPROC)(GLuint program, GLint location, GLdouble x, GLdouble y);
	static PFNGLPROGRAMUNIFORM2DEXTPROC pfnProgramUniform2dEXT = 0;
	LOAD_ENTRYPOINT("glProgramUniform2dEXT", pfnProgramUniform2dEXT, PFNGLPROGRAMUNIFORM2DEXTPROC);
	pfnProgramUniform2dEXT(program, location, x, y);
}

void glProgramUniform3dEXT(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM3DEXTPROC)(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z);
	static PFNGLPROGRAMUNIFORM3DEXTPROC pfnProgramUniform3dEXT = 0;
	LOAD_ENTRYPOINT("glProgramUniform3dEXT", pfnProgramUniform3dEXT, PFNGLPROGRAMUNIFORM3DEXTPROC);
	pfnProgramUniform3dEXT(program, location, x, y, z);
}

void glProgramUniform4dEXT(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM4DEXTPROC)(GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	static PFNGLPROGRAMUNIFORM4DEXTPROC pfnProgramUniform4dEXT = 0;
	LOAD_ENTRYPOINT("glProgramUniform4dEXT", pfnProgramUniform4dEXT, PFNGLPROGRAMUNIFORM4DEXTPROC);
	pfnProgramUniform4dEXT(program, location, x, y, z, w);
}

void glProgramUniform1dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM1DVEXTPROC)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
	static PFNGLPROGRAMUNIFORM1DVEXTPROC pfnProgramUniform1dvEXT = 0;
	LOAD_ENTRYPOINT("glProgramUniform1dvEXT", pfnProgramUniform1dvEXT, PFNGLPROGRAMUNIFORM1DVEXTPROC);
	pfnProgramUniform1dvEXT(program, location, count, value);
}

void glProgramUniform2dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM2DVEXTPROC)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
	static PFNGLPROGRAMUNIFORM2DVEXTPROC pfnProgramUniform2dvEXT = 0;
	LOAD_ENTRYPOINT("glProgramUniform2dvEXT", pfnProgramUniform2dvEXT, PFNGLPROGRAMUNIFORM2DVEXTPROC);
	pfnProgramUniform2dvEXT(program, location, count, value);
}

void glProgramUniform3dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM3DVEXTPROC)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
	static PFNGLPROGRAMUNIFORM3DVEXTPROC pfnProgramUniform3dvEXT = 0;
	LOAD_ENTRYPOINT("glProgramUniform3dvEXT", pfnProgramUniform3dvEXT, PFNGLPROGRAMUNIFORM3DVEXTPROC);
	pfnProgramUniform3dvEXT(program, location, count, value);
}

void glProgramUniform4dvEXT(GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM4DVEXTPROC)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
	static PFNGLPROGRAMUNIFORM4DVEXTPROC pfnProgramUniform4dvEXT = 0;
	LOAD_ENTRYPOINT("glProgramUniform4dvEXT", pfnProgramUniform4dvEXT, PFNGLPROGRAMUNIFORM4DVEXTPROC);
	pfnProgramUniform4dvEXT(program, location, count, value);
}

void glProgramUniformMatrix2dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX2DVEXTPROC)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLPROGRAMUNIFORMMATRIX2DVEXTPROC pfnProgramUniformMatrix2dvEXT = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix2dvEXT", pfnProgramUniformMatrix2dvEXT, PFNGLPROGRAMUNIFORMMATRIX2DVEXTPROC);
	pfnProgramUniformMatrix2dvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix3dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX3DVEXTPROC)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLPROGRAMUNIFORMMATRIX3DVEXTPROC pfnProgramUniformMatrix3dvEXT = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix3dvEXT", pfnProgramUniformMatrix3dvEXT, PFNGLPROGRAMUNIFORMMATRIX3DVEXTPROC);
	pfnProgramUniformMatrix3dvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix4dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX4DVEXTPROC)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLPROGRAMUNIFORMMATRIX4DVEXTPROC pfnProgramUniformMatrix4dvEXT = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix4dvEXT", pfnProgramUniformMatrix4dvEXT, PFNGLPROGRAMUNIFORMMATRIX4DVEXTPROC);
	pfnProgramUniformMatrix4dvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix2x3dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX2X3DVEXTPROC)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLPROGRAMUNIFORMMATRIX2X3DVEXTPROC pfnProgramUniformMatrix2x3dvEXT = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix2x3dvEXT", pfnProgramUniformMatrix2x3dvEXT, PFNGLPROGRAMUNIFORMMATRIX2X3DVEXTPROC);
	pfnProgramUniformMatrix2x3dvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix2x4dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX2X4DVEXTPROC)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLPROGRAMUNIFORMMATRIX2X4DVEXTPROC pfnProgramUniformMatrix2x4dvEXT = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix2x4dvEXT", pfnProgramUniformMatrix2x4dvEXT, PFNGLPROGRAMUNIFORMMATRIX2X4DVEXTPROC);
	pfnProgramUniformMatrix2x4dvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix3x2dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX3X2DVEXTPROC)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLPROGRAMUNIFORMMATRIX3X2DVEXTPROC pfnProgramUniformMatrix3x2dvEXT = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix3x2dvEXT", pfnProgramUniformMatrix3x2dvEXT, PFNGLPROGRAMUNIFORMMATRIX3X2DVEXTPROC);
	pfnProgramUniformMatrix3x2dvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix3x4dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX3X4DVEXTPROC)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLPROGRAMUNIFORMMATRIX3X4DVEXTPROC pfnProgramUniformMatrix3x4dvEXT = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix3x4dvEXT", pfnProgramUniformMatrix3x4dvEXT, PFNGLPROGRAMUNIFORMMATRIX3X4DVEXTPROC);
	pfnProgramUniformMatrix3x4dvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix4x2dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX4X2DVEXTPROC)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLPROGRAMUNIFORMMATRIX4X2DVEXTPROC pfnProgramUniformMatrix4x2dvEXT = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix4x2dvEXT", pfnProgramUniformMatrix4x2dvEXT, PFNGLPROGRAMUNIFORMMATRIX4X2DVEXTPROC);
	pfnProgramUniformMatrix4x2dvEXT(program, location, count, transpose, value);
}

void glProgramUniformMatrix4x3dvEXT(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX4X3DVEXTPROC)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLPROGRAMUNIFORMMATRIX4X3DVEXTPROC pfnProgramUniformMatrix4x3dvEXT = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix4x3dvEXT", pfnProgramUniformMatrix4x3dvEXT, PFNGLPROGRAMUNIFORMMATRIX4X3DVEXTPROC);
	pfnProgramUniformMatrix4x3dvEXT(program, location, count, transpose, value);
}

GLint glGetSubroutineUniformLocation(GLuint program, GLenum shadertype, const GLchar *name)
{
	typedef GLint (APIENTRY * PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC)(GLuint program, GLenum shadertype, const GLchar *name);
	static PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC pfnGetSubroutineUniformLocation = 0;
	LOAD_ENTRYPOINT("glGetSubroutineUniformLocation", pfnGetSubroutineUniformLocation, PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC);
	return pfnGetSubroutineUniformLocation(program, shadertype, name);
}

GLuint glGetSubroutineIndex(GLuint program, GLenum shadertype, const GLchar *name)
{
	typedef GLuint (APIENTRY * PFNGLGETSUBROUTINEINDEXPROC)(GLuint program, GLenum shadertype, const GLchar *name);
	static PFNGLGETSUBROUTINEINDEXPROC pfnGetSubroutineIndex = 0;
	LOAD_ENTRYPOINT("glGetSubroutineIndex", pfnGetSubroutineIndex, PFNGLGETSUBROUTINEINDEXPROC);
	return pfnGetSubroutineIndex(program, shadertype, name);
}

void glGetActiveSubroutineUniformiv(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values)
{
	typedef void (APIENTRY * PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC)(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values);
	static PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC pfnGetActiveSubroutineUniformiv = 0;
	LOAD_ENTRYPOINT("glGetActiveSubroutineUniformiv", pfnGetActiveSubroutineUniformiv, PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC);
	pfnGetActiveSubroutineUniformiv(program, shadertype, index, pname, values);
}

void glGetActiveSubroutineUniformName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name)
{
	typedef void (APIENTRY * PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name);
	static PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC pfnGetActiveSubroutineUniformName = 0;
	LOAD_ENTRYPOINT("glGetActiveSubroutineUniformName", pfnGetActiveSubroutineUniformName, PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC);
	pfnGetActiveSubroutineUniformName(program, shadertype, index, bufsize, length, name);
}

void glGetActiveSubroutineName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name)
{
	typedef void (APIENTRY * PFNGLGETACTIVESUBROUTINENAMEPROC)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name);
	static PFNGLGETACTIVESUBROUTINENAMEPROC pfnGetActiveSubroutineName = 0;
	LOAD_ENTRYPOINT("glGetActiveSubroutineName", pfnGetActiveSubroutineName, PFNGLGETACTIVESUBROUTINENAMEPROC);
	pfnGetActiveSubroutineName(program, shadertype, index, bufsize, length, name);
}

void glUniformSubroutinesuiv(GLenum shadertype, GLsizei count, const GLuint *indices)
{
	typedef void (APIENTRY * PFNGLUNIFORMSUBROUTINESUIVPROC)(GLenum shadertype, GLsizei count, const GLuint *indices);
	static PFNGLUNIFORMSUBROUTINESUIVPROC pfnUniformSubroutinesuiv = 0;
	LOAD_ENTRYPOINT("glUniformSubroutinesuiv", pfnUniformSubroutinesuiv, PFNGLUNIFORMSUBROUTINESUIVPROC);
	pfnUniformSubroutinesuiv(shadertype, count, indices);
}

void glGetUniformSubroutineuiv(GLenum shadertype, GLint location, GLuint *params)
{
	typedef void (APIENTRY * PFNGLGETUNIFORMSUBROUTINEUIVPROC)(GLenum shadertype, GLint location, GLuint *params);
	static PFNGLGETUNIFORMSUBROUTINEUIVPROC pfnGetUniformSubroutineuiv = 0;
	LOAD_ENTRYPOINT("glGetUniformSubroutineuiv", pfnGetUniformSubroutineuiv, PFNGLGETUNIFORMSUBROUTINEUIVPROC);
	pfnGetUniformSubroutineuiv(shadertype, location, params);
}

void glGetProgramStageiv(GLuint program, GLenum shadertype, GLenum pname, GLint *values)
{
	typedef void (APIENTRY * PFNGLGETPROGRAMSTAGEIVPROC)(GLuint program, GLenum shadertype, GLenum pname, GLint *values);
	static PFNGLGETPROGRAMSTAGEIVPROC pfnGetProgramStageiv = 0;
	LOAD_ENTRYPOINT("glGetProgramStageiv", pfnGetProgramStageiv, PFNGLGETPROGRAMSTAGEIVPROC);
	pfnGetProgramStageiv(program, shadertype, pname, values);
}

void glPatchParameteri(GLenum pname, GLint value)
{
	typedef void (APIENTRY * PFNGLPATCHPARAMETERIPROC)(GLenum pname, GLint value);
	static PFNGLPATCHPARAMETERIPROC pfnPatchParameteri = 0;
	LOAD_ENTRYPOINT("glPatchParameteri", pfnPatchParameteri, PFNGLPATCHPARAMETERIPROC);
	pfnPatchParameteri(pname, value);
}

void glPatchParameterfv(GLenum pname, const GLfloat *values)
{
	typedef void (APIENTRY * PFNGLPATCHPARAMETERFVPROC)(GLenum pname, const GLfloat *values);
	static PFNGLPATCHPARAMETERFVPROC pfnPatchParameterfv = 0;
	LOAD_ENTRYPOINT("glPatchParameterfv", pfnPatchParameterfv, PFNGLPATCHPARAMETERFVPROC);
	pfnPatchParameterfv(pname, values);
}

void glBindTransformFeedback(GLenum target, GLuint id)
{
	typedef void (APIENTRY * PFNGLBINDTRANSFORMFEEDBACKPROC)(GLenum target, GLuint id);
	static PFNGLBINDTRANSFORMFEEDBACKPROC pfnBindTransformFeedback = 0;
	LOAD_ENTRYPOINT("glBindTransformFeedback", pfnBindTransformFeedback, PFNGLBINDTRANSFORMFEEDBACKPROC);
	pfnBindTransformFeedback(target, id);
}

void glDeleteTransformFeedbacks(GLsizei n, const GLuint *ids)
{
	typedef void (APIENTRY * PFNGLDELETETRANSFORMFEEDBACKSPROC)(GLsizei n, const GLuint *ids);
	static PFNGLDELETETRANSFORMFEEDBACKSPROC pfnDeleteTransformFeedbacks = 0;
	LOAD_ENTRYPOINT("glDeleteTransformFeedbacks", pfnDeleteTransformFeedbacks, PFNGLDELETETRANSFORMFEEDBACKSPROC);
	pfnDeleteTransformFeedbacks(n, ids);
}

void glGenTransformFeedbacks(GLsizei n, GLuint *ids)
{
	typedef void (APIENTRY * PFNGLGENTRANSFORMFEEDBACKSPROC)(GLsizei n, GLuint *ids);
	static PFNGLGENTRANSFORMFEEDBACKSPROC pfnGenTransformFeedbacks = 0;
	LOAD_ENTRYPOINT("glGenTransformFeedbacks", pfnGenTransformFeedbacks, PFNGLGENTRANSFORMFEEDBACKSPROC);
	pfnGenTransformFeedbacks(n, ids);
}

GLboolean glIsTransformFeedback(GLuint id)
{
	typedef GLboolean (APIENTRY * PFNGLISTRANSFORMFEEDBACKPROC)(GLuint id);
	static PFNGLISTRANSFORMFEEDBACKPROC pfnIsTransformFeedback = 0;
	LOAD_ENTRYPOINT("glIsTransformFeedback", pfnIsTransformFeedback, PFNGLISTRANSFORMFEEDBACKPROC);
	return pfnIsTransformFeedback(id);
}

void glPauseTransformFeedback()
{
	typedef void (APIENTRY * PFNGLPAUSETRANSFORMFEEDBACKPROC)();
	static PFNGLPAUSETRANSFORMFEEDBACKPROC pfnPauseTransformFeedback = 0;
	LOAD_ENTRYPOINT("glPauseTransformFeedback", pfnPauseTransformFeedback, PFNGLPAUSETRANSFORMFEEDBACKPROC);
	pfnPauseTransformFeedback();
}

void glResumeTransformFeedback()
{
	typedef void (APIENTRY * PFNGLRESUMETRANSFORMFEEDBACKPROC)();
	static PFNGLRESUMETRANSFORMFEEDBACKPROC pfnResumeTransformFeedback = 0;
	LOAD_ENTRYPOINT("glResumeTransformFeedback", pfnResumeTransformFeedback, PFNGLRESUMETRANSFORMFEEDBACKPROC);
	pfnResumeTransformFeedback();
}

void glDrawTransformFeedback(GLenum mode, GLuint id)
{
	typedef void (APIENTRY * PFNGLDRAWTRANSFORMFEEDBACKPROC)(GLenum mode, GLuint id);
	static PFNGLDRAWTRANSFORMFEEDBACKPROC pfnDrawTransformFeedback = 0;
	LOAD_ENTRYPOINT("glDrawTransformFeedback", pfnDrawTransformFeedback, PFNGLDRAWTRANSFORMFEEDBACKPROC);
	pfnDrawTransformFeedback(mode, id);
}

void glDrawTransformFeedbackStream(GLenum mode, GLuint id, GLuint stream)
{
	typedef void (APIENTRY * PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC)(GLenum mode, GLuint id, GLuint stream);
	static PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC pfnDrawTransformFeedbackStream = 0;
	LOAD_ENTRYPOINT("glDrawTransformFeedbackStream", pfnDrawTransformFeedbackStream, PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC);
	pfnDrawTransformFeedbackStream(mode, id, stream);
}

void glBeginQueryIndexed(GLenum target, GLuint index, GLuint id)
{
	typedef void (APIENTRY * PFNGLBEGINQUERYINDEXEDPROC)(GLenum target, GLuint index, GLuint id);
	static PFNGLBEGINQUERYINDEXEDPROC pfnBeginQueryIndexed = 0;
	LOAD_ENTRYPOINT("glBeginQueryIndexed", pfnBeginQueryIndexed, PFNGLBEGINQUERYINDEXEDPROC);
	pfnBeginQueryIndexed(target, index, id);
}

void glEndQueryIndexed(GLenum target, GLuint index)
{
	typedef void (APIENTRY * PFNGLENDQUERYINDEXEDPROC)(GLenum target, GLuint index);
	static PFNGLENDQUERYINDEXEDPROC pfnEndQueryIndexed = 0;
	LOAD_ENTRYPOINT("glEndQueryIndexed", pfnEndQueryIndexed, PFNGLENDQUERYINDEXEDPROC);
	pfnEndQueryIndexed(target, index);
}

void glGetQueryIndexediv(GLenum target, GLuint index, GLenum pname, GLint *params)
{
	typedef void (APIENTRY * PFNGLGETQUERYINDEXEDIVPROC)(GLenum target, GLuint index, GLenum pname, GLint *params);
	static PFNGLGETQUERYINDEXEDIVPROC pfnGetQueryIndexediv = 0;
	LOAD_ENTRYPOINT("glGetQueryIndexediv", pfnGetQueryIndexediv, PFNGLGETQUERYINDEXEDIVPROC);
	pfnGetQueryIndexediv(target, index, pname, params);
}

//
// OpenGL 4.1
//

void glReleaseShaderCompiler(void)
{
	typedef void (APIENTRY * PFNGLRELEASESHADERCOMPILER)(void);
	static PFNGLRELEASESHADERCOMPILER pfnReleaseShaderCompiler = 0;
	LOAD_ENTRYPOINT("glReleaseShaderCompiler", pfnReleaseShaderCompiler, PFNGLRELEASESHADERCOMPILER);
	pfnReleaseShaderCompiler();
}

void glShaderBinary(GLsizei count, const GLuint *shaders, GLenum binaryformat, const GLvoid *binary, GLsizei length)
{
	typedef void (APIENTRY * PFNGLSHADERBINARY)(GLsizei count, const GLuint *shaders, GLenum binaryformat, const GLvoid *binary, GLsizei length);
	static PFNGLSHADERBINARY pfnShaderBinary = 0;
	LOAD_ENTRYPOINT("glShaderBinary", pfnShaderBinary, PFNGLSHADERBINARY);
	pfnShaderBinary(count, shaders, binaryformat, binary, length);
}

void glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision)
{
	typedef void (APIENTRY * PFNGLGETSHADERPRECISIONFORMAT)(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
	static PFNGLGETSHADERPRECISIONFORMAT pfnGetShaderPrecisionFormat = 0;
	LOAD_ENTRYPOINT("glGetShaderPrecisionFormat", pfnGetShaderPrecisionFormat, PFNGLGETSHADERPRECISIONFORMAT);
	pfnGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
}

void glDepthRangef(GLclampf n, GLclampf f)
{
	typedef void (APIENTRY * PFNGLDEPTHRANGEF)(GLclampf n, GLclampf f);
	static PFNGLDEPTHRANGEF pfnDepthRangef = 0;
	LOAD_ENTRYPOINT("glDepthRangef", pfnDepthRangef, PFNGLDEPTHRANGEF);
	pfnDepthRangef(n, f);
}

void glClearDepthf(GLclampf d)
{
	typedef void (APIENTRY * PFNGLCLEARDEPTHF)(GLclampf d);
	static PFNGLCLEARDEPTHF pfnClearDepthf = 0;
	LOAD_ENTRYPOINT("glClearDepthf", pfnClearDepthf, PFNGLCLEARDEPTHF);
	pfnClearDepthf(d);
}

void glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary)
{
	typedef void (APIENTRY * PFNGLGETPROGRAMBINARY)(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary);
	static PFNGLGETPROGRAMBINARY pfnGetProgramBinary = 0;
	LOAD_ENTRYPOINT("glGetProgramBinary", pfnGetProgramBinary, PFNGLGETPROGRAMBINARY);
	pfnGetProgramBinary(program, bufSize, length, binaryFormat, binary);
}

void glProgramBinary(GLuint program, GLenum binaryFormat, const GLvoid *binary, GLsizei length)
{
	typedef void (APIENTRY * PFNGLPROGRAMBINARY)(GLuint program, GLenum binaryFormat, const GLvoid *binary, GLsizei length);
	static PFNGLPROGRAMBINARY pfnProgramBinary = 0;
	LOAD_ENTRYPOINT("glProgramBinary", pfnProgramBinary, PFNGLPROGRAMBINARY);
	pfnProgramBinary(program, binaryFormat, binary, length);
}

void glProgramParameteri(GLuint program, GLenum pname, GLint value)
{
	typedef void (APIENTRY * PFNGLPROGRAMPARAMETERI)(GLuint program, GLenum pname, GLint value);
	static PFNGLPROGRAMPARAMETERI pfnProgramParameteri = 0;
	LOAD_ENTRYPOINT("glProgramParameteri", pfnProgramParameteri, PFNGLPROGRAMPARAMETERI);
	pfnProgramParameteri(program, pname, value);
}

void glUseProgramStages(GLuint pipeline, GLbitfield stages, GLuint program)
{
	typedef void (APIENTRY * PFNGLUSEPROGRAMSTAGES)(GLuint pipeline, GLbitfield stages, GLuint program);
	static PFNGLUSEPROGRAMSTAGES pfnUseProgramStages = 0;
	LOAD_ENTRYPOINT("glUseProgramStages", pfnUseProgramStages, PFNGLUSEPROGRAMSTAGES);
	pfnUseProgramStages(pipeline, stages, program);
}

void glActiveShaderProgram(GLuint pipeline, GLuint program)
{
	typedef void (APIENTRY * PFNGLACTIVESHADERPROGRAM)(GLuint pipeline, GLuint program);
	static PFNGLACTIVESHADERPROGRAM pfnActiveShaderProgram = 0;
	LOAD_ENTRYPOINT("glActiveShaderProgram", pfnActiveShaderProgram, PFNGLACTIVESHADERPROGRAM);
	pfnActiveShaderProgram(pipeline, program);
}

GLuint glCreateShaderProgramv(GLenum type, GLsizei count, const GLchar* *strings)
{
	typedef GLuint (APIENTRY * PFNGLCREATESHADERPROGRAMV)(GLenum type, GLsizei count, const GLchar* *strings);
	static PFNGLCREATESHADERPROGRAMV pfnCreateShaderProgramv = 0;
	LOAD_ENTRYPOINT("glCreateShaderProgramv", pfnCreateShaderProgramv, PFNGLCREATESHADERPROGRAMV);
	return pfnCreateShaderProgramv(type, count, strings);
}

void glBindProgramPipeline(GLuint pipeline)
{
	typedef void (APIENTRY * PFNGLBINDPROGRAMPIPELINE)(GLuint pipeline);
	static PFNGLBINDPROGRAMPIPELINE pfnBindProgramPipeline = 0;
	LOAD_ENTRYPOINT("glBindProgramPipeline", pfnBindProgramPipeline, PFNGLBINDPROGRAMPIPELINE);
	pfnBindProgramPipeline(pipeline);
}

void glDeleteProgramPipelines(GLsizei n, const GLuint *pipelines)
{
	typedef void (APIENTRY * PFNGLDELETEPROGRAMPIPELINES)(GLsizei n, const GLuint *pipelines);
	static PFNGLDELETEPROGRAMPIPELINES pfnDeleteProgramPipelines = 0;
	LOAD_ENTRYPOINT("glDeleteProgramPipelines", pfnDeleteProgramPipelines, PFNGLDELETEPROGRAMPIPELINES);
	pfnDeleteProgramPipelines(n, pipelines);
}

void glGenProgramPipelines(GLsizei n, GLuint *pipelines)
{
	typedef void (APIENTRY * PFNGLGENPROGRAMPIPELINES)(GLsizei n, GLuint *pipelines);
	static PFNGLGENPROGRAMPIPELINES pfnGenProgramPipelines = 0;
	LOAD_ENTRYPOINT("glGenProgramPipelines", pfnGenProgramPipelines, PFNGLGENPROGRAMPIPELINES);
	pfnGenProgramPipelines(n, pipelines);
}

GLboolean glIsProgramPipeline(GLuint pipeline)
{
	typedef GLboolean (APIENTRY * PFNGLISPROGRAMPIPELINE)(GLuint pipeline);
	static PFNGLISPROGRAMPIPELINE pfnIsProgramPipeline = 0;
	LOAD_ENTRYPOINT("glIsProgramPipeline", pfnIsProgramPipeline, PFNGLISPROGRAMPIPELINE);
	return pfnIsProgramPipeline(pipeline);
}

void glGetProgramPipelineiv(GLuint pipeline, GLenum pname, GLint *params)
{
	typedef void (APIENTRY * PFNGLGETPROGRAMPIPELINEIV)(GLuint pipeline, GLenum pname, GLint *params);
	static PFNGLGETPROGRAMPIPELINEIV pfnGetProgramPipelineiv = 0;
	LOAD_ENTRYPOINT("glGetProgramPipelineiv", pfnGetProgramPipelineiv, PFNGLGETPROGRAMPIPELINEIV);
	pfnGetProgramPipelineiv(pipeline, pname, params);
}

void glProgramUniform1i(GLuint program, GLint location, GLint v0)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM1I)(GLuint program, GLint location, GLint v0);
	static PFNGLPROGRAMUNIFORM1I pfnProgramUniform1i = 0;
	LOAD_ENTRYPOINT("glProgramUniform1i", pfnProgramUniform1i, PFNGLPROGRAMUNIFORM1I);
	pfnProgramUniform1i(program, location, v0);
}

void glProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM1IV)(GLuint program, GLint location, GLsizei count, const GLint *value);
	static PFNGLPROGRAMUNIFORM1IV pfnProgramUniform1iv = 0;
	LOAD_ENTRYPOINT("glProgramUniform1iv", pfnProgramUniform1iv, PFNGLPROGRAMUNIFORM1IV);
	pfnProgramUniform1iv(program, location, count, value);
}

void glProgramUniform1f(GLuint program, GLint location, GLfloat v0)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM1F)(GLuint program, GLint location, GLfloat v0);
	static PFNGLPROGRAMUNIFORM1F pfnProgramUniform1f = 0;
	LOAD_ENTRYPOINT("glProgramUniform1f", pfnProgramUniform1f, PFNGLPROGRAMUNIFORM1F);
	pfnProgramUniform1f(program, location, v0);
}

void glProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM1FV)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
	static PFNGLPROGRAMUNIFORM1FV pfnProgramUniform1fv = 0;
	LOAD_ENTRYPOINT("glProgramUniform1fv", pfnProgramUniform1fv, PFNGLPROGRAMUNIFORM1FV);
	pfnProgramUniform1fv(program, location, count, value);
}

void glProgramUniform1d(GLuint program, GLint location, GLdouble v0)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM1D)(GLuint program, GLint location, GLdouble v0);
	static PFNGLPROGRAMUNIFORM1D pfnProgramUniform1d = 0;
	LOAD_ENTRYPOINT("glProgramUniform1d", pfnProgramUniform1d, PFNGLPROGRAMUNIFORM1D);
	pfnProgramUniform1d(program, location, v0);
}

void glProgramUniform1dv(GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM1DV)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
	static PFNGLPROGRAMUNIFORM1DV pfnProgramUniform1dv = 0;
	LOAD_ENTRYPOINT("glProgramUniform1dv", pfnProgramUniform1dv, PFNGLPROGRAMUNIFORM1DV);
	pfnProgramUniform1dv(program, location, count, value);
}

void glProgramUniform1ui(GLuint program, GLint location, GLuint v0)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM1UI)(GLuint program, GLint location, GLuint v0);
	static PFNGLPROGRAMUNIFORM1UI pfnProgramUniform1ui = 0;
	LOAD_ENTRYPOINT("glProgramUniform1ui", pfnProgramUniform1ui, PFNGLPROGRAMUNIFORM1UI);
	pfnProgramUniform1ui(program, location, v0);
}

void glProgramUniform1uiv(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM1UIV)(GLuint program, GLint location, GLsizei count, const GLuint *value);
	static PFNGLPROGRAMUNIFORM1UIV pfnProgramUniform1uiv = 0;
	LOAD_ENTRYPOINT("glProgramUniform1uiv", pfnProgramUniform1uiv, PFNGLPROGRAMUNIFORM1UIV);
	pfnProgramUniform1uiv(program, location, count, value);
}

void glProgramUniform2i(GLuint program, GLint location, GLint v0, GLint v1)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM2I)(GLuint program, GLint location, GLint v0, GLint v1);
	static PFNGLPROGRAMUNIFORM2I pfnProgramUniform2i = 0;
	LOAD_ENTRYPOINT("glProgramUniform2i", pfnProgramUniform2i, PFNGLPROGRAMUNIFORM2I);
	pfnProgramUniform2i(program, location, v0, v1);
}

void glProgramUniform2iv(GLuint program, GLint location, GLsizei count, const GLint *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM2IV)(GLuint program, GLint location, GLsizei count, const GLint *value);
	static PFNGLPROGRAMUNIFORM2IV pfnProgramUniform2iv = 0;
	LOAD_ENTRYPOINT("glProgramUniform2iv", pfnProgramUniform2iv, PFNGLPROGRAMUNIFORM2IV);
	pfnProgramUniform2iv(program, location, count, value);
}

void glProgramUniform2f(GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM2F)(GLuint program, GLint location, GLfloat v0, GLfloat v1);
	static PFNGLPROGRAMUNIFORM2F pfnProgramUniform2f = 0;
	LOAD_ENTRYPOINT("glProgramUniform2f", pfnProgramUniform2f, PFNGLPROGRAMUNIFORM2F);
	pfnProgramUniform2f(program, location, v0, v1);
}

void glProgramUniform2fv(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM2FV)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
	static PFNGLPROGRAMUNIFORM2FV pfnProgramUniform2fv = 0;
	LOAD_ENTRYPOINT("glProgramUniform2fv", pfnProgramUniform2fv, PFNGLPROGRAMUNIFORM2FV);
	pfnProgramUniform2fv(program, location, count, value);
}

void glProgramUniform2d(GLuint program, GLint location, GLdouble v0, GLdouble v1)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM2D)(GLuint program, GLint location, GLdouble v0, GLdouble v1);
	static PFNGLPROGRAMUNIFORM2D pfnProgramUniform2d = 0;
	LOAD_ENTRYPOINT("glProgramUniform2d", pfnProgramUniform2d, PFNGLPROGRAMUNIFORM2D);
	pfnProgramUniform2d(program, location, v0, v1);
}

void glProgramUniform2dv(GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM2DV)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
	static PFNGLPROGRAMUNIFORM2DV pfnProgramUniform2dv = 0;
	LOAD_ENTRYPOINT("glProgramUniform2dv", pfnProgramUniform2dv, PFNGLPROGRAMUNIFORM2DV);
	pfnProgramUniform2dv(program, location, count, value);
}

void glProgramUniform2ui(GLuint program, GLint location, GLuint v0, GLuint v1)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM2UI)(GLuint program, GLint location, GLuint v0, GLuint v1);
	static PFNGLPROGRAMUNIFORM2UI pfnProgramUniform2ui = 0;
	LOAD_ENTRYPOINT("glProgramUniform2ui", pfnProgramUniform2ui, PFNGLPROGRAMUNIFORM2UI);
	pfnProgramUniform2ui(program, location, v0, v1);
}

void glProgramUniform2uiv(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM2UIV)(GLuint program, GLint location, GLsizei count, const GLuint *value);
	static PFNGLPROGRAMUNIFORM2UIV pfnProgramUniform2uiv = 0;
	LOAD_ENTRYPOINT("glProgramUniform2uiv", pfnProgramUniform2uiv, PFNGLPROGRAMUNIFORM2UIV);
	pfnProgramUniform2uiv(program, location, count, value);
}

void glProgramUniform3i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM3I)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
	static PFNGLPROGRAMUNIFORM3I pfnProgramUniform3i = 0;
	LOAD_ENTRYPOINT("glProgramUniform3i", pfnProgramUniform3i, PFNGLPROGRAMUNIFORM3I);
	pfnProgramUniform3i(program, location, v0, v1, v2);
}

void glProgramUniform3iv(GLuint program, GLint location, GLsizei count, const GLint *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM3IV)(GLuint program, GLint location, GLsizei count, const GLint *value);
	static PFNGLPROGRAMUNIFORM3IV pfnProgramUniform3iv = 0;
	LOAD_ENTRYPOINT("glProgramUniform3iv", pfnProgramUniform3iv, PFNGLPROGRAMUNIFORM3IV);
	pfnProgramUniform3iv(program, location, count, value);
}

void glProgramUniform3f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM3F)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
	static PFNGLPROGRAMUNIFORM3F pfnProgramUniform3f = 0;
	LOAD_ENTRYPOINT("glProgramUniform3f", pfnProgramUniform3f, PFNGLPROGRAMUNIFORM3F);
	pfnProgramUniform3f(program, location, v0, v1, v2);
}

void glProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM3FV)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
	static PFNGLPROGRAMUNIFORM3FV pfnProgramUniform3fv = 0;
	LOAD_ENTRYPOINT("glProgramUniform3fv", pfnProgramUniform3fv, PFNGLPROGRAMUNIFORM3FV);
	pfnProgramUniform3fv(program, location, count, value);
}

void glProgramUniform3d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM3D)(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2);
	static PFNGLPROGRAMUNIFORM3D pfnProgramUniform3d = 0;
	LOAD_ENTRYPOINT("glProgramUniform3d", pfnProgramUniform3d, PFNGLPROGRAMUNIFORM3D);
	pfnProgramUniform3d(program, location, v0, v1, v2);
}

void glProgramUniform3dv(GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM3DV)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
	static PFNGLPROGRAMUNIFORM3DV pfnProgramUniform3dv = 0;
	LOAD_ENTRYPOINT("glProgramUniform3dv", pfnProgramUniform3dv, PFNGLPROGRAMUNIFORM3DV);
	pfnProgramUniform3dv(program, location, count, value);
}

void glProgramUniform3ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM3UI)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
	static PFNGLPROGRAMUNIFORM3UI pfnProgramUniform3ui = 0;
	LOAD_ENTRYPOINT("glProgramUniform3ui", pfnProgramUniform3ui, PFNGLPROGRAMUNIFORM3UI);
	pfnProgramUniform3ui(program, location, v0, v1, v2);
}

void glProgramUniform3uiv(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM3UIV)(GLuint program, GLint location, GLsizei count, const GLuint *value);
	static PFNGLPROGRAMUNIFORM3UIV pfnProgramUniform3uiv = 0;
	LOAD_ENTRYPOINT("glProgramUniform3uiv", pfnProgramUniform3uiv, PFNGLPROGRAMUNIFORM3UIV);
	pfnProgramUniform3uiv(program, location, count, value);
}

void glProgramUniform4i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM4I)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
	static PFNGLPROGRAMUNIFORM4I pfnProgramUniform4i = 0;
	LOAD_ENTRYPOINT("glProgramUniform4i", pfnProgramUniform4i, PFNGLPROGRAMUNIFORM4I);
	pfnProgramUniform4i(program, location, v0, v1, v2, v3);
}

void glProgramUniform4iv(GLuint program, GLint location, GLsizei count, const GLint *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM4IV)(GLuint program, GLint location, GLsizei count, const GLint *value);
	static PFNGLPROGRAMUNIFORM4IV pfnProgramUniform4iv = 0;
	LOAD_ENTRYPOINT("glProgramUniform4iv", pfnProgramUniform4iv, PFNGLPROGRAMUNIFORM4IV);
	pfnProgramUniform4iv(program, location, count, value);
}

void glProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM4F)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
	static PFNGLPROGRAMUNIFORM4F pfnProgramUniform4f = 0;
	LOAD_ENTRYPOINT("glProgramUniform4f", pfnProgramUniform4f, PFNGLPROGRAMUNIFORM4F);
	pfnProgramUniform4f(program, location, v0, v1, v2, v3);
}

void glProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM4FV)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
	static PFNGLPROGRAMUNIFORM4FV pfnProgramUniform4fv = 0;
	LOAD_ENTRYPOINT("glProgramUniform4fv", pfnProgramUniform4fv, PFNGLPROGRAMUNIFORM4FV);
	pfnProgramUniform4fv(program, location, count, value);
}

void glProgramUniform4d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM4D)(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
	static PFNGLPROGRAMUNIFORM4D pfnProgramUniform4d = 0;
	LOAD_ENTRYPOINT("glProgramUniform4d", pfnProgramUniform4d, PFNGLPROGRAMUNIFORM4D);
	pfnProgramUniform4d(program, location, v0, v1, v2, v3);
}

void glProgramUniform4dv(GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM4DV)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
	static PFNGLPROGRAMUNIFORM4DV pfnProgramUniform4dv = 0;
	LOAD_ENTRYPOINT("glProgramUniform4dv", pfnProgramUniform4dv, PFNGLPROGRAMUNIFORM4DV);
	pfnProgramUniform4dv(program, location, count, value);
}

void glProgramUniform4ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM4UI)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
	static PFNGLPROGRAMUNIFORM4UI pfnProgramUniform4ui = 0;
	LOAD_ENTRYPOINT("glProgramUniform4ui", pfnProgramUniform4ui, PFNGLPROGRAMUNIFORM4UI);
	pfnProgramUniform4ui(program, location, v0, v1, v2, v3);
}

void glProgramUniform4uiv(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORM4UIV)(GLuint program, GLint location, GLsizei count, const GLuint *value);
	static PFNGLPROGRAMUNIFORM4UIV pfnProgramUniform4uiv = 0;
	LOAD_ENTRYPOINT("glProgramUniform4uiv", pfnProgramUniform4uiv, PFNGLPROGRAMUNIFORM4UIV);
	pfnProgramUniform4uiv(program, location, count, value);
}

void glProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX2FV)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static PFNGLPROGRAMUNIFORMMATRIX2FV pfnProgramUniformMatrix2fv = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix2fv", pfnProgramUniformMatrix2fv, PFNGLPROGRAMUNIFORMMATRIX2FV);
	pfnProgramUniformMatrix2fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX3FV)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static PFNGLPROGRAMUNIFORMMATRIX3FV pfnProgramUniformMatrix3fv = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix3fv", pfnProgramUniformMatrix3fv, PFNGLPROGRAMUNIFORMMATRIX3FV);
	pfnProgramUniformMatrix3fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX4FV)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static PFNGLPROGRAMUNIFORMMATRIX4FV pfnProgramUniformMatrix4fv = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix4fv", pfnProgramUniformMatrix4fv, PFNGLPROGRAMUNIFORMMATRIX4FV);
	pfnProgramUniformMatrix4fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX2DV)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLPROGRAMUNIFORMMATRIX2DV pfnProgramUniformMatrix2dv = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix2dv", pfnProgramUniformMatrix2dv, PFNGLPROGRAMUNIFORMMATRIX2DV);
	pfnProgramUniformMatrix2dv(program, location, count, transpose, value);
}

void glProgramUniformMatrix3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX3DV)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLPROGRAMUNIFORMMATRIX3DV pfnProgramUniformMatrix3dv = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix3dv", pfnProgramUniformMatrix3dv, PFNGLPROGRAMUNIFORMMATRIX3DV);
	pfnProgramUniformMatrix3dv(program, location, count, transpose, value);
}

void glProgramUniformMatrix4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX4DV)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLPROGRAMUNIFORMMATRIX4DV pfnProgramUniformMatrix4dv = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix4dv", pfnProgramUniformMatrix4dv, PFNGLPROGRAMUNIFORMMATRIX4DV);
	pfnProgramUniformMatrix4dv(program, location, count, transpose, value);
}

void glProgramUniformMatrix2x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX2X3FV)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static PFNGLPROGRAMUNIFORMMATRIX2X3FV pfnProgramUniformMatrix2x3fv = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix2x3fv", pfnProgramUniformMatrix2x3fv, PFNGLPROGRAMUNIFORMMATRIX2X3FV);
	pfnProgramUniformMatrix2x3fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix3x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX3X2FV)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static PFNGLPROGRAMUNIFORMMATRIX3X2FV pfnProgramUniformMatrix3x2fv = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix3x2fv", pfnProgramUniformMatrix3x2fv, PFNGLPROGRAMUNIFORMMATRIX3X2FV);
	pfnProgramUniformMatrix3x2fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix2x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX2X4FV)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static PFNGLPROGRAMUNIFORMMATRIX2X4FV pfnProgramUniformMatrix2x4fv = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix2x4fv", pfnProgramUniformMatrix2x4fv, PFNGLPROGRAMUNIFORMMATRIX2X4FV);
	pfnProgramUniformMatrix2x4fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix4x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX4X2FV)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static PFNGLPROGRAMUNIFORMMATRIX4X2FV pfnProgramUniformMatrix4x2fv = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix4x2fv", pfnProgramUniformMatrix4x2fv, PFNGLPROGRAMUNIFORMMATRIX4X2FV);
	pfnProgramUniformMatrix4x2fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix3x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX3X4FV)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static PFNGLPROGRAMUNIFORMMATRIX3X4FV pfnProgramUniformMatrix3x4fv = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix3x4fv", pfnProgramUniformMatrix3x4fv, PFNGLPROGRAMUNIFORMMATRIX3X4FV);
	pfnProgramUniformMatrix3x4fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix4x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX4X3FV)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
	static PFNGLPROGRAMUNIFORMMATRIX4X3FV pfnProgramUniformMatrix4x3fv = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix4x3fv", pfnProgramUniformMatrix4x3fv, PFNGLPROGRAMUNIFORMMATRIX4X3FV);
	pfnProgramUniformMatrix4x3fv(program, location, count, transpose, value);
}

void glProgramUniformMatrix2x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX2X3DV)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLPROGRAMUNIFORMMATRIX2X3DV pfnProgramUniformMatrix2x3dv = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix2x3dv", pfnProgramUniformMatrix2x3dv, PFNGLPROGRAMUNIFORMMATRIX2X3DV);
	pfnProgramUniformMatrix2x3dv(program, location, count, transpose, value);
}

void glProgramUniformMatrix3x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX3X2DV)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLPROGRAMUNIFORMMATRIX3X2DV pfnProgramUniformMatrix3x2dv = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix3x2dv", pfnProgramUniformMatrix3x2dv, PFNGLPROGRAMUNIFORMMATRIX3X2DV);
	pfnProgramUniformMatrix3x2dv(program, location, count, transpose, value);
}

void glProgramUniformMatrix2x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX2X4DV)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLPROGRAMUNIFORMMATRIX2X4DV pfnProgramUniformMatrix2x4dv = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix2x4dv", pfnProgramUniformMatrix2x4dv, PFNGLPROGRAMUNIFORMMATRIX2X4DV);
	pfnProgramUniformMatrix2x4dv(program, location, count, transpose, value);
}

void glProgramUniformMatrix4x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX4X2DV)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLPROGRAMUNIFORMMATRIX4X2DV pfnProgramUniformMatrix4x2dv = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix4x2dv", pfnProgramUniformMatrix4x2dv, PFNGLPROGRAMUNIFORMMATRIX4X2DV);
	pfnProgramUniformMatrix4x2dv(program, location, count, transpose, value);
}

void glProgramUniformMatrix3x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX3X4DV)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLPROGRAMUNIFORMMATRIX3X4DV pfnProgramUniformMatrix3x4dv = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix3x4dv", pfnProgramUniformMatrix3x4dv, PFNGLPROGRAMUNIFORMMATRIX3X4DV);
	pfnProgramUniformMatrix3x4dv(program, location, count, transpose, value);
}

void glProgramUniformMatrix4x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
	typedef void (APIENTRY * PFNGLPROGRAMUNIFORMMATRIX4X3DV)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
	static PFNGLPROGRAMUNIFORMMATRIX4X3DV pfnProgramUniformMatrix4x3dv = 0;
	LOAD_ENTRYPOINT("glProgramUniformMatrix4x3dv", pfnProgramUniformMatrix4x3dv, PFNGLPROGRAMUNIFORMMATRIX4X3DV);
	pfnProgramUniformMatrix4x3dv(program, location, count, transpose, value);
}

void glValidateProgramPipeline(GLuint pipeline)
{
	typedef void (APIENTRY * PFNGLVALIDATEPROGRAMPIPELINE)(GLuint pipeline);
	static PFNGLVALIDATEPROGRAMPIPELINE pfnValidateProgramPipeline = 0;
	LOAD_ENTRYPOINT("glValidateProgramPipeline", pfnValidateProgramPipeline, PFNGLVALIDATEPROGRAMPIPELINE);
	pfnValidateProgramPipeline(pipeline);
}

void glGetProgramPipelineInfoLog(GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
	typedef void (APIENTRY * PFNGLGETPROGRAMPIPELINEINFOLOG)(GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
	static PFNGLGETPROGRAMPIPELINEINFOLOG pfnGetProgramPipelineInfoLog = 0;
	LOAD_ENTRYPOINT("glGetProgramPipelineInfoLog", pfnGetProgramPipelineInfoLog, PFNGLGETPROGRAMPIPELINEINFOLOG);
	pfnGetProgramPipelineInfoLog(pipeline, bufSize, length, infoLog);
}

void glVertexAttribL1d(GLuint index, GLdouble x)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBL1D)(GLuint index, GLdouble x);
	static PFNGLVERTEXATTRIBL1D pfnVertexAttribL1d = 0;
	LOAD_ENTRYPOINT("glVertexAttribL1d", pfnVertexAttribL1d, PFNGLVERTEXATTRIBL1D);
	pfnVertexAttribL1d(index, x);
}

void glVertexAttribL2d(GLuint index, GLdouble x, GLdouble y)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBL2D)(GLuint index, GLdouble x, GLdouble y);
	static PFNGLVERTEXATTRIBL2D pfnVertexAttribL2d = 0;
	LOAD_ENTRYPOINT("glVertexAttribL2d", pfnVertexAttribL2d, PFNGLVERTEXATTRIBL2D);
	pfnVertexAttribL2d(index, x, y);
}

void glVertexAttribL3d(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBL3D)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
	static PFNGLVERTEXATTRIBL3D pfnVertexAttribL3d = 0;
	LOAD_ENTRYPOINT("glVertexAttribL3d", pfnVertexAttribL3d, PFNGLVERTEXATTRIBL3D);
	pfnVertexAttribL3d(index, x, y, z);
}

void glVertexAttribL4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBL4D)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
	static PFNGLVERTEXATTRIBL4D pfnVertexAttribL4d = 0;
	LOAD_ENTRYPOINT("glVertexAttribL4d", pfnVertexAttribL4d, PFNGLVERTEXATTRIBL4D);
	pfnVertexAttribL4d(index, x, y, z, w);
}

void glVertexAttribL1dv(GLuint index, const GLdouble *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBL1DV)(GLuint index, const GLdouble *v);
	static PFNGLVERTEXATTRIBL1DV pfnVertexAttribL1dv = 0;
	LOAD_ENTRYPOINT("glVertexAttribL1dv", pfnVertexAttribL1dv, PFNGLVERTEXATTRIBL1DV);
	pfnVertexAttribL1dv(index, v);
}

void glVertexAttribL2dv(GLuint index, const GLdouble *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBL2DV)(GLuint index, const GLdouble *v);
	static PFNGLVERTEXATTRIBL2DV pfnVertexAttribL2dv = 0;
	LOAD_ENTRYPOINT("glVertexAttribL2dv", pfnVertexAttribL2dv, PFNGLVERTEXATTRIBL2DV);
	pfnVertexAttribL2dv(index, v);
}

void glVertexAttribL3dv(GLuint index, const GLdouble *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBL3DV)(GLuint index, const GLdouble *v);
	static PFNGLVERTEXATTRIBL3DV pfnVertexAttribL3dv = 0;
	LOAD_ENTRYPOINT("glVertexAttribL3dv", pfnVertexAttribL3dv, PFNGLVERTEXATTRIBL3DV);
	pfnVertexAttribL3dv(index, v);
}

void glVertexAttribL4dv(GLuint index, const GLdouble *v)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBL4DV)(GLuint index, const GLdouble *v);
	static PFNGLVERTEXATTRIBL4DV pfnVertexAttribL4dv = 0;
	LOAD_ENTRYPOINT("glVertexAttribL4dv", pfnVertexAttribL4dv, PFNGLVERTEXATTRIBL4DV);
	pfnVertexAttribL4dv(index, v);
}

void glVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
	typedef void (APIENTRY * PFNGLVERTEXATTRIBLPOINTER)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
	static PFNGLVERTEXATTRIBLPOINTER pfnVertexAttribLPointer = 0;
	LOAD_ENTRYPOINT("glVertexAttribLPointer", pfnVertexAttribLPointer, PFNGLVERTEXATTRIBLPOINTER);
	pfnVertexAttribLPointer(index, size, type, stride, pointer);
}

void glGetVertexAttribLdv(GLuint index, GLenum pname, GLdouble *params)
{
	typedef void (APIENTRY * PFNGLGETVERTEXATTRIBLDV)(GLuint index, GLenum pname, GLdouble *params);
	static PFNGLGETVERTEXATTRIBLDV pfnGetVertexAttribLdv = 0;
	LOAD_ENTRYPOINT("glGetVertexAttribLdv", pfnGetVertexAttribLdv, PFNGLGETVERTEXATTRIBLDV);
	pfnGetVertexAttribLdv(index, pname, params);
}

void glViewportArrayv(GLuint first, GLsizei count, const GLfloat *v)
{
	typedef void (APIENTRY * PFNGLVIEWPORTARRAYV)(GLuint first, GLsizei count, const GLfloat *v);
	static PFNGLVIEWPORTARRAYV pfnViewportArrayv = 0;
	LOAD_ENTRYPOINT("glViewportArrayv", pfnViewportArrayv, PFNGLVIEWPORTARRAYV);
	pfnViewportArrayv(first, count, v);
}

void glViewportIndexedf(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
	typedef void (APIENTRY * PFNGLVIEWPORTINDEXEDF)(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h);
	static PFNGLVIEWPORTINDEXEDF pfnViewportIndexedf = 0;
	LOAD_ENTRYPOINT("glViewportIndexedf", pfnViewportIndexedf, PFNGLVIEWPORTINDEXEDF);
	pfnViewportIndexedf(index, x, y, w, h);
}

void glViewportIndexedfv(GLuint index, const GLfloat *v)
{
	typedef void (APIENTRY * PFNGLVIEWPORTINDEXEDFV)(GLuint index, const GLfloat *v);
	static PFNGLVIEWPORTINDEXEDFV pfnViewportIndexedfv = 0;
	LOAD_ENTRYPOINT("glViewportIndexedfv", pfnViewportIndexedfv, PFNGLVIEWPORTINDEXEDFV);
	pfnViewportIndexedfv(index, v);
}

void glScissorArrayv(GLuint first, GLsizei count, const GLint *v)
{
	typedef void (APIENTRY * PFNGLSCISSORARRAYV)(GLuint first, GLsizei count, const GLint *v);
	static PFNGLSCISSORARRAYV pfnScissorArrayv = 0;
	LOAD_ENTRYPOINT("glScissorArrayv", pfnScissorArrayv, PFNGLSCISSORARRAYV);
	pfnScissorArrayv(first, count, v);
}

void glScissorIndexed(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height)
{
	typedef void (APIENTRY * PFNGLSCISSORINDEXED)(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height);
	static PFNGLSCISSORINDEXED pfnScissorIndexed = 0;
	LOAD_ENTRYPOINT("glScissorIndexed", pfnScissorIndexed, PFNGLSCISSORINDEXED);
	pfnScissorIndexed(index, left, bottom, width, height);
}

void glScissorIndexedv(GLuint index, const GLint *v)
{
	typedef void (APIENTRY * PFNGLSCISSORINDEXEDV)(GLuint index, const GLint *v);
	static PFNGLSCISSORINDEXEDV pfnScissorIndexedv = 0;
	LOAD_ENTRYPOINT("glScissorIndexedv", pfnScissorIndexedv, PFNGLSCISSORINDEXEDV);
	pfnScissorIndexedv(index, v);
}

void glDepthRangeArrayv(GLuint first, GLsizei count, const GLclampd *v)
{
	typedef void (APIENTRY * PFNGLDEPTHRANGEARRAYV)(GLuint first, GLsizei count, const GLclampd *v);
	static PFNGLDEPTHRANGEARRAYV pfnDepthRangeArrayv = 0;
	LOAD_ENTRYPOINT("glDepthRangeArrayv", pfnDepthRangeArrayv, PFNGLDEPTHRANGEARRAYV);
	pfnDepthRangeArrayv(first, count, v);
}

void glDepthRangeIndexed(GLuint index, GLclampd n, GLclampd f)
{
	typedef void (APIENTRY * PFNGLDEPTHRANGEINDEXED)(GLuint index, GLclampd n, GLclampd f);
	static PFNGLDEPTHRANGEINDEXED pfnDepthRangeIndexed = 0;
	LOAD_ENTRYPOINT("glDepthRangeIndexed", pfnDepthRangeIndexed, PFNGLDEPTHRANGEINDEXED);
	pfnDepthRangeIndexed(index, n, f);
}

void glGetFloati_v(GLenum target, GLuint index, GLfloat *data)
{
	typedef void (APIENTRY * PFNGLGETFLOATI_V)(GLenum target, GLuint index, GLfloat *data);
	static PFNGLGETFLOATI_V pfnGetFloati_v = 0;
	LOAD_ENTRYPOINT("glGetFloati_v", pfnGetFloati_v, PFNGLGETFLOATI_V);
	pfnGetFloati_v(target, index, data);
}

void glGetDoublei_v(GLenum target, GLuint index, GLdouble *data)
{
	typedef void (APIENTRY * PFNGLGETDOUBLEI_V)(GLenum target, GLuint index, GLdouble *data);
	static PFNGLGETDOUBLEI_V pfnGetDoublei_v = 0;
	LOAD_ENTRYPOINT("glGetDoublei_v", pfnGetDoublei_v, PFNGLGETDOUBLEI_V);
	pfnGetDoublei_v(target, index, data);
}

//
// OpenGL 4.2
//

void glDrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count, GLsizei primcount, GLuint baseinstance)
{
	typedef void (APIENTRY * PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC)(GLenum mode, GLint first, GLsizei count, GLsizei primcount, GLuint baseinstance);
	static PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC pfnDrawArraysInstancedBaseInstance = 0;
	LOAD_ENTRYPOINT("glDrawArraysInstancedBaseInstance", pfnDrawArraysInstancedBaseInstance, PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEPROC);
	pfnDrawArraysInstancedBaseInstance(mode, first, count, primcount, baseinstance);
}

void glDrawElementsInstancedBaseInstance(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount, GLuint baseinstance)
{
	typedef void (APIENTRY * PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount, GLuint baseinstance);
	static PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC pfnDrawElementsInstancedBaseInstance = 0;
	LOAD_ENTRYPOINT("glDrawElementsInstancedBaseInstance", pfnDrawElementsInstancedBaseInstance, PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEPROC);
	pfnDrawElementsInstancedBaseInstance(mode, count, type, indices, primcount, baseinstance);
}

void glDrawElementsInstancedBaseVertexBaseInstance(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount, GLint basevertex, GLuint baseinstance)
{
	typedef void (APIENTRY * PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount, GLint basevertex, GLuint baseinstance);
	static PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC pfnDrawElementsInstancedBaseVertexBaseInstance = 0;
	LOAD_ENTRYPOINT("glDrawElementsInstancedBaseVertexBaseInstance", pfnDrawElementsInstancedBaseVertexBaseInstance, PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEPROC);
	pfnDrawElementsInstancedBaseVertexBaseInstance(mode, count, type, indices, primcount, basevertex, baseinstance);
}

void glDrawTransformFeedbackInstanced(GLenum mode, GLuint id, GLsizei primcount)
{
	typedef void (APIENTRY * PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC)(GLenum mode, GLuint id, GLsizei primcount);
	static PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC pfnDrawTransformFeedbackInstanced = 0;
	LOAD_ENTRYPOINT("glDrawTransformFeedbackInstanced", pfnDrawTransformFeedbackInstanced, PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDPROC);
	pfnDrawTransformFeedbackInstanced(mode, id, primcount);
}

void glDrawTransformFeedbackStreamInstanced(GLenum mode, GLuint id, GLuint stream, GLsizei primcount)
{
	typedef void (APIENTRY * PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC)(GLenum mode, GLuint id, GLuint stream, GLsizei primcount);
	static PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC pfnDrawTransformFeedbackStreamInstanced = 0;
	LOAD_ENTRYPOINT("glDrawTransformFeedbackStreamInstanced", pfnDrawTransformFeedbackStreamInstanced, PFNGLDRAWTRANSFORMFEEDBACKSTREAMINSTANCEDPROC);
	pfnDrawTransformFeedbackStreamInstanced(mode, id, stream, primcount);
}

void glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params)
{
	typedef void (APIENTRY * PFNGLGETINTERNALFORMATIVPROC)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params);
	static PFNGLGETINTERNALFORMATIVPROC pfnGetInternalformativ = 0;
	LOAD_ENTRYPOINT("glGetInternalformativ", pfnGetInternalformativ, PFNGLGETINTERNALFORMATIVPROC);
	pfnGetInternalformativ(target, internalformat, pname, bufSize, params);
}

void glGetActiveAtomicCounterBufferiv(GLuint program, GLuint bufferIndex, GLenum pname, GLint *params)
{
	typedef void (APIENTRY * PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC)(GLuint program, GLuint bufferIndex, GLenum pname, GLint *params);
	static PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC pfnGetActiveAtomicCounterBufferiv = 0;
	LOAD_ENTRYPOINT("glGetActiveAtomicCounterBufferiv", pfnGetActiveAtomicCounterBufferiv, PFNGLGETACTIVEATOMICCOUNTERBUFFERIVPROC);
	pfnGetActiveAtomicCounterBufferiv(program, bufferIndex, pname, params);
}

void glBindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
	typedef void (APIENTRY * PFNGLBINDIMAGETEXTUREPROC)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
	static PFNGLBINDIMAGETEXTUREPROC pfnBindImageTexture = 0;
	LOAD_ENTRYPOINT("glBindImageTexture", pfnBindImageTexture, PFNGLBINDIMAGETEXTUREPROC);
	pfnBindImageTexture(unit, texture, level, layered, layer, access, format);
}

void glMemoryBarrier(GLbitfield barriers)
{
	typedef void (APIENTRY * PFNGLMEMORYBARRIERPROC)(GLbitfield barriers);
	static PFNGLMEMORYBARRIERPROC pfnMemoryBarrier = 0;
	LOAD_ENTRYPOINT("glMemoryBarrier", pfnMemoryBarrier, PFNGLMEMORYBARRIERPROC);
	pfnMemoryBarrier(barriers);
}

void glTexStorage1D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
	typedef void (APIENTRY * PFNGLTEXSTORAGE1DPROC)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
	static PFNGLTEXSTORAGE1DPROC pfnTexStorage1D = 0;
	LOAD_ENTRYPOINT("glTexStorage1D", pfnTexStorage1D, PFNGLTEXSTORAGE1DPROC);
	pfnTexStorage1D(target, levels, internalformat, width);
}

void glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
	typedef void (APIENTRY * PFNGLTEXSTORAGE2DPROC)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
	static PFNGLTEXSTORAGE2DPROC pfnTexStorage2D = 0;
	LOAD_ENTRYPOINT("glTexStorage2D", pfnTexStorage2D, PFNGLTEXSTORAGE2DPROC);
	pfnTexStorage2D(target, levels, internalformat, width, height);
}

void glTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
	typedef void (APIENTRY * PFNGLTEXSTORAGE3DPROC)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
	static PFNGLTEXSTORAGE3DPROC pfnTexStorage3D = 0;
	LOAD_ENTRYPOINT("glTexStorage3D", pfnTexStorage3D, PFNGLTEXSTORAGE3DPROC);
	pfnTexStorage3D(target, levels, internalformat, width, height, depth);
}