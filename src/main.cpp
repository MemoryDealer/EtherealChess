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

#if !defined(WIN32_LEAN_AND_MEAN)
	#define WIN32_LEAN_AND_MEAN
#endif

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Windows/C++ headers
#include <Windows.h>
#include <cassert>
#include <string>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <iostream>

// OpenGL headers
#include <GL/GL.h>
#include <GL/GLU.h>

// Debugging headers
#if defined(_DEBUG)
	#include <crtdbg.h>
#endif

// Project headers
#include "game.h"
#include "cam.h"
#include "graphics.h"
#include "menu.h"
#include "model.h"
#include "particle.h"
#include "shader.h"
#include "sound.h"
#include "config.h"
#include "font.h"
#include "texFont.h"
#include "arcane_lib.h"
#include "GL_ARB_multitexture.h"
#include "resource.h"

//==========================================================================//
// Constants
//==========================================================================//

const char*		WINDOW_TITLE = "Ethereal Chess - Alpha Build";

const float     CAMERA_FOVX = 90.0f;
const float     CAMERA_ZFAR = 5000.0f;
const float     CAMERA_ZNEAR = 0.1f;
const float     CAMERA_SPEED_ORBIT_ROLL = 100.0f;
const float		SKYBOX_SIZE = 1500.0f;

const Vec3		DEFAULT_ORBIT_CAM_POS(0.0f, 0.2f, 0.0f);

//==========================================================================//
// Global variables
//==========================================================================//
// misc. primitives
GLuint			g_textures[30];
GLuint			g_texFonts[5];
bool			g_hasFocus;
bool			g_fullscreen;
bool			g_enterKey;
bool			g_displayPieceInfo = true;
bool			g_sphereMap;
float			g_elapsedTimeSec;
unsigned		g_numBoards = 25;		// for main menu particles
char			g_arbText[1024];		// arbitrary text
bool			g_displayArbText;
char			g_workingDir[MAX_PATH];

// windows types
HWND			g_hWnd;
HINSTANCE		g_hInst;
HDC				g_hDC;
HGLRC			g_hRC;

// game objects
Cam				g_cam;
Quaternion		g_meshOrientation;
Vec3			g_meshPosition;
GLFont			g_font;
ModelOBJ		g_models[8];
Particle*		g_boardParticles;
Particle		g_snow[100];
Shader			g_shader;
Music			g_music;
ConfigParser*	g_config;
Menu			g_pauseMenu(8), g_pieceInfo(10);
GLuint			g_boardList, g_boardFrame, g_boardBox, g_defModels[6];

//==========================================================================//
// Function prototypes
//==========================================================================//

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd);
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void SwitchFullscreen(void);
void Cleanup(void);

// rendering
void RenderFrame(void);
void RenderMenu(void);
void RenderModel(unsigned int n);
void RenderPieces(bool reflection);
void Render2DModel(int board);
void RenderDefaultModel(unsigned int n);
void RenderBoard(bool fullBoard);
void RenderBoardBox(void);
void RenderCredits(void);
void RenderSkybox(void);
void RenderPlanet(void);
void RenderText(GLFont& font, std::ostringstream& o, int x, int y, float color[3]);
void RenderPieceInfoText(void);
void RenderText(void);
void DisplayArbitraryText(const char* str, DWORD time);
void UpdateFrame(void);
void UpdateCam(float elapsedTimeSec);
void GetMovementDirection(Vector3 &direction);
void DetectCollision(void);
void ProcessUserInput(void);

//==========================================================================//
// Functions
//==========================================================================//

// rendering functions
void RenderFrame(void)
{
	Game& game = Game::inst();
	Graphics& graphics = Graphics::inst();
	Vec3 pos = g_cam.getCurrentPos();
	const float modelLightPos[4] = {pos.x, pos.y, pos.z, 1.0f};
	const float modelAmbient[4] = {0.0f, 0.0f, 0.0f, 1.0f};

	glViewport(0, 0, graphics.getWidth(), graphics.getHeight());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMultMatrixf(&g_cam.getProjectionMatrix()[0][0]);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(&g_cam.getViewMatrix()[0][0]);

	glEnable(GL_DEPTH_TEST);
	if(graphics.cullFace()){
		glEnable(GL_CULL_FACE);
	}
	glEnable(GL_TEXTURE_2D);

	// check what needs to be rendered
	if(game.getState() == Game::STATE_PAUSED ||
		game.getState() == Game::STATE_LOADING ||
		game.getState() == Game::STATE_CREDITS){
		RenderMenu(); // render main menu, not in loading mode
		return;
	}

	// render the model lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	//glPushAttrib(GL_LIGHTING_BIT);

	glLightfv(GL_LIGHT0, GL_POSITION, modelLightPos);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, modelAmbient);
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, 1.0f);

	// shader
	if(g_shader.isEnabled()){
		g_shader.uniform1("tex0", 0);
	}

	glColor3f(1.0f, 1.0f, 1.0f);

	// render reflected pieces if enabled
	if(graphics.useReflection()){
		double eqr[] = {0.0f, -1.0f, 0.0f, 0.0f};

		glColorMask(false, false, false, false);

		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 1, 1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glDisable(GL_DEPTH_TEST);

		/*if(game.getChessSet() == Game::SET_NATIVE ||
			game.getChessSet() == Game::SET_FANCY){
			glCullFace(GL_BACK);
		}
		else{
			glCullFace(GL_FRONT);
		}*/
		glDisable(GL_CULL_FACE);

		RenderBoard(true); // draw the board into stencil buffer

		glEnable(GL_DEPTH_TEST);
		glColorMask(true, true, true, true);
		glStencilFunc(GL_EQUAL, 1, 1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glEnable(GL_CLIP_PLANE0);
		glClipPlane(GL_CLIP_PLANE0, eqr);

		RenderPieces(true); // draw clipped pieces into stencil buffer

		glDisable(GL_CLIP_PLANE0);
		glDisable(GL_STENCIL_TEST);
		glLightfv(GL_LIGHT0, GL_POSITION, modelLightPos);
		glEnable(GL_BLEND);
		glDisable(GL_LIGHTING);
		glColor4f(1.0f, 1.0f, 1.0f, 0.85f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if(game.getChessSet() == Game::SET_NATIVE ||
			game.getChessSet() == Game::SET_FANCY){
			if(graphics.cullFace()){
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
			}
		}

		RenderBoard(true);
		RenderBoardBox();

		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING);

		RenderPieces(false);
	}
	// or just render the pieces themselves
	else{
		glLightfv(GL_LIGHT0, GL_POSITION, modelLightPos);

		if(game.getChessSet() == Game::SET_NATIVE ||
			game.getChessSet() == Game::SET_FANCY){
			if(graphics.cullFace()){
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
			}
		}

		RenderBoard(true);
		RenderBoardBox();

		RenderPieces(false);
	}

	RenderPlanet();

	// render skybox/backgrounds
	if(graphics.cullFace()){
		glCullFace(GL_FRONT);
	}
	
	// detect selection change
	if(game.allowSelectionChange() && game.drawSelection()){
		Vec3 mouse3D = Graphics::inst().getMouseVec3();
		float y = mouse3D.x; // lol?
		float x = mouse3D.z; // ...
		int selectionX, selectionY;

		//printf("Mouse at (%.2f, %.2f)\n", x, y);
		if( !((y < -3.55f || y > 3.55f) || (x < -2.0f || x > 2.0f)) ){
			selectionX = (x < -1.50f) ? 1 : (x < -1.0f) ? 2 : (x < -0.5f) ? 3 : (x < 0.0f) ? 4 :
				(x < 0.5f) ? 5 : (x < 1.0f) ? 6 : (x < 1.5f) ? 7 : 8;
			selectionY = (y < -2.70f) ? 1 : (y < -1.80f) ? 2 : (y < -0.90f) ? 3 : (y < 0.0f) ? 4 :
				(y < 0.90f) ? 5 : (y < 1.80f) ? 6 : (y < 2.70f) ? 7 : 8;

			//printf("selectionX: %d / selectionY: %d\n", selectionX, selectionY);

			if(game.isSelected()){
				game.setNewSelection(selectionX, selectionY);
			}
			else{
				game.setSelection(selectionX, selectionY);
			}		
		}
		else{
			game.setDrawSelection(false);
		}
	}

	glColor3f(1.0f, 1.0f, 1.0f);
	glPushMatrix();
		glTranslatef(pos.x, pos.y, pos.z);
		RenderSkybox();
	glPopMatrix();

	// render the ring in blended mode (must be after skybox)
	if(game.getPlanet() == Game::PLANET_SATURN){
		glPushMatrix();
		
		glTranslatef(450.0f, -250.0f, -350.0f);
		glRotatef(80.0f, 1.0f, 0.0f, 0.0f);
		glCullFace(GL_FRONT);
		glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_SATURN_RINGS]);
		glEnable(GL_BLEND);
		glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		graphics.renderRadialRing();
		glDisable(GL_BLEND);

		glPopMatrix();
	}

	//glPopAttrib(); // lighting attrib

	if(graphics.cullFace()){
		glDisable(GL_CULL_FACE);
	}
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	RenderText();

	if(g_displayPieceInfo){
		RenderPieceInfoText();
		g_pieceInfo.render();
	}

	if(g_displayArbText){
		std::ostringstream o;
		float white[3] = {1.0f, 1.0f, 1.0f};
		int strWidth;

		o << g_arbText << std::endl;

		strWidth = strlen(o.str().c_str());

		RenderText(g_font, o, (graphics.getWidth() / 2) - (strWidth * 4), 20, white);
	}
	
	if(game.inCheck(WHITE)){
		std::ostringstream o;
		int strWidth;
		float checkColor[] = {1.0f, 0.1f, 0.1f};

		if(game.getState() == Game::STATE_WHITE_CHECKMATE){
			o << "White in CHECKMATE" << std::endl;
		}
		else{
			o << "White in Check!" << std::endl;
		}
		strWidth = strlen(o.str().c_str());

		RenderText(g_font, o, (graphics.getWidth() / 2) - (strWidth * 4), 40, checkColor);
	}
	else if(game.inCheck(BLACK)){
		std::ostringstream o;
		int strWidth;
		float checkColor[] = {1.0f, 0.1f, 0.1f};

		o << "Black in Check!" << std::endl;
		strWidth = strlen(o.str().c_str());

		RenderText(g_font, o, (graphics.getWidth() / 2) - (strWidth * 4), 40, checkColor);
	}
	else if(game.getState() == Game::STATE_STALEMATE){
		std::ostringstream o;
		int strWidth;
		float stalemateColor[] = {0.8f, 0.8f, 0.8f};

		o << "Stalemate!" << std::endl;
		strWidth = strlen(o.str().c_str());

		RenderText(g_font, o, (graphics.getWidth() / 2) - (strWidth * 4), 40, stalemateColor);
	}
}

