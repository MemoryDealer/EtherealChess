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

#include "game.h"

#define SAVE_WILDCARD	"*.ecf"

extern Cam			g_cam;
extern Particle*	g_boardParticles;
extern Music		g_music;

const float Game::DEFAULT_ANIMATION_SPEED = 0.045f;
const float Game::DEFAULT_KNIGHT_ANIMATION_SPEED = 0.020f;
const DWORD Game::DEFAULT_TIME = 600000;

Game::Game()
{
	m_chessSet		= SET_FANCY;
	m_skybox		= SKYBOX_SPACE;
	m_planet		= PLANET_SATURN;
	m_textureMode	= METALLIC;
	m_time			= m_whiteTime = m_blackTime = DEFAULT_TIME;
	m_aiLevel		= LION;
	m_gameState		= STATE_LOADING;

	memset(m_checkDirection, 0, sizeof(m_checkDirection));
	
	m_streamOffset	= Graphics::inst().getHeight() - 20;
	m_gameplayMode  = GAMEPLAY_NORMAL;
	m_animation		= true;
	m_drawSelection = false;
	m_playerColor	= WHITE;
	m_turn			= WHITE;
	m_selectionX	= m_newSelectionX = m_selectionY = m_newSelectionY = BOARD_MIN;
	m_freeMove		= false;
	m_whiteCastle	= m_blackCastle = true;
	m_allowSelectionChange = false;
	m_saved			= true;

	memset(m_captureState, 0, sizeof(m_captureState));
	
	resetBoard();
}

Game::~Game() // some of these must be loaded from a config file
{
	DeleteFile("Saves\\temp.ecf");
	delete[] g_boardParticles;
}

Game& Game::inst(void)
{
	static Game game;
	return game;
}

void Game::init(HWND& hwnd)
{
	extern unsigned g_numBoards;

	// seed the random number generator
	srand(GetTickCount());

	// set the game timer
	SetTimer(hwnd, IDT_GAME_TIMER, 1000, (TIMERPROC)NULL);

	// allocate particles
	g_boardParticles = new Particle[g_numBoards];
}

void Game::resetBoard(void)
{
	const int board_rep[10][10] = { 
	{INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID},
	{INVALID, WHITE_ROOK, WHITE_KNIGHT, WHITE_BISHOP, WHITE_QUEEN, WHITE_KING, WHITE_BISHOP, WHITE_KNIGHT, WHITE_ROOK, INVALID},
	{INVALID, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, WHITE_PAWN, INVALID},
	{INVALID, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, INVALID},
	{INVALID, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, INVALID},
	{INVALID, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, INVALID},
	{INVALID, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, INVALID},
	{INVALID, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, BLACK_PAWN, INVALID},
	{INVALID, BLACK_ROOK, BLACK_KNIGHT, BLACK_BISHOP, BLACK_QUEEN, BLACK_KING, BLACK_BISHOP, BLACK_KNIGHT, BLACK_ROOK, INVALID},
	{INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID},
	};

	memcpy(m_board, board_rep, sizeof(board_rep));

	m_whiteKingX = 1;
	m_blackKingX = 8;
	m_whiteKingY = m_blackKingY = 5;
}

static int fileExists(const char* str)
{
	DWORD attrib = 0;

	attrib = GetFileAttributes(str);
	if(attrib == INVALID_FILE_ATTRIBUTES)
		return 0;

	if(attrib & FILE_ATTRIBUTE_DIRECTORY)
		return 2;

	return 1;
}

void Game::newGame(void)
{
	extern Cam g_cam;
	static bool firstRun = true;

	if(!m_saved){
		int x = MessageBox(g_hWnd, "Save before starting a new game?", "Ethereal Chess", MB_YESNOCANCEL);

		if(x == IDYES){
			saveFile();
		}
		else if(x == IDCANCEL){
			return;
		}

		m_saved = true;
	}

	// game objects
	m_gameState = STATE_ACTIVE;
	m_whiteTime = m_blackTime = m_time;
	m_turn = WHITE;
	m_whiteCastle = m_blackCastle = true;
	memset(&m_captureState, 0, sizeof(m_captureState));
	resetBoard();

	m_whiteKingInCheck = m_blackKingInCheck = false;

	// AI
	if(!firstRun){
		AI::inst().reset();
	}
	else{
		firstRun = false;
	}
	
	// graphics/camera
	g_cam.resetView();
}

bool Game::saveFile(void)
{
	char buf[1024]		= {0};
	char file[MAX_PATH] = SAVE_WILDCARD;
	OPENFILENAME ofn	= {0};
	int i;

	GetModuleFileName(GetModuleHandle(NULL), buf, sizeof(buf));
	for(i=strlen(buf); buf[i] != '\\'; --i){
		buf[i] = 0;
	}
	buf[i] = 0;

	SetCurrentDirectory(buf);
	(void)CreateDirectory("Saves", NULL);
	//strcat(buf, "Saves");
	//SetCurrentDirectory(buf);

	// get number of saves
	/*for(i=0; i<1000; ++i){
		char path[MAX_PATH];

		sprintf(file, "save%d.ecf", i + 1);

		sprintf(path, "Saves\\%s", file);

		if(fileExists(path) == 0){
			break;
		}
	}*/

	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner	= g_hWnd;
	ofn.lpstrFilter = "Ethereal Chess Files (*.ecf)\0*.ecf\0\0";
	ofn.lpstrFile	= file;
	ofn.lpstrInitialDir = "Saves";
	ofn.lpstrDefExt = SAVE_WILDCARD;
	ofn.lpstrTitle	= "Choose where to save your game...";
	ofn.nMaxFile	= MAX_PATH;
	ofn.Flags		= OFN_ENABLESIZING | OFN_EXPLORER | OFN_OVERWRITEPROMPT;

	if(GetSaveFileName(&ofn)){
		writeSave(file);

		return true;
	}

	return false;
}

bool Game::quickSave(void)
{
	(void)CreateDirectory("Saves", NULL);
	return writeSave("Saves\\quick-save.ecf");
}

bool Game::checkmateSave(void)
{
	(void)CreateDirectory("Saves", NULL);
	bool ret = writeSave("Saves\\temp.ecf"); // a cheap hack to prevent false checkmates

	if(ret){
		ret = load("Saves\\temp.ecf");

		if(isKingInCheckmate((m_playerColor == WHITE) ? m_whiteKingX : m_blackKingX,
			(m_playerColor == WHITE) ? m_whiteKingY : m_blackKingY, m_playerColor)){
			m_gameState = (m_playerColor == WHITE) ? STATE_WHITE_CHECKMATE : STATE_BLACK_CHECKMATE;
			if(m_playerColor == WHITE)
				m_whiteKingInCheck = true;
			else
				m_whiteKingInCheck = true;
		}
		else{
			m_gameState = STATE_ACTIVE;
			m_whiteKingInCheck = m_blackKingInCheck = false;
		}
	}

	return ret;
}

