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

#include "particle.h"

extern float g_elapsedTimeSec;

Particle::Particle()
{
	randomize();
}

Particle::~Particle()
{

}

void Particle::randomize(void)
{
	// this will use the boundries set for this game
	//  in which the particles are moving downwards for the main menu

	// position
	m_pos.x = fRand(-50.0f, 50.0f);
	m_pos.y = fRand(10.5f, 50.0f);
	m_pos.z = fRand(-50.0f, 50.0f);

	// direction
	m_direction.x = fRand(-500.0f, 500.0f);
	m_direction.y = -300.0f;
	m_direction.z = fRand(-500.0f, 500.0f);

	// rotation
	m_rotation.x = fRand(0.0f, 1.0f);
	m_rotation.y = fRand(0.0f, 1.0f);
	m_rotation.z = fRand(0.0f, 1.0f);

	// angle
	m_angle = 0.0f;

	// speed
	m_speed = (fRand(0.1f, 0.5f) / 2.0f);

	// size
	m_size = fRand(1.0f, 2.0f);

	// set fade to none
	m_fade = 1.0f;
}

void Particle::update(void)
{
	float offset = (m_speed * g_elapsedTimeSec * 300);

	m_pos.y -= offset;

	m_fade -= (offset / 500.0f);

	m_angle += offset;

	if(m_fade <= 0.0f)
		randomize();
}