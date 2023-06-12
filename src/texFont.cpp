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

#include "texFont.h"

TexFont::TexFont()
{

}

TexFont::~TexFont()
{
	glDeleteLists(m_list, 256);
}

bool TexFont::loadFonts(void)
{
	float cx, cy;

	m_list = glGenLists(256);
	glBindTexture(GL_TEXTURE_2D, g_texFonts[0]);
	for(int j=0; j<256; ++j){
		cx = float(j % 16) / 16.0f;
		cy = float(j / 16) / 16.0f;

		glNewList(m_list + j, GL_COMPILE);
			glBegin(GL_QUADS);
				glTexCoord2f(cx, 1 - cy - 0.0625f);
				glVertex2i(0, 0);
				glTexCoord2f(cx + 0.0625f, 1 - cy - 0.0625f);
				glVertex2i(16, 0);
				glTexCoord2f(cx + 0.0625f, 1 - cy);
				glVertex2i(16, 16);
				glTexCoord2f(cx, 1 - cy);
				glVertex2i(0, 16);
			glEnd();
			glTranslated(10, 0, 0);
		glEndList();
	}

	return true;
}

void TexFont::draw(unsigned x, unsigned y, const char* str, unsigned font)
{
	glBindTexture(GL_TEXTURE_2D, g_texFonts[0]);

	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 640, 0, 480, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslated(x, y, 0);
	glListBase(m_list - 32 + (128 * font));
	glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	glEnable(GL_DEPTH_TEST);
}