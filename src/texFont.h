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
#include <GL/GL.h>

#include "graphics.h"

extern GLuint g_texFonts[];

class TexFont{
	public:
		TexFont();
		~TexFont();

		bool loadFonts(void);
		void draw(unsigned x, unsigned y, const char* str, unsigned font);

		// getter functions
		GLuint getList(void);

	private:
		GLuint m_list;
};

inline GLuint TexFont::getList(void)
{
	return m_list;
}