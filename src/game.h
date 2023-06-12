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
#include <cstring>
#include <cmath>

#include "particle.h"
#include "cam.h"
#include "ai.h"
#include "input.h"
#include "arcane_lib.h"
#include "sound.h"
#include "graphics.h"

#define IDT_GAME_TIMER	101
#define NUM_SETS 3

#define WHITE	true
#define BLACK	false

#define BOARD_MIN	1
#define BOARD_MAX	8

#define MAGIC_STR	"_|ETHEREAL|_"
#define MAGIC_STR_SIZE 13

extern HWND g_hWnd;
//extern Cam	g_cam;

class Game{
public:
	// file saving/loading data
	struct save_t{
		// header
		char magic[MAGIC_STR_SIZE];		// magic string for header

		// game data
		int m_board[10][10];
		int m_captureState[11];
		unsigned int m_lastMoveFromX, m_lastMoveFromY;
		unsigned int m_lastMoveToX, m_lastMoveToY;
		unsigned int m_whiteKingX, m_whiteKingY;
		unsigned int m_blackKingX, m_blackKingY;
		unsigned int m_whiteTime, m_blackTime;

		bool m_turn;
		bool m_playerColor;
		bool m_whiteCastle, m_blackCastle;
		bool m_whiteKingInCheck, m_blackKingInCheck;

		// AI
		char m_pos[4096];
	};

	// piece values for board representation
	enum pieces{
		BLACK_KING = -6,
		BLACK_QUEEN,
		BLACK_BISHOP,
		BLACK_KNIGHT,
		BLACK_ROOK,
		BLACK_PAWN,
		EMPTY = 0,
		WHITE_PAWN,
		WHITE_ROOK,
		WHITE_KNIGHT,
		WHITE_BISHOP,
		WHITE_QUEEN,
		WHITE_KING,

		PAWN = 1,
		ROOK = 2,
		KNIGHT = 3,
		BISHOP = 4,
		QUEEN = 5,
		KING = 6,

		// for capture states
		BLACK_PAWN_ = 6,
		BLACK_ROOK_,
		BLACK_KNIGHT_,
		BLACK_BISHOP_,
		BLACK_QUEEN_,

		INVALID = 99
	};

	// different modes of gameplay
	enum gameplay_types{
		GAMEPLAY_NORMAL = 0,
		GAMEPLAY_2PLAYER,
		GAMEPLAY_FREEMOVE
	};

	// texture types for the pieces
	enum texture_types{
		METALLIC = 0,
		GOLD,
		GOLD_SHINY,
		GLASS,
		MARBLE
	};

	// texture names
	enum textures{
		TEX_BOARD = 0,
		TEX_BOARD_FRAME,
		TEX_BOARD_BOX,
		TEX_CREDITS,
		TEX_EARTH,
		TEX_SATURN,
		TEX_SATURN_RINGS,
		TEX_WHITE,
		TEX_BLACK,
		TEX_SKY_NORTH,
		TEX_SKY_SOUTH,
		TEX_SKY_EAST,
		TEX_SKY_WEST,
		TEX_SKY_UP,
		TEX_SKY_DOWN,
		TEX_PAUSEMENU_RESUME,
		TEX_PAUSEMENU_NEWGAME,
		TEX_PAUSEMENU_LOADGAME,
		TEX_PAUSEMENU_SAVEGAME,
		TEX_PAUSEMENU_OPTIONS,
		TEX_PAUSEMENU_HELP,
		TEX_PAUSEMENU_CREDITS,
		TEX_PAUSEMENU_QUIT,
		TEX_2D_PAWN_WHITE,
		TEX_2D_PAWN_BLACK,
		TEX_2D_ROOK_WHITE,
		TEX_2D_ROOK_BLACK,
		TEX_2D_KNIGHT_WHITE,
		TEX_2D_KNIGHT_BLACK,
		TEX_2D_BISHOP_WHITE,
		TEX_2D_BISHOP_BLACK,
		TEX_2D_QUEEN_WHITE,
		TEX_2D_QUEEN_BLACK,
		TEX_2D_KING_WHITE,
		TEX_2D_KING_BLACK,
	};

	// chess sets
	enum chess_sets{
		SET_NATIVE = 0,
		SET_NORMAL,
		SET_FANCY,
	};

	// game states
	enum game_states{
		STATE_LOADING = 0,
		STATE_ACTIVE,
		STATE_MENU,
		STATE_PAUSED,
		STATE_CREDITS,
		STATE_WHITE_CHECKMATE,
		STATE_BLACK_CHECKMATE,
		STATE_STALEMATE
	};

