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

#include "mathlib.h"
#include "arcane_lib.h"

extern float g_elapsedTimeSec;

class Particle{
public:
	Particle();
	~Particle();

	// particle functions
	void randomize(void);
	void update(void);

	// getter functions
	const Vec3 getPosition(void) const;
	const Vec3 getRotation(void) const;
	const float getAngle(void) const;
	const float getSpeed(void) const;
	const float getSize(void) const;
	const float getFade(void) const;

	// setter functions


private:
	Vec3 m_pos;
	Vec3 m_direction;
	Vec3 m_rotation;
	float m_angle;
	float m_speed;
	float m_size;
	float m_fade;	// how much to blend from 0.0f to 1.0f
};

inline const Vec3 Particle::getPosition(void) const
{ return m_pos; }

inline const Vec3 Particle::getRotation(void) const
{ return m_rotation; }

inline const float Particle::getAngle(void) const
{ return m_angle; }

inline const float Particle::getSpeed(void) const
{ return m_speed * g_elapsedTimeSec; }

inline const float Particle::getSize(void) const
{ return m_size; }

inline const float Particle::getFade(void) const
{ return m_fade; }