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

#include "ai.h"

AI::AI()
{
	strcpy(m_pos,"position startpos moves ");
	m_searchDepth = 10;
	m_customEngine = false;
}

AI::~AI()
{
	cleanup();
}

AI& AI::inst(void)
{
	static AI ai;
	return ai;
}

/* test if the AI process is still running for some reason, and terminate */
static void checkProcess(const char* name)
{
	PROCESSENTRY32 PE32		= {0};
	HANDLE hProc			= NULL,
		   hSnap			= NULL;

	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PE32.dwSize = sizeof(PROCESSENTRY32);

	Process32First(hSnap, &PE32);
	do{
		if(strcmp(PE32.szExeFile, name) == 0){
			hProc = OpenProcess(PROCESS_TERMINATE, 0, PE32.th32ProcessID);

			if(!TerminateProcess(hProc, 0)){
				char buf[1024];

				sprintf(buf, "Failed to terminate the previous AI engine process \"%s\", please open task manager and end the process to prevent memory errors.", name);
				MessageBox(g_hWnd, buf, "Error", MB_ICONERROR);
			}
		}
	} while(Process32Next(hSnap, &PE32));

	CloseHandle(hProc);
	CloseHandle(hSnap);
}

bool AI::init(int engine)
{
	bool engine_started = false;
	bool b64bit = isOS64Bit();
	bool jobSuccess = false;

	m_engine = engine;
	memset(m_engine_path, 0, sizeof(m_engine_path));

	// set up the security attributes
	m_sa.bInheritHandle = true;
	m_sa.lpSecurityDescriptor = NULL;
	m_sa.nLength = sizeof(SECURITY_ATTRIBUTES);

	// setup job to kill the child process when calling process terminates
	m_hJob = CreateJobObject(NULL, NULL);
	if(m_hJob != NULL){
		m_jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

		jobSuccess = SetInformationJobObject(m_hJob, JobObjectExtendedLimitInformation, &m_jeli, sizeof(m_jeli));
	}

	// open a pipe to the AI engine
	if(!CreatePipe(&m_child_stdin, &m_hWrite, &m_sa, 0)){
		throw std::runtime_error("CreatePipe - stdin");
		return false;
	}

	if(!CreatePipe(&m_hRead, &m_child_stdout, &m_sa, 0)){
		throw std::runtime_error("CreatePipe - stdout");
		cleanup();
		return false;
	}

	// prepare to launch the process
	GetStartupInfo(&m_si);
	m_si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	m_si.wShowWindow = SW_HIDE;
	m_si.hStdOutput = m_child_stdout;
	m_si.hStdError = m_child_stdout;
	m_si.hStdInput = m_child_stdin;

	for(;!engine_started;){
		switch(m_engine){
			case ENGINE_HOUDINI:
				if(b64bit){
					strcpy(m_engine_path, "Data/Engines/Houdini_x64.exe");
					checkProcess("Houdini_x64.exe");
				}
				else{
					strcpy(m_engine_path, "Data/Engines/Houdini_x86.exe");
					checkProcess("Houdini_x86.exe");
				}
				break;

			case ENGINE_STOCKFISH:
				if(b64bit){
					strcpy(m_engine_path, "Data/Engines/Stockfish_x64.exe");
					checkProcess("Stockfish_x64.exe");
				}
				else{
					strcpy(m_engine_path, "Data/Engines/Stockfish_x86.exe");
					checkProcess("Stockfish_x86.exe");
				}
				break;

			case ENGINE_CRITTER:
				if(b64bit){
					strcpy(m_engine_path, "Data/Engines/Critter_x64.exe");
					checkProcess("Critter_x64.exe");
				}
				else{
					strcpy(m_engine_path, "Data/Engines/Critter_x86.exe");
					checkProcess("Critter_x86.exe");
				}
				break;

			case ENGINE_CUSTOM:
				strcpy(m_engine_path, "Data/Engines/Custom.exe");
				break;

			default:
				throw std::runtime_error("No valid engine found");
				cleanup();
				return false;
		}

		// attempt to launch the process
		engine_started =
			CreateProcess(m_engine_path, 
						  NULL,
						  NULL,
						  NULL,
						  TRUE,
						  0,
						  NULL,
						  NULL,
						  &m_si,
						  &m_pi);

		if(!engine_started){
			if(engine > 0)
				engine = 0;
			else
				++engine;

			continue;
		}

		break;
	}

	// assign the job to the process
	if(jobSuccess){
		jobSuccess = AssignProcessToJobObject(m_hJob, m_pi.hProcess);
	}

	// change states
	//m_active = true;
	//m_thinking = true;
	
	// set process priority to below normal to prevent destruction of the CPU
	SetPriorityClass(m_pi.hProcess, BELOW_NORMAL_PRIORITY_CLASS);

	// set AI to active
	if(engine_started){
		m_active = true;

		// start the state machine thread
		(void)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&InitThread, this, 0, 0);
	}

	return true;
}

