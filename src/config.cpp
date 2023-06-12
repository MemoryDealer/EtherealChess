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

#include "config.h"

extern Music g_music;

ConfigParser::ConfigParser(const char* file)
{
	if((m_fp = fopen(file, "r")) != NULL){
		m_loaded = true;
	}
	else{
		m_loaded = false;
	}

	m_file = (char*)calloc(strlen(file) + 1, sizeof(char));
	strcpy(m_file, file);
}

ConfigParser::~ConfigParser(void)
{
	if(m_fp != NULL){
		fclose(m_fp);
	}

	if(m_file)
		free(m_file);
}

int ConfigParser::parseValue(const char* valueName)
{
	char buf[1024] = {0};
	char* p = 0;
	int len = strlen(valueName);
	int n = 0;

	if(m_fp == NULL){
		if((m_fp = fopen(m_file, "r")) == NULL)
			return false;
	}

	fseek(m_fp, 0, SEEK_SET);

	for(;fscanf(m_fp, "%s", buf) != EOF;){
		if(strncmp(buf, valueName, len) == 0){
			p = strchr(buf, '=');
			++p;

			sscanf(p, "%d", &n);
			return n;
		}
	}

	fseek(m_fp, 0, SEEK_SET);
	return 0;
}

char* ConfigParser::parseStrValue(const char* valueName)
{
	char buf[1024] = {0};
	char* p = 0;
	int len = strlen(valueName);
	int n = 0;

	if(m_fp == NULL){
		if((m_fp = fopen(m_file, "r")) == NULL)
			return false;
	}

	fseek(m_fp, 0, SEEK_SET);

	for(;fscanf(m_fp, "%s", buf) != EOF;){
		if(strncmp(buf, valueName, len) == 0){
			p = strchr(buf, '=');
			++p;

			return p;
		}
	}

	fseek(m_fp, 0, SEEK_SET);
	return NULL;
}

bool ConfigParser::saveAll(void)
{
	Game& game = Game::inst();
	Graphics& graphics = Graphics::inst();

	if(m_loaded)
		fclose(m_fp);

	m_fp = fopen(m_file, "w+");
	if(m_fp != NULL){
		fprintf(m_fp, 
			"[Game]\n"
			"set=%d\n"
			"sky=%d\n"
			"planet=%d\n"
			"texture=%d\n"
			"time=%d\n"
			"music=%d\n"
			"ai_engine=%d\n"
			"ai_custom_engine=%d\n"
			"ai=%d\n"
			"\n[Graphics]\n"
			"AA=%d\n"
			"reflections=%d\n"
			"verticalsync=%d\n"
			"geometry=%d\n"
			"boardmode=%d\n"
			"debug=%d\n",

			game.getChessSet(),
			game.getSkybox(),
			game.getPlanet(),
			game.getTextureMode(),
			game.getTime(),
			g_music.isPlaying(),
			
			AI::inst().getEngine(),
			AI::inst().getCustomEngine(),
			game.getAILevel(),

			graphics.getAntialiasing(),
			graphics.useReflection(),
			graphics.getVerticalSync(),
			graphics.getGeometryQuality(),
			graphics.getBoardMode(),
			graphics.getDebugConsole()
			);
	}
	else{

		return false;
	}

	// re-open for reading
	fclose(m_fp);
	m_fp = fopen(m_file, "r");
	return true;
}