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

#include "menu.h"
#include "dialog.h"
#include "resource.h"

const float Button::BUTTON_ACTIVE_COLOR[4] = {1.0f, 0.0f, 0.0f, 0.7f};
const float Button::BUTTON_INACTIVE_COLOR[4] = {0.0f, 0.0f, 1.0f, 0.7f};
extern GLuint g_textures[];
extern HWND g_hWnd;

void InitMenu(void)
{
	Graphics& graphics = graphics.inst();
	extern Menu g_pauseMenu;
	extern Menu g_pieceInfo;
	int height = graphics.getHeight();
	int width = graphics.getWidth();

	/// main menu
	// new game button
	/*g_mainMenu.buttons[0].setSize(200, 80);
	g_mainMenu.buttons[0].setPos(0, 20);
	g_mainMenu.buttons[0].setColor(0.0f, 0.0f, 1.0f, 0.7f);
	g_mainMenu.buttons[0].setTexture(g_textures[Game::TEX_MAINMENU_NEWGAME]);
	g_mainMenu.buttons[0].setLoaded(true);*/

	/// pause menu
	g_pauseMenu.setStatic(false);
	g_pauseMenu.setBlend(true);

	// resume button
	g_pauseMenu.buttons[0].setSize(200, 80);
	g_pauseMenu.buttons[0].setPos(-210, 40);
	g_pauseMenu.buttons[0].setColor(1.0f, 1.0f, 1.0f, 0.7f);
	g_pauseMenu.buttons[0].setTexture(g_textures[Game::TEX_PAUSEMENU_RESUME]);
	g_pauseMenu.buttons[0].setLoaded(true);
	g_pauseMenu.buttons[0].enable(false);

	// newgame button
	g_pauseMenu.buttons[1].setSize(200, 80);
	g_pauseMenu.buttons[1].setPos(10, 40);
	g_pauseMenu.buttons[1].setColor(1.0f, 1.0f, 1.0f, 0.7f);
	g_pauseMenu.buttons[1].setTexture(g_textures[Game::TEX_PAUSEMENU_NEWGAME]);
	g_pauseMenu.buttons[1].setLoaded(true);
	g_pauseMenu.buttons[1].enable(true);

	// loadgame button
	g_pauseMenu.buttons[2].setSize(200, 80);
	g_pauseMenu.buttons[2].setPos(10, 140);
	g_pauseMenu.buttons[2].setColor(1.0f, 1.0f, 1.0f, 0.7f);
	g_pauseMenu.buttons[2].setTexture(g_textures[Game::TEX_PAUSEMENU_LOADGAME]);
	g_pauseMenu.buttons[2].setLoaded(true);
	g_pauseMenu.buttons[2].enable(true);

	// savegame button
	g_pauseMenu.buttons[3].setSize(200, 80);
	g_pauseMenu.buttons[3].setPos(-210, 140);
	g_pauseMenu.buttons[3].setColor(1.0f, 1.0f, 1.0f, 0.7f);
	g_pauseMenu.buttons[3].setTexture(g_textures[Game::TEX_PAUSEMENU_SAVEGAME]);
	g_pauseMenu.buttons[3].setLoaded(true);
	g_pauseMenu.buttons[3].enable(false);

	// options button
	g_pauseMenu.buttons[4].setSize(200, 80);
	g_pauseMenu.buttons[4].setPos(-210, 240);
	g_pauseMenu.buttons[4].setColor(1.0f, 1.0f, 1.0f, 0.7f);
	g_pauseMenu.buttons[4].setTexture(g_textures[Game::TEX_PAUSEMENU_OPTIONS]);
	g_pauseMenu.buttons[4].setLoaded(true);
	g_pauseMenu.buttons[4].enable(true);

	// help button
	g_pauseMenu.buttons[5].setSize(200, 80);
	g_pauseMenu.buttons[5].setPos(10, 240);
	g_pauseMenu.buttons[5].setColor(1.0f, 1.0f, 1.0f, 0.7f);
	g_pauseMenu.buttons[5].setTexture(g_textures[Game::TEX_PAUSEMENU_HELP]);
	g_pauseMenu.buttons[5].setLoaded(true);
	g_pauseMenu.buttons[5].enable(true);

	// credits button
	g_pauseMenu.buttons[6].setSize(200, 80);
	g_pauseMenu.buttons[6].setPos(-210, 340);
	g_pauseMenu.buttons[6].setColor(1.0f, 1.0f, 1.0f, 0.7f);
	g_pauseMenu.buttons[6].setTexture(g_textures[Game::TEX_PAUSEMENU_CREDITS]);
	g_pauseMenu.buttons[6].setLoaded(true);
	g_pauseMenu.buttons[6].enable(true);

	// quit button
	g_pauseMenu.buttons[7].setSize(200, 80);
	g_pauseMenu.buttons[7].setPos(10, 340);
	g_pauseMenu.buttons[7].setColor(1.0f, 1.0f, 1.0f, 0.7f);
	g_pauseMenu.buttons[7].setTexture(g_textures[Game::TEX_PAUSEMENU_QUIT]);
	g_pauseMenu.buttons[7].setLoaded(true);
	g_pauseMenu.buttons[7].enable(true);

	/* piece info */
	g_pieceInfo.setStatic(true);
	g_pieceInfo.setBlend(true);

	const int size = 24;
	const int offset = 172;

	// white pawn
	g_pieceInfo.buttons[0].setSize(size, size);
	g_pieceInfo.buttons[0].setPos(0, offset);
	g_pieceInfo.buttons[0].setColor(1.0f, 1.0f, 1.0f, 1.0f);
	g_pieceInfo.buttons[0].setTexture(g_textures[Game::TEX_2D_PAWN_WHITE]);
	g_pieceInfo.buttons[0].setLoaded(true);
	g_pieceInfo.buttons[0].setNum(0);

	// white rook
	g_pieceInfo.buttons[1].setSize(size, size);
	g_pieceInfo.buttons[1].setPos(0, (offset - size));
	g_pieceInfo.buttons[1].setColor(1.0f, 1.0f, 1.0f, 1.0f);
	g_pieceInfo.buttons[1].setTexture(g_textures[Game::TEX_2D_ROOK_WHITE]);
	g_pieceInfo.buttons[1].setLoaded(true);
	g_pieceInfo.buttons[1].setNum(1);

	// white knight
	g_pieceInfo.buttons[2].setSize(size, size);
	g_pieceInfo.buttons[2].setPos(0, (offset - (size * 2)));
	g_pieceInfo.buttons[2].setColor(1.0f, 1.0f, 1.0f, 1.0f);
	g_pieceInfo.buttons[2].setTexture(g_textures[Game::TEX_2D_KNIGHT_WHITE]);
	g_pieceInfo.buttons[2].setLoaded(true);
	g_pieceInfo.buttons[2].setNum(2);

	// white bishop
	g_pieceInfo.buttons[3].setSize(size, size);
	g_pieceInfo.buttons[3].setPos(0, (offset - (size * 3)));
	g_pieceInfo.buttons[3].setColor(1.0f, 1.0f, 1.0f, 1.0f);
	g_pieceInfo.buttons[3].setTexture(g_textures[Game::TEX_2D_BISHOP_WHITE]);
	g_pieceInfo.buttons[3].setLoaded(true);
	g_pieceInfo.buttons[3].setNum(3);

	// white queen
	g_pieceInfo.buttons[4].setSize(size, size);
	g_pieceInfo.buttons[4].setPos(0, (offset - (size * 4)));
	g_pieceInfo.buttons[4].setColor(1.0f, 1.0f, 1.0f, 1.0f);
	g_pieceInfo.buttons[4].setTexture(g_textures[Game::TEX_2D_QUEEN_WHITE]);
	g_pieceInfo.buttons[4].setLoaded(true);
	g_pieceInfo.buttons[4].setNum(4);

	// black pawn
	g_pieceInfo.buttons[5].setSize(size, size);
	g_pieceInfo.buttons[5].setPos(width - size, offset);
	g_pieceInfo.buttons[5].setColor(0.5f, 0.5f, 0.5f, 1.0f);
	g_pieceInfo.buttons[5].setTexture(g_textures[Game::TEX_2D_PAWN_WHITE]);
	g_pieceInfo.buttons[5].setLoaded(true);
	g_pieceInfo.buttons[5].setNum(5);

	// black rook
	g_pieceInfo.buttons[6].setSize(size, size);
	g_pieceInfo.buttons[6].setPos(width - size, (offset - size));
	g_pieceInfo.buttons[6].setColor(0.5f, 0.5f, 0.5f, 1.0f);
	g_pieceInfo.buttons[6].setTexture(g_textures[Game::TEX_2D_ROOK_WHITE]);
	g_pieceInfo.buttons[6].setLoaded(true);
	g_pieceInfo.buttons[6].setNum(6);

	// black knight
	g_pieceInfo.buttons[7].setSize(size, size);
	g_pieceInfo.buttons[7].setPos(width - size, (offset - (size * 2)));
	g_pieceInfo.buttons[7].setColor(0.5f, 0.5f, 0.5f, 1.0f);
	g_pieceInfo.buttons[7].setTexture(g_textures[Game::TEX_2D_KNIGHT_WHITE]);
	g_pieceInfo.buttons[7].setLoaded(true);
	g_pieceInfo.buttons[7].setNum(7);

	// black bishop
	g_pieceInfo.buttons[8].setSize(size, size);
	g_pieceInfo.buttons[8].setPos(width - size, (offset - (size * 3)));
	g_pieceInfo.buttons[8].setColor(0.5f, 0.5f, 0.5f, 1.0f);
	g_pieceInfo.buttons[8].setTexture(g_textures[Game::TEX_2D_BISHOP_WHITE]);
	g_pieceInfo.buttons[8].setLoaded(true);
	g_pieceInfo.buttons[8].setNum(8);

	// black queen
	g_pieceInfo.buttons[9].setSize(size, size);
	g_pieceInfo.buttons[9].setPos(width - size, (offset - (size * 4)));
	g_pieceInfo.buttons[9].setColor(0.5f, 0.5f, 0.5f, 1.0f);
	g_pieceInfo.buttons[9].setTexture(g_textures[Game::TEX_2D_QUEEN_WHITE]);
	g_pieceInfo.buttons[9].setLoaded(true);
	g_pieceInfo.buttons[9].setNum(9);
}

