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

#include "sound.h"

Music::Music()
{
	strcpy(m_file, "Data/Sound/ChessMusic.mp3");
}

Music::~Music()
{

}

void Music::play(void)
{
	char buf[1024] = {0};

	sprintf(buf, "open %s type mpegvideo alias music", m_file);

	mciSendString(buf, NULL, 0, 0);
	mciSendString("play music repeat", NULL, 0, 0);
	
	m_playing = true;
}

void Music::stop(void)
{
	mciSendString("close music", NULL, 0, 0);

	m_playing = false;
}