bool Game::writeSave(const char* file)
{
	FILE* fp = 0;
	struct save_t save = {0};

	// fill the file data
	memset(&save, 0, sizeof(save));
	strcpy(save.magic, MAGIC_STR);
	memcpy(save.m_board, m_board, sizeof(m_board));
	memcpy(save.m_captureState, m_captureState, sizeof(m_captureState));
	save.m_lastMoveFromX = m_lastMoveFromX;
	save.m_lastMoveFromY = m_lastMoveFromY;
	save.m_lastMoveToX = m_lastMoveToX;
	save.m_lastMoveToY = m_lastMoveToY;
	save.m_whiteKingX = m_whiteKingX;
	save.m_whiteKingY = m_whiteKingY;
	save.m_blackKingX = m_blackKingX;
	save.m_blackKingY = m_blackKingY;
	save.m_whiteTime = m_whiteTime;
	save.m_blackTime = m_blackTime;
	save.m_turn = m_turn;
	save.m_playerColor = m_playerColor;
	save.m_whiteCastle = m_whiteCastle;
	save.m_blackCastle = m_blackCastle;
	save.m_whiteKingInCheck = m_whiteKingInCheck;
	save.m_blackKingInCheck = m_blackKingInCheck;
	strcpy(save.m_pos, AI::inst().getPos());

	// create the file
	fp = fopen(file, "wb");
	if(fp != NULL){
		fwrite(&save, sizeof(save), 1, fp);
		fclose(fp);
		m_saved = true;
		return true;
	}

	return false;
}

bool Game::loadFile()
{
	OPENFILENAME ofn	= {0};
	char file[MAX_PATH] = {0};
	char buf[1024]		= {0};
	int i;

	GetModuleFileName(GetModuleHandle(NULL), buf, sizeof(buf));
	for(i=strlen(buf); buf[i] != '\\'; --i){
		buf[i] = 0;
	}
	buf[i] = 0;

	SetCurrentDirectory(buf);
	(void)CreateDirectory("Saves", NULL);

	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner	= g_hWnd;
	ofn.lpstrFilter = "Ethereal Chess Files (*.ecf)\0*.ecf\0\0";
	ofn.lpstrFile	= file;
	ofn.lpstrInitialDir = "Saves";
	ofn.lpstrDefExt = SAVE_WILDCARD;
	ofn.lpstrTitle	= "Choose a saved game to load...";
	ofn.nMaxFile	= MAX_PATH;
	ofn.Flags		= OFN_ENABLESIZING | OFN_EXPLORER | OFN_OVERWRITEPROMPT;

	if(GetOpenFileName(&ofn)){
		load(file);
	}

	g_cam.resetView();

	if(isKingInCheckmate((m_playerColor == WHITE) ? m_whiteKingX : m_blackKingX,
		(m_playerColor == WHITE) ? m_whiteKingY : m_blackKingY, m_playerColor)){
		m_gameState = (m_playerColor == WHITE) ? STATE_WHITE_CHECKMATE : STATE_BLACK_CHECKMATE;
		if(m_playerColor == WHITE)
			m_whiteKingInCheck = true;
		else
			m_whiteKingInCheck = true;
	}
	else{
		m_gameState = STATE_ACTIVE;
		m_whiteKingInCheck = m_blackKingInCheck = false;
	}

	return false;
}

bool Game::quickLoad(void)
{
	bool ret =  load("Saves\\quick-save.ecf");

	/*if(testCheckmate(m_playerColor)){
		m_gameState = (m_playerColor) ? STATE_WHITE_CHECKMATE : STATE_BLACK_CHECKMATE;
	}*/

	if(isKingInCheckmate((m_playerColor == WHITE) ? m_whiteKingX : m_blackKingX,
		(m_playerColor == WHITE) ? m_whiteKingY : m_blackKingY, m_playerColor)){
		m_gameState = (m_playerColor == WHITE) ? STATE_WHITE_CHECKMATE : STATE_BLACK_CHECKMATE;
		if(m_playerColor == WHITE)
			m_whiteKingInCheck = true;
		else
			m_whiteKingInCheck = true;
	}
	else{
		m_gameState = STATE_ACTIVE;
		m_whiteKingInCheck = m_blackKingInCheck = false;
	}

	return ret;
}

bool Game::checkmateLoad(void)
{
	bool ret = load("Saves\\temp.ecf");

	if(testCheckmate(m_playerColor)){
		m_gameState = (m_playerColor) ? STATE_WHITE_CHECKMATE : STATE_BLACK_CHECKMATE;
	}

	return ret;
}

bool Game::load(const char* file)
{
	extern Cam g_cam;
	FILE* fp = 0;
	char buf[MAX_PATH];
	struct save_t save = {0};

	fp = fopen(file, "rb");
	if(fp == NULL){
		return false;
	}

	// read the binary data
	memset(&save, 0, sizeof(save));
	fread(&save, sizeof(save), 1, fp);
	fclose(fp);

	// check the file header
	if(strncmp(save.magic, MAGIC_STR, MAGIC_STR_SIZE) != 0){
		return false;
	}

	// fill the game data
	memcpy(m_board, save.m_board, sizeof(m_board));
	memcpy(m_captureState, save.m_captureState, sizeof(m_captureState));
	m_lastMoveFromX = save.m_lastMoveFromX;
	m_lastMoveFromY = save.m_lastMoveFromY;
	m_lastMoveToX = save.m_lastMoveToX;
	m_lastMoveToY = save.m_lastMoveToY;
	m_whiteKingX = save.m_whiteKingX;
	m_whiteKingY = save.m_whiteKingY;
	m_blackKingX = save.m_blackKingX;
	m_blackKingY = save.m_blackKingY;
	m_whiteTime = save.m_whiteTime;
	m_blackTime = save.m_blackTime;
	m_turn = save.m_turn;
	m_playerColor = save.m_playerColor;
	m_whiteCastle = save.m_whiteCastle;
	m_blackCastle = save.m_blackCastle;
	m_whiteKingInCheck = save.m_whiteKingInCheck;
	m_blackKingInCheck = save.m_blackKingInCheck;

	// AI
	AI::inst().reset();
	AI::inst().setPos(save.m_pos);

	// set game state
	m_gameState = STATE_ACTIVE;
	m_saved = true;

	return true;
}

bool Game::exit(void)
{
	if(!m_saved){
		int x = MessageBox(g_hWnd, "Save game before quitting?", "Ethereal Chess", MB_YESNOCANCEL);
		if(x == IDYES){
			saveFile();
			PostQuitMessage(0);
			return true;
		}
		else if(x == IDNO){
			PostQuitMessage(0);
			return true;
		}
		else{
			return false;
		}
	}
	else{
		PostQuitMessage(0);
	}

	return true;
}

