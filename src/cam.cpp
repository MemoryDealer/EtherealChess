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

#define NOMINXMAX

#include "cam.h"

const float Cam::DEFAULT_ROTATION_SPEED = 0.3f;
const float Cam::DEFAULT_FOVX = 90.0f;
const float Cam::DEFAULT_ZNEAR = 0.1f;
const float Cam::DEFAULT_ZFAR = 500.0f;

const float Cam::DEFAULT_ORBIT_MIN_ZOOM = DEFAULT_ZNEAR + 1.0f;
const float Cam::DEFAULT_ORBIT_MAX_ZOOM = DEFAULT_ZFAR * 0.5f;

const float Cam::DEFAULT_ORBIT_OFFSET_DISTANCE = DEFAULT_ORBIT_MIN_ZOOM +
    (DEFAULT_ORBIT_MAX_ZOOM - DEFAULT_ORBIT_MIN_ZOOM) * 0.25f;

const Vec3	Cam::DEFAULT_ORBIT_CAM_POS(0.0f, 0.4f, 0.0f);

const Vec3 Cam::WORLD_XAXIS(1.0f, 0.0f, 0.0f);
const Vec3 Cam::WORLD_YAXIS(0.0f, 1.0f, 0.0f);
const Vec3 Cam::WORLD_ZAXIS(0.0f, 0.0f, 1.0f);

const float     CAMERA_FOVX = 90.0f;
const float     CAMERA_ZFAR = 5000.0f;
const float     CAMERA_ZNEAR = 0.1f;
const float     CAMERA_ZOOM_MAX = 10.0f;
const float     CAMERA_ZOOM_MIN = 1.5f;

const float     CAMERA_SPEED_FLIGHT_YAW = 100.0f;
const float     CAMERA_SPEED_ORBIT_ROLL = 100.0f;

const Vector3   CAMERA_ACCELERATION(3.0f, 3.0f, 3.0f);
const Vector3   CAMERA_VELOCITY(2.0f, 2.0f, 2.0f);

Cam::Cam()
{
	m_behavior = CAM_BEHAVIOR_FIRST_PERSON;
	m_preferTargetYAxisOrbiting = true;

	m_accumPitchDegrees = 0.0f;
	m_savedAccumPitchDegrees = 0.0f;

	m_rotationSpeed = DEFAULT_ROTATION_SPEED;
	m_fovx = DEFAULT_FOVX;
	m_aspectRatio = (static_cast<float>(Graphics::inst().getWidth()) / static_cast<float>(Graphics::inst().getHeight()));
	m_znear = DEFAULT_ZNEAR;
	m_zfar = DEFAULT_ZFAR;

	m_orbitMinZoom = DEFAULT_ORBIT_MIN_ZOOM;
	m_orbitMaxZoom = DEFAULT_ORBIT_MAX_ZOOM;
	m_orbitOffsetDistance = DEFAULT_ORBIT_OFFSET_DISTANCE;
        
    m_eye.set(0.0f, 0.0f, 0.0f);
    m_savedEye.set(0.0f, 0.0f, 0.0f);
    m_target.set(0.0f, 0.0f, 0.0f);
    m_xAxis.set(1.0f, 0.0f, 0.0f);
    m_yAxis.set(0.0f, 1.0f, 0.0f);
    m_targetYAxis.set(0.0f, 1.0f, 0.0f);
    m_zAxis.set(0.0f, 0.0f, 1.0f);
    m_viewDir.set(0.0f, 0.0f, -1.0f);
        
    m_acceleration.set(4.0f, 4.0f, 4.0f);
    m_currentVelocity.set(1.0f, 1.0f, 1.0f);
    m_velocity.set(1.0f, 1.0f, 1.0f);

    m_orientation.identity();
    m_savedOrientation.identity();

    m_viewMatrix.identity();
    m_projMatrix.identity();
    m_viewProjMatrix.identity();
}

Cam::~Cam()
{

}

Cam& Cam::defaultInst(void)
{
	static Cam inst;
	return inst;
}