	// skybox textures
	enum skyboxes{
		SKYBOX_SPACE = 0,
		SKYBOX_MOONLIGHT,
		SKYBOX_DESERT
	};

	// planet modes
	enum planets{
		PLANET_NONE = 0,
		PLANET_EARTH,
		PLANET_EARTH_NIGHT,
		PLANET_SATURN
	};

	enum aiLevels{
		CHILD = 0,
		LION,
		WALRUS,
		RAPTOR,
		GRANDMASTER
	};

	enum{
		ABOVE = 0,
		UPPER_LEFT,
		LEFT,
		LOWER_LEFT,
		BELOW,
		LOWER_RIGHT,
		RIGHT,
		UPPER_RIGHT
	};

	// retrieve instance of class (since there is only one)
	static Game& inst(void);

	// game functions
	void init(HWND& hwnd);
	bool movePiece(void);								// move the selection to new selection
	void newGame(void);
	bool saveFile(void);
	bool loadFile(void);
	bool quickSave(void);
	bool quickLoad(void);
	bool exit(void);

	// getter functions
	unsigned int getSelectionX(void);
	unsigned int getSelectionY(void);
	unsigned int getNewSelectionX(void);
	unsigned int getNewSelectionY(void);
	unsigned int getLastSelectionX(void);
	unsigned int getLastSelectionY(void);
	unsigned int getLastMoveX(bool from);
	unsigned int getLastMoveY(bool from);
	unsigned int getAnimateFromX(void);
	unsigned int getAnimateFromY(void);
	unsigned int getAnimateToX(void);
	unsigned int getAnimateToY(void);
	unsigned int getState(void);
	int  getPieceAt(int x, int y);						// return piece value at x-y location
	bool getPlayerColor(void);
	bool isSelected(void);
	bool getColor(void);								// retrieve piece color at current selection
	bool getNewColor(void);								// retrieve piece color at new selection
	bool getPieceColor(int piece);
	bool getColorAt(int x, int y);
	bool getTurn(void);									// retrieve who's turn it is
	bool drawSelection(void);
	bool allowSelectionChange(void);
	bool getGameplayMode(void);
	bool getFreeMove(void);
	bool isSetLoaded(unsigned int set);
	unsigned int getAILevel(void);
	unsigned int getTime(bool color);					// retrieve time left for player
	unsigned int getTime(void);							// get the game setting for time
	unsigned int getChessSet(void);
	unsigned int getSkybox(void);
	unsigned int getPlanet(void);
	unsigned int getTextureMode(void);
	int getCaptureState(int piece);

	bool isAnimating(void);
	bool inCheck(bool color);
	bool isSaved(void);

	std::ostringstream& getStatusStream(void);
	std::ostringstream& getStreamBuffer(void);
	unsigned int getStreamOffset(void);

	// setter functions
	void setSelectionX(int x);
	void setSelectionY(int y);
	void setSelection(int x, int y);
	void setNewSelectionX(int x);
	void setNewSelectionY(int y);
	void setNewSelection(int x, int y);
	void setLastSelectionX(int x);
	void setLastSelectionY(int y);
	void setLastSelection(int x, int y);
	void setSelected(bool selected);
	void setPlayerColor(bool color);
	void setState(unsigned int state);
	void setAnimating(bool animating);
	void setTurn(bool turn);
	void setTime(bool color, unsigned int time);
	void setTime(unsigned int time);
	void appendStatusStream(const char* str);
	void setStreamBuffer(const char* str);
	void setChessSet(unsigned int set);
	void setSkybox(unsigned int sky);
	void setAILevel(unsigned int level);
	void setPlanet(unsigned int planet);
	void setTextureMode(unsigned int mode);
	void setDrawSelection(bool draw);
	void setFreeMove(bool freeMove);
	void setAllowSelectionChange(bool allow);
	void addCapture(int piece);
	void setSetLoaded(unsigned int set, bool loaded);

	// debugging
	void printBoard(void);

protected:
	// constructor/destructor
	Game();
	~Game();

// protected member functions
	void resetBoard(void);

	// file functions
	bool writeSave(const char* file);
	bool load(const char* file);

	// movement functions
	bool isXRangeClear(void);
	bool isYRangeClear(void);
	bool isDiagonalRangeClear(void);
	void checkSpaceBlockable(int x, int y, bool color);
	bool isKingInCheck(int x, int y, bool color);
	int isKingInCheckmate(int x, int y, bool color);
	bool testCheckmate(bool color);
	bool checkmateSave(void);
	bool checkmateLoad(void);

	// constants
	static const float DEFAULT_ANIMATION_SPEED;
	static const float DEFAULT_KNIGHT_ANIMATION_SPEED;
	static const DWORD DEFAULT_TIME;

	// member variables
	int m_board[10][10];								// board representation
	int m_captureState[11];
	unsigned int m_selectionX, m_selectionY;
	unsigned int m_newSelectionX, m_newSelectionY;
	unsigned int m_lastSelectionX, m_lastSelectionY;
	unsigned int m_lastMoveFromX, m_lastMoveFromY;
	unsigned int m_lastMoveToX, m_lastMoveToY;
	unsigned int m_whiteKingX, m_whiteKingY;
	unsigned int m_blackKingX, m_blackKingY;
	unsigned int m_gameState;
	unsigned int m_chessSet;
	unsigned int m_skybox;
	unsigned int m_planet;

	unsigned int m_textureMode;							// texture mode for pieces
	unsigned int m_gameplayMode;
	unsigned int m_aiLevel;
	
	bool m_turn;										// who's turn it is (true = white, false = black)
	bool m_playerColor;									// color of the player's pieces
	bool m_animation;									// enable or disable animation
	bool m_animating;									// is the game animating a movement
	bool m_selected;									// has the user selected the "from" space
	bool m_drawSelection;								// draw the selector tool
	bool m_allowSelectionChange;
	bool m_freeMove;
	bool m_whiteCastle;
	bool m_blackCastle;
	bool m_whiteKingInCheck, m_blackKingInCheck;
	bool m_finalCheck;
	bool m_checkmateBlockable;
	bool m_checkDirection[8];
	bool m_saved;
	bool m_setLoaded[NUM_SETS];

	unsigned int m_animateFromX, m_animateFromY;			// where to animate from
	unsigned int m_animateToX, m_animateToY;				// where to animate to
	unsigned int m_animationSpeed;						// how fast the piece will move
	unsigned int m_time;
	unsigned int m_whiteTime;
	unsigned int m_blackTime;

	std::ostringstream m_output;
	std::ostringstream m_streamBuffer;
	unsigned int m_streamOffset;						// offset for moving text to the next line
};

// getter functions
inline unsigned int Game::getSelectionX(void)
{
	return m_selectionX;
}

inline unsigned int Game::getSelectionY(void)
{
	return m_selectionY;
}

inline unsigned int Game::getNewSelectionX(void)
{
	return m_newSelectionX;
}

inline unsigned int Game::getNewSelectionY(void)
{
	return m_newSelectionY;
}

inline unsigned int Game::getLastSelectionX(void)
{
	return m_lastSelectionX;
}

inline unsigned int Game::getLastSelectionY(void)
{
	return m_lastSelectionY;
}

inline unsigned int Game::getLastMoveX(bool from)
{
	return (from) ? m_lastMoveFromX : m_lastMoveToX;
}

inline unsigned int Game::getLastMoveY(bool from)
{
	return (from) ? m_lastMoveFromY : m_lastMoveToY;
}

inline unsigned int Game::getAnimateFromX(void)
{
	return m_animateFromX;
}

inline unsigned int Game::getAnimateFromY(void)
{
	return m_animateFromY;
}

inline unsigned int Game::getAnimateToX(void)
{
	return m_animateToX;
}

inline unsigned int Game::getAnimateToY(void)
{
	return m_animateToY;
}

inline unsigned int Game::getAILevel(void)
{
	return m_aiLevel;
}

inline bool Game::isSelected(void)
{
	return m_selected;
}

inline unsigned int Game::getState(void)
{
	return m_gameState;
}

inline bool Game::isSetLoaded(unsigned int set)
{
	return m_setLoaded[set];
}

inline int Game::getPieceAt(int x, int y)
{
	return m_board[x][y];
}

inline bool Game::getPlayerColor(void)
{
	return m_playerColor;
}

inline bool Game::getColor(void)
{
	return (m_board[m_selectionX][m_selectionY] < 0) ? BLACK : WHITE;
}

inline bool Game::getNewColor(void)
{
	return (m_board[m_newSelectionX][m_newSelectionY] < 0) ? BLACK : WHITE;
}

inline bool Game::getPieceColor(int piece)
{
	return (piece > 0) ? true : false;
}

inline bool Game::getColorAt(int x, int y)
{
	return (m_board[x][y] < 0) ? BLACK : WHITE;
}

inline bool Game::getTurn(void)
{
	return m_turn;
}

inline bool Game::drawSelection(void)
{
	return m_drawSelection;
}

inline bool Game::allowSelectionChange(void)
{
	return m_allowSelectionChange;
}

inline bool Game::getGameplayMode(void)
{
	return m_gameplayMode;
}

inline bool Game::getFreeMove(void)
{
	return m_freeMove;
}

inline unsigned int Game::getTime(bool color)
{
	return (color == WHITE) ? m_whiteTime : m_blackTime;
}

inline unsigned int Game::getTime(void)
{ 
	return m_time;
}

inline bool Game::isAnimating(void)
{
	return m_animating;
}

inline unsigned int Game::getChessSet(void)
{
	return m_chessSet;
}

inline unsigned int Game::getSkybox(void)
{
	return m_skybox;
}

inline unsigned int Game::getPlanet(void)
{
	return m_planet;
}

inline unsigned int Game::getTextureMode(void)
{
	return m_textureMode;
}

inline std::ostringstream& Game::getStatusStream(void)
{
	return m_output;
}

inline std::ostringstream& Game::getStreamBuffer(void)
{
	return m_streamBuffer;
}

inline unsigned int Game::getStreamOffset(void)
{
	return m_streamOffset;
}

inline bool Game::inCheck(bool color)
{
	return (color == WHITE) ? m_whiteKingInCheck : m_blackKingInCheck;
}

inline bool Game::isSaved(void)
{
	return m_saved;
}

inline int Game::getCaptureState(int piece)
{
	return m_captureState[piece];
}

// setter functions
inline void Game::setSelectionX(int x)
{
	m_selectionX = x;
}

inline void Game::setSelectionY(int y)
{
	m_selectionY = y;
}

inline void Game::setSelection(int x, int y)
{
	m_selectionX = x;
	m_selectionY = y;
}

inline void Game::setNewSelectionX(int x)
{
	m_newSelectionX = x;
}

inline void Game::setNewSelectionY(int y)
{
	m_newSelectionY = y;
}

inline void Game::setNewSelection(int x, int y)
{
	m_newSelectionX = x;
	m_newSelectionY = y;
}

inline void Game::setLastSelectionX(int x)
{
	m_lastSelectionX = x;
}

inline void Game::setLastSelectionY(int y)
{
	m_lastSelectionY = y;
}

inline void Game::setLastSelection(int x, int y)
{
	m_lastSelectionX = x;
	m_lastSelectionY = y;
}

inline void Game::setPlayerColor(bool color)
{
	m_playerColor = color;
}

inline void Game::setSelected(bool selected)
{
	m_selected = selected;
}

inline void Game::setAILevel(unsigned int level)
{
	m_aiLevel = level;
}

inline void Game::setDrawSelection(bool draw)
{
	m_drawSelection = draw;
}

inline void Game::setFreeMove(bool freeMove)
{
	m_freeMove = freeMove;
}

inline void Game::setSetLoaded(unsigned int set, bool loaded)
{
	m_setLoaded[set] = loaded;
}

inline void Game::setState(unsigned int state)
{
	m_gameState = state;

	if(state == STATE_ACTIVE)
		Mouse::inst().hideCursor(true);
	else
		Mouse::inst().hideCursor(false);
}

inline void Game::setAnimating(bool animating)
{
	m_animating = animating;
}

inline void Game::setTurn(bool turn)
{
	m_turn = turn;
}

inline void Game::addCapture(int piece)
{
	m_captureState[piece]++;
}

inline void Game::setTime(bool color, unsigned int time)
{
	(color == WHITE) ? m_whiteTime = time : m_blackTime = time;
}

inline void Game::setTime(unsigned int time)
{
	m_time = time;
}

inline void Game::appendStatusStream(const char* str)
{
	m_output << str << std::endl;
	m_streamOffset -= 20; // *** this value only works for Segoe UI, 9, BOLD
}

inline void Game::setStreamBuffer(const char* str)
{
	m_streamBuffer.str("");
	m_streamBuffer << str;
}

inline void Game::setChessSet(unsigned int set)
{
	m_chessSet = set;
}

inline void Game::setSkybox(unsigned int skybox)
{
	m_skybox = skybox;
}

inline void Game::setPlanet(unsigned int planet)
{
	m_planet = planet;
}

inline void Game::setTextureMode(unsigned int mode)
{
	m_textureMode = mode;
}

inline void Game::setAllowSelectionChange(bool allow)
{
	m_allowSelectionChange = allow;
}