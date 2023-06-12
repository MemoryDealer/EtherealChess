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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "game.h"
#include "graphics.h"
#include "sound.h"
#include "ai.h"

#include <stdio.h>
#include <string.h>

#define CONFIG_FILE "config.ini"

class ConfigParser
{
public:
	ConfigParser(const char* file);
	~ConfigParser(void);

	int parseValue(const char* valueName);
	char* parseStrValue(const char* valueName);
	bool isLoaded(void) const;
	bool saveAll(void);

private:
	FILE* m_fp;
	bool m_loaded;
	char* m_file;
};

inline bool ConfigParser::isLoaded(void) const
{ return m_loaded; }

#endif