Cam& Cam::lastInst(void)
{
	static Cam lastInst;
	return lastInst;
}

void Cam::init(void)
{
	this->perspective(CAMERA_FOVX,
		static_cast<float>(Graphics::inst().getWidth()) / static_cast<float>(Graphics::inst().getHeight()),
		CAMERA_ZNEAR, CAMERA_ZFAR);

	this->setPosition(DEFAULT_ORBIT_CAM_POS);
	this->setOrbitMinZoom(CAMERA_ZOOM_MIN);
	this->setOrbitMaxZoom(CAMERA_ZOOM_MAX);
	this->setOrbitOffsetDistance(CAMERA_ZOOM_MIN + (CAMERA_ZOOM_MAX - CAMERA_ZOOM_MIN) * 0.3f);

	this->setAcceleration(CAMERA_ACCELERATION);
	this->setVelocity(CAMERA_VELOCITY);

	this->changeCamBehavior(CAM_BEHAVIOR_ORBIT);

	this->rotateSmoothly(0.0f, -45.0f, 0.0f);
	this->zoom(3.0f, CAMERA_ZOOM_MIN, CAMERA_ZOOM_MAX);

	// change camera behavior
	Mouse::inst().hideCursor(false);
	Mouse::inst().setPosition(Graphics::inst().getWidth() / 2, Graphics::inst().getHeight() / 2);

	// save the default instance for resetting the camera (a simple solution)
	memcpy(&this->defaultInst(), this, sizeof(Cam));
}

void Cam::resetView(void)
{
	memcpy(this, &this->defaultInst(), sizeof(Cam));
}

void Cam::lookAt(const Vec3& target)
{
	lookAt(m_eye, target, m_yAxis);
}

// basically gluLookAt()
void Cam::lookAt(const Vec3& eye, const Vec3& target, const Vec3& up)
{
	m_eye = eye;
	m_target = target;

	m_zAxis = eye - target;
	m_zAxis.normalize();

	m_yAxis = Vec3::cross(m_zAxis, m_xAxis);
	m_xAxis.normalize();

	m_viewMatrix[0][0] = m_xAxis.x;
	m_viewMatrix[1][0] = m_xAxis.y;
	m_viewMatrix[2][0] = m_xAxis.z;
	m_viewMatrix[3][0] = -Vec3::dot(m_xAxis, eye);

	m_viewMatrix[0][1] = m_yAxis.x;
	m_viewMatrix[1][1] = m_yAxis.y;
	m_viewMatrix[2][1] = m_yAxis.z;
	m_viewMatrix[3][1] = -Vec3::dot(m_yAxis, eye);

	m_viewMatrix[0][2] = m_zAxis.x;
	m_viewMatrix[1][2] = m_zAxis.y;
	m_viewMatrix[2][2] = m_zAxis.z;
	m_viewMatrix[3][2] = -Vec3::dot(m_zAxis, eye);

	m_accumPitchDegrees = Math::radiansToDegrees(asinf(m_viewMatrix[1][2]));

	m_orientation.fromMatrix(m_viewMatrix);
	updateViewMatrix();
}

void Cam::move(float dx, float dy, float dz)
{
	if(m_behavior == CAM_BEHAVIOR_ORBIT){
		return;
	}

	Vec3 eye = m_eye;
	Vec3 forwards;

	if(m_behavior == CAM_BEHAVIOR_FIRST_PERSON){
		// calculate the forward direction to prevent slower movement when looking up or down
		forwards = Vec3::cross(WORLD_YAXIS, m_xAxis);
		forwards.normalize();
	}
	else{
		forwards = m_viewDir;
	}

	eye += m_xAxis * dx;
	eye += WORLD_YAXIS * dy;
	eye += forwards * dz;

	setPosition(eye);
}

void Cam::move(const Vec3& dir, const Vec3& amount)
{
	if(m_behavior == CAM_BEHAVIOR_ORBIT){
		return;
	}

	m_eye.x += dir.x * amount.x;
	m_eye.y += dir.y * amount.y;
	m_eye.z += dir.z * amount.z;

	updateViewMatrix();
}