bool Game::movePiece(void)
{
	// NOTE: My X's and Y's are reversed :)
	/*
		Board representation:
		x = 8 [ ][ ][ ][ ][ ][ ][ ][ ] <-- BLACK SIDE
		x = 7 [ ][ ][ ][ ][ ][ ][ ][ ] <-- BLACK SIDE
		x = 6 [ ][ ][ ][ ][ ][ ][ ][ ]
		x = 5 [ ][ ][ ][ ][ ][ ][ ][ ]
		x = 4 [ ][ ][ ][ ][ ][ ][ ][ ]
		x = 3 [ ][ ][ ][ ][ ][ ][ ][ ]
		x = 2 [ ][ ][ ][ ][ ][ ][ ][ ] <-- WHITE SIDE
		x = 1 [ ][ ][ ][ ][ ][ ][ ][ ] <-- WHITE SIDE
		   y = 1  2  3  4  5  6  7  8
	*/

	int piece = m_board[m_selectionX][m_selectionY];
	int oldPiece = m_board[m_newSelectionX][m_newSelectionY];
	bool color = getColor();
	bool newColor = getNewColor();

	// check for no movement before proceeding
	if(m_selectionX == m_newSelectionX && m_selectionY == m_newSelectionY){
		return false;
	}

	if(color != m_turn){ // prevent player from moving AI pieces
		return false;
	}

	// we will now check for a valid move
	if(m_freeMove == false && m_turn == m_playerColor){
		// determine the type of piece
		switch(abs(piece)){
			case EMPTY:
			default:
				return false;

			case PAWN:
				// check for double first move
				if((m_newSelectionX - m_selectionX) > 1){
					if(m_selectionX != 2 || ((m_newSelectionX - m_selectionX) > 2))
						return false;
					if(m_board[3][m_newSelectionY] != EMPTY)
						return false;
					if(m_board[4][m_newSelectionY] != EMPTY)
						return false;
				}
				// make sure it's one move otherwise
				else if((m_newSelectionX - m_selectionX) != 1){
					return false;
				}

				//// check for backwards movement
				//if((m_newSelectionX - m_selectionX) < 0){
				//	return false;
				//}

				// check for diagonal movements
				if(m_newSelectionY != m_selectionY){
					// is there an enemy piece at the diagonal?
					if(m_board[m_newSelectionX][m_newSelectionY] != EMPTY){
						if(newColor != color){
							break;
						}
					}
					else{
						return false; // not valid to move diagonally to a blank space
					}
				}

				// check for a piece blocking the front space
				if(m_board[m_newSelectionX][m_newSelectionY]){
					return false;
				}

				break; // valid move

			case ROOK:
				// prevent diagonal movements
				if((m_selectionX != m_newSelectionX) && (m_selectionY != m_newSelectionY)){
					return false;
				}

				// check range for x-axis
				if(m_selectionX != m_newSelectionX){
					if(!isXRangeClear()){
						return false;
					}
				}

				// check range for y-axis
				if(m_selectionY != m_newSelectionY){
					if(!isYRangeClear()){
						return false;
					}
				}

				// check for friendly capture
				if(m_board[m_newSelectionX][m_newSelectionY] != EMPTY){
					if(newColor == color){
						return false;
					}
				}
				break;

			case KNIGHT:
				// make sure the 'L' shape is formed
				{
					int dx = abs(static_cast<int>(m_newSelectionX - m_selectionX));
					int dy = abs(static_cast<int>(m_newSelectionY - m_selectionY));

					if((dx == 2 && dy == 1) ||
					   (dx == 1 && dy == 2)){
						   if(m_board[m_newSelectionX][m_newSelectionY] != EMPTY){
							   if(newColor == color)
								   return false;
						   }
					}
					else{
						return false;
					}
				}
				break;

			case BISHOP:
				// make sure the movement is diagonal
				if(abs(static_cast<int>(m_newSelectionX - m_selectionX)) !=
				   abs(static_cast<int>(m_newSelectionY - m_selectionY))){
						return false;
				}

				// check for friendly capture
				if(m_board[m_newSelectionX][m_newSelectionY] != EMPTY){
					if(newColor == color){
						return false;
					}
				}

				// check the diagonal range
				if(!isDiagonalRangeClear()){
					return false;
				}
				break;

			case QUEEN:
				// check for friendly capture
				if(m_board[m_newSelectionX][m_newSelectionY] != EMPTY){
					if(newColor == color){
						return false;
					}
				}

				// diagonal movement?
				if(abs(static_cast<int>(m_newSelectionX - m_selectionX)) ==
				   abs(static_cast<int>(m_newSelectionY - m_selectionY))){
					   if(!isDiagonalRangeClear()){
						   return false;
					   }
				}
				// a horizontal movement
				else{
					// prevent cheating
					if((m_selectionX != m_newSelectionX) && 
						(m_selectionY != m_newSelectionY)){
 							return false;
					}

					// check horizontal range
					if(m_selectionX != m_newSelectionX){
						if(!isXRangeClear()){
							return false;
						}
					}
					else{
						if(!isYRangeClear()){
							return false;
						}
					}
				}
				break;

			case KING:
				// stop movements greater than one
				if((abs(static_cast<int>(m_newSelectionX - m_selectionX)) > 1) ||
				   (abs(static_cast<int>(m_newSelectionY - m_selectionY)) > 1)){
					   // check for castle
					   if(color == WHITE && m_whiteCastle == true){
						   if(m_selectionX == 1 && m_selectionY == 5){
							   if(m_newSelectionY == 3 || m_newSelectionY == 7){
								   break;
							   }
						   }
					   }
					   else if(m_blackCastle == true){
						   if(m_selectionX == 8 && m_selectionY == 5){
							   if(m_newSelectionY == 3 || m_newSelectionY == 7){
								   break;
							   }
						   }
					   }

					   return false;
				}

				// check for friendly capture
				if(m_board[m_newSelectionX][m_newSelectionY] != EMPTY){
					if(newColor == color){
						return false;
					}
				}
				
				// temporarily remove king's own piece to prevent errors
				//if(color == WHITE){
				//	m_board[m_whiteKingX][m_whiteKingY] = EMPTY;
				//}
				//else{
				//	m_board[m_blackKingX][m_blackKingY] = EMPTY;
				//}

				//// see if king will be in check
				//if(isKingInCheck(m_newSelectionX, m_newSelectionY, color)){
				//	// print check!

				//	// reset king's position
				//	if(color == WHITE){
				//		m_whiteKingInCheck = true;
				//		m_board[m_whiteKingX][m_whiteKingY] = WHITE_KING;
				//	}
				//	else{
				//		m_blackKingInCheck = true;
				//		m_board[m_blackKingX][m_blackKingY] = BLACK_KING;
				//	}
				//	return false;
				//}

				//// set the new king position
				//if(color == WHITE){
				//	m_whiteKingX = m_newSelectionX;
				//	m_whiteKingY = m_newSelectionY;
				//}
				//else{
				//	m_blackKingX = m_newSelectionX;
				//	m_blackKingY = m_newSelectionY;
				//}

				break;

		} // switch
	}

	int shit = m_board[m_newSelectionX][m_newSelectionY];

	// update capture data
	if(shit != EMPTY){
		if(shit < 0){
			switch(shit){
				case BLACK_PAWN:
					shit = BLACK_PAWN_;
					break;

				case BLACK_ROOK:
					shit = BLACK_ROOK_;
					break;

				case BLACK_KNIGHT:
					shit = BLACK_KNIGHT_;
					break;

				case BLACK_BISHOP:
					shit = BLACK_BISHOP_;
					break;

				case BLACK_QUEEN:
					shit = BLACK_QUEEN_;
					break;

				default:
					break;
			}
		}

		addCapture(shit); 
	}

	// set the new board data
	m_board[m_selectionX][m_selectionY] = EMPTY;
	m_board[m_newSelectionX][m_newSelectionY] = piece;

	if(abs(piece) == KING){
		if(piece > 0){
			m_whiteKingX = m_newSelectionX;
			m_whiteKingY = m_newSelectionY;
		}
		else{
			m_blackKingX = m_newSelectionX;
			m_blackKingY = m_newSelectionY;
		}
	}

	// test for check of player's own king now
	if(color == WHITE){
		//m_board[m_whiteKingX][m_whiteKingY] = EMPTY;

		if(isKingInCheck(m_whiteKingX, m_whiteKingY, WHITE)){

			//m_board[m_whiteKingX][m_whiteKingY] = WHITE_KING;
			if(m_playerColor == WHITE){
				// reset piece
				m_board[m_selectionX][m_selectionY] = piece;
				m_board[m_newSelectionX][m_newSelectionY] = oldPiece;
				if(piece == WHITE_KING){
					m_whiteKingX = m_selectionX;
					m_whiteKingY = m_selectionY;
				}

				return false;
			}
		}
		else{
			m_whiteKingInCheck = false;

			// check if black is in check
			if(isKingInCheck(m_blackKingX, m_blackKingY, BLACK)){
				// checkmate?
				if(isKingInCheckmate(m_blackKingX, m_blackKingY, BLACK)){
					m_gameState = STATE_BLACK_CHECKMATE;
					checkmateSave();
					//checkmateLoad();
				}
				m_blackKingInCheck = true;
			}
			else{
				m_blackKingInCheck = false;
			}
		}
	}
	else{
		//m_board[m_blackKingX][m_blackKingY] = EMPTY;

		if(isKingInCheck(m_blackKingX, m_blackKingY, BLACK)){

			//m_board[m_blackKingX][m_blackKingY] = BLACK_KING;
			if(m_playerColor == BLACK){ // allow AI to move
				// reset piece
				m_board[m_selectionX][m_selectionY] = piece;
				m_board[m_newSelectionX][m_newSelectionY] = oldPiece;
				if(piece == BLACK_KING){
					m_blackKingX = m_selectionX;
					m_blackKingY = m_selectionY;
				}

				return false;
			}
		}
		else{
			m_blackKingInCheck = false;

			// check if white is in check
			if(isKingInCheck(m_whiteKingX, m_whiteKingY, WHITE)){
				// checkmate?
				if(isKingInCheckmate(m_whiteKingX, m_whiteKingY, WHITE)){
					m_gameState = STATE_WHITE_CHECKMATE;
					checkmateSave();
					//checkmateLoad();
					//Game::inst().setDrawSelection(false);
				}
				m_whiteKingInCheck = true;
			}
			else{
				m_whiteKingInCheck = false;
			}
		}
	}

	// check for castling here 
	if((color == WHITE) ? m_whiteCastle == true : m_blackCastle == true){
		if(abs(piece) == KING){
			if(abs(static_cast<int>(m_newSelectionY - m_selectionY)) == 2){
				if(m_selectionY == 5){ // check for proper x value for each color (1 or 8)
					// short castle
					if(m_newSelectionY > m_selectionY){
						if(color == WHITE){
							if(m_board[1][8] == WHITE_ROOK){
								if(m_board[1][6] == EMPTY && m_board[1][7] == WHITE_KING){
									//if(!isKingInCheck(m_selectionX
									m_board[1][6] = WHITE_ROOK;
									m_board[1][8] = EMPTY;
									m_whiteKingX = 1;
									m_whiteKingY = 7;
									m_whiteCastle = false; // disable future castling
								}
								else
									return false;
							}
						}
						else{
							if(m_board[8][8] == BLACK_ROOK){
								if(m_board[8][6] == EMPTY && m_board[8][7] == BLACK_KING){
									m_board[8][6] = BLACK_ROOK;
									m_board[8][8] = EMPTY;
									m_blackKingX = 8;
									m_blackKingY = 7;
									m_blackCastle = false;
								}
								else
									return false;
							}
						}
					}
					else{
						if(color == WHITE){
							if(m_board[1][1] == WHITE_ROOK){
								if(m_board[1][2] == EMPTY && m_board[1][3] == WHITE_KING &&
									m_board[1][4] == EMPTY){
										m_board[1][4] = WHITE_ROOK;
										m_board[1][1] = EMPTY;
										m_whiteKingX = 1;
										m_whiteKingY = 3;
										m_whiteCastle = false;
								}
								else
									return false;
							}
						}
						else{
							if(m_board[8][1] == BLACK_ROOK){
								if(m_board[8][2] == EMPTY && m_board[8][3] == BLACK_KING &&
									m_board[8][4] == EMPTY){
										m_board[8][4] = BLACK_ROOK;
										m_board[8][1] = EMPTY;
										m_blackKingX = 8;
										m_blackKingY = 3;
										m_blackCastle = false;
								}
								else
									return false;
							}
						}
					}
				}
			}
		}
	}

	// check for check
	//if(isKingInCheck(m_whiteKingX, m_whiteKingY, WHITE)){
	//	m_whiteKingInCheck = true;
	//}
	//else{
	//	m_whiteKingInCheck = false;
	//}
	//if(isKingInCheck(m_blackKingX, m_blackKingY, BLACK)){
	//	m_blackKingInCheck = true;
	//}
	//else{
	//	m_blackKingInCheck = false;
	//}

	//// white
	//if(color == WHITE && m_whiteKingInCheck){
	//	// checkmate
	//	if(isKingInCheckmate(m_whiteKingX, m_whiteKingY, WHITE)){
	//		m_gameState = STATE_WHITE_CHECKMATE;
	//	}

	//	if(color == m_playerColor){
	//		return false;
	//	}
	//}

	//// black
	//if(color == BLACK && m_blackKingInCheck){
	//	// checkmate
	//	if(isKingInCheckmate(m_blackKingX, m_blackKingY, BLACK)){
	//		m_gameState = STATE_BLACK_CHECKMATE;
	//	}

	//	if(color == m_playerColor){
	//		return false;
	//	}
	//}

	// ---------------------------- //

	// move is valid, proceed
	if(m_animation){
		m_animateFromX	= m_selectionX;
		m_animateFromY	= m_selectionY;
		m_animateToX	= m_newSelectionX;
		m_animateToY	= m_newSelectionY;

		m_animating = true;
	}

	// set the new board data
	m_board[m_selectionX][m_selectionY] = EMPTY;
	m_board[m_newSelectionX][m_newSelectionY] = piece;

	// test for pawn promotion
	if(color == WHITE){
		if(abs(piece) == PAWN && m_newSelectionX == 8){
			m_board[m_newSelectionX][m_newSelectionY] = WHITE_QUEEN;
		}
	}
	else{
		if(abs(piece) == PAWN && m_newSelectionX == 1){
			m_board[m_newSelectionX][m_newSelectionY] = BLACK_QUEEN;
		}
	}

	// set last move
	m_lastMoveFromX = m_selectionX;
	m_lastMoveFromY = m_selectionY;
	m_lastMoveToX	= m_newSelectionX;
	m_lastMoveToY	= m_newSelectionY;

	// start the AI's turn
	if(m_gameplayMode != GAMEPLAY_FREEMOVE){

		if(m_turn == WHITE){
			AI::inst().moveAgainst();

			m_lastSelectionX = m_newSelectionX;
			m_lastSelectionY = m_newSelectionY;

			m_drawSelection = false;
		}
		else{
			m_drawSelection = true;
		}

		m_turn = !m_turn;
	}

	m_saved = false;

	return true;
}