static void SetMenuCamera(void)
{
	g_cam.setBehavior(Cam::CAM_BEHAVIOR_SPECTATOR);
	g_cam.setPosition(Vec3(0.0f, 10.0f, 0.0f));
	g_cam.lookAt(Vec3(0.0f, 0.0f, 0.0f));
}

void RenderMenu(void)
{
	Game& game = Game::inst();
	Graphics& graphics = Graphics::inst();
	int width = graphics.getWidth();
	int height = graphics.getHeight();
	int maxBoards = (game.getState() == Game::STATE_LOADING) ? 10 : g_numBoards;

	if(graphics.cullFace()){
		glDisable(GL_CULL_FACE);
	}

	// move camera to orthogonal viewing position for flying boards

	// render flying boards
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	for(int i=0; i<maxBoards; ++i){
		glPushMatrix();

		g_boardParticles[i].update();

		Vec3 pos = g_boardParticles[i].getPosition();
		Vec3 rot = g_boardParticles[i].getRotation();
		float size = g_boardParticles[i].getSize();
		
		glColor4f(1.0f, 1.0f, 1.0f, g_boardParticles[i].getFade());
		glTranslatef(pos.x, pos.y, pos.z);
		glRotatef(g_boardParticles[i].getAngle(), rot.x, rot.y, rot.z); 
		glScalef(size, size, size);

		RenderBoard(false);

		glPopMatrix();
	}
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	if(game.getState() == Game::STATE_LOADING){
		float color[] = {1.0f, 1.0f, 1.0f};

		// print loading text
		RenderText(g_font, game.getStatusStream(), 5, game.getStreamOffset(), color);

		if(!graphics.isLoadingResources()){
			game.setState(Game::STATE_PAUSED);
			Mouse::inst().setPosition(1, 1); // avoid accidental clicks on the menu
		}
	}
	else if(game.getState() == Game::STATE_CREDITS){
		RenderCredits();
	}
	else{
		// render the pause menu
		g_pauseMenu.render();

		RenderText();
	}

	if(graphics.cullFace()){
		glEnable(GL_CULL_FACE);
	}
}

void RenderModel(unsigned int n) 
{
	if(!g_models[n].isLoaded())
		return;

	//glPushMatrix();

	float modelOffset[] = {0.108f, 0.15f, 0.22f, 0.215f, 0.318f, 0.39f};

	if(Game::inst().getChessSet() == Game::SET_NORMAL){
		modelOffset[0] = 0.108f;
		modelOffset[1] = 0.235f;
		modelOffset[2] = 0.32f;
		modelOffset[3] = 0.395f;
		modelOffset[4] = 0.688f;
		modelOffset[5] = 0.560f;
			
		switch(n){
		default:
		case 0:
			glScalef(1.55f, 1.55f, 1.55f);
			break;

		case 1:
			glScalef(1.05f, 0.95f, 1.05f);
			break;

		case 2:
			glScalef(0.85f, 0.85f, 0.85f);
			break;

		case 3:
			glScalef(0.85f, 0.75f, 0.85f);
			break;

		case 4:
			glScalef(0.75f, 0.75f, 0.75f);
			break;

		case 5:
			glScalef(0.85f, 0.90f, 0.85f);
			break;
		}
	}
	else{
		glScalef(2.0f, 2.0f, 2.0f);
	}

	glTranslatef(0.0f, modelOffset[n], 0.0f);

	//glPopMatrix();

	GLuint id = 0;
	const ModelOBJ::Mesh* pMesh = 0;
	const ModelOBJ::Material* pMaterial = 0;
	const ModelOBJ::Vertex* pVertices = 0;

	for(int i=0; i<g_models[n].getNumberOfMeshes(); ++i){
		pMesh = &g_models[n].getMesh(i);
		pMaterial = &g_models[n].getMaterial(pMesh->materialIndex);
		pVertices = g_models[n].getVertexBuffer();

		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, pMaterial->ambient);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, pMaterial->diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, pMaterial->specular);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, pMaterial->shininess * 128.0f);

		/*if((id = g_models[n].getTexture()) != 0){
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, id);
		}
		else{
			glDisable(GL_TEXTURE_2D);
		}*/
		
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, g_models[n].getVertexSize(), pVertices->position);

		if(g_models[n].hasTextureCoords()){
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, g_models[n].getVertexSize(), pVertices->texCoord);
		}

		if(g_models[n].hasVertexNormals()){
			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT, g_models[n].getVertexSize(), pVertices->normal);
		}

		// draw the model
		glDrawElements(GL_TRIANGLES, pMesh->triangleCount * 3,
			GL_UNSIGNED_INT, g_models[n].getIndexBuffer() + pMesh->startIndex);

		if(g_models[n].hasVertexNormals())
			glDisableClientState(GL_NORMAL_ARRAY);

		if(g_models[n].hasTextureCoords())
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		glDisableClientState(GL_VERTEX_ARRAY);
	}

	glTranslatef(0.0f, -modelOffset[n], 0.0f);
}

void RenderPieces(bool reflection)
{
	const float NEW		= 999.0f;
	const float ANIMATION_SPEED = 1.8f * g_elapsedTimeSec; // allow frame rate independent animation
	Game& game			= Game::inst();
	bool sphereMap		= g_sphereMap;
	//bool sphereMap		= (game.getChessSet() == Game::SET_FANCY) ? true : false;//(game.getTextureMode() == Game::METALLIC || game.getTextureMode() == Game::MARBLE) ? true : false;
	int animateFromX	= game.getAnimateFromX();
	int animateFromY	= game.getAnimateFromY();
	int animateToX		= game.getAnimateToX();
	int animateToY		= game.getAnimateToY();
	unsigned int lastMoveFromX = game.getLastMoveX(true),
				 lastMoveFromY = game.getLastMoveY(true),
				 lastMoveToX   = game.getLastMoveX(false),
				 lastMoveToY   = game.getLastMoveY(false);
	bool animating		= game.isAnimating();
	int board			= 0;
	int chessSet		= game.getChessSet();
	float animationSpeed = ANIMATION_SPEED; 

	if(!Graphics::inst().useReflection()){
		animationSpeed *= 2.0f; // double the speed since the rendering is half as much
	}

	if(sphereMap){
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
	}

	glPushMatrix();
	
	glTranslatef(-1.75f, 0.0f, 1.75f);
	for(int x=BOARD_MIN; x<BOARD_MAX+1; ++x){
		for(int y=BOARD_MIN; y<BOARD_MAX+1; ++y){	

			glColor3f(1.0f, 1.0f, 1.0f);

			// draw the selection tool
			if(game.drawSelection() && reflection == false){
				glDisable(GL_TEXTURE_2D);
				if(game.getSelectionX() == x && game.getSelectionY() == y){
					glPushMatrix();

					glTranslatef(-0.25f, 0.01f, 0.25f);
					glColor3f(1.0f, 1.0f, 0.0f);
					glLineWidth(2.0f);
					glBegin(GL_LINES);
						glVertex3f(0.0f, 0.0f, 0.0f);
						glVertex3f(0.0f, 0.0f, -0.5f);

						glVertex3f(0.0f, 0.0f, -0.5f);
						glVertex3f(0.5f, 0.0f, -0.5f);

						glVertex3f(0.5f, 0.0f, -0.5f);
						glVertex3f(0.5f, 0.0f, 0.0f);

						glVertex3f(0.5f, 0.0f, 0.0f);
						glVertex3f(0.0f, 0.0f, 0.0f);
					glEnd();

					glPopMatrix();
				}
				if(game.getNewSelectionX() == x && game.getNewSelectionY() == y && game.isSelected()){
					glPushMatrix();

					glTranslatef(-0.25f, 0.01f, 0.25f);
					glColor3f(0.0f, 0.8f, 1.0f);
					glLineWidth(2.0f);
					glBegin(GL_LINES);
						glVertex3f(0.0f, 0.0f, 0.0f);
						glVertex3f(0.0f, 0.0f, -0.5f);

						glVertex3f(0.0f, 0.0f, -0.5f);
						glVertex3f(0.5f, 0.0f, -0.5f);

						glVertex3f(0.5f, 0.0f, -0.5f);
						glVertex3f(0.5f, 0.0f, 0.0f);

						glVertex3f(0.5f, 0.0f, 0.0f);
						glVertex3f(0.0f, 0.0f, 0.0f);
					glEnd();

					glPopMatrix();
				}

				glColor3f(1.0f, 1.0f, 1.0f);
				glEnable(GL_TEXTURE_2D);
			}

			// draw last move - from 
			if(x == lastMoveFromX && y == lastMoveFromY){
				glDisable(GL_TEXTURE_2D);
				glPushMatrix();
					
				glTranslatef(-0.25f, 0.01f, 0.25f);
				glColor3f(0.0f, 1.0f, 0.0f);
				glLineWidth(3.0f);
				glBegin(GL_LINES);
					glVertex3f(0.0f, 0.0f, 0.0f);
					glVertex3f(0.0f, 0.0f, -0.5f);

					glVertex3f(0.0f, 0.0f, -0.5f);
					glVertex3f(0.5f, 0.0f, -0.5f);

					glVertex3f(0.5f, 0.0f, -0.5f);
					glVertex3f(0.5f, 0.0f, 0.0f);

					glVertex3f(0.5f, 0.0f, 0.0f);
					glVertex3f(0.0f, 0.0f, 0.0f);
				glEnd();

				glPopMatrix();
				glColor3f(1.0f, 1.0f, 1.0f);
				glEnable(GL_TEXTURE_2D);
			}
			// draw last move - to
			if(x == lastMoveToX && y == lastMoveToY){
				glDisable(GL_TEXTURE_2D);
				glPushMatrix();
					
				glTranslatef(-0.25f, 0.01f, 0.25f);
				glColor3f(1.0f, 0.0f, 0.0f);
				glLineWidth(3.0f);
				glBegin(GL_LINES);
					glVertex3f(0.0f, 0.0f, 0.0f);
					glVertex3f(0.0f, 0.0f, -0.5f);

					glVertex3f(0.0f, 0.0f, -0.5f);
					glVertex3f(0.5f, 0.0f, -0.5f);

					glVertex3f(0.5f, 0.0f, -0.5f);
					glVertex3f(0.5f, 0.0f, 0.0f);

					glVertex3f(0.5f, 0.0f, 0.0f);
					glVertex3f(0.0f, 0.0f, 0.0f);
				glEnd();

				glPopMatrix();
				glColor3f(1.0f, 1.0f, 1.0f);
				glEnable(GL_TEXTURE_2D);
			}

			if((board = game.getPieceAt(x, y)) != Game::EMPTY){
				glPushMatrix();

				if(reflection){
					glScalef(1.0f, -1.0f, 1.0f);
				}

				// do animation
				if(animateToX == x && animateToY == y && animating){
					const float STEP = 0.01f;
					float LIMIT = 0.0f;
					switch(chessSet){
					case Game::SET_NATIVE:
					default:
						LIMIT = 4.3f;
						break;

					case Game::SET_FANCY:
						LIMIT = 6.3f;
						break;
					}
					static float xOffset = NEW;
					static float yOffset = 0.0f;
					static float zOffset = NEW;
					//static float acceleration = 0.1f;
					static float magnitude = 0.0f;
					static float jmpStrength = LIMIT * g_elapsedTimeSec;

					if(zOffset == NEW){
						xOffset = (static_cast<float>(animateToY) - static_cast<float>(animateFromY)) * 0.5f;
						zOffset = (static_cast<float>(animateToX) - static_cast<float>(animateFromX)) * 0.5f;

						Vec2 dir(xOffset, zOffset);
						magnitude = dir.magnitude();
					}

					// animate knight jumping
					if(abs(board) == Game::KNIGHT){
						Vec2 dir(xOffset, zOffset);
						float dist = dir.magnitude();

						if(dist > (magnitude / 2.0f)){
							yOffset += (STEP * jmpStrength);
							jmpStrength -= animationSpeed;
						}
						else{
							if(yOffset - (STEP * jmpStrength) < 0.0f){
								yOffset = 0.0f;
							}
							else{
								yOffset -= (STEP * jmpStrength);
							}

							jmpStrength += animationSpeed;
						}
					}

					// translate the model backwards
					glTranslatef(-xOffset, yOffset, zOffset);

					// decrease the offset for the next frame
					if(xOffset != 0.0f){
						xOffset += (xOffset > 0.0f) ? -animationSpeed : animationSpeed;
					}
					if(zOffset != 0.0f){
						zOffset += (zOffset > 0.0f) ? -animationSpeed : animationSpeed;
					}

					// check if the piece is done animating
					if((zOffset <= animationSpeed && zOffset >= -animationSpeed)
						&& (xOffset <= animationSpeed && xOffset >= -animationSpeed)){
						game.setAnimating(false);
						xOffset = zOffset = NEW;
						magnitude = 0.0f;
						jmpStrength = LIMIT;
						animationSpeed = ANIMATION_SPEED; 
					}
				}

				// rotate knights and kings if needed
				if(chessSet == Game::SET_NATIVE){
					if(board < 0)
						glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
				}
				else if(chessSet == Game::SET_NORMAL){
					if(abs(board) == Game::KNIGHT){
						if(board > 0)
							glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
						else
							glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
					}
					else if(abs(board) == Game::KING){
						glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
					}
				}
				else if(chessSet == Game::SET_FANCY){
					if(abs(board) == Game::KNIGHT){
						if(board > 0)
							glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
						else
							glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
					}
					else if(abs(board) == Game::KING){
						glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
					}
				}

				// render the model
				glEnable(GL_TEXTURE_2D);
				if(board > 0){
					//glColor3f(0.3f, 0.3f, 0.3f);
					glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_WHITE]);
				}
				else{
					glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_BLACK]);	
				}

				if(chessSet == Game::SET_NATIVE){
					if(game.getTextureMode() == Game::GLASS){
						glEnable(GL_BLEND);
						glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
						glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
						RenderDefaultModel(abs(board) - 1);
						glDisable(GL_BLEND);
					}
					else{
						RenderDefaultModel(abs(board) - 1);
					}
				}
				else{
					if(game.getTextureMode() == Game::GLASS){
						glEnable(GL_BLEND);
						glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
						glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
						RenderModel(abs(board) - 1);
						glDisable(GL_BLEND);
					}
					else{
						RenderModel(abs(board) - 1);
					}
				}

				glPopMatrix();	
			}

			// move up a square on the x-axis
			glTranslatef(0.5f, 0.0f, 0.0f);
		}

		// move back to the beginning of the x-axis and up the z-axis
		glTranslatef(-4.0f, 0.0f, -0.5f);
	}

	if(sphereMap){
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
	}

	glPopMatrix();
}

void RenderDefaultModel(unsigned int n)
{
	//glCallList(Graphics::inst().getDefModel(0));
	//glDisable(GL_TEXTURE_2D);
	glPushMatrix();

	//glTranslatef(0.0f, 0.05, 0.0f);
	glScalef(0.12f, 0.15f, 0.12f); // adjust the size of the models
	glCallList(g_defModels[n]);
	
	glPopMatrix();
	//glEnable(GL_TEXTURE_2D);
}

void Render2DModel(int board)
{
	switch(board){
		case Game::WHITE_PAWN:
			glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_2D_PAWN_WHITE]);
			break;

		case Game::BLACK_PAWN:
			glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_2D_PAWN_BLACK]);
			break;

		case Game::WHITE_ROOK:
			glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_2D_ROOK_WHITE]);
			break;

		case Game::BLACK_ROOK:
			glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_2D_ROOK_BLACK]);
			break;

		case Game::WHITE_KNIGHT:
			glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_2D_KNIGHT_WHITE]);
			break;

		case Game::BLACK_KNIGHT:
			glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_2D_KNIGHT_BLACK]);
			break;

		case Game::WHITE_BISHOP:
			glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_2D_BISHOP_WHITE]);
			break;

		case Game::BLACK_BISHOP:
			glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_2D_BISHOP_BLACK]);
			break;

		case Game::WHITE_QUEEN:
			glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_2D_QUEEN_WHITE]);
			break;

		case Game::BLACK_QUEEN:
			glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_2D_QUEEN_BLACK]);
			break;

		case Game::WHITE_KING:
			glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_2D_KING_WHITE]);
			break;

		case Game::BLACK_KING:
			glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_2D_KING_BLACK]);
			break;

		default:
			glBindTexture(GL_TEXTURE_2D, 0);
			break;
	}


	//glColor4b(211, 14, 14, 255);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	//glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPushMatrix();

	glScalef(0.5f, 1.0f, 0.5f);
	glTranslatef(0.0f, 0.01f, 0.0f);

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 0.0f, 0.5f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.0f, -0.5f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.0f, -0.5f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, 0.0f, 0.5f);
	glEnd();

	glPopMatrix();

	//glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void RenderBoard(bool fullBoard)
{
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_BOARD]);

	//glActiveTextureARB(GL_TEXTURE1_ARB);
	//glEnable(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_BOARD2]);

	glCallList(g_boardList);

	//glActiveTextureARB(GL_TEXTURE1_ARB);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//glDisable(GL_TEXTURE_2D);

	//glActiveTextureARB(GL_TEXTURE0_ARB);

	if(!fullBoard){
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
		return;
	}

	glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_BOARD_FRAME]);
	glCallList(g_boardFrame);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}

void RenderBoardBox(void)
{
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_BOARD_BOX]);

	glDisable(GL_BLEND);

	glCallList(g_boardBox);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}

void RenderCredits(void)
{
	int width = Graphics::inst().getWidth();
	int height = Graphics::inst().getHeight();

	Graphics::inst().setMode(Graphics::ORTHO);

	glEnable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_CREDITS]);

	glBegin(GL_QUADS);
		glTexCoord2i(0, 0); glVertex2i(0, height);
		glTexCoord2i(1, 0); glVertex2i(width, height);
		glTexCoord2i(1, 1); glVertex2i(width, 0);
		glTexCoord2i(0, 1); glVertex2i(0, 0);
	glEnd();

	glDisable(GL_BLEND);

	Graphics::inst().setMode(Graphics::PROJ);
}

void RenderSkybox(void)
{
	//glDisable(GL_DEPTH_TEST);
	//glDepthMask(false);

	// support for multi-texturing
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);

	// up
	glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_SKY_UP]);
	glBegin(GL_QUADS);
		glNormal3f(0.0f, -1.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
	glEnd();

	// down
	glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_SKY_DOWN]);
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.0f);	
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);				
	glEnd();

	// north
	glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_SKY_NORTH]);
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, 1.0f); 
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	glEnd();

	// south
	glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_SKY_SOUTH]);
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glEnd();

	// east
	glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_SKY_EAST]);
	glBegin(GL_QUADS);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
	glEnd();

	// west
	glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_SKY_WEST]);
	glBegin(GL_QUADS);
		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-SKYBOX_SIZE, SKYBOX_SIZE, -SKYBOX_SIZE);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE);
	glEnd();

	//glEnable(GL_DEPTH_TEST);
	//glDepthMask(true);
}

void RenderPlanet(void)
{
	Graphics& graphics = Graphics::inst();
	static float rot = 0.0f;
	glColor3f(1.0f, 1.0f, 1.0f);
	GLint quality = 16;

	switch(graphics.getGeometryQuality()){
	case Graphics::MEDIOCRE:
		quality = 8;
		break;

	case Graphics::LOW:
	default:
		quality = 16;
		break;

	case Graphics::MEDIUM:
		quality = 32;
		break;

	case Graphics::HIGH:
		quality = 64;
		break;

	case Graphics::ULTRA:
		quality = 256;
		break;
	}
		
	switch(Game::inst().getPlanet()){
		case Game::PLANET_NONE:
			return;

		case Game::PLANET_EARTH:
		case Game::PLANET_EARTH_NIGHT:
			glPushMatrix();
			glTranslatef(-80.0f, -360.0f, -200.0f);
			glRotatef(rot+=(0.9f * g_elapsedTimeSec), 0.0f, 1.0f, 0.0f);
			glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
			glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_EARTH]);
			gluSphere(graphics.inst().getQuadric(0), 250, quality, quality);
			glPopMatrix();
			break;

		case Game::PLANET_SATURN:
			glPushMatrix();
			glTranslatef(450.0f, -250.0f, -350.0f);
			glRotatef(80.0f, 1.0f, 0.0f, 0.0f);
			glPushMatrix();
				glRotatef(rot+=(1.2f * g_elapsedTimeSec), 0.0f, 0.0f, 1.0f);
				glBindTexture(GL_TEXTURE_2D, g_textures[Game::TEX_SATURN]);
				gluSphere(graphics.inst().getQuadric(0), 250, quality, quality);
			glPopMatrix();

			//glEnable(GL_BLEND);
			/*glEnable(GL_BLEND);
						glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
						glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			graphics.renderRadialRing();
			glDisable(GL_BLEND);*/
			glPopMatrix();
			break;
	}
}

void RenderText(GLFont& font, std::ostringstream& o, int x, int y, float color[3])
{
	font.begin();
	font.setColor(color[0], color[1], color[2]);
	font.drawText(x, y, o.str().c_str());
	font.end();
}

static DWORD WINAPI _displayArbitraryText(LPVOID lpBuffer)
{
	DWORD time = (DWORD)lpBuffer;

	Sleep(time);

	g_displayArbText = false;

	ExitThread(0);
}

void DisplayArbitraryText(const char* str, DWORD time)
{
	if(strlen(str) > 1023){
		return;
	}
	strcpy(g_arbText, str);

	g_displayArbText = true;

	(void)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&_displayArbitraryText, (LPVOID)time, 0, 0);
}

void RenderPieceInfoText(void)
{
	Game& game = Game::inst();
	std::ostringstream o;
	float color[3] = {1.0f, 1.0f, 1.0f};
	int yOffset = 170;
	int height = Graphics::inst().getHeight();
	int xOffset = 32;

	o << game.getCaptureState(Game::WHITE_PAWN) << std::endl;

	RenderText(g_font, o, xOffset, height - yOffset, color);

	o.str("");
	o << game.getCaptureState(Game::WHITE_ROOK) << std::endl;
	yOffset -= 24; // from menu.cpp

	RenderText(g_font, o, xOffset, height - yOffset, color);

	o.str("");
	o << game.getCaptureState(Game::WHITE_KNIGHT) << std::endl;
	yOffset -= 24;

	RenderText(g_font, o, xOffset, height - yOffset, color);

	o.str("");
	o << game.getCaptureState(Game::WHITE_BISHOP) << std::endl;
	yOffset -= 24;

	RenderText(g_font, o, xOffset, height - yOffset, color);

	o.str("");
	o << game.getCaptureState(Game::WHITE_QUEEN) << std::endl;
	yOffset -= 24;

	RenderText(g_font, o, xOffset, height - yOffset, color);

	// black pieces
	xOffset = Graphics::inst().getWidth() - 42;

	o.str("");
	o << game.getCaptureState(Game::BLACK_PAWN_) << std::endl;
	yOffset = 170;

	RenderText(g_font, o, xOffset, height - yOffset, color);

	o.str("");
	o << game.getCaptureState(Game::BLACK_ROOK_) << std::endl;
	yOffset -= 24;

	RenderText(g_font, o, xOffset, height - yOffset, color);

	o.str("");
	o << game.getCaptureState(Game::BLACK_KNIGHT_) << std::endl;
	yOffset -= 24;

	RenderText(g_font, o, xOffset, height - yOffset, color);

	o.str("");
	o << game.getCaptureState(Game::BLACK_BISHOP_) << std::endl;
	yOffset -= 24;

	RenderText(g_font, o, xOffset, height - yOffset, color);

	o.str("");
	o << game.getCaptureState(Game::BLACK_QUEEN_) << std::endl;
	yOffset -= 24;

	RenderText(g_font, o, xOffset, height - yOffset, color);
}

void RenderText(void)
{
	Game& game = Game::inst();
	std::ostringstream o;
	unsigned int whiteSec = (game.getTime(WHITE) / 1000 % 60);
	unsigned int blackSec = (game.getTime(BLACK) / 1000 % 60);

	//o << "FPS: " << Graphics::inst().getFPS() << std::endl;
	//o << "FreeMove: " << game.getFreeMove() << std::endl;
	o << "White Time: [" << (game.getTime(WHITE) / 1000 / 60) << ((whiteSec < 10) ? ":0" : ":") << whiteSec << "]" << std::endl;
	o << "Black Time: [" << (game.getTime(BLACK) / 1000 / 60) << ((blackSec < 10) ? ":0" : ":") << blackSec << "]" << std::endl;
	if(game.getTurn() == BLACK){
		static float f = 0.0f;
		o << std::endl << "Black is thinking" << ((f < 10.0f) ? "" : (f < 20.0f) ? "." : (f < 30.0f) ? ".." : "...");

		f += (g_elapsedTimeSec * 20.0f);
		if(f > 40.0f)
			f = 0.0f;
	}

	// draw the font
	g_font.begin();
	g_font.setColor(1.0f, 1.0f, 1.0f);
	g_font.drawText(5, 1, o.str().c_str());
	g_font.end();
}

void UpdateFrame(void)
{
	g_elapsedTimeSec = GetElapsedTimeInSeconds();
	Graphics::inst().updateFrameRate(g_elapsedTimeSec);

	Mouse::inst().update();
	Keyboard::inst().update();

	UpdateCam(g_elapsedTimeSec);
	ProcessUserInput();
}

void UpdateCam(float elapsedTimeSec)
{
	unsigned int state = Game::inst().getState();

	if(state == Game::STATE_ACTIVE ||
		state == Game::STATE_WHITE_CHECKMATE ||
		state == Game::STATE_BLACK_CHECKMATE){
		float dx = 0.0f,
			  dy = 0.0f,
			  dz = 0.0f;
		Vec3 pos = g_cam.getCurrentPos();
		Vec3 direction;
		Mouse& mouse = Mouse::inst();

		if(g_cam.getBehavior() == Cam::CAM_BEHAVIOR_ORBIT){
			if(mouse.buttonDown(Mouse::BUTTON_RIGHT)){
				mouse.attach(g_hWnd);
				mouse.hideCursor(true);
				Game::inst().setAllowSelectionChange(false);
				Game::inst().setDrawSelection(false);
			}
			else{
				mouse.detach();
				Game::inst().setAllowSelectionChange(true);
				Game::inst().setDrawSelection(true);
				return;
			}
		}
		else{
			mouse.attach(g_hWnd);
			Game::inst().setDrawSelection(false);
			mouse.hideCursor(true); // btw, using this alone produces a cool camera effect
		}

		GetMovementDirection(direction);

		switch(g_cam.getBehavior()){
			default:
				break;

			case Cam::CAM_BEHAVIOR_SPECTATOR:
				dx = -mouse.xPosRelative();
				dy = -mouse.yPosRelative();

				g_cam.rotateSmoothly(dx, dy, 0.0f);
				g_cam.updatePos(direction, elapsedTimeSec);
				DetectCollision();
				break;

			case Cam::CAM_BEHAVIOR_ORBIT:
				dx = mouse.xPosRelative();
				dy = mouse.yPosRelative();

				//Quaternion q = g_cam.getOrientation();
				/*Matrix4 m = g_cam.getViewMatrix();

				if(pos.y < 0.1f){
					dy = -(0.01f / g_elapsedTimeSec);
					g_cam.rotateSmoothly(dx, dy, 0.0f);
					break;
				}
				else if(m[1][1] <= 0.05f){
					dy = (0.01f / g_elapsedTimeSec);
					g_cam.rotateSmoothly(dx, dy, 0.0f);
				}
				else{
					g_cam.rotateSmoothly(dx, dy, 0.0f);
				}*/
				/*if(q.x > 0.01f){
					g_cam.rotateSmoothly(dx, dy, 0.0f);
				}
				else{
					do{
						g_cam.rotateSmoothly(0.0f, -0.001f, 0.0f);

						q = g_cam.getOrientation();
					}while(q.x < 0.01f);
				}*/

				g_cam.rotateSmoothly(dx, dy, 0.0f);

				if(!g_cam.preferTargetYAxisOrbiting()){
					if((dz = direction.x * CAMERA_SPEED_ORBIT_ROLL * elapsedTimeSec) != 0.0f){
						g_cam.rotate(0.0f, 0.0f, dz);
					}
				}

				if((dz = -mouse.wheelPos()) != 0.0f){
					g_cam.zoom(dz / 4.0f, g_cam.getOrbitMinZoom(), g_cam.getOrbitMaxZoom());
				}

				DetectCollision();
				break;
		}

		mouse.setPosition(Graphics::inst().getWidth() / 2, Graphics::inst().getHeight() / 2);

		if(mouse.buttonPressed(Mouse::BUTTON_MIDDLE)){
			g_cam.undoRoll();
		}
	}
}

void GetMovementDirection(Vector3 &direction) // can go in another file
{
    static bool moveForwardsPressed = false;
    static bool moveBackwardsPressed = false;
    static bool moveRightPressed = false;
    static bool moveLeftPressed = false;
    static bool moveUpPressed = false;
    static bool moveDownPressed = false;

    Vec3 velocity = g_cam.getCurrentVelocity();
	Keyboard& keyboard = Keyboard::inst();

    direction.set(0.0f, 0.0f, 0.0f);

    if (keyboard.keyDown(Keyboard::KEY_W))
    {
        if (!moveForwardsPressed)
        {
            moveForwardsPressed = true;
            g_cam.setCurrentVelocity(velocity.x, velocity.y, 0.0f);
        }

        direction.z += 1.0f;
    }
    else
    {
        moveForwardsPressed = false;
    }

    if (keyboard.keyDown(Keyboard::KEY_S))
    {
        if (!moveBackwardsPressed)
        {
            moveBackwardsPressed = true;
            g_cam.setCurrentVelocity(velocity.x, velocity.y, 0.0f);
        }

        direction.z -= 1.0f;
    }
    else
    {
        moveBackwardsPressed = false;
    }

    if (keyboard.keyDown(Keyboard::KEY_D))
    {
        if (!moveRightPressed)
        {
            moveRightPressed = true;
            g_cam.setCurrentVelocity(0.0f, velocity.y, velocity.z);
        }

        direction.x += 1.0f;
    }
    else
    {
        moveRightPressed = false;
    }

    if (keyboard.keyDown(Keyboard::KEY_A))
    {
        if (!moveLeftPressed)
        {
            moveLeftPressed = true;
            g_cam.setCurrentVelocity(0.0f, velocity.y, velocity.z);
        }

        direction.x -= 1.0f;
    }
    else
    {
        moveLeftPressed = false;
    }

    if (keyboard.keyDown(Keyboard::KEY_E))
    {
        if (!moveUpPressed)
        {
            moveUpPressed = true;
            g_cam.setCurrentVelocity(velocity.x, 0.0f, velocity.z);
        }

        direction.y += 1.0f;
    }
    else
    {
        moveUpPressed = false;
    }

    if (keyboard.keyDown(Keyboard::KEY_Q))
    {
        if (!moveDownPressed)
        {
            moveDownPressed = true;
            g_cam.setCurrentVelocity(velocity.x, 0.0f, velocity.z);
        }

        direction.y -= 1.0f;
    }
    else
    {
        moveDownPressed = false;
    }
}

void DetectCollision(void)
{
	const Vec3& pos = g_cam.getCurrentPos();
	Vec3 newPos(pos);
	Matrix4 m = g_cam.getViewMatrix();

	if(g_cam.getBehavior() == Cam::CAM_BEHAVIOR_SPECTATOR){
		if(pos.y < 0.2f)
			newPos.y = 0.2f;

		g_cam.setPosition(newPos);
	}
	else if(g_cam.getBehavior() == Cam::CAM_BEHAVIOR_ORBIT){
		if(pos.y < 0.25f){ // keep camera above board
			do{
				g_cam.rotateSmoothly(0.0f, -0.01f, 0.0f);

				newPos = g_cam.getCurrentPos();
			}while(newPos.y < 0.25f);
		}
		else if(m[1][1] < 0.08f){ // keep it from going upside down
			do{
				g_cam.rotateSmoothly(0.0f, 0.01f, 0.0f);

				m = g_cam.getViewMatrix();
			}while(m[1][1] < 0.08f);
		}
	}
}

void ProcessUserInput(void)
{
	Keyboard& keyboard = Keyboard::inst();
	Mouse& mouse = Mouse::inst();
	Game& game = Game::inst();
	static int checkmate = false;

	if(game.getState() == Game::STATE_CREDITS){
		for(int i=0; i<222; ++i){
			if(keyboard.keyPressed((Keyboard::Key)i)){
				//mouse.setPosition(1, 1);
				game.setState(Game::STATE_PAUSED);
				return;
			}
		}
	}

	// toggle full screen
	if(keyboard.keyDown(Keyboard::KEY_LALT) || keyboard.keyDown(Keyboard::KEY_RALT)){
		if(keyboard.keyPressed(Keyboard::KEY_F1)){
			SwitchFullscreen();

			// adjust the camera
			if(game.getState() == Game::STATE_PAUSED){
				memcpy(&g_cam, &g_cam.lastInst(), sizeof(Cam));
				SetMenuCamera();
			}
		}
	}

	if(keyboard.keyPressed(Keyboard::KEY_1) && g_cam.getBehavior() != Cam::CAM_BEHAVIOR_ORBIT){
		//g_cam.setPosition(DEFAULT_ORBIT_CAM_POS);
		g_cam.changeCamBehavior(Cam::CAM_BEHAVIOR_ORBIT); 
		memcpy(&g_cam, &g_cam.lastInst(), sizeof(Cam));
	}

	if(keyboard.keyPressed(Keyboard::KEY_2) && g_cam.getBehavior() != Cam::CAM_BEHAVIOR_SPECTATOR){
		memcpy(&g_cam.lastInst(), &g_cam, sizeof(Cam));
		g_cam.changeCamBehavior(Cam::CAM_BEHAVIOR_SPECTATOR);
	}

	// misc
	if(( (keyboard.keyPressed(Keyboard::KEY_ENTER) || keyboard.keyPressed(Keyboard::KEY_ESCAPE)) && 
		game.getState() != Game::STATE_LOADING) || g_enterKey){
		if(game.getState() == Game::STATE_ACTIVE){
			memcpy(&g_cam.lastInst(), &g_cam, sizeof(Cam));
			game.setState(Game::STATE_PAUSED);
			SetMenuCamera();
		}
		else if(game.getState() == Game::STATE_WHITE_CHECKMATE ||
			game.getState() == Game::STATE_BLACK_CHECKMATE){
				memcpy(&g_cam.lastInst(), &g_cam, sizeof(Cam));
				game.setState(Game::STATE_PAUSED);
				checkmate = (game.getState() == Game::STATE_WHITE_CHECKMATE) ? 1 : 2;
				SetMenuCamera();
		}
		else{ // returning from pause menu
			memcpy(&g_cam, &g_cam.lastInst(), sizeof(Cam));
			if(checkmate == 1){
				game.setState(Game::STATE_WHITE_CHECKMATE);
			}
			else if(checkmate == 2){
				game.setState(Game::STATE_BLACK_CHECKMATE);
			}
			else{
				game.setState(Game::STATE_ACTIVE);
			}
		}

		keybd_event(VK_RETURN, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
		g_enterKey = false;
	}

	if(keyboard.keyPressed(Keyboard::KEY_B)){
		game.printBoard();
	}

	if(keyboard.keyPressed(Keyboard::KEY_C)){
		g_cam.printInfo();
	}

	// free movement
	if(keyboard.keyPressed(Keyboard::KEY_F)){
		game.setFreeMove(!game.getFreeMove());
	}

	// reset camera
	if(keyboard.keyPressed(Keyboard::KEY_R)){
		g_cam.resetView();
	}

	// music
	if(keyboard.keyPressed(Keyboard::KEY_M)){
		if(g_music.isPlaying())
			g_music.stop();
		else
			g_music.play();
	}

	// piece info
	if(keyboard.keyPressed(Keyboard::KEY_I)){
		g_displayPieceInfo = !g_displayPieceInfo;
	}

	// quick save
	if(keyboard.keyPressed(Keyboard::KEY_F5)){
		if(game.quickSave()){
			DisplayArbitraryText("Game saved.", 1500);
		}
		else{
			DisplayArbitraryText("Failed to save game.", 1500);
		}
	}
	// quick load
	if(keyboard.keyPressed(Keyboard::KEY_F9)){
		if(game.quickLoad()){
			DisplayArbitraryText("Game loaded.", 1500);
		}
		else{
			DisplayArbitraryText("Failed to load game.", 1500);
		}
	}

	// misc. sphere mapping
	if(keyboard.keyPressed(Keyboard::KEY_U)){
		g_sphereMap = !g_sphereMap;
	}

	if(game.getTurn() == game.getPlayerColor()){
		if(keyboard.keyPressed(Keyboard::KEY_DOWN)){
			if(!game.isSelected()){
				if(game.getSelectionX() > BOARD_MIN)
					game.setSelectionX(game.getSelectionX() - 1);
			}
			else{
				if(game.getNewSelectionX() > BOARD_MIN)
					game.setNewSelectionX(game.getNewSelectionX() - 1);
			}
		}

		if(keyboard.keyPressed(Keyboard::KEY_UP)){
			if(!game.isSelected()){
				if(game.getSelectionX() < BOARD_MAX)
					game.setSelectionX(game.getSelectionX() + 1);
			}
			else{
				if(game.getNewSelectionX() < BOARD_MAX)
					game.setNewSelectionX(game.getNewSelectionX() + 1);
			}
		}

		if(keyboard.keyPressed(Keyboard::KEY_LEFT)){
			if(!game.isSelected()){
				if(game.getSelectionY() > BOARD_MIN)
					game.setSelectionY(game.getSelectionY() - 1);
			}
			else{
				if(game.getNewSelectionY() > BOARD_MIN)
					game.setNewSelectionY(game.getNewSelectionY() - 1);
			}
		}

		if(keyboard.keyPressed(Keyboard::KEY_RIGHT)){
			if(!game.isSelected()){
				if(game.getSelectionY() < BOARD_MAX)
					game.setSelectionY(game.getSelectionY() + 1);
			}
			else{
				if(game.getNewSelectionY() < BOARD_MAX)
					game.setNewSelectionY(game.getNewSelectionY() + 1);
			}
		}

		if(mouse.buttonPressed(Mouse::BUTTON_LEFT)){
			if(game.getState() == Game::STATE_ACTIVE){
				if(game.getPieceAt(game.getSelectionX(), game.getSelectionY()) != Game::EMPTY){
					if(!game.isSelected()){
						game.setSelected(true);
						game.setNewSelectionX(game.getSelectionX());
						game.setNewSelectionY(game.getSelectionY());
					}
					else{
						game.setSelected(false);

						// move the piece
						if(game.movePiece()){
							game.setSelectionX(game.getNewSelectionX());
							game.setSelectionY(game.getNewSelectionY());
						}
					}
				}
			}
		}
	}
}

void SwitchFullscreen(void)
{
	Graphics::inst().toggleFullScreen(g_hWnd);
	
	/*g_cam.perspective(CAMERA_FOVX, 
		static_cast<float>(graphics.getWidth()) / static_cast<float>(graphics.getHeight()),
		CAMERA_ZNEAR, CAMERA_ZFAR);*/
	//g_cam.resetView();
}

void Cleanup(void)
{
	if(g_hDC){
		if(g_hRC){
			wglMakeCurrent(g_hDC, 0);
			wglDeleteContext(g_hRC);
			g_hRC = 0;
		}

		ReleaseDC(g_hWnd, g_hDC);
		g_hDC = 0;
	}

	if(g_music.isPlaying()){
		g_music.stop();
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg){
		case WM_ACTIVATE:
			switch(wParam){
				default:
					break;

				case WA_ACTIVE:
				case WA_CLICKACTIVE:
					Mouse::inst().attach(hwnd);
					g_hasFocus = true;
					break;

				case WA_INACTIVE:
					if(Graphics::inst().isFullscreen())
						ShowWindow(hwnd, SW_MINIMIZE);
					Mouse::inst().detach();
					g_hasFocus = false;
					break;
			}
			break;

		case WM_TIMER:
			switch(wParam){
				case IDT_GAME_TIMER:
					if(Game::inst().getState() == Game::STATE_ACTIVE){
						Game& game = Game::inst();
						unsigned int time = 1;

						if(game.getTurn() == WHITE){
							time = game.getTime(WHITE);

							time -= 1000;

							game.setTime(WHITE, time);

							if(time <= 0){
								MessageBox(hwnd, "White time out!", 0, 0);
								game.setState(Game::STATE_WHITE_CHECKMATE);
							}
						}
						else{
							time = game.getTime(BLACK);

							time -= 1000;

							game.setTime(BLACK, time);

							if(time <= 0){
								MessageBox(hwnd, "Black time out!", 0, 0);
								game.setState(Game::STATE_BLACK_CHECKMATE);
							}
						}
					}
					break;

				default:
					break;
			}
			break;

		case WM_SIZE:
			//Graphics::inst().setWidth(static_cast<int>(LOWORD(wParam)));
			//Graphics::inst().setHeight(static_cast<int>(HIWORD(lParam)));
			break;
		
		case WM_CLOSE:
			if(Game::inst().exit()){
				Cleanup();
			}
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			Mouse::inst().handleMsg(hwnd, msg, wParam, lParam);
			Keyboard::inst().handleMsg(hwnd, msg, wParam, lParam);
			break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd)
{
	#if defined(_DEBUG)
		_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
		_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
	#endif

	// create the main window
	MSG msg = {0};
	WNDCLASSEX wcl = {0};
	Graphics& graphics = Graphics::inst();

	wcl.cbSize = sizeof(wcl);
    wcl.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wcl.lpfnWndProc = WndProc;
    wcl.cbClsExtra = 0;
    wcl.cbWndExtra = 0;
    wcl.hInstance = g_hInst = hInst;
    wcl.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
    wcl.hCursor = LoadCursor(0, IDC_ARROW);
    wcl.hbrBackground = 0;
    wcl.lpszMenuName = 0;
    wcl.lpszClassName = "__|Ethereal_Chess|__";
    wcl.hIconSm = wcl.hIcon;

	if(!RegisterClassEx(&wcl)){
		return 1;
	}
	else{
		DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | 
			WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
		DWORD dwExStyle = WS_EX_OVERLAPPEDWINDOW;

		g_hWnd = CreateWindowEx(dwExStyle, wcl.lpszClassName, WINDOW_TITLE, dwStyle,
			0, 0, 0, 0, HWND_DESKTOP, 0, wcl.hInstance, NULL);

		if(g_hWnd){
			int width = GetSystemMetrics(SM_CXSCREEN);
			int height = GetSystemMetrics(SM_CYSCREEN);
			int left = (width - (width / 2)) / 2;
			int top = (height - (height / 2)) / 2;
			RECT rc = {0};

			SetRect(&rc, left, top, left + (width / 2), top + (height / 2));
			AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);
			MoveWindow(g_hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);

			GetClientRect(g_hWnd, &rc);
			graphics.setWidth(rc.right - rc.left);
			graphics.setHeight(rc.bottom - rc.top);
		}
		else{
			return 1;
		}
	}

	Game& game = Game::inst();

	// set the game window up
	ShowWindow(g_hWnd, nShowCmd);
	UpdateWindow(g_hWnd);

	int ai_engine = 0;

	// load config options
	g_config = new ConfigParser("options.ini");
	if(g_config->isLoaded()){
		game.setChessSet(g_config->parseValue("set"));
		game.setSkybox(g_config->parseValue("sky"));
		game.setPlanet(g_config->parseValue("planet"));
		game.setTextureMode(g_config->parseValue("texture"));
		game.setTime(g_config->parseValue("time"));
		graphics.setGeometryQuality(g_config->parseValue("geometry"));
		
		if(g_config->parseValue("music"))
			g_music.play();

		AI::inst().setEngine(g_config->parseValue("ai_engine"));
		AI::inst().setCustomEngine(g_config->parseValue("ai_custom_engine"));
		game.setAILevel(g_config->parseValue("ai"));

		graphics.setAntialiasing(g_config->parseValue("AA"));
		graphics.setReflection(g_config->parseValue("reflections"));
		graphics.enableVerticalSync(g_config->parseValue("verticalsync"));
		graphics.setBoardMode(g_config->parseValue("boardmode"));

		if(g_config->parseValue("debug")){
			graphics.allocDebugConsole();
			graphics.setDebugConsole(true);
		}

		game.setState(Game::STATE_LOADING);
	}

	GetCurrentDirectory(sizeof(g_workingDir), g_workingDir);

	g_sphereMap = true;

	// initialize the graphics
	graphics.setProcessorAffinity();

	if(!(g_hDC = GetDC(g_hWnd))){
		throw std::runtime_error("GetDC() failed");
		return 1;
	}

	if(!graphics.initPixels(g_hDC, g_hRC)){
		return 1;
	}

	// OpenGL initialization
	if(graphics.init()){
		//SwitchFullscreen();
		graphics.loadResources();

		// initialize the game elements
		game.init(g_hWnd);

		if(game.getGameplayMode() == Game::GAMEPLAY_NORMAL){
			AI::inst().init(AI::inst().getEngine());
		}

		// initialize menu system
		InitMenu();

		// initialize the camera
		g_cam.init();
		memcpy(&g_cam.lastInst(), &g_cam, sizeof(Cam));
		SetMenuCamera();

		// initialize shaders
		// if using shaders:
		/*g_shader.loadVertexShader("Data/Shaders/swirl.vert");
		g_shader.loadFragmentShader("Data/Shaders/swirl.frag");
		g_shader.link();*/
		//g_shader.enable();

		// enter the message loop
		for(;;){

			if(graphics.saveCPU())
				Sleep(1);

			for(;PeekMessage(&msg, 0, 0, 0, PM_REMOVE);){
				if(msg.message == WM_QUIT){
					break;
				}

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if(msg.message == WM_QUIT){
				break;
			}

			if(g_hasFocus){
				UpdateFrame();
				RenderFrame();
				SwapBuffers(g_hDC);
			}
			else{
				WaitMessage();
			}
		}

		ReleaseDC(g_hWnd, g_hDC);
		UnregisterClass(wcl.lpszClassName, hInst);
	}

	g_config->saveAll();
	delete g_config;

	return static_cast<int>(msg.wParam);
}