void AI::reset(void)
{
	char buf[1024];

	//sprintf(buf, "stop ");
	//WriteFile(m_hWrite, buf, sizeof(buf), &m_bread, NULL); // allowing this command on the first run caused a hang-up when trying again or quitting the program (only in release mode, weird)
	//WriteFile(m_hWrite, "\n", 1, &m_bread, NULL);

	Sleep(200);

	if(m_engine == ENGINE_HOUDINI){
		sprintf(buf, "ucinewgame ");
		WriteFile(m_hWrite, buf, sizeof(buf), &m_bread, NULL);
		WriteFile(m_hWrite, "\n", 1, &m_bread, NULL);
	}

	strcpy(m_pos,"position startpos moves ");
}

unsigned long WINAPI AI::InitThread(void* lpThread)
{
	if(lpThread){
		return ((AI*)lpThread)->_AI(NULL);
	}

	return 0;
}

// thread for state machine
DWORD WINAPI AI::_AI(LPVOID lpBuffer)
{
	Game& game = Game::inst();
	char buf[BUFSIZE] = {0};
	memset(buf, 0, sizeof(buf));
	int depth = 1;
	static unsigned int aiLevel = 99;

	Sleep(500);

	for(int i=0;m_active;++i){
		GetExitCodeProcess(m_pi.hProcess, &m_exit);
		if(m_exit != STILL_ACTIVE){
			m_active = false;
			printf("NOT ACTIVE...quitting\n");
			//break;
		}
		
		// check for output, and obtain the bread
		PeekNamedPipe(m_hRead, buf, BUFSIZE, &m_bread, &m_avail, NULL);
		if(m_bread > 0){
			for(;;){
				memset(buf, 0, sizeof(buf));

				ReadFile(m_hRead, buf, BUFSIZE, &m_bread, NULL);
				printf("%s", buf);
				if(!game.getTurn())
					parseAIMove(buf);

				Sleep(100);

				PeekNamedPipe(m_hRead, buf, BUFSIZE, &m_bread, &m_avail, NULL);
				if(m_bread > 0)
					continue;
				else
					break;
			}
		}

		if(m_engine == ENGINE_STOCKFISH){
			//sprintf(buf, (i == 0) ? "uci " : (i == 1) ? "isready " : (i == 2) ? "" : (i == 3) ? "" : "");
			sprintf(buf, (i == 0) ? "uci " : "");
		}
		else{
			sprintf(buf, (i == 0) ? "uci " : (i == 1) ? "isready " : (i == 2) ? "setoption name Hash value 512 " : (i == 3) ? "setoption name UCI_LimitStrength value true " : "");
		}
		if(m_sendMove == true){
			strcat(m_pos, m_lastUserMove);
			strcat(m_pos, " ");

			WriteFile(m_hWrite, m_pos, sizeof(m_pos), &m_bread, NULL);
			WriteFile(m_hWrite, "\n", 1, &m_bread, NULL);

			//sprintf(buf, "go wtime %ld btime %ld depth %d ", g_whiteTime, g_blackTime, m_searchDepth);
			while(game.isAnimating())
				Sleep(50);

			// calculate depth to search
			if(m_engine != ENGINE_STOCKFISH){
				if(aiLevel != game.getAILevel()){
					aiLevel = game.getAILevel();
					switch(aiLevel){
					case Game::CHILD:
						depth = 1;
						sprintf(buf, "setoption name UCI_Elo value 1000 "); // it's questionable whether or not this works
						break;

					case Game::WALRUS:
						depth = 3;
						sprintf(buf, "setoption name UCI_Elo value 1200 ");
						break;

					case Game::LION:
					default:
						depth = 5;
						sprintf(buf, "setoption name UCI_Elo value 1500 ");
						break;

					case Game::RAPTOR:
						depth = 10;
						sprintf(buf, "setoption name UCI_Elo value 1800 ");
						break;

					case Game::GRANDMASTER:
						depth = 15;
						sprintf(buf, "setoption name UCI_Elo value 2900 ");
						break;
					}

					WriteFile(m_hWrite, buf, sizeof(buf), &m_bread, NULL);
					WriteFile(m_hWrite, "\n", 1, &m_bread, NULL);
				}
			}

			//sprintf(buf, "go wtime %ld btime %ld ", game.getTime(WHITE), game.getTime(BLACK));
			sprintf(buf, "go wtime %ld btime %ld depth %d ", game.getTime(WHITE), game.getTime(BLACK), depth);
			WriteFile(m_hWrite, buf, sizeof(buf), &m_bread, NULL);
			WriteFile(m_hWrite, "\n", 1, &m_bread, NULL);

			//printf("NEW POS: [%s]\n", m_pos);

			m_sendMove = false;
			game.setTurn(BLACK);
		}
		else{
			WriteFile(m_hWrite, buf, sizeof(buf), &m_bread, NULL);
			WriteFile(m_hWrite, "\n", 1, &m_bread, NULL);

			if(i > 100000)
				i = 6;
		}

		Sleep(500);
		
		//if(game.getTurn() == WHITE)
			//;///printf("Waiting for user to move...\n");
	}

	cleanup();

	ExitThread(0);
}