// checks for empty spaces between X selection and new X selection
bool Game::isXRangeClear(void)
{
	int x1 = m_selectionX;
	int x2 = m_newSelectionX;

	if(x2 < x1){
		swap(x1, x2);
	}

	for(int i=x1+1; i<x2; ++i){
		if(m_board[i][m_selectionY] != EMPTY){

			return false;
		}
	}

	return true;
}

// checks for empty spaces between Y selection and new Y selection
bool Game::isYRangeClear(void)
{
	int y1 = m_selectionY;
	int y2 = m_newSelectionY;

	if(y2 < y1){
		swap(y1, y2);
	}

	for(int i=y1+1; i<y2; ++i){
		if(m_board[m_selectionX][i] != EMPTY){
			//printf("Piece [%d] at i=%d\n", m_board[i][m_selectionY], i);
			return false;
		}
	}

	return true;
}

// checks for empty spaces in diagonal range
bool Game::isDiagonalRangeClear(void)
{
	int x=m_selectionX, y=m_selectionY;

	for(;;){
		x += (x < m_newSelectionX) ? 1 : -1;
		y += (y < m_newSelectionY) ? 1 : -1;

		if(x == m_newSelectionX || y == m_newSelectionY){
			return true; // whole range cleared
		}

		if(m_board[x][y] != EMPTY){
			return false;
		}
	}

	return true;
}