void Cam::perspective(float fovx, float aspect, float zNear, float zFar)
{
	// construct projection matrix based on horizontal view

	float e = 1.0f / tanf(Math::degreesToRadians(fovx) / 2.0f);
	float aspectInv = 1.0f / aspect;
	float fovy = 2.0f * atanf(aspectInv / e);
	float xScale = 1.0f / tanf(0.5f * fovy);
	float yScale = xScale / aspectInv;

	m_projMatrix[0][0] = xScale;
	m_projMatrix[0][1] = 0.0f;
	m_projMatrix[0][2] = 0.0f;
	m_projMatrix[0][3] = 0.0f;

	m_projMatrix[1][0] = 0.0f;
	m_projMatrix[1][1] = yScale;
	m_projMatrix[1][2] = 0.0f;
	m_projMatrix[1][3] = 0.0f;

	m_projMatrix[2][0] = 0.0f;
	m_projMatrix[2][1] = 0.0f;
	m_projMatrix[2][2] = (zFar + zNear) / (zNear - zFar);
	m_projMatrix[2][3] = -1.0f;

	m_projMatrix[3][0] = 0.0f;
	m_projMatrix[3][1] = 0.0f;
	m_projMatrix[3][2] = (2.0f * zFar * zNear) / (zNear - zFar);
	m_projMatrix[3][3] = 0.0f;

	m_viewProjMatrix = m_viewMatrix * m_projMatrix;

	m_fovx = fovx;
	m_aspectRatio = aspect;
	m_znear = zNear;
	m_zfar = zFar;
}

void Cam::rotate(float heading, float pitch, float roll)
{
	pitch = -pitch;
	heading = -heading;
	roll = -roll;

	switch(m_behavior){
		case CAM_BEHAVIOR_FIRST_PERSON:
		case CAM_BEHAVIOR_SPECTATOR:
			rotateFirstPerson(heading, pitch);
			break;

		case CAM_BEHAVIOR_FLIGHT:
			rotateFlight(heading, pitch, roll);
			break;

		case CAM_BEHAVIOR_ORBIT:
			rotateOrbit(heading, pitch, roll);
			break;

		default:
			break;
	}

	updateViewMatrix();
}

void Cam::rotateSmoothly(float heading, float pitch, float roll)
{
	heading *= m_rotationSpeed;
	pitch *= m_rotationSpeed;
	roll *= m_rotationSpeed;

	rotate(heading, pitch, roll);
}

void Cam::undoRoll(void)
{
	// level the camera

	if(m_behavior == CAM_BEHAVIOR_ORBIT)
		lookAt(m_eye, m_target, m_targetYAxis);
	else
		lookAt(m_eye, m_eye + m_viewDir, WORLD_YAXIS);
}

void Cam::updatePos(const Vec3& dir, float elapsedTimeSec)
{


	// Newton's 2nd law

	if(m_currentVelocity.magnitudeSq() != 0.0f){
		// keep the camera from incorrectly moving due to floating point rounding errors

		Vec3 disp = (m_currentVelocity * elapsedTimeSec) + 
			(0.5f * m_acceleration * elapsedTimeSec * elapsedTimeSec);

		if(dir.x == 0.0f && Math::closeEnough(m_currentVelocity.x, 0.0f))
			disp.x = 0.0f;

		if(dir.y == 0.0f && Math::closeEnough(m_currentVelocity.y, 0.0f))
			disp.y = 0.0f;

		if(dir.z == 0.0f && Math::closeEnough(m_currentVelocity.z, 0.0f))
			disp.z = 0.0f;

		move(disp.x, disp.y, disp.z);
	}

	updateVelocity(dir, elapsedTimeSec);
}

