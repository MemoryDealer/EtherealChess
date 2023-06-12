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
#include <TlHelp32.h>
#include <cstdio>
#include <stdexcept>

#include "game.h"
#include "graphics.h"

#define BUFSIZE 65535

// a state machine for the AI

class AI{
public:

	enum engines{
		ENGINE_HOUDINI = 0,
		ENGINE_CRITTER,
		ENGINE_CUSTOM,
		ENGINE_STOCKFISH
	};

	char customEngine[MAX_PATH];

	AI();
	~AI();

	static AI& inst(void);

	bool init(int engine);
	void reset(void);

	// moving functions
	void moveAgainst(void); // sends user's move

	// getter functions
	bool isActive(void);
	bool isThinking(void);
	char* getPos(void);
	int getEngine(void);
	char* getEnginePath(void);
	bool getCustomEngine(void);
	
	// setter functions
	void stop(void);
	void setThinking(bool think);
	void setELO(unsigned int elo);
	void setPos(const char* pos);
	void setCustomEngine(bool custom);
	void setEngine(int engine);

private:
	void cleanup(void);
	static unsigned long WINAPI InitThread(void* lpThread);
	DWORD WINAPI _AI(LPVOID lpBuffer);
	void parseAIMove(const char* str);
	void moveAIPiece(void);

	char m_pos[4096];			// holds all the moves for engine to interpret
	char m_lastAIMove[32];
	char m_lastUserMove[32];

	int m_engine;
	char m_engine_path[MAX_PATH];
	bool m_customEngine;

	// AI difficulty options
	unsigned int m_searchDepth;
	unsigned int m_elo;

	// states
	bool m_thinking;
	bool m_active;
	bool m_init;
	bool m_sendMove;
	
	unsigned long m_exit;
	unsigned long m_bread;
	unsigned long m_avail;

	// engine process variables
	STARTUPINFO m_si;
	PROCESS_INFORMATION m_pi;
	SECURITY_ATTRIBUTES m_sa;
	HANDLE m_child_stdin, m_child_stdout, m_hRead, m_hWrite;
	HANDLE m_hJob;
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION m_jeli;
};

inline bool AI::isActive(void)
{
	return m_active;
}

inline char* AI::getPos(void)
{
	return m_pos;
}

inline int AI::getEngine(void)
{
	return m_engine;
}

inline char* AI::getEnginePath(void)
{
	return m_engine_path;
}

inline bool AI::getCustomEngine(void)
{
	return m_customEngine;
}

inline void AI::setELO(unsigned int elo){
	char buf[256] = {0};

	sprintf(buf, "setoption name UCI_Elo value %d ", elo);
	WriteFile(m_hWrite, buf, sizeof(buf), &m_bread, NULL);
	WriteFile(m_hWrite, "\n", 1, &m_bread, NULL);

	m_elo = elo;
}

inline void AI::stop(void)
{
	m_active = false;
}

inline void AI::setPos(const char* pos)
{
	strcpy(m_pos, pos);
}

inline void AI::setCustomEngine(bool custom)
{
	m_customEngine = custom;
}

inline void AI::setEngine(int engine)
{
	m_engine = engine;
}