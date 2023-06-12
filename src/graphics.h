/*
 *  Ethereal Chess - OpenGL 3D Chess - <http://etherealchess.sourceforge.net/>
 *  Copyright (C) 2012 Jordan Sparks - unixunited@live.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Special thanks to http://www.dhpoware.com/ for providing some OpenGL code. 
 */

#pragma once

#include <Windows.h>
#include <stdexcept>
#include <vector>
#include <cassert>

// OpenGL headers
#include <GL/GL.h>
#include <GL/GLU.h>

// DevIL headers
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>


#include "cam.h"
#include "model.h"
#include "font.h"
#include "mathlib.h"
#include "arcane_lib.h"
#include "WGL_ARB_multisample.h"
#include "GL_ARB_multitexture.h"
#include "game.h"

#define GL_TEXTURE_MAX_ANISOTROPY_EXT		0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT	0x84FF
#define GL_CLAMP_TO_EDGE					0x812F

#define MENU_BOARDS							10
#define NUM_QUADS							5
#define PI									3.14159265f
#define MAX_SLICES							4096
#define MAX_STACKS							4096

class Graphics{
public:

	enum modes{
		PROJ = 0,
		ORTHO
	};

	enum qualities{
		MEDIOCRE = 0,
		LOW,
		MEDIUM,
		HIGH, 
		ULTRA
	};

	enum boardModes{
		STACKED = 0,
		BORDERED
	};

	Graphics();
	~Graphics();

	static Graphics& inst(void);

	int init(void);
	void allocDebugConsole(void);
	int loadResources(void);
	void loadDefaultModels(void);
	void reloadModels(void);
	void reloadSkyboxTextures(void);
	void reloadPieceTextures(void);
	void loadBoard(void);

	bool loadSettings(void);
	bool saveSettings(void);

	void setProcessorAffinity(void);
	bool initPixels(HDC& hDC, HGLRC& hRC);
	void enableVerticalSync(bool enableVerticalSync);
	void toggleFullScreen(HWND hwnd);
	void updateFrameRate(float elapsedTimeSec);

	Vec3 getMouseVec3(void);

	void renderRadialRing(void);

	// getter functions
	unsigned int getWidth(void);
	unsigned int getHeight(void);
	unsigned int getFPS(void);
	unsigned int getAnisotropy(void);
	unsigned int getGeometryQuality(void);
	bool useReflection(void);
	bool saveCPU(void);
	bool isFullscreen(void);
	bool getAntialiasing(void);
	bool getVerticalSync(void);
	bool cullFace(void);
	bool isLoadingResources(void);
	bool useShaders(void);
	bool getBoardMode(void);
	bool getDebugConsole(void);
	unsigned getMode(void);
	GLUquadricObj* getQuadric(unsigned n);
	GLuint getDefModel(unsigned n);

	// setter functions
	void setWidth(unsigned int width);
	void setHeight(unsigned int height);
	void setReflection(bool reflection);
	void setAntialiasing(bool value);
	void setMode(unsigned mode);
	void setCullFace(bool cull);
	void setGeometryQuality(unsigned int quality);
	void setDebugConsole(bool debug);
	void setBoardMode(bool mode);

// image loading
	bool loadTexture(GLuint& id, const char* file);

	// setter functions
	void clamp(bool b);

private:
	void initDevIL(void);
	void initFonts(void);
	static unsigned long WINAPI initResourceThread(void* lpThread);
	DWORD WINAPI _loadResources(LPVOID lpBuffer);
	void loadTextures(void);
	void loadRadialRing(void);

	bool m_verticalSync;
	bool m_antiAliasing;
	bool m_reflection;
	int  m_anisotropy; 
	bool m_fullscreen;
	bool m_cullFace;
	bool m_loadingResources;
	bool m_saveCPU;
	bool m_mode;
	bool m_shaders;
	bool m_debugConsole;
	bool m_boardMode;

	unsigned int m_geometryQuality;

	unsigned int m_width, m_height;
	unsigned int m_fps;

	GLUquadricObj* m_quad[NUM_QUADS];

	// default models
	GLuint m_defModels[6];

	// image loading
	int m_imageWidth, m_imageHeight;
	int m_size;
	int m_format;
	int m_bpp;
	char* m_data;
	bool m_clamp;
	bool m_loaded;
};

// getter functions
inline unsigned int Graphics::getWidth(void)
{
	return m_width;
}

inline unsigned int Graphics::getHeight(void)
{
	return m_height;
}

inline unsigned int Graphics::getFPS(void)
{
	return m_fps;
}

inline bool Graphics::useReflection(void)
{
	return m_reflection;
}

inline unsigned int Graphics::getAnisotropy(void)
{
	return m_anisotropy;
}

inline unsigned int Graphics::getGeometryQuality(void)
{
	return m_geometryQuality;
}

inline bool Graphics::saveCPU(void)
{
	return m_saveCPU;
}

inline bool Graphics::isFullscreen(void)
{
	return m_fullscreen;
}

inline bool Graphics::getAntialiasing(void)
{
	return m_antiAliasing;
}

inline bool Graphics::getVerticalSync(void)
{
	return m_verticalSync;
}

inline bool Graphics::cullFace(void)
{
	return m_cullFace;
}

inline GLUquadricObj* Graphics::getQuadric(unsigned n)
{
	return m_quad[n];
}

inline bool Graphics::getBoardMode(void)
{
	return m_boardMode;
}

inline GLuint Graphics::getDefModel(unsigned n)
{
	return m_defModels[n];
}

inline bool Graphics::isLoadingResources(void)
{
	return m_loadingResources;
}

inline bool Graphics::useShaders(void)
{
	return m_shaders;
}

inline unsigned Graphics::getMode(void)
{
	return m_mode;
}

inline bool Graphics::getDebugConsole(void)
{
	return m_debugConsole;
}

// setter functions
inline void Graphics::setWidth(unsigned int width)
{
	m_width = width;
}

inline void Graphics::setHeight(unsigned int height)
{
	m_height = height;
}

inline void Graphics::setReflection(bool reflection)
{
	m_reflection = reflection;
}

inline void Graphics::setAntialiasing(bool value)
{
	m_antiAliasing = value;
}

inline void Graphics::setCullFace(bool cull)
{
	m_cullFace = cull;
}

inline void Graphics::setGeometryQuality(unsigned int quality)
{
	m_geometryQuality = quality;
}

inline void Graphics::setDebugConsole(bool debug)
{
	m_debugConsole = debug;
}

inline void Graphics::setBoardMode(bool mode)
{
	m_boardMode = mode;
}

inline void Graphics::clamp(bool b)
{
	m_clamp = b;
}