/* checks if a friendly piece can move onto the space */
void Game::checkSpaceBlockable(int x, int y, bool color)
{
	int dx = 0;
	int dy = 0;
	int kingX = (color == WHITE) ? m_whiteKingX : m_blackKingX;
	int kingY = (color == WHITE) ? m_whiteKingY : m_blackKingY;
	int piece = 0;

	// check above for friendly units
	// NOTE: There are obvious flaws in the code below, at this point I'd just have to start from scratch to get it right.
	dx = x + 1;
	dy = y;
	if((piece = getPieceAt(dx, dy)) != INVALID){ // make sure there is space above
		if(piece != EMPTY && getColorAt(dx, dy) == color){
			if(abs(piece) == ROOK ||
				abs(piece) == QUEEN){
					m_checkmateBlockable = true;
					return;
			}
		}

		if(piece == EMPTY){
			// check full range above
			for(;; ++dx){
				if((piece = getPieceAt(dx, dy)) != EMPTY){
					if(piece == INVALID){
						break;
					}

					if(getColorAt(dx, dy) == color){
						if(abs(piece) == ROOK ||
							abs(piece) == QUEEN){
								m_checkmateBlockable = true;
								return;
						}
					}
					else
						break;
				}
			}
		}
	}

	// upper-left
	dx = x + 1;
	dy = y - 1;
	if((piece = getPieceAt(dx, dy)) != INVALID){
		if(piece != EMPTY && getColorAt(dx, dy) == color){
			if(abs(piece) == BISHOP ||
				abs(piece) == QUEEN){
					m_checkmateBlockable = true;
					return;
			}
		}

		if(piece == EMPTY){
			for(;; ++dx, --dy){
				if((piece = getPieceAt(dx, dy)) != EMPTY){
					if(piece == INVALID){
						break;
					}
					if(getColorAt(dx, dy) == color){
						if(abs(piece) == BISHOP ||
							abs(piece) == QUEEN){
								m_checkmateBlockable = true;
								return;
						}
					}
					else
						break;
				}
			}
		}
	}

	// left
	dx = x;
	dy = y - 1;
	if((piece = getPieceAt(dx, dy)) != INVALID){
		if(piece != EMPTY && getColorAt(dx, dy) == color){
			if(abs(piece) == ROOK ||
				abs(piece) == QUEEN){
					m_checkmateBlockable = true;
					return;
			}
		}

		if(piece == EMPTY){
			for(;; --dy){
				if((piece = getPieceAt(dx, dy)) != EMPTY){
					if(piece == INVALID){
						break;
					}
					if(getColorAt(dx, dy) == color){
						if(abs(piece) == ROOK ||
							abs(piece) == QUEEN){
								m_checkmateBlockable = true;
								return;
						}
					}
					else
						break;
				}
			}
		}
	}

	// lower-left
	dx = x - 1;
	dy = y - 1;
	if((piece = getPieceAt(dx, dy)) != INVALID){
		if(piece != EMPTY && getColorAt(dx, dy) == color){
			if(/*abs(piece) == PAWN ||*/
				abs(piece) == BISHOP ||
				abs(piece) == QUEEN){
					m_checkmateBlockable = true;
					return;
			}

		}

		if(piece == EMPTY){
			for(;; --dx, --dy){
				if((piece = getPieceAt(dx, dy)) != EMPTY){
					if(piece == INVALID){
						break;
					}
					if(getColorAt(dx, dy) == color){
						if(abs(piece) == BISHOP ||
							abs(piece) == QUEEN){
								m_checkmateBlockable = true;
								return;
						}
					}
					else
						break;
				}
			}
		}
	}

	// below
	dx = x - 1;
	dy = y;
	if((piece = getPieceAt(dx, dy) != INVALID)){
		if(piece != EMPTY && getColorAt(dx, dy) == color){
			if(abs(piece) == PAWN ||
				abs(piece) == ROOK ||
				abs(piece) == QUEEN){
					m_checkmateBlockable = true;
					return;
			}
		}

		if(piece == EMPTY){
			for(;; --dx){
				if((piece = getPieceAt(dx, dy)) != EMPTY){
					if(piece == INVALID){
						break;
					}
					if(getColorAt(dx, dy) == color){
						if(abs(piece) == ROOK ||
							abs(piece) == QUEEN){
								m_checkmateBlockable = true;
								return;
						}
					}
					else
						break;
				}
			}
		}
	}

	// lower-right
	dx = x - 1;
	dy = y + 1;
	if((piece = getPieceAt(dx, dy)) != INVALID){
		if(piece != EMPTY && getColorAt(dx, dy) == color){
			if(abs(piece) == BISHOP ||
				abs(piece) == QUEEN){
					m_checkmateBlockable = true;
					return;
			}
		}

		if(piece == EMPTY){
			for(;; --dx, ++dy){
				if((piece = getPieceAt(dx, dy)) != EMPTY){
					if(piece == INVALID){
						break;
					}
					if(getColorAt(dx, dy) == color){
						if(abs(piece) == BISHOP ||
							abs(piece) == QUEEN){
								m_checkmateBlockable = true;
								return;
						}
					}
					else
						break;
				}
			}
		}
	}

	// right
	dx = x;
	dy = y + 1;
	if((piece = getPieceAt(dx, dy)) != INVALID){
		if(piece != EMPTY && getColorAt(dx, dy) == color){
			if(abs(piece) == ROOK ||
				abs(piece) == QUEEN){
					m_checkmateBlockable = true;
					return;
			}
		}

		if(piece == EMPTY){
			for(;; ++dy){
				if((piece = getPieceAt(dx, dy)) != EMPTY){
					if(piece == INVALID){
						break;
					}
					if(getColorAt(dx, dy) == color){
						if(abs(piece) == ROOK ||
							abs(piece) == QUEEN){
								m_checkmateBlockable = true;
								return;
						}
					}
					else
						break;
				}
			}
		}
	}

	// upper-right
	dx = x + 1;
	dy = y + 1;
	if((piece = getPieceAt(dx, dy)) != INVALID){
		if(piece != EMPTY && getColorAt(dx, dy) == color){
			if(abs(piece) == BISHOP ||
				abs(piece) == QUEEN){
					m_checkmateBlockable = true;
					return;
			}
		}

		if(piece == EMPTY){
			for(;; ++dx, ++dy){
				if((piece = getPieceAt(dx, dy)) != EMPTY){
					if(piece == INVALID){
						break;
					}
					if(getColorAt(dx, dy) == color){
						if(abs(piece) == BISHOP ||
							abs(piece) == QUEEN){
								m_checkmateBlockable = true;
								return;
						}
					}
					else
						break;
				}
			}
		}
	}

	// check for friendly knights
	// high upper-right
	dx = x + 2;
	dy = y + 1;
	if((dx < 9 && dx > 0) && (dy < 9 && dy > 0)){
		if(abs(getPieceAt(dx, dy) == KNIGHT && getColorAt(dx, dy) == color)){
			m_checkmateBlockable = true;
			return;
		}
	}

	// high upper-left
	dy -= 2;
	if((dx < 9 && dx > 0) && (dy < 9 && dy > 0)){
		if(abs(getPieceAt(dx, dy) == KNIGHT && getColorAt(dx, dy) == color)){
			m_checkmateBlockable = true;
			return;
		}
	}

	// far upper-right
	dx = x + 1;
	dy = y + 2;
	if((dx < 9 && dx > 0) && (dy < 9 && dy > 0)){
		if(abs(getPieceAt(dx, dy) == KNIGHT && getColorAt(dx, dy) == color)){
			m_checkmateBlockable = true;
			return;
		}
	}

	// far upper-left
	dy -= 4;
	if((dx < 9 && dx > 0) && (dy < 9 && dy > 0)){
		if(abs(getPieceAt(dx, dy) == KNIGHT && getColorAt(dx, dy) == color)){
			m_checkmateBlockable = true;
			return;
		}
	}

	// low lower-right
	dx = x - 2;
	dy = y + 1;
	if((dx < 9 && dx > 0) && (dy < 9 && dy > 0)){
		if(abs(getPieceAt(dx, dy) == KNIGHT && getColorAt(dx, dy) == color)){
			m_checkmateBlockable = true;
			return;
		}
	}

	// low lower-left
	dy -= 2;
	if((dx < 9 && dx > 0) && (dy < 9 && dy > 0)){
		if(abs(getPieceAt(dx, dy) == KNIGHT && getColorAt(dx, dy) == color)){
			m_checkmateBlockable = true;
			return;
		}
	}

	// far lower-right
	dx = x - 1;
	dy = y + 2;
	if((dx < 9 && dx > 0) && (dy < 9 && dy > 0)){
		if(abs(getPieceAt(dx, dy) == KNIGHT && getColorAt(dx, dy) == color)){
			m_checkmateBlockable = true;
			return;
		}
	}

	// far lower-left
	dy -= 4;
	if((dx < 9 && dx > 0) && (dy < 9 && dy > 0)){
		if(abs(getPieceAt(dx, dy) == KNIGHT && getColorAt(dx, dy) == color)){
			m_checkmateBlockable = true;
			return;
		}
	}

	//m_checkmateBlockable = false;
	return;
}

