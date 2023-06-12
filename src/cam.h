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

#define NOMINMAX

#include <algorithm>

#include "mathlib.h"
#include "graphics.h"
#include "input.h"

//==========================================================================//

class Cam
{
public:
	enum CamBehavior{
		CAM_BEHAVIOR_FIRST_PERSON,
		CAM_BEHAVIOR_THIRD_PERSON,
		CAM_BEHAVIOR_SPECTATOR,
		CAM_BEHAVIOR_FLIGHT,
		CAM_BEHAVIOR_ORBIT
	};

	Cam();
	~Cam();

	static Cam& defaultInst(void);
	static Cam& lastInst(void);

	// camera functions
	void init(void);
	void lookAt(const Vec3& target);
	void lookAt(const Vec3& eye, const Vec3& target, const Vec3& up);
	void move(float dx, float dy, float dz);
	void move(const Vec3& dir, const Vec3& amount);
	void perspective(float fovx, float aspect, float zNear, float zFar);
	void resetView(void);
	void rotate(float heading, float pitch, float roll);
	void rotateSmoothly(float heading, float pitch, float roll);
	void undoRoll(void);
	void updatePos(const Vec3& dir, float elapsedTimeSec);
	void zoom(float zoom, float min, float max);
	void changeCamBehavior(CamBehavior behavior);
	

	// retrieval functions
	const Vec3& getAcceleration(void) const;
	CamBehavior getBehavior(void) const;
	const Vec3& getCurrentVelocity(void) const;
	const Vec3& getCurrentPos(void) const;
	float getOrbitMinZoom(void) const;
	float getOrbitMaxZoom(void) const;
	float getOrbitOffsetDistance(void) const;
	float getOrbitPitchMinDegrees(void) const;
	float getOrbitPitchMaxDegrees(void) const;
	const Quaternion &getOrientation(void) const;
    float getRotationSpeed(void) const;
    const Matrix4& getProjectionMatrix(void) const;
    const Vec3& getVelocity(void) const;
    const Vec3& getViewDirection(void) const;
    const Matrix4& getViewMatrix(void) const;
    const Matrix4& getViewProjectionMatrix(void) const;
    const Vec3& getXAxis(void) const;
    const Vec3& getYAxis(void) const;
    const Vec3& getZAxis(void) const;
    bool preferTargetYAxisOrbiting(void) const;
	bool isLocked(void) const;
    
    // setting functions
    void setAcceleration(const Vec3 &acceleration);
    void setBehavior(CamBehavior newBehavior);
    void setCurrentVelocity(const Vec3 &currentVelocity);
    void setCurrentVelocity(float x, float y, float z);
    void setOrbitMaxZoom(float orbitMaxZoom);
    void setOrbitMinZoom(float orbitMinZoom);
    void setOrbitOffsetDistance(float orbitOffsetDistance);
    void setOrbitPitchMaxDegrees(float orbitPitchMaxDegrees);
    void setOrbitPitchMinDegrees(float orbitPitchMinDegrees);
    void setOrientation(const Quaternion &newOrientation);
    void setPosition(const Vec3 &newEye);
    void setPreferTargetYAxisOrbiting(bool preferTargetYAxisOrbiting);
    void setRotationSpeed(float rotationSpeed);
    void setVelocity(const Vec3 &velocity);
    void setVelocity(float x, float y, float z);
	void lock(bool locked);

	// debugging
	void printInfo(void);

private:
	void rotateFirstPerson(float heading, float pitch);
	void rotateFlight(float heading, float pitch, float roll);
	void rotateOrbit(float heading, float pitch, float roll);
	void updateVelocity(const Vec3& dir, float elapsedTimeSec);
	void updateViewMatrix(void);

	// constants
	static const float DEFAULT_ROTATION_SPEED;
	static const float DEFAULT_FOVX;
    static const float DEFAULT_ZNEAR;
    static const float DEFAULT_ZFAR;
    static const float DEFAULT_ORBIT_MIN_ZOOM;
    static const float DEFAULT_ORBIT_MAX_ZOOM;
    static const float DEFAULT_ORBIT_OFFSET_DISTANCE;
	static const Vec3 DEFAULT_ORBIT_CAM_POS;
    static const Vec3 WORLD_XAXIS;
    static const Vec3 WORLD_YAXIS;
    static const Vec3 WORLD_ZAXIS;

	// member variables
	CamBehavior m_behavior;
	bool m_locked;
	bool m_preferTargetYAxisOrbiting;
	float m_accumPitchDegrees;
    float m_savedAccumPitchDegrees;
    float m_rotationSpeed;
    float m_fovx;
    float m_aspectRatio;
    float m_znear;
    float m_zfar;
    float m_orbitMinZoom;
    float m_orbitMaxZoom;
    float m_orbitOffsetDistance;
    float m_firstPersonYOffset;
    Vec3 m_eye;
    Vec3 m_savedEye;
    Vec3 m_target;
    Vec3 m_targetYAxis;
    Vec3 m_xAxis;
    Vec3 m_yAxis;
    Vec3 m_zAxis;
    Vec3 m_viewDir;
    Vec3 m_acceleration;
    Vec3 m_currentVelocity;
    Vec3 m_velocity;
    Quaternion m_orientation;
    Quaternion m_savedOrientation;
    Matrix4 m_viewMatrix;
    Matrix4 m_projMatrix;
    Matrix4 m_viewProjMatrix;
	
};

inline const Vec3 &Cam::getAcceleration() const
{ return m_acceleration; }

inline Cam::CamBehavior Cam::getBehavior() const
{ return m_behavior; }

inline const Vec3 &Cam::getCurrentVelocity() const
{ return m_currentVelocity; }

inline const Vec3 &Cam::getCurrentPos() const
{ return m_eye; }

inline float Cam::getOrbitMinZoom() const
{ return m_orbitMinZoom; }

inline float Cam::getOrbitMaxZoom() const
{ return m_orbitMaxZoom; }

inline float Cam::getOrbitOffsetDistance() const
{ return m_orbitOffsetDistance; }

inline const Quaternion &Cam::getOrientation() const
{ return m_orientation; }

inline float Cam::getRotationSpeed() const
{ return m_rotationSpeed; }

inline const Matrix4 &Cam::getProjectionMatrix() const
{ return m_projMatrix; }

inline const Vec3 &Cam::getVelocity() const
{ return m_velocity; }

inline const Vec3 &Cam::getViewDirection() const
{ return m_viewDir; }

inline const Matrix4 &Cam::getViewMatrix() const
{ return m_viewMatrix; }

inline const Matrix4 &Cam::getViewProjectionMatrix() const
{ return m_viewProjMatrix; }

inline const Vec3 &Cam::getXAxis() const
{ return m_xAxis; }

inline const Vec3 &Cam::getYAxis() const
{ return m_yAxis; }

inline const Vec3 &Cam::getZAxis() const
{ return m_zAxis; }

inline bool Cam::preferTargetYAxisOrbiting() const
{ return m_preferTargetYAxisOrbiting; }

inline bool Cam::isLocked(void) const
{ return m_locked; }

inline void Cam::lock(bool locked)
{
	m_locked = locked;
}