void Cam::zoom(float zoom, float min, float max)
{
	if(m_behavior == CAM_BEHAVIOR_ORBIT){
		m_orbitMaxZoom = max;
		m_orbitMinZoom = min;

		Vec3 offset = m_eye - m_target;

		m_orbitOffsetDistance = offset.magnitude();
		offset.normalize();
		m_orbitOffsetDistance += zoom;
		m_orbitOffsetDistance = std::min(std::max(m_orbitOffsetDistance, min), max);

		offset *= m_orbitOffsetDistance;
		m_eye = offset + m_target;

		updateViewMatrix();
	}
	else{
		zoom = std::min(std::max(zoom, min), max);
		perspective(zoom, m_aspectRatio, m_znear, m_zfar);
	}
}

void Cam::changeCamBehavior(CamBehavior behavior)
{
	if(this->getBehavior() == behavior){
		return;
	}

	/*if(behavior == CAM_BEHAVIOR_ORBIT){
		meshPosition = this->getCurrentPos();
		meshOrientation = this->getOrientation().inverse();
	}*/

	this->setBehavior(behavior);

	if(behavior == CAM_BEHAVIOR_ORBIT){
		this->rotate(0.0f, -30.0f, 0.0f);
	}
}

void Cam::setAcceleration(const Vec3& acceleration)
{
	m_acceleration = acceleration;
}

void Cam::setBehavior(CamBehavior newBehavior)
{
    CamBehavior prevBehavior = m_behavior;

    if (prevBehavior == newBehavior)
        return;

    m_behavior = newBehavior;

    switch (newBehavior)
    {
    case CAM_BEHAVIOR_FIRST_PERSON:
        switch (prevBehavior)
        {
        default:
            break;

        case CAM_BEHAVIOR_FLIGHT:
            m_eye.y = m_firstPersonYOffset;
            updateViewMatrix();
            break;

        case CAM_BEHAVIOR_SPECTATOR:
            m_eye.y = m_firstPersonYOffset;
            updateViewMatrix();
            break;

        case CAM_BEHAVIOR_ORBIT:
            m_eye.x = m_savedEye.x;
            m_eye.z = m_savedEye.z;
            m_eye.y = m_firstPersonYOffset;
            m_orientation.identity();
            m_accumPitchDegrees = m_savedAccumPitchDegrees;
            updateViewMatrix();
            break;
        }

        undoRoll();
        break;

    case CAM_BEHAVIOR_SPECTATOR:
        switch (prevBehavior)
        {
        default:
            break;

        case CAM_BEHAVIOR_FLIGHT:
            updateViewMatrix();
            break;

        case CAM_BEHAVIOR_ORBIT:
            //m_eye = Vec3(0.0f, 10.0f, 0.0f);//m_savedEye;
            //m_orientation = m_savedOrientation;
            //m_accumPitchDegrees = m_savedAccumPitchDegrees;
            updateViewMatrix();
            break;
        }

        undoRoll();
        break;
    
    case CAM_BEHAVIOR_FLIGHT:
        if (prevBehavior == CAM_BEHAVIOR_ORBIT)
        {
            m_eye = m_savedEye;
            m_orientation = m_savedOrientation;
            m_accumPitchDegrees = m_savedAccumPitchDegrees;
            updateViewMatrix();
        }
        else
        {
            m_savedEye = m_eye;
            updateViewMatrix();
        }
        break;
    
    case CAM_BEHAVIOR_ORBIT:
        if (prevBehavior == CAM_BEHAVIOR_FIRST_PERSON)
            m_firstPersonYOffset = m_eye.y;

        m_savedEye = m_eye;
        m_savedOrientation = m_orientation;
        m_savedAccumPitchDegrees = m_accumPitchDegrees;
        
        m_targetYAxis = m_yAxis;

        Vector3 newEye = m_eye + m_zAxis * m_orbitOffsetDistance;
        Vector3 newTarget = m_eye;
        
        lookAt(newEye, newTarget, m_targetYAxis);
        break;
    }
}

void Cam::setCurrentVelocity(const Vector3 &currentVelocity)
{
    m_currentVelocity = currentVelocity;
}

