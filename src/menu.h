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

#include "game.h"
#include "graphics.h"
#include "input.h"
#include "mathlib.h"

void InitMenu(void); // custom function for this game

class Button{
public:

	Button();
	~Button();

	// button functions
	void render(bool bStatic);
	void testMouse(void);

	// getter functions
	const int getX(void) const;
	const int getY(void) const;
	const int getWidth(void) const;
	const int getHeight(void) const;
	const bool isActive(void) const;
	const bool isClicked(void) const;
	const bool isLoaded(void) const;
	const bool isEnabled(void) const;

	// setter functions
	void setPos(int x, int y);
	void setSize(int width, int height);
	void setClicked(bool clicked);
	void setActive(bool active);
	void setColor(float r, float g, float b, float w);
	void setTexture(GLuint texture);
	void setLoaded(bool loaded);
	void setNum(int num);
	void enable(bool enable);

private:
	static const float BUTTON_ACTIVE_COLOR[4];
	static const float BUTTON_INACTIVE_COLOR[4];
	int	m_width, m_height;
	int m_x, m_y;
	float m_color[4];
	bool m_active;
	bool m_clicked;
	bool m_loaded;
	bool m_enabled;
	int  m_num;
	GLuint m_texture;
};

inline const int Button::getX(void) const
{ return m_x; }

inline const int Button::getY(void) const
{ return m_y; }

inline const int Button::getWidth(void) const
{ return m_width; }

inline const int Button::getHeight(void) const
{ return m_height; }

inline const bool Button::isActive(void) const
{ return m_active; }

inline const bool Button::isClicked(void) const
{ return m_clicked; }

inline const bool Button::isLoaded(void) const
{ return m_loaded; }

inline const bool Button::isEnabled(void) const
{ return m_enabled; }

inline void Button::setPos(int x, int y)
{ m_x = x; m_y = y; }

inline void Button::setSize(int width, int height)
{ m_width = width; m_height = height; }

inline void Button::setClicked(bool clicked)
{ m_clicked = clicked; }

inline void Button::setActive(bool active)
{ m_active = active; }

inline void Button::setColor(float r, float g, float b, float w)
{ m_color[0] = r; m_color[1] = g; m_color[2] = b; m_color[3] = w; }

inline void Button::setTexture(GLuint texture)
{ m_texture = texture; }

inline void Button::setLoaded(bool loaded)
{ m_loaded = loaded; }

inline void Button::setNum(int num)
{ m_num = num; }

inline void Button::enable(bool enable)
{ m_enabled = enable; }

//=====================================================//

class Menu{
public:
	Menu();
	Menu(unsigned numButtons);
	~Menu();

	Button* buttons;

	// menu functions
	void render(void);

	// setter functions
	void setStatic(bool bStatic);
	void setBlend(bool blend);

private:
	int m_numButtons;
	bool m_static;
	bool m_blend;
};

inline void Menu::setStatic(bool bStatic)
{ m_static = bStatic; }

inline void Menu::setBlend(bool blend)
{ m_blend = blend; }

// declare two menu classes for main menu and pause menu