Button::Button()
{
	m_loaded = false;
}

Button::~Button()
{

}

void Button::render(bool bStatic)
{
	Graphics& graphics = Graphics::inst();

	if(graphics.getMode() != Graphics::ORTHO)
		return;

	int wd2 = graphics.getWidth() / 2;
	int hd2 = graphics.getHeight() / 2;
	int bwd2 = m_width / 2;
	//int x1 = wd2 - bwd2;
	//int x2 = wd2 + bwd2;
	int x1 = wd2 + m_x;
	int x2 = x1 + m_width;
	//int y1 = (m_height * 2) + m_y;	// lower y
	//int y2 = m_height + m_y;		// upper y
	int y1 = graphics.getHeight() - (graphics.getHeight() - m_y);
	int y2 = y1 + m_height;

	if(bStatic){
		if(m_num >= 5){ // this is a lot of bullshit
			x1 = graphics.getWidth() - 28;
			x2 = x1 + m_width;
		}
		else{
			x1 = 4;
			x2 = m_width + 4;
		}

		y1 = graphics.getHeight() - m_y;
		y2 = y1 + m_height;

		glColor4f(m_color[0], m_color[1], m_color[2], m_color[3]);
	}
	else{
		if(m_active)
			glColor4f(BUTTON_ACTIVE_COLOR[0], BUTTON_ACTIVE_COLOR[1], BUTTON_ACTIVE_COLOR[2], BUTTON_ACTIVE_COLOR[3]);
		else
			glColor4f(BUTTON_INACTIVE_COLOR[0], BUTTON_INACTIVE_COLOR[1], BUTTON_INACTIVE_COLOR[2], BUTTON_INACTIVE_COLOR[3]);
	}

	if(!m_enabled){
		glColor4f(0.3f, 0.3f, 0.3f, 1.0f);
	}

	// draw the coloured button first
	if(!bStatic){
		glBegin(GL_QUADS);
			glVertex2i(x1, y1);
			glVertex2i(x1, y2);
			glVertex2i(x2, y2);
			glVertex2i(x2, y1);
		glEnd();
	}

	// draw the text on the button
	glEnable(GL_TEXTURE_2D);
	if(bStatic){
		glColor4f(m_color[0], m_color[1], m_color[2], m_color[3]);
	}
	else{
		glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
	}
	
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glBegin(GL_QUADS);
		glTexCoord2i(1, 0); glVertex2i(x1, y1);
		glTexCoord2i(1, 1); glVertex2i(x1, y2);
		glTexCoord2i(0, 1); glVertex2i(x2, y2);
		glTexCoord2i(0, 0); glVertex2i(x2, y1);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void Button::testMouse(void)
{
	int x = Mouse::inst().xPosAbsolute();
	int y = Mouse::inst().yPosAbsolute();
	int wd2 = (Graphics::inst().getWidth() / 2);
	int hd2 = (Graphics::inst().getHeight() / 2);
	int height = hd2 - (hd2 - m_y);
	int width = wd2 + m_x;

	if((x > width && x < (width + m_width)) &&
		(y > height && y < (height + m_height))){
			m_active = true;

			if(Mouse::inst().buttonDown(Mouse::BUTTON_LEFT)){
				m_clicked = true;
			}
	}
	else{
		m_active = false;
		m_clicked = false;
	}
}

//=====================================================//

Menu::Menu()
{

}

Menu::Menu(unsigned numButtons)
{
	m_numButtons = numButtons;
	buttons = new Button[m_numButtons];
}

Menu::~Menu()
{
	delete[] buttons;
}

void Menu::render(void)
{
	extern bool g_enterKey;
	Game& game = Game::inst();
	Graphics& graphics = Graphics::inst();

	graphics.setMode(Graphics::ORTHO);

	if(m_blend){
		// enable blending
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}

	for(int i=0; i<m_numButtons; ++i){ // change constant value
		if(buttons[i].isLoaded()){
			buttons[i].render(m_static);
			if(!m_static){ // pause menu
				if(buttons[i].isEnabled()){
					buttons[i].testMouse();

					switch(i){
						case 0:
							if(buttons[0].isClicked()){
								g_enterKey = true;
							}
							break;

						case 1:
							if(buttons[1].isClicked()){

								game.newGame();

								if(!buttons[0].isEnabled()){
									buttons[0].enable(true);
								}
								if(!buttons[3].isEnabled()){
									buttons[3].enable(true);
								}
								keybd_event(VK_RETURN, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
							}
							break;

						case 2:
							if(buttons[2].isClicked()){
								game.loadFile();
							}
							break;

						case 3:
							if(buttons[3].isClicked()){
								game.saveFile();
							}
							break;

						case 4: // options
							if(buttons[4].isClicked()){
								DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG_OPTIONS), g_hWnd, OptionsProc);
							}
							break;

						case 5: // help
							if(buttons[5].isClicked()){
								//DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOG_HELP), NULL, HelpProc);
								//MessageBox(g_hWnd, "Test", 0, 0);
								HWND hDialog = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG_HELP), NULL, (DLGPROC)HelpProc);
								ShowWindow(hDialog, SW_SHOW);
							}
							break;

						case 6: // credits
							if(buttons[6].isClicked()){
								Game::inst().setState(Game::STATE_CREDITS);
							}
							break;

						case 7:
							if(buttons[7].isClicked()){
								Game::inst().exit();
							}
							break;

						default:
							break;
					}

					buttons[i].setClicked(false);
				}
			}
		}
	}

	if(m_blend){
		// disable blending
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}

	graphics.setMode(Graphics::PROJ);
}