void Cam::setCurrentVelocity(float x, float y, float z)
{
    m_currentVelocity.set(x, y, z);
}

void Cam::setOrbitMaxZoom(float orbitMaxZoom)
{
    m_orbitMaxZoom = orbitMaxZoom;
}

void Cam::setOrbitMinZoom(float orbitMinZoom)
{
    m_orbitMinZoom = orbitMinZoom;
}

void Cam::setOrbitOffsetDistance(float orbitOffsetDistance)
{
    m_orbitOffsetDistance = orbitOffsetDistance;
}

void Cam::setOrientation(const Quaternion &newOrientation)
{
    Matrix4 m = newOrientation.toMatrix4();

    m_accumPitchDegrees = Math::radiansToDegrees(asinf(m[1][2]));

    m_orientation = newOrientation;

    if (m_behavior == CAM_BEHAVIOR_FIRST_PERSON || m_behavior == CAM_BEHAVIOR_SPECTATOR)
        lookAt(m_eye, m_eye + m_viewDir, WORLD_YAXIS);

    updateViewMatrix();
}

void Cam::setPosition(const Vector3 &newEye)
{
    m_eye = newEye;
    updateViewMatrix();
}

void Cam::setPreferTargetYAxisOrbiting(bool preferTargetYAxisOrbiting)
{
    m_preferTargetYAxisOrbiting = preferTargetYAxisOrbiting;

    if (m_preferTargetYAxisOrbiting)
        undoRoll();
}

void Cam::setRotationSpeed(float rotationSpeed)
{
    m_rotationSpeed = rotationSpeed;
}

void Cam::setVelocity(const Vector3 &velocity)
{
    m_velocity = velocity;
}

void Cam::setVelocity(float x, float y, float z)
{
    m_velocity.set(x, y, z);
}

void Cam::rotateFirstPerson(float heading, float pitch)
{
	m_accumPitchDegrees += pitch;

	if(m_accumPitchDegrees > 90.0f){
		pitch = 90.0f - (m_accumPitchDegrees - pitch);
		m_accumPitchDegrees = 90.0f;
	}

	if(m_accumPitchDegrees < -90.0f){
		pitch = -90.0f - (m_accumPitchDegrees - pitch);
		m_accumPitchDegrees = -90.0f;
	}

	Quaternion rot;

	if(heading != 0.0f){
		rot.fromAxisAngle(WORLD_YAXIS, heading);
		m_orientation = rot * m_orientation;
	}

	if(pitch != 0.0f){
		rot.fromAxisAngle(WORLD_XAXIS, pitch);
		m_orientation = m_orientation * rot;
	}
}

void Cam::rotateFlight(float headingDegrees, float pitchDegrees, float rollDegrees)
{
    // Implements the rotation logic for the flight style camera behavior.

    m_accumPitchDegrees += pitchDegrees;

    if (m_accumPitchDegrees > 360.0f)
        m_accumPitchDegrees -= 360.0f;

    if (m_accumPitchDegrees < -360.0f)
        m_accumPitchDegrees += 360.0f;
   
    Quaternion rot;

    rot.fromHeadPitchRoll(headingDegrees, pitchDegrees, rollDegrees);
    m_orientation *= rot;
}

void Cam::rotateOrbit(float headingDegrees, float pitchDegrees, float rollDegrees)
{  
    Quaternion rot;

    if (m_preferTargetYAxisOrbiting)
    {
        if (headingDegrees != 0.0f)
        {
            rot.fromAxisAngle(m_targetYAxis, headingDegrees);
            m_orientation = rot * m_orientation;
        }

        if (pitchDegrees != 0.0f)
        {
            rot.fromAxisAngle(WORLD_XAXIS, pitchDegrees);
            m_orientation = m_orientation * rot;
        }
    }
    else
    {
        rot.fromHeadPitchRoll(headingDegrees, pitchDegrees, rollDegrees);
        m_orientation *= rot;
    }
}