void AI::parseAIMove(const char* str)
{
	const int BESTMOVE_OFFSET		= 9;
	const int BESTMOVE_END_OFFSET	= 4;
	char buf[BUFSIZE];
    char* token;
    char newmove[32];

    strcpy(buf, str);
    token = strtok(buf, "\n");
    while(token != NULL){
        if(strstr(token, "bestmove")){
			char* p = token + BESTMOVE_OFFSET;
            printf("FOUND: %s\n", p);
            
            *(p + BESTMOVE_END_OFFSET) = 0;

            strcat(m_pos, p);
            strcat(m_pos, " ");
            printf("NEW POS: [%s]\n", m_pos);

			strcpy(m_lastAIMove, p);
			moveAIPiece();

			Game::inst().setTurn(WHITE);
        }

        token = strtok(NULL, "\n");
    }
}

void AI::moveAIPiece(void)
{
	Game& game = Game::inst();

	switch(m_lastAIMove[0]){
		case 'a': game.setSelectionY(1); break;
		case 'b': game.setSelectionY(2); break;
		case 'c': game.setSelectionY(3); break;
		case 'd': game.setSelectionY(4); break;
		case 'e': game.setSelectionY(5); break;
		case 'f': game.setSelectionY(6); break;
		case 'g': game.setSelectionY(7); break;
		case 'h': game.setSelectionY(8); break;
		default: break;
	}
	
	// get i component
	game.setSelectionX(m_lastAIMove[1] - '0');

	// get new position j component
	switch(m_lastAIMove[2]){
		case 'a': game.setNewSelectionY(1); break;
		case 'b': game.setNewSelectionY(2); break;
		case 'c': game.setNewSelectionY(3); break;
		case 'd': game.setNewSelectionY(4); break;
		case 'e': game.setNewSelectionY(5); break;
		case 'f': game.setNewSelectionY(6); break;
		case 'g': game.setNewSelectionY(7); break;
		case 'h': game.setNewSelectionY(8); break;
		default: break;
	}

	// get new i component
	game.setNewSelectionX(m_lastAIMove[3] - '0');

	while(game.isAnimating())
		Sleep(100);

	game.movePiece();

	game.setSelectionX(game.getLastSelectionX());
	game.setNewSelectionX(game.getLastSelectionX());
	game.setSelectionY(game.getLastSelectionY());
	game.setNewSelectionY(game.getLastSelectionY());
}

void AI::moveAgainst(void)
{
	Game& game = Game::inst();
	// convert the selections to the move format
	
	m_lastUserMove[1] = (char)(((int)'0') + game.getSelectionX());
	
	switch(game.getSelectionY() - 1){
		case 0: m_lastUserMove[0] = 'a'; break;
		case 1: m_lastUserMove[0] = 'b'; break;
		case 2: m_lastUserMove[0] = 'c'; break;
		case 3: m_lastUserMove[0] = 'd'; break;
		case 4: m_lastUserMove[0] = 'e'; break;
		case 5: m_lastUserMove[0] = 'f'; break;
		case 6: m_lastUserMove[0] = 'g'; break;
		case 7: m_lastUserMove[0] = 'h'; break;
	}

	m_lastUserMove[3] = (char)(((int)'0') + game.getNewSelectionX());

	switch(game.getNewSelectionY() - 1){
		case 0: m_lastUserMove[2] = 'a'; break;
		case 1: m_lastUserMove[2] = 'b'; break;
		case 2: m_lastUserMove[2] = 'c'; break;
		case 3: m_lastUserMove[2] = 'd'; break;
		case 4: m_lastUserMove[2] = 'e'; break;
		case 5: m_lastUserMove[2] = 'f'; break;
		case 6: m_lastUserMove[2] = 'g'; break;
		case 7: m_lastUserMove[2] = 'h'; break;
	}
	
	m_sendMove = true;
}

void AI::cleanup(void)
{
	char buf[1024];

	// tell the AI engine to terminate
	if(m_engine != ENGINE_STOCKFISH){ // stockfish is a real piece of work
		sprintf(buf, "stop ");
		WriteFile(m_hWrite, buf, sizeof(buf), &m_bread, NULL);
		WriteFile(m_hWrite, "\n", 1, &m_bread, NULL);

		sprintf(buf, "quit ");
		WriteFile(m_hWrite, buf, sizeof(buf), &m_bread, NULL);
		WriteFile(m_hWrite, "\n", 1, &m_bread, NULL);
	}

	if(m_pi.hThread) CloseHandle(m_pi.hThread);
	if(m_pi.hProcess) CloseHandle(m_pi.hProcess);
	if(m_child_stdin)	CloseHandle(m_child_stdin);
	if(m_child_stdout)	CloseHandle(m_child_stdout);
	if(m_hRead)		CloseHandle(m_hRead);
	if(m_hWrite)	CloseHandle(m_hWrite);
}