bool Game::isKingInCheck(int x, int y, bool color)
{
	int dx, dy;
	int piece;

	// we shall now check every space around the king for possible attacks

	// check above
	dx = x + 1;
	dy = y;
	if((piece = getPieceAt(dx, dy)) != INVALID){
		// check for immediate danger
		if(getColorAt(dx, dy) != color){
			if(piece == EMPTY){
				if(m_finalCheck){
					checkSpaceBlockable(dx, dy, color);
				}
			}
			if(abs(piece) == ROOK ||
				abs(piece) == QUEEN ||
				abs(piece) == KING){
					if(m_finalCheck){
						checkSpaceBlockable(dx, dy, color);
					}

					return true; // king is in check
			}
		}

		// now check the entire range above
		for(;; ++dx){
			if((piece = getPieceAt(dx, dy)) == EMPTY){
				if(m_finalCheck){
					checkSpaceBlockable(dx, dy, color);
				}
			}
			else{
				if(piece == INVALID){
					break;
				}
				if(getColorAt(dx, dy) != color){
					if(abs(piece) == ROOK ||
						abs(piece) == QUEEN){
							if(m_finalCheck){
								checkSpaceBlockable(dx, dy, color);
							}

							return true;
					}
					else
						break; // non-offending piece in the way
				}
				else
					break; // piece is friendly
			}
		}
	}

	if(m_finalCheck){
		m_checkmateBlockable = false;
	}

	// upper-left
	dx = x + 1;
	dy = y - 1;
	if((piece = getPieceAt(dx, dy)) != INVALID){
		if(getColorAt(dx, dy) != color){
			if(piece == EMPTY){
				if(m_finalCheck){
					checkSpaceBlockable(dx, dy, color);
				}
			}
			if(abs(piece) == PAWN ||
				abs(piece) == BISHOP ||
				abs(piece) == QUEEN ||
				abs(piece) == KING){
					if(m_finalCheck){
						checkSpaceBlockable(dx, dy, color);
					}

					return true;
			}
		}

		// check diagonal range
		for(;; ++dx, --dy){
			if((piece = getPieceAt(dx, dy)) == EMPTY){
				if(m_finalCheck){
					checkSpaceBlockable(dx, dy, color);
				}
			}
			else{
				if(piece == INVALID){
					break;
				}
				if(getColorAt(dx, dy) != color){
					if(abs(piece) == BISHOP ||
						abs(piece) == QUEEN){
							if(m_finalCheck){
								checkSpaceBlockable(dx, dy, color);
							}

							return true;
					}
					else
						break;
				}
				else
					break;
			}
		}
	}

	if(m_finalCheck){
		m_checkmateBlockable = false;
	}

	// left
	dx = x;
	dy = y - 1;
	if((piece = getPieceAt(dx, dy)) != INVALID){
		if(getColorAt(dx, dy) != color){
			if(piece == EMPTY){
				if(m_finalCheck){
					checkSpaceBlockable(dx, dy, color);
				}
			}
			if(abs(piece) == ROOK ||
				abs(piece) == QUEEN ||
				abs(piece) == KING){
					if(m_finalCheck){
						checkSpaceBlockable(dx, dy, color);
					}

					return true;
			}
		}

		// check row to the left
		for(;; --dy){
			if((piece = getPieceAt(dx, dy)) == EMPTY){
				if(m_finalCheck){
					checkSpaceBlockable(dx, dy, color);
				}
			}
			else{
				if(piece == INVALID){
					break;
				}
				if(getColorAt(dx, dy) != color){
					if(abs(piece) == ROOK ||
						abs(piece) == QUEEN){
							if(m_finalCheck){
								checkSpaceBlockable(dx, dy, color);
							}

							return true;
					}
					else
						break;
				}
				else
					break;
			}
		}
	}

	if(m_finalCheck){
		m_checkmateBlockable = false;
	}

	// lower-left
	dx = x - 1;
	dy = y - 1;
	if((piece = getPieceAt(dx, dy)) != INVALID){
		if(getColorAt(dx, dy) != color){
			if(piece == EMPTY){
				if(m_finalCheck){
					checkSpaceBlockable(dx, dy, color);
				}
			}
			if(abs(piece) == BISHOP ||
				abs(piece) == QUEEN ||
				abs(piece) == KING){
					if(m_finalCheck){
						checkSpaceBlockable(dx, dy, color);
					}

					return true;
			}
		}

		// check diagonal range
		for(;; --dx, --dy){
			if((piece = getPieceAt(dx, dy)) == EMPTY){
				if(m_finalCheck){
					checkSpaceBlockable(dx, dy, color);
				}
			}
			else{
				if(piece == INVALID){
					break;
				}
				if(getColorAt(dx, dy) != color){
					if(abs(piece) == BISHOP ||
						abs(piece) == QUEEN){
							if(m_finalCheck){
								checkSpaceBlockable(dx, dy, color);
							}

							return true;
					}
					else
						break;
				}
				else
					break;
			}
		}
	}

	if(m_finalCheck){
		m_checkmateBlockable = false;
	}

	// below
	dx = x - 1;
	dy = y;
	if((piece = getPieceAt(dx, dy)) != INVALID){
		if(getColorAt(dx, dy) != color){
			if(piece == EMPTY){
				if(m_finalCheck){
					checkSpaceBlockable(dx, dy, color);
				}
			}
			if(abs(piece) == ROOK ||
				abs(piece) == QUEEN ||
				abs(piece) == KING){
					if(m_finalCheck){
						checkSpaceBlockable(dx, dy, color);
					}

					return true;
			}
		}

		// check range below
		for(;; --dx){
			if((piece = getPieceAt(dx, dy)) == EMPTY){
				if(m_finalCheck){
					checkSpaceBlockable(dx, dy, color);
				}
			}
			else{
				if(piece == INVALID){
					break;
				}
				if(getColorAt(dx, dy) != color){
					if(abs(piece) == ROOK ||
						abs(piece) == QUEEN){
							if(m_finalCheck){
								checkSpaceBlockable(dx, dy, color);
							}

							return true;
					}
					else
						break;
				}
				else
					break;
			}
		}
	}

	if(m_finalCheck){
		m_checkmateBlockable = false;
	}

	// lower-right
	dx = x - 1;
	dy = y + 1;
	if((piece = getPieceAt(dx, dy)) != INVALID){
		if(getColorAt(dx, dy) != color){
			if(piece == EMPTY){
				checkSpaceBlockable(dx, dy, color);
			}
			if(abs(piece) == BISHOP ||
				abs(piece) == QUEEN ||
				abs(piece) == KING){
					if(m_finalCheck){
						checkSpaceBlockable(dx, dy, color);
					}

					return true;
			}
		}

		// check diagonal range
		for(;; --dx, ++dy){
			if((piece = getPieceAt(dx, dy)) == EMPTY){
				if(m_finalCheck){
					checkSpaceBlockable(dx, dy, color);
				}
			}
			else{
				if(piece == INVALID){
					break;
				}
				if(getColorAt(dx, dy) != color){
					if(abs(piece) == BISHOP ||
						abs(piece) == QUEEN){
							if(m_finalCheck){
								checkSpaceBlockable(dx, dy, color);
							}

							return true;
					}
					else
						break;
				}
				else
					break;
			}
		}
	}

	if(m_finalCheck){
		m_checkmateBlockable = false;
	}

	// right
	dx = x;
	dy = y + 1;
	if((piece = getPieceAt(dx, dy)) != INVALID){
		if(getColorAt(dx, dy) != color){
			if(piece == EMPTY){
				if(m_finalCheck){
					checkSpaceBlockable(dx, dy, color);
				}
			}
			if(abs(piece) == ROOK ||
				abs(piece) == QUEEN ||
				abs(piece) == KING){
					if(m_finalCheck){
						checkSpaceBlockable(dx, dy, color);
					}

					return true;
			}
		}

		// check row to the right
		for(;; ++dy){
			if((piece = getPieceAt(dx, dy)) == EMPTY){
				if(m_finalCheck){
					checkSpaceBlockable(dx, dy, color);
				}
			}
			else{
				if(piece == INVALID){
					break;
				}
				if(getColorAt(dx, dy) != color){
					if(abs(piece) == ROOK ||
						abs(piece) == QUEEN){
							if(m_finalCheck){
								checkSpaceBlockable(dx, dy, color);
							}

							return true;
					}
					else
						break;
				}
				else
					break;
			}
		}
	}

	if(m_finalCheck){
		m_checkmateBlockable = false;
	}

	// upper-right
	dx = x + 1;
	dy = y + 1;
	if((piece = getPieceAt(dx, dy)) != INVALID){
		if(getColorAt(dx, dy) != color){
			if(piece == EMPTY){
				if(m_finalCheck){
					checkSpaceBlockable(dx, dy, color);
				}
			}
			if(abs(piece) == PAWN ||
				abs(piece) == BISHOP ||
				abs(piece) == QUEEN ||
				abs(piece) == KING){
					if(m_finalCheck){
						checkSpaceBlockable(dx, dy, color);
					}

					return true;
			}
		}

		// check the diagonal range
		for(;; ++dx, ++dy){
			if((piece = getPieceAt(dx, dy)) == EMPTY){
				if(m_finalCheck){
					checkSpaceBlockable(dx, dy, color);
				}
			}
			else{
				if(piece == INVALID){
					break;
				}
				if(getColorAt(dx, dy) != color){
					if(abs(piece) == BISHOP ||
						abs(piece) == QUEEN){
							if(m_finalCheck){
								checkSpaceBlockable(dx, dy, color);
							}

							return true;
					}
					else
						break;
				}
				else
					break;
			}
		}
	}

	//if(m_finalCheck){
	//	m_checkmateBlockable = false; // ???
	//}

	// check for offending knights
	// high upper-right
	dx = x + 2;
	dy = y + 1;
	if((dx < 9 && dx > 0) && (dy < 9 && dy > 0)){
		if(abs(getPieceAt(dx, dy)) == KNIGHT && getColorAt(dx, dy) != color){
			return true;
		}
	}

	// high upper-left
	dy -= 2;
	if((dx < 9 && dx > 0) && (dy < 9 && dy > 0)){
		if(abs(getPieceAt(dx, dy)) == KNIGHT && getColorAt(dx, dy) != color){
			return true;
		}
	}

	// far upper-right
	dx = x + 1;
	dy = y + 2;
	if((dx < 9 && dx > 0) && (dy < 9 && dy > 0)){
		if(abs(getPieceAt(dx, dy)) == KNIGHT && getColorAt(dx, dy) != color){
			return true;
		}
	}

	// far upper-left
	dy -= 4;
	if((dx < 9 && dx > 0) && (dy < 9 && dy > 0)){
		if(abs(getPieceAt(dx, dy)) == KNIGHT && getColorAt(dx, dy) != color){
			return true;
		}
	}

	// low lower-right
	dx = x - 2;
	dy = y + 1;
	if((dx < 9 && dx > 0) && (dy < 9 && dy > 0)){
		if(abs(getPieceAt(dx, dy)) == KNIGHT && getColorAt(dx, dy) != color){
			return true;
		}
	}

	// low lower-left
	dy -= 2;
	if((dx < 9 && dx > 0) && (dy < 9 && dy > 0)){
		if(abs(getPieceAt(dx, dy)) == KNIGHT && getColorAt(dx, dy) != color){
			return true;
		}
	}

	// far lower-right
	dx = x - 1;
	dy = y + 2;
	if((dx < 9 && dx > 0) && (dy < 9 && dy > 0)){
		if(abs(getPieceAt(dx, dy)) == KNIGHT && getColorAt(dx, dy) != color){
			return true;
		}
	}

	// far lower-left
	dy -= 4;
	if((dx < 9 && dx > 0) && (dy < 9 && dy > 0)){
		if(abs(getPieceAt(dx, dy)) == KNIGHT && getColorAt(dx, dy) != color){
			return true;
		}
	}

	// king is not in check
	return false;
}

int Game::isKingInCheckmate(int x, int y, bool color)
{
	int dx, dy;
	int piece;

	m_checkmateBlockable = false;
	m_finalCheck = false;

	if(color == WHITE){
		m_board[m_whiteKingX][m_whiteKingY] = EMPTY;
	}
	else{
		m_board[m_blackKingX][m_blackKingY] = EMPTY;
	}

	// test every surrounding valid square

	// above
	dx = x + 1;
	dy = y;
	piece = getPieceAt(dx, dy);
	if(piece != INVALID){
		if(piece == EMPTY || getColorAt(dx, dy) != color){
			if(!isKingInCheck(dx, dy, color)){
				goto safe;
			}
		}
	}

	// upper-left
	dx = x + 1;
	dy = y - 1;
	piece = getPieceAt(dx, dy);
	if(piece != INVALID){
		if(piece == EMPTY || getColorAt(dx, dy) != color){
			if(!isKingInCheck(dx, dy, color)){
				goto safe;
			}
		}
	}

	// left
	dx = x;
	dy = y - 1;
	piece = getPieceAt(dx, dy);
	if(piece != INVALID){
		if(piece == EMPTY || getColorAt(dx, dy) != color){
			if(!isKingInCheck(dx, dy, color)){
				goto safe;
			}
		}
	}

	// lower-left
	dx = x - 1;
	dy = y - 1;
	piece = getPieceAt(dx, dy);
	if(piece != INVALID){
		if(piece == EMPTY || getColorAt(dx, dy) != color){
			if(!isKingInCheck(dx, dy, color)){
				goto safe;
			}
		}
	}

	// below
	dx = x - 1;
	dy = y;
	piece = getPieceAt(dx, dy);
	if(piece != INVALID){
		if(piece == EMPTY || getColorAt(dx, dy) != color){
			if(!isKingInCheck(x - 1, y, color)){
				goto safe;
			}
		}
	}

	// lower-right
	dx = x - 1;
	dy = y + 1;
	piece = getPieceAt(dx, dy);
	if(piece != INVALID){
		if(piece == EMPTY || getColorAt(dx, dy) != color){
			if(!isKingInCheck(x - 1, y + 1, color)){
				goto safe;
			}
		}
	}

	// right
	dx = x;
	dy = y + 1;
	piece = getPieceAt(dx, dy);
	if(piece != INVALID){
		if(piece == EMPTY || getColorAt(dx, dy) != color){
			if(!isKingInCheck(x, y + 1, color)){
				goto safe;
			}
		}
	}

	// upper-right
	dx = x + 1;
	dy = y + 1;
	piece = getPieceAt(dx, dy);
	if(piece != INVALID){
		if(piece == EMPTY || getColorAt(dx, dy) != color){
			if(!isKingInCheck(x + 1, y + 1, color)){
				goto safe;
			}
		}
	}

	// itself
	m_finalCheck = true;
	if(!isKingInCheck(x, y, color)){
		return STATE_STALEMATE;
	}

	// check for friendly pieces that can save...

	// if not
	goto checkmate;

safe:{
		// restore original king
		if(color == WHITE){
			m_board[m_whiteKingX][m_whiteKingY] = WHITE_KING;
		}
		else{
			m_board[m_blackKingX][m_blackKingY] = BLACK_KING;
		}

		return false;
	 }

checkmate:
	if(color == WHITE){
		m_board[m_whiteKingX][m_whiteKingY] = WHITE_KING;
	}
	else{
		m_board[m_blackKingX][m_blackKingY] = BLACK_KING;
	}

	// one last check for blockable spaces
	if(m_checkmateBlockable){
		return false;
	}

	return true; // checkmate
}

/* simpler version for testing after the checkmate cheap hack I did ;) */
bool Game::testCheckmate(bool color)
{
	int x = (color == WHITE) ? m_whiteKingX : m_blackKingX;
	int y = (color == WHITE) ? m_whiteKingY : m_blackKingY;
	int dx, dy;
	int piece = EMPTY;

	if(color == WHITE){
		m_board[m_whiteKingX][m_whiteKingY] = EMPTY;
	}
	else{
		m_board[m_blackKingX][m_blackKingY] = EMPTY;
	}

	// test every surrounding valid square

	// above
	dx = x + 1;
	dy = y;
	piece = getPieceAt(dx, dy);
	if(piece != INVALID){
		if(piece == EMPTY || getColorAt(dx, dy) != color){
			if(!isKingInCheck(dx, dy, color)){
				goto safe;
			}
		}
	}

	// upper-left
	dx = x + 1;
	dy = y - 1;
	piece = getPieceAt(dx, dy);
	if(piece != INVALID){
		if(piece == EMPTY || getColorAt(dx, dy) != color){
			if(!isKingInCheck(dx, dy, color)){
				goto safe;
			}
		}
	}

	// left
	dx = x;
	dy = y - 1;
	piece = getPieceAt(dx, dy);
	if(piece != INVALID){
		if(piece == EMPTY || getColorAt(dx, dy) != color){
			if(!isKingInCheck(dx, dy, color)){
				goto safe;
			}
		}
	}

	// lower-left
	dx = x - 1;
	dy = y - 1;
	piece = getPieceAt(dx, dy);
	if(piece != INVALID){
		if(piece == EMPTY || getColorAt(dx, dy) != color){
			if(!isKingInCheck(dx, dy, color)){
				goto safe;
			}
		}
	}

	// below
	dx = x - 1;
	dy = y;
	piece = getPieceAt(dx, dy);
	if(piece != INVALID){
		if(piece == EMPTY || getColorAt(dx, dy) != color){
			if(!isKingInCheck(x - 1, y, color)){
				goto safe;
			}
		}
	}

	// lower-right
	dx = x - 1;
	dy = y + 1;
	piece = getPieceAt(dx, dy);
	if(piece != INVALID){
		if(piece == EMPTY || getColorAt(dx, dy) != color){
			if(!isKingInCheck(x - 1, y + 1, color)){
				goto safe;
			}
		}
	}

	// right
	dx = x;
	dy = y + 1;
	piece = getPieceAt(dx, dy);
	if(piece != INVALID){
		if(piece == EMPTY || getColorAt(dx, dy) != color){
			if(!isKingInCheck(x, y + 1, color)){
				goto safe;
			}
		}
	}

	// upper-right
	dx = x + 1;
	dy = y + 1;
	piece = getPieceAt(dx, dy);
	if(piece != INVALID){
		if(piece == EMPTY || getColorAt(dx, dy) != color){
			if(!isKingInCheck(x + 1, y + 1, color)){
				goto safe;
			}
		}
	}

	// itself
	m_finalCheck = true;
	if(!isKingInCheck(x, y, color)){
		return STATE_STALEMATE;
	}

	// check for friendly pieces that can save...

	// if not
	goto checkmate;

safe:{
		// restore original king
		if(color == WHITE){
			m_board[m_whiteKingX][m_whiteKingY] = WHITE_KING;
		}
		else{
			m_board[m_blackKingX][m_blackKingY] = BLACK_KING;
		}

		return false;
	 }

checkmate:
	if(color == WHITE){
		m_board[m_whiteKingX][m_whiteKingY] = WHITE_KING;
	}
	else{
		m_board[m_blackKingX][m_blackKingY] = BLACK_KING;
	}

	return true;
}

void Game::printBoard(void)
{
	printf("Printing board:\n\n");

	for(int i=0; i<10; ++i){
		for(int j=0; j<10; ++j){
			printf("[%d]", m_board[i][j]);
		}
		printf("\n");
	}
}