void Cam::updateVelocity(const Vec3& dir, float elapsedTimeSec)
{
	if(dir.x != 0.0f){
		m_currentVelocity.x += dir.x * m_acceleration.x * elapsedTimeSec;

        if (m_currentVelocity.x > m_velocity.x)
            m_currentVelocity.x = m_velocity.x;
        else if (m_currentVelocity.x < -m_velocity.x)
            m_currentVelocity.x = -m_velocity.x;
	}
	else{
		if (m_currentVelocity.x > 0.0f)
        {
            if ((m_currentVelocity.x -= m_acceleration.x * elapsedTimeSec) < 0.0f)
                m_currentVelocity.x = 0.0f;
        }
        else
        {
            if ((m_currentVelocity.x += m_acceleration.x * elapsedTimeSec) > 0.0f)
                m_currentVelocity.x = 0.0f;
        }
	}

	if(dir.y != 0.0f){
		m_currentVelocity.y += dir.y * m_acceleration.y * elapsedTimeSec;
        
        if (m_currentVelocity.y > m_velocity.y)
            m_currentVelocity.y = m_velocity.y;
        else if (m_currentVelocity.y < -m_velocity.y)
            m_currentVelocity.y = -m_velocity.y;
	}
	else{
		if (m_currentVelocity.y > 0.0f)
        {
            if ((m_currentVelocity.y -= m_acceleration.y * elapsedTimeSec) < 0.0f)
                m_currentVelocity.y = 0.0f;
        }
        else
        {
            if ((m_currentVelocity.y += m_acceleration.y * elapsedTimeSec) > 0.0f)
                m_currentVelocity.y = 0.0f;
        }
	}

	if(dir.z != 0.0f){
		m_currentVelocity.z += dir.z * m_acceleration.z * elapsedTimeSec;

		if(m_currentVelocity.z > m_velocity.z)
			m_currentVelocity.z = m_velocity.z;
		else if(m_currentVelocity.z < -m_velocity.z)
			m_currentVelocity.z = -m_velocity.z;
	}
	else{
		if(m_currentVelocity.z > 0.0f){
			if((m_currentVelocity.z -= m_acceleration.z * elapsedTimeSec) < 0.0f)
				m_currentVelocity.z = 0.0f;
		}
		else{
			if((m_currentVelocity.z += m_acceleration.z * elapsedTimeSec) > 0.0f)
				m_currentVelocity.z = 0.0f;
		}
	}
}

void Cam::updateViewMatrix(void)
{
	m_viewMatrix = m_orientation.toMatrix4();

	m_xAxis.set(m_viewMatrix[0][0], m_viewMatrix[1][0], m_viewMatrix[2][0]);
	m_yAxis.set(m_viewMatrix[0][1], m_viewMatrix[1][1], m_viewMatrix[2][1]);
	m_zAxis.set(m_viewMatrix[0][2], m_viewMatrix[1][2], m_viewMatrix[2][2]);
	m_viewDir = -m_zAxis;

	if(m_behavior == CAM_BEHAVIOR_ORBIT){
		m_eye = m_target + m_zAxis * m_orbitOffsetDistance;
	}

	m_viewMatrix[3][0] = -Vec3::dot(m_xAxis, m_eye);
	m_viewMatrix[3][1] = -Vec3::dot(m_yAxis, m_eye);
	m_viewMatrix[3][2] = -Vec3::dot(m_zAxis, m_eye);
}

void Cam::printInfo(void)
{
	printf("<-- Cam -->\n");
	printf("Eye: <%.2f, %.2f, %.2f>\nTarget: <%.2f, %.2f, %.2f>\n", m_eye.x, m_eye.y, m_eye.z, m_target.x, m_target.y, m_target.z);
	printf("Orientation: <%.2f, %.2f, %.2f, %.2f>\n<----->\n", m_orientation.x, m_orientation.y, m_orientation.z, m_orientation.w);
	printf("Matrix: %.2f, %.2f, %.2f\n", m_viewMatrix[0][1], m_viewMatrix[1][1], m_viewMatrix[2][1]);
}