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

#include "graphics.h"

const float     CAMERA_FOVX = 90.0f;
const float     CAMERA_ZFAR = 5000.0f;
const float     CAMERA_ZNEAR = 0.1f;

GLuint			g_ringList;

Graphics::Graphics()
{
	m_antiAliasing = true;
	m_reflection = true;
	m_cullFace = true;
	m_verticalSync = true;
	m_geometryQuality = LOW;
}

Graphics::~Graphics()
{
	for(int i=0; i<NUM_QUADS; ++i){
		gluDeleteQuadric(m_quad[i]);
	}
}

Graphics& Graphics::inst(void)
{
	static Graphics graphics;
	return graphics;
}

int Graphics::init(void)
{
	if(!m_verticalSync){
		enableVerticalSync(false);
	}

	initDevIL();
	initFonts();

	// basic lighting from above
	float ambient[] = {1.0f, 1.0f, 1.0f, 1.0f};
	float diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
	float specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
	float lightPos[] = {0.0f, 100.0f, 0.0f, 1.0f};

	glEnable(GL_COLOR_MATERIAL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glClearStencil(0);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
	glShadeModel(GL_SMOOTH); // graphics modes
	glDepthFunc(GL_LEQUAL);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// enable sphere mapping
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

	// enable texturing
	glEnable(GL_TEXTURE_2D);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPos);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT1);

	// quadrics
	for(int i=0; i<5; ++i){
		m_quad[i] = gluNewQuadric();
		gluQuadricNormals(m_quad[i], GLU_SMOOTH);
		gluQuadricTexture(m_quad[i], GL_TRUE);
	}

	if(ExtensionSupported("GL_EXT_texture_filter_anisotropic")){
		glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_anisotropy);
	}
	else{
		m_anisotropy = 1;
	}

	return true;
}

void Graphics::initDevIL(void)
{
	ilInit();
	iluInit(); // check if these are all needed
	ilutInit();

	ilEnable(IL_CONV_PAL);
	ilutEnable(ILUT_OPENGL_CONV);
}

void Graphics::initFonts(void)
{
	extern GLFont g_font;

	if(!g_font.create("Segoe UI", 9, GLFont::BOLD)){
		throw std::runtime_error("Failed to create g_font");
	}
}

void Graphics::allocDebugConsole(void)
{
	if(AllocConsole()){
		freopen("CONOUT$", "w", stdout);
		SetConsoleTitle("Ethereal Chess Debug Console");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
	}
}

int Graphics::loadResources(void)
{
	// create a thread so the game can display a loading screen
	extern GLuint g_textures[];
	extern GLuint g_texFonts[];

	Game& game = Game::inst();

	m_loadingResources = true;

	// load the textures within this context 
	Game::inst().appendStatusStream("Loading textures...");
	//clamp(true);
	loadTexture(g_textures[Game::TEX_BOARD], "Data/Images/Board/board.jpg");
	
	loadTexture(g_textures[Game::TEX_CREDITS], "Data/Images/thecredits.png");
	clamp(true);
	loadTexture(g_textures[Game::TEX_BOARD_BOX], "Data/Images/Board/frame-tiled-marble.jpg");
	g_textures[Game::TEX_BOARD_FRAME] = g_textures[Game::TEX_BOARD_BOX];
	//loadTexture(g_textures[Game::TEX_BOARD_FRAME], "Data/Images/Board/frame-tiled-marble.jpg");
	if(game.getPlanet() == Game::PLANET_EARTH){
		loadTexture(g_textures[Game::TEX_EARTH], "Data/Images/earth.jpg");
	}
	else if(game.getPlanet() == Game::PLANET_EARTH_NIGHT){
		loadTexture(g_textures[Game::TEX_EARTH], "Data/Images/earth-night.jpg");
	}
	loadTexture(g_textures[Game::TEX_SATURN], "Data/Images/saturn.jpg");
	loadTexture(g_textures[Game::TEX_SATURN_RINGS], "Data/Images/saturn-rings.jpg");
	clamp(false);
	loadTexture(g_textures[Game::TEX_PAUSEMENU_RESUME], "Data/Images/resume.png");
	loadTexture(g_textures[Game::TEX_PAUSEMENU_NEWGAME], "Data/Images/newgame.png");
	loadTexture(g_textures[Game::TEX_PAUSEMENU_LOADGAME], "Data/Images/loadgame.png");
	loadTexture(g_textures[Game::TEX_PAUSEMENU_SAVEGAME], "Data/Images/savegame.png");
	loadTexture(g_textures[Game::TEX_PAUSEMENU_OPTIONS], "Data/Images/options.png");
	loadTexture(g_textures[Game::TEX_PAUSEMENU_HELP], "Data/Images/help.png");
	loadTexture(g_textures[Game::TEX_PAUSEMENU_CREDITS], "Data/Images/credits.png");
	loadTexture(g_textures[Game::TEX_PAUSEMENU_QUIT], "Data/Images/exit.png");
	reloadPieceTextures();

	// load 2D textures
	loadTexture(g_textures[Game::TEX_2D_PAWN_WHITE], "Data/Images/Pieces/2D/pawn_white.png");
	loadTexture(g_textures[Game::TEX_2D_PAWN_BLACK], "Data/Images/Pieces/2D/pawn_black.png");
	loadTexture(g_textures[Game::TEX_2D_ROOK_WHITE], "Data/Images/Pieces/2D/rook_white.png");
	loadTexture(g_textures[Game::TEX_2D_ROOK_BLACK], "Data/Images/Pieces/2D/rook_black.png");
	loadTexture(g_textures[Game::TEX_2D_KNIGHT_WHITE], "Data/Images/Pieces/2D/knight_white.png");
	loadTexture(g_textures[Game::TEX_2D_KNIGHT_BLACK], "Data/Images/Pieces/2D/knight_black.png");
	loadTexture(g_textures[Game::TEX_2D_BISHOP_WHITE], "Data/Images/Pieces/2D/bishop_white.png");
	loadTexture(g_textures[Game::TEX_2D_BISHOP_BLACK], "Data/Images/Pieces/2D/bishop_black.png");
	loadTexture(g_textures[Game::TEX_2D_QUEEN_WHITE], "Data/Images/Pieces/2D/queen_white.png");
	loadTexture(g_textures[Game::TEX_2D_QUEEN_BLACK], "Data/Images/Pieces/2D/queen_black.png");
	loadTexture(g_textures[Game::TEX_2D_KING_WHITE], "Data/Images/Pieces/2D/king_white.png");
	loadTexture(g_textures[Game::TEX_2D_KING_BLACK], "Data/Images/Pieces/2D/king_black.png");
	
	// skybox textures
	reloadSkyboxTextures();

	// ring
	loadRadialRing();

	// load the board list
	loadBoard();

	// load default models
	Game::inst().appendStatusStream("Loading native models...");
	loadDefaultModels();

	(void)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&initResourceThread, this, 0, 0);

	return true;
}

unsigned long WINAPI Graphics::initResourceThread(void* lpThread)
{
	if(lpThread){
		return ((Graphics*)lpThread)->_loadResources(NULL);
	}

	return -1;
}

DWORD WINAPI Graphics::_loadResources(LPVOID lpBuffer)
{
	// 3d models, music, sounds, etc...
	Game& game = Game::inst();
	extern ModelOBJ g_ring;

	// models
	game.appendStatusStream("Loading models...");
	reloadModels();
	game.appendStatusStream("Models loaded.");

	m_loadingResources = false;
	
	ExitThread(0);
}

void Graphics::reloadModels(void)
{
	extern ModelOBJ g_models[];
	const int numModels = 6;
	const char* modelNames[] = {"pawn", "rook", "knight", "bishop", "queen", "king", 0};
	char buf[MAX_PATH] = {0};
	char modelPath[MAX_PATH] = {0};

	switch(Game::inst().getChessSet()){
		case Game::SET_NORMAL:
			strcpy(buf, "Data/Models/Normal");
			break;

		case Game::SET_FANCY:
			strcpy(buf, "Data/Models/Fancy");
			break;

		default:
			return;
	}

	Game::inst().setSetLoaded(Game::SET_NORMAL, false);
	Game::inst().setSetLoaded(Game::SET_FANCY, false);

	// load the piece models
	for(int i=0; i<numModels; ++i){
		wsprintf(modelPath, "%s/%s/%s.obj", buf, modelNames[i], modelNames[i]);
		char status[1024] = {0};

		if(Game::inst().getState() == Game::STATE_LOADING){
			sprintf(status, "Loading %s...", modelNames[i]);
			Game::inst().appendStatusStream(status);
		}

		// remove any previously loaded models
		g_models[i].unload();

		if(g_models[i].import(modelPath)){
			g_models[i].normalize();

			/*for(int j=0; j<g_models[j].getNumberOfMaterials(); ++j){
				const ModelOBJ::Material& material = g_models[j].getMaterial(j);

				if(material.colorMapFilename.empty())
					continue;

				std::string objName(modelNames[i]);
			}*/

			g_models[i].setLoaded(true);
		}
	}

	Game::inst().setSetLoaded(Game::inst().getChessSet(), true);
}

void Graphics::reloadSkyboxTextures(void)
{
	extern GLuint g_textures[];
	Game& game = Game::inst();
	char* texNames[] = {"north", "south", "east", "west", "up", "down", 0};
	char path[MAX_PATH] = "Data/Images/Sky";
	char buf[MAX_PATH] = {0};
	char ext[32] = "jpg";

	switch(game.getSkybox()){
		case Game::SKYBOX_SPACE:
		default:
			strcat(path, "/space");
			sprintf(buf, "%s/%s.%s", path, "sky", ext);
			clamp(true);
			for(int i=0; i<6; ++i){
				loadTexture(g_textures[i + Game::TEX_SKY_NORTH], buf);
			}			
			clamp(false);
			return;
		
		case Game::SKYBOX_MOONLIGHT:
			strcat(path, "/moonlight");
			strcpy(ext, "bmp");
			break;

		case Game::SKYBOX_DESERT:
			strcat(path, "/desert");
			strcpy(ext, "bmp");
			break;
	}

	game.appendStatusStream("Loading skybox...");

	clamp(true);

	for(int i=0; i<6; ++i){
		sprintf(buf, "%s/%s.%s", path, texNames[i], ext);

		loadTexture(g_textures[Game::TEX_SKY_NORTH + i], buf);
	}

	clamp(false);
}

void Graphics::reloadPieceTextures(void)
{
	extern GLuint g_textures[];

	// load textures for 3D pieces
	switch(Game::inst().getTextureMode()){
		default:
		case Game::METALLIC:
			loadTexture(g_textures[Game::TEX_WHITE], "Data/Images/Pieces/white_metallic.bmp");
			loadTexture(g_textures[Game::TEX_BLACK], "Data/Images/Pieces/black_metallic.bmp");
			break;

		case Game::GOLD:
			loadTexture(g_textures[Game::TEX_WHITE], "Data/Images/Pieces/gold.jpg");
			loadTexture(g_textures[Game::TEX_BLACK], "Data/Images/Pieces/silver.jpg");
			break;

		case Game::GOLD_SHINY:
			loadTexture(g_textures[Game::TEX_WHITE], "Data/Images/Pieces/gold_shiny.jpg");
			loadTexture(g_textures[Game::TEX_BLACK], "Data/Images/Pieces/silver_shiny.jpg");
			break;

		case Game::GLASS:
			loadTexture(g_textures[Game::TEX_WHITE], "Data/Images/Pieces/water_drops_on_glass.jpg");
			loadTexture(g_textures[Game::TEX_BLACK], "Data/Images/Pieces/glass_rainbow.jpg");
			break;

		case Game::MARBLE:
			clamp(true);
			loadTexture(g_textures[Game::TEX_WHITE], "Data/Images/Pieces/white_marble.jpg");
			loadTexture(g_textures[Game::TEX_BLACK], "Data/Images/Pieces/black_marble.jpg");
			clamp(false);
			break;
	}
}

void Graphics::loadBoard(void)
{
	extern GLuint g_boardList, g_boardFrame, g_boardBox;
	const float BOARD_WIDTH = 2.0f;
	const float BOARD_HEIGHT = BOARD_WIDTH;
	float BOX_HEIGHT;
	if(m_antiAliasing){
		BOX_HEIGHT = -0.5f;
	}
	else{
		BOX_HEIGHT = -0.9f; // without AA, reflections appear underneath, WTF
	}

	g_boardList = glGenLists(1);
	glNewList(g_boardList, GL_COMPILE);
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.0f);

		// lower-left
		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
		glVertex3f(-BOARD_WIDTH, 0.0f, BOARD_HEIGHT);

		// lower-right
		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 0.0f);
		glVertex3f(BOARD_WIDTH, 0.0f, BOARD_HEIGHT);

		// upper-right
		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 1.0f);
		glVertex3f(BOARD_WIDTH, 0.0f, -BOARD_HEIGHT);

		// upper-left
		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 1.0f);
		glVertex3f(-BOARD_WIDTH, 0.0f, -BOARD_HEIGHT);
	glEnd();
	glEndList();

	// board frame
	const float FRAME_WIDTH = BOARD_WIDTH + 0.5f;
	float FRAME_HEIGHT = -0.0005f;
		
	g_boardFrame = glGenLists(1);
	glNewList(g_boardFrame, GL_COMPILE);

	if(m_boardMode == STACKED){
		glBegin(GL_QUADS);
			glNormal3f(0.0f, 1.0f, 0.0f);

			// lower-left
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
			glVertex3f(-FRAME_WIDTH, FRAME_HEIGHT, FRAME_WIDTH);

			// lower-right
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 0.0f);
			glVertex3f(FRAME_WIDTH, FRAME_HEIGHT, FRAME_WIDTH);

			// upper-right
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 1.0f);
			glVertex3f(FRAME_WIDTH, FRAME_HEIGHT, -FRAME_WIDTH);

			// upper-left
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 1.0f);
			glVertex3f(-FRAME_WIDTH, FRAME_HEIGHT, -FRAME_WIDTH);	
		glEnd();
		glEndList();
	}
	else{
		FRAME_HEIGHT = 0.0f;
		const float FRAME_DIFF = FRAME_WIDTH - BOARD_WIDTH;
		
		glBegin(GL_QUADS);
			glNormal3f(0.0f, 1.0f, 0.0f);

			// left piece
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
			glVertex3f(-FRAME_WIDTH, FRAME_HEIGHT, -FRAME_WIDTH);

			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 0.0f);
			glVertex3f(-FRAME_WIDTH, FRAME_HEIGHT, FRAME_WIDTH);

			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 1.0f);
			glVertex3f(-BOARD_WIDTH, FRAME_HEIGHT, BOARD_WIDTH);

			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 1.0f);
			glVertex3f(-BOARD_WIDTH, FRAME_HEIGHT, -BOARD_WIDTH);

			// bottom piece
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
			glVertex3f(-FRAME_WIDTH, FRAME_HEIGHT, FRAME_WIDTH);

			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 0.0f);
			glVertex3f(FRAME_WIDTH, FRAME_HEIGHT, FRAME_WIDTH);

			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 1.0f);
			glVertex3f(BOARD_WIDTH, FRAME_HEIGHT, BOARD_WIDTH);

			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 1.0f);
			glVertex3f(-BOARD_WIDTH, FRAME_HEIGHT, BOARD_WIDTH);

			// right piece
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
			glVertex3f(FRAME_WIDTH, FRAME_HEIGHT, FRAME_WIDTH);

			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 0.0f);
			glVertex3f(FRAME_WIDTH, FRAME_HEIGHT, -FRAME_WIDTH);

			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 1.0f);
			glVertex3f(BOARD_WIDTH, FRAME_HEIGHT, -BOARD_WIDTH);

			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 1.0f);
			glVertex3f(BOARD_WIDTH, FRAME_HEIGHT, BOARD_WIDTH);

			// top piece
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
			glVertex3f(FRAME_WIDTH, FRAME_HEIGHT, -FRAME_WIDTH);

			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 0.0f);
			glVertex3f(-FRAME_WIDTH, FRAME_HEIGHT, -FRAME_WIDTH);

			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 1.0f);
			glVertex3f(-BOARD_WIDTH, FRAME_HEIGHT, -BOARD_WIDTH);

			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 1.0f);
			glVertex3f(BOARD_WIDTH, FRAME_HEIGHT, -BOARD_WIDTH);
		glEnd();
		glEndList();
	}

	// board box
	g_boardBox = glGenLists(1);
	glNewList(g_boardBox, GL_COMPILE);
	glBegin(GL_QUADS);
		
		// right
		glNormal3f(1.0f, 0.0f, 0.0f);

		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
		glVertex3f(FRAME_WIDTH, BOX_HEIGHT, FRAME_WIDTH);

		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 0.0f);
		glVertex3f(FRAME_WIDTH, BOX_HEIGHT, -FRAME_WIDTH);

		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 1.0f);
		glVertex3f(FRAME_WIDTH, 0.0f, -FRAME_WIDTH);

		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 1.0f);
		glVertex3f(FRAME_WIDTH, 0.0f, FRAME_WIDTH);

		// front
		glNormal3f(0.0f, 0.0f, 1.0f);

		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
		glVertex3f(-FRAME_WIDTH, BOX_HEIGHT, FRAME_WIDTH);

		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 0.0f);
		glVertex3f(FRAME_WIDTH, BOX_HEIGHT, FRAME_WIDTH);

		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 1.0f);
		glVertex3f(FRAME_WIDTH, 0.0f, FRAME_WIDTH);

		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 1.0f);
		glVertex3f(-FRAME_WIDTH, 0.0f, FRAME_WIDTH);

		// left
		glNormal3f(-1.0f, 0.0f, 0.0f);

		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
		glVertex3f(-FRAME_WIDTH, BOX_HEIGHT, -FRAME_WIDTH);

		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 0.0f);
		glVertex3f(-FRAME_WIDTH, BOX_HEIGHT, FRAME_WIDTH);

		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 1.0f);
		glVertex3f(-FRAME_WIDTH, 0.0f, FRAME_WIDTH);

		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 1.0f);
		glVertex3f(-FRAME_WIDTH, 0.0f, -FRAME_WIDTH);

		// back
		glNormal3f(0.0f, 0.0f, -1.0f);

		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 0.0f);
		glVertex3f(FRAME_WIDTH, BOX_HEIGHT, -FRAME_WIDTH);

		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 0.0f);
		glVertex3f(-FRAME_WIDTH, BOX_HEIGHT, -FRAME_WIDTH);

		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1.0f, 1.0f);
		glVertex3f(-FRAME_WIDTH, 0.0f, -FRAME_WIDTH);

		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0.0f, 1.0f);
		glVertex3f(FRAME_WIDTH, 0.0f, -FRAME_WIDTH);
	glEnd();
	glEndList();
}

bool Graphics::loadSettings(void)
{
	// load from .ini file here
	m_antiAliasing = true;

	return true;
}

bool Graphics::saveSettings(void)
{

	return true;
}

void Graphics::setProcessorAffinity(void)
{
    // Assign the current thread to one processor. This ensures that timing
    // code runs on only one processor, and will not suffer any ill effects
    // from power management.
    //
    // Based on the DXUTSetProcessorAffinity() function in the DXUT framework.

    DWORD_PTR dwProcessAffinityMask = 0;
    DWORD_PTR dwSystemAffinityMask = 0;
    HANDLE hCurrentProcess = GetCurrentProcess();

    if (!GetProcessAffinityMask(hCurrentProcess, &dwProcessAffinityMask, &dwSystemAffinityMask))
        return;

    if (dwProcessAffinityMask)
    {
        // Find the lowest processor that our process is allowed to run against.

        DWORD_PTR dwAffinityMask = (dwProcessAffinityMask & ((~dwProcessAffinityMask) + 1));

        // Set this as the processor that our thread must always run against.
        // This must be a subset of the process affinity mask.

        HANDLE hCurrentThread = GetCurrentThread();

        if (hCurrentThread != INVALID_HANDLE_VALUE)
        {
            SetThreadAffinityMask(hCurrentThread, dwAffinityMask);
            CloseHandle(hCurrentThread);
        }
    }

    CloseHandle(hCurrentProcess);
}

bool Graphics::initPixels(HDC& hDC, HGLRC& hRC)
{
	int pf = 0;
	PIXELFORMATDESCRIPTOR pfd = {0};
	OSVERSIONINFO osvi = {0};

	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if(!GetVersionEx(&osvi)){
		throw std::runtime_error("GetVersionEx() failed");
		return false;
	}

	// support desktop composition for Windows greater than 6.0
	if(osvi.dwMajorVersion > 6 || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 0)){
		pfd.dwFlags |= PFD_SUPPORT_COMPOSITION;
	}

	// enable antialiasing if specified
	if(m_antiAliasing){
		int maxSamples;
		ChooseBestMultiSampleAntiAliasingPixelFormat(pf, maxSamples);
	}

	if(!pf){
		m_antiAliasing = false; // AA failed lol
		pf = ChoosePixelFormat(hDC, &pfd);
	}

	if(!SetPixelFormat(hDC, pf, &pfd)){
		throw std::runtime_error("SetPixelFormat() failed");
		return false;
	}

	if(!(hRC = wglCreateContext(hDC))){
		throw std::runtime_error("wglCreateContext() failed");
		return false;
	}

	if(!wglMakeCurrent(hDC, hRC)){
		throw std::runtime_error("wglMakeCurrent() failed");
		return false;
	}

	return true;
}

void Graphics::enableVerticalSync(bool enableVerticalSync)
{
    // WGL_EXT_swap_control.

    typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC)(GLint);

    static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT =
        reinterpret_cast<PFNWGLSWAPINTERVALEXTPROC>(
        wglGetProcAddress("wglSwapIntervalEXT"));

    if (wglSwapIntervalEXT)
    {
        wglSwapIntervalEXT(enableVerticalSync ? 1 : 0);
    }

	m_verticalSync = enableVerticalSync;
}

void Graphics::toggleFullScreen(HWND hwnd)
{
	extern Cam g_cam;
    static DWORD savedExStyle;
    static DWORD savedStyle;
    static RECT rcSaved;

    m_fullscreen = !m_fullscreen;

    if (m_fullscreen)
    {
        // Moving to full screen mode.

        savedExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        savedStyle = GetWindowLong(hwnd, GWL_STYLE);
        GetWindowRect(hwnd, &rcSaved);

        SetWindowLongPtr(hwnd, GWL_EXSTYLE, 0);
        SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

        m_width = GetSystemMetrics(SM_CXSCREEN);
        m_height = GetSystemMetrics(SM_CYSCREEN);

        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0,
            m_width, m_height, SWP_SHOWWINDOW);
    }
    else
    {
        // Moving back to windowed mode.

		//savedStyle &= ~(WS_BORDER);
        SetWindowLong(hwnd, GWL_EXSTYLE, savedExStyle);
        SetWindowLong(hwnd, GWL_STYLE, savedStyle);
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

        m_width = rcSaved.right - rcSaved.left;
        m_height = rcSaved.bottom - rcSaved.top;

        SetWindowPos(hwnd, HWND_NOTOPMOST, rcSaved.left, rcSaved.top,
            m_width, m_height, SWP_SHOWWINDOW);

		//ChangeDisplaySettings(NULL, 0);
    }

	g_cam.perspective(CAMERA_FOVX, static_cast<float>(m_width) / static_cast<float>(m_height), CAMERA_ZNEAR, CAMERA_ZFAR);
}

void Graphics::updateFrameRate(float elapsedTimeSec)
{
	static float accumTimeSec = 0.0f;
	static int frames = 0;

	accumTimeSec += elapsedTimeSec;

	if(accumTimeSec > 1.0f){
		m_fps = frames;

		frames = 0;
		accumTimeSec = 0.0f;
	}
	else{
		++frames;
	}
}

/* converts mouse window coordinates to 3-space coordinates */
Vec3 Graphics::getMouseVec3(void)
{
	extern Cam g_cam;
	GLint mouseX = Mouse::inst().xPosAbsolute();
	GLint mouseY = Mouse::inst().yPosAbsolute();
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY, winZ;
	GLdouble x, y, z;

	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	winX = (GLfloat)mouseX;
	winY = (GLfloat)viewport[3] - (GLfloat)mouseY;
	glReadPixels(mouseX, GLint(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &x, &y, &z);

	Vec3 pos(x, y, z);

	pos = pos * g_cam.getViewProjectionMatrix();
	return pos;
}

static float cos_sin_theta[2*(MAX_SLICES+1)];

static void ComputeCosSinTheta(float phi,int segments)
{
	float *cos_sin = cos_sin_theta;
	float *cos_sin_rev = cos_sin + 2*(segments+1);
	const float c = std::cos(phi);
	const float s = std::sin(phi);
	*cos_sin++ = 1.f;
	*cos_sin++ = 0.f;
	*--cos_sin_rev = -cos_sin[-1];
	*--cos_sin_rev =  cos_sin[-2];
	*cos_sin++ = c;
	*cos_sin++ = s;
	*--cos_sin_rev = -cos_sin[-1];
	*--cos_sin_rev =  cos_sin[-2];
	while (cos_sin < cos_sin_rev)
	{
		cos_sin[0] = cos_sin[-2]*c - cos_sin[-1]*s;
		cos_sin[1] = cos_sin[-2]*s + cos_sin[-1]*c;
		cos_sin += 2;
		*--cos_sin_rev = -cos_sin[-1];
		*--cos_sin_rev =  cos_sin[-2];
	}
}

void Graphics::loadRadialRing(void)
{
	const float rMin = 375.0f;
	const float rMax = 750.0f;
	const int	slices = (m_geometryQuality >= HIGH) ? 128 : 64; 
	const int	stacks = (m_geometryQuality >= HIGH) ? 128 : 64;
	const float dr = (rMax - rMin) / stacks;
	const float dtheta = 2.0f * PI / slices;
	float* cos_sin_theta_p;
	int x, y, j;

	assert(slices <= MAX_SLICES);
	ComputeCosSinTheta(dtheta, slices);

	g_ringList = glGenLists(1);
	glNewList(g_ringList, GL_COMPILE);
			for(float r=rMin; r<rMax; r+=dr){
			const float tex_r0 = (r-rMin)/(rMax-rMin);
			const float tex_r1 = (r+dr-rMin)/(rMax-rMin);

			glBegin(GL_QUAD_STRIP);
			for(j=0, cos_sin_theta_p=cos_sin_theta; j<=slices; ++j, cos_sin_theta_p+=2){
				x = r * cos_sin_theta_p[0];
				y = r * cos_sin_theta_p[1];

				glVertex3f(x, y, 0.0f);
				glTexCoord2f(tex_r0, 0.5f);

				x = (r + dr) * cos_sin_theta_p[0];
				y = (r + dr) * cos_sin_theta_p[1];

				glVertex3f(x, y, 0.0f);
				glTexCoord2f(tex_r1, 0.5f);
			}
			glEnd();
		}
	glEndList();
}

void Graphics::renderRadialRing(void)
{
	glCallList(g_ringList);
}

bool Graphics::loadTexture(GLuint& id, const char* file)
{
	ILuint temp_id;
	unsigned int result = 0;

	if(wglGetCurrentContext() == NULL){
		throw std::runtime_error("wglGetCurrentContext() = NULL");
		return false;
	}

	// test the version of DevIL
	if(ilGetInteger(IL_VERSION_NUM) < IL_VERSION ||
	   iluGetInteger(ILU_VERSION_NUM) < ILU_VERSION ||
	   ilutGetInteger(ILUT_VERSION_NUM) < ILUT_VERSION){
		   MessageBox(GetForegroundWindow(), "The image library version is invalid, unable to load images.", 0, MB_ICONERROR);
		   return false;
	}

	ilGenImages(1, &temp_id);
	ilBindImage(temp_id);

	// check for bitmap format
	if(ext(file, "bmp")){
		result = ilLoad(IL_BMP, file);
	}
	// generic image loading
	else{
		result = ilLoadImage(file);
	}

	if(!result){
		ilDeleteImages(1, &temp_id);

		char buf[1024] = {0};

		ILenum error;
		while((error = ilGetError()) != IL_NO_ERROR){
			sprintf(buf, "%d: %s (%s)", error, iluErrorString(error), file);
			MessageBox(GetForegroundWindow(), buf, 0, MB_ICONERROR);
		}

		return false;
	}

	if(!ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE)){
		return false;
	}

	// retrieve the image data
	m_imageWidth		= ilGetInteger(IL_IMAGE_WIDTH);
	m_imageHeight	= ilGetInteger(IL_IMAGE_HEIGHT);
	m_size		= ilGetInteger(IL_IMAGE_SIZE_OF_DATA);
	m_bpp		= ilGetInteger(IL_IMAGE_BPP);
	m_format	= ilGetInteger(IL_IMAGE_FORMAT);
	m_data		= (char*)ilGetData();

	// generate the texture
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexImage2D(GL_TEXTURE_2D, 4, m_bpp, m_imageWidth, m_imageHeight, 0, m_format, GL_UNSIGNED_BYTE, m_data);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	if(Graphics::inst().getAnisotropy() > 1){
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, Graphics::inst().getAnisotropy());
	}

	gluBuild2DMipmaps(GL_TEXTURE_2D, m_format, m_imageWidth, m_imageHeight, m_format, GL_UNSIGNED_BYTE, m_data);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	if(m_clamp){
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	ilDeleteImages(1, &temp_id);

	return true;
}

void Graphics::setMode(unsigned mode)
{
	if(mode == PROJ){
		/*glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);*/
		glViewport(0, 0, m_width, m_height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		glEnable(GL_DEPTH_TEST);
		
		m_mode = PROJ;
	}
	else if(mode == ORTHO){
		int viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0, viewport[2], viewport[3], 0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glDisable(GL_DEPTH_TEST);

		/*glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, m_width, 0, m_height);
		glScalef(1.0f, -1.0f, 1.0f);
		glTranslatef(0.0f, (GLfloat)-m_height, 0.0f);
		glMatrixMode(GL_MODELVIEW);*/
		m_mode = ORTHO;
	}
}

void Graphics::loadDefaultModels(void)
{
	extern GLuint g_defModels[];
	const float PAWN_HEIGHT = 2.0f;
	static GLUquadric* knightEye	= gluNewQuadric();
	static GLUquadric* queenSphere  = gluNewQuadric();
	
	const float ROOK_HEIGHT_MID = 1.3f;
	const float ROOK_HEIGHT_TOP = ROOK_HEIGHT_MID * 2.0f;
	const float ROOK_LOWER_WIDTH = 1.0f;
	const float ROOK_UPPER_WIDTH = 0.75f;

	const float KNIGHT_HEIGHT_MID	= 1.4f;
	const float KNIGHT_HEIGHT_TOP	= KNIGHT_HEIGHT_MID * 1.5f;
	const float KNIGHT_HEAD_WIDTH	= 0.3f;
	const float KNIGHT_HEAD_HEIGHT	= 0.5f;
	const float KNIGHT_LOWER_WIDTH	= 1.0f;
	const float KNIGHT_UPPER_WIDTH	= 0.4f;
	const float KNIGHT_SNOUT_LENGTH	= 1.3f;
	const float KNIGHT_SNOUT_DROP	= 0.3f;
	const float KNIGHT_EAR_HEIGHT	= 0.35f;
	const float KNIGHT_EAR_WIDTH	= 0.1f;
	const float KNIGHT_EYE_SIZE		= 0.7f;

	const float BISHOP_HEIGHT_MID	= 2.2f;
	const float BISHOP_NECK_WIDTH	= 0.25f;
	const float BISHOP_HEAD_HEIGHT  = 1.68f;
	const float BISHOP_HEAD_HEIGHT_MID = BISHOP_HEIGHT_MID + (BISHOP_HEAD_HEIGHT / 2.0f);
	const float BISHOP_HEIGHT_TOP	= BISHOP_HEIGHT_MID + BISHOP_HEAD_HEIGHT;
	const float BISHOP_HEAD_WIDTH	= 0.5f;

	const float QUEEN_HEIGHT_MID	= 3.0f;
	const float QUEEN_NECK_WIDTH	= 0.35f;
	const float QUEEN_HEAD_HEIGHT	= 2.0f;
	const float QUEEN_HEAD_HEIGHT_MID = QUEEN_HEIGHT_MID + (QUEEN_HEAD_HEIGHT / 2.0f);
	const float QUEEN_HEIGHT_TOP	= QUEEN_HEIGHT_MID + QUEEN_HEAD_HEIGHT - 0.5f;
	const float QUEEN_HEAD_WIDTH	= 0.65f;
	const float QUEEN_SPHERE_SIZE	= 2.0f;

	const float KING_HEIGHT_MID		= 3.5f;
	const float KING_NECK_WIDTH		= 0.40f;
	const float KING_HEAD_HEIGHT	= 2.2f;
	const float KING_HEAD_HEIGHT_MID = KING_HEIGHT_MID + (KING_HEAD_HEIGHT / 2.0f);
	const float KING_HEIGHT_TOP		= 0.0f;
	const float KING_HEAD_WIDTH		= 0.70f;
	const float KING_CROSS_SIZE		= KING_HEAD_WIDTH / 2.0f;
	const float KING_CROSS_HEIGHT	= 0.6f;

	// -------------- //
	// NOTE: "front face" is drawn counter-clockwise (relevant for face culling)

	// pawn
	g_defModels[0] = glGenLists(1);
	glNewList(g_defModels[0], GL_COMPILE);

	// left side
	glBegin(GL_TRIANGLES);
		glNormal3f(-1.0f, 1.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, 0.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, PAWN_HEIGHT, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 0.0f, -1.0f);
	glEnd();

	// front side
	glBegin(GL_TRIANGLES);
		glNormal3f(0.0f, 1.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 0.0f, 1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, PAWN_HEIGHT, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 0.0f, 1.0f);
	glEnd();

	// right side
	glBegin(GL_TRIANGLES);
		glNormal3f(1.0f, 1.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 0.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, PAWN_HEIGHT, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 0.0f, 1.0f);
	glEnd();

	// back side
	glBegin(GL_TRIANGLES);
		glNormal3f(0.0f, 1.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, 0.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, PAWN_HEIGHT, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 0.0f, -1.0f);
	glEnd();

	glEndList();

	// -------------- //

	/*
	glBegin(GL_QUADS);
		glNormal3f();
		glTexCoord2f(0.0f, 0.0f); glVertex3f();
		glTexCoord2f(1.0f, 0.0f); glVertex3f();
		glTexCoord2f(1.0f, 1.0f); glVertex3f();
		glTexCoord2f(0.0f, 1.0f); glVertex3f();
	glEnd();
	*/

	// rook
	g_defModels[1] = glGenLists(1);
	glNewList(g_defModels[1], GL_COMPILE);

	// lower left side
	glBegin(GL_QUADS);
		glNormal3f(-0.5f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-ROOK_LOWER_WIDTH, 0.0f, -ROOK_LOWER_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-ROOK_LOWER_WIDTH, 0.0f, ROOK_LOWER_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, ROOK_HEIGHT_MID, 0.5f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, ROOK_HEIGHT_MID, -0.5f);
	glEnd();

	// upper left side
	glBegin(GL_QUADS);
		glNormal3f(-0.5, -1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, ROOK_HEIGHT_MID, -0.5f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, ROOK_HEIGHT_MID, 0.5f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-ROOK_UPPER_WIDTH, ROOK_HEIGHT_TOP, ROOK_UPPER_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-ROOK_UPPER_WIDTH, ROOK_HEIGHT_TOP, -ROOK_UPPER_WIDTH);
	glEnd();

	// lower front side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.5f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-ROOK_LOWER_WIDTH, 0.0f, ROOK_LOWER_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(ROOK_LOWER_WIDTH, 0.0f, ROOK_LOWER_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, ROOK_HEIGHT_MID, 0.5f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, ROOK_HEIGHT_MID, 0.5f);
	glEnd();

	// upper front side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, -1.0f, 0.5f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, ROOK_HEIGHT_MID, 0.5f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, ROOK_HEIGHT_MID, 0.5f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(ROOK_UPPER_WIDTH, ROOK_HEIGHT_TOP, ROOK_UPPER_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-ROOK_UPPER_WIDTH, ROOK_HEIGHT_TOP, ROOK_UPPER_WIDTH);
	glEnd();

	// lower right side
	glBegin(GL_QUADS);
		glNormal3f(0.5f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(ROOK_LOWER_WIDTH, 0.0f, ROOK_LOWER_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(ROOK_LOWER_WIDTH, 0.0f, -ROOK_LOWER_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, ROOK_HEIGHT_MID, -0.5f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, ROOK_HEIGHT_MID, 0.5f);
	glEnd();

	// upper right side
	glBegin(GL_QUADS);
		glNormal3f(0.5f, -1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, ROOK_HEIGHT_MID, 0.5f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, ROOK_HEIGHT_MID, -0.5f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(ROOK_UPPER_WIDTH, ROOK_HEIGHT_TOP, -ROOK_UPPER_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(ROOK_UPPER_WIDTH, ROOK_HEIGHT_TOP, ROOK_UPPER_WIDTH);
	glEnd();

	// lower back side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, -0.5f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(ROOK_LOWER_WIDTH, 0.0f, -ROOK_LOWER_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-ROOK_LOWER_WIDTH, 0.0f, -ROOK_LOWER_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, ROOK_HEIGHT_MID, -0.5f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, ROOK_HEIGHT_MID, -0.5f);
	glEnd();

	// upper back side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, -1.0f, -0.5f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, ROOK_HEIGHT_MID, -0.5f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, ROOK_HEIGHT_MID, -0.5f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-ROOK_UPPER_WIDTH, ROOK_HEIGHT_TOP, -ROOK_UPPER_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(ROOK_UPPER_WIDTH, ROOK_HEIGHT_TOP, -ROOK_UPPER_WIDTH);
	glEnd();

	// top plate
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-ROOK_UPPER_WIDTH, ROOK_HEIGHT_TOP, ROOK_UPPER_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(ROOK_UPPER_WIDTH, ROOK_HEIGHT_TOP, ROOK_UPPER_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(ROOK_UPPER_WIDTH, ROOK_HEIGHT_TOP, -ROOK_UPPER_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-ROOK_UPPER_WIDTH, ROOK_HEIGHT_TOP, -ROOK_UPPER_WIDTH);
	glEnd();

	glEndList();

	// -------------- //

	// knight
	g_defModels[2] = glGenLists(1);
	glNewList(g_defModels[2], GL_COMPILE);

	// lower left side
	glBegin(GL_QUADS);
		glNormal3f(-0.5f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-KNIGHT_LOWER_WIDTH, 0.0f, -KNIGHT_LOWER_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-KNIGHT_LOWER_WIDTH, 0.0f, KNIGHT_LOWER_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, KNIGHT_HEIGHT_MID, 0.5f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, KNIGHT_HEIGHT_MID, -0.5f);
	glEnd();

	// upper left side
	glBegin(GL_QUADS);
		glNormal3f(-0.5, -1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, KNIGHT_HEIGHT_MID, -0.5f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, KNIGHT_HEIGHT_MID, 0.5f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, KNIGHT_UPPER_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, -KNIGHT_UPPER_WIDTH);
	glEnd();

	// lower front side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.5f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-KNIGHT_LOWER_WIDTH, 0.0f, KNIGHT_LOWER_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(KNIGHT_LOWER_WIDTH, 0.0f, KNIGHT_LOWER_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, KNIGHT_HEIGHT_MID, 0.5f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, KNIGHT_HEIGHT_MID, 0.5f);
	glEnd();

	// upper front side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, -1.0f, 0.5f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, KNIGHT_HEIGHT_MID, 0.5f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, KNIGHT_HEIGHT_MID, 0.5f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, KNIGHT_UPPER_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, KNIGHT_UPPER_WIDTH);
	glEnd();

	// lower right side
	glBegin(GL_QUADS);
		glNormal3f(0.5f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(KNIGHT_LOWER_WIDTH, 0.0f, KNIGHT_LOWER_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(KNIGHT_LOWER_WIDTH, 0.0f, -KNIGHT_LOWER_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, KNIGHT_HEIGHT_MID, -0.5f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, KNIGHT_HEIGHT_MID, 0.5f);
	glEnd();

	// upper right side
	glBegin(GL_QUADS);
		glNormal3f(0.5f, -1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, KNIGHT_HEIGHT_MID, 0.5f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, KNIGHT_HEIGHT_MID, -0.5f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, -KNIGHT_UPPER_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, KNIGHT_UPPER_WIDTH);
	glEnd();

	// lower back side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, -0.5f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(KNIGHT_LOWER_WIDTH, 0.0f, -KNIGHT_LOWER_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-KNIGHT_LOWER_WIDTH, 0.0f, -KNIGHT_LOWER_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, KNIGHT_HEIGHT_MID, -0.5f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, KNIGHT_HEIGHT_MID, -0.5f);
	glEnd();

	// upper back side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, -1.0f, -0.5f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, KNIGHT_HEIGHT_MID, -0.5f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, KNIGHT_HEIGHT_MID, -0.5f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, -KNIGHT_UPPER_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, -KNIGHT_UPPER_WIDTH);
	glEnd();

	// head
	glBegin(GL_QUADS);
		// left plate
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, -KNIGHT_UPPER_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, KNIGHT_UPPER_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, KNIGHT_UPPER_WIDTH + KNIGHT_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, -KNIGHT_UPPER_WIDTH);

		// front plate
		glNormal3f(0.0f, -1.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, KNIGHT_UPPER_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, KNIGHT_UPPER_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, KNIGHT_UPPER_WIDTH + KNIGHT_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, KNIGHT_UPPER_WIDTH + KNIGHT_HEAD_WIDTH);

		// right plate
		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, KNIGHT_UPPER_WIDTH);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, -KNIGHT_UPPER_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, -KNIGHT_UPPER_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, KNIGHT_UPPER_WIDTH + KNIGHT_HEAD_WIDTH);

		// top plate
		glNormal3f(0.0f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, KNIGHT_UPPER_WIDTH + KNIGHT_HEAD_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, KNIGHT_UPPER_WIDTH + KNIGHT_HEAD_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, -KNIGHT_UPPER_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, -KNIGHT_UPPER_WIDTH);
	glEnd();

	// snout
	glBegin(GL_QUADS);
		// left plate
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP - KNIGHT_SNOUT_DROP, -KNIGHT_SNOUT_LENGTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, -KNIGHT_UPPER_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, -KNIGHT_UPPER_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, -KNIGHT_SNOUT_LENGTH);

		// bottom plate
		glNormal3f(0.0f, -1.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, -KNIGHT_UPPER_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP - KNIGHT_SNOUT_DROP, -KNIGHT_SNOUT_LENGTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP - KNIGHT_SNOUT_DROP, -KNIGHT_SNOUT_LENGTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, -KNIGHT_UPPER_WIDTH);
		
		// right plate
		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, -KNIGHT_UPPER_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP - KNIGHT_SNOUT_DROP, -KNIGHT_SNOUT_LENGTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, -KNIGHT_SNOUT_LENGTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, -KNIGHT_UPPER_WIDTH);

		// top plate
		glNormal3f(0.0f, 1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, -KNIGHT_SNOUT_LENGTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, -KNIGHT_UPPER_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, -KNIGHT_UPPER_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, -KNIGHT_SNOUT_LENGTH);

		// snout plate
		glNormal3f(0.0f, 0.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP - KNIGHT_SNOUT_DROP, -KNIGHT_SNOUT_LENGTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP - KNIGHT_SNOUT_DROP, -KNIGHT_SNOUT_LENGTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, -KNIGHT_SNOUT_LENGTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP, -KNIGHT_SNOUT_LENGTH);
	glEnd();

	// ears
	glBegin(GL_TRIANGLES);
		// left plates
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, KNIGHT_UPPER_WIDTH + KNIGHT_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT + KNIGHT_EAR_HEIGHT, (KNIGHT_UPPER_WIDTH));
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, (KNIGHT_UPPER_WIDTH / 2.0f));

		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, (KNIGHT_UPPER_WIDTH / 2.0f));
		glTexCoord2f(0.0f, 1.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT + KNIGHT_EAR_HEIGHT, (KNIGHT_UPPER_WIDTH));
		glTexCoord2f(0.0f, 0.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, KNIGHT_UPPER_WIDTH + KNIGHT_HEAD_WIDTH);
		
		// right plates
		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-(KNIGHT_UPPER_WIDTH - KNIGHT_EAR_WIDTH), KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, (KNIGHT_UPPER_WIDTH / 2.0f));
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-(KNIGHT_UPPER_WIDTH - KNIGHT_EAR_WIDTH), KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT + KNIGHT_EAR_HEIGHT, KNIGHT_UPPER_WIDTH);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-(KNIGHT_UPPER_WIDTH - KNIGHT_EAR_WIDTH), KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, KNIGHT_UPPER_WIDTH + KNIGHT_HEAD_WIDTH);

		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f((KNIGHT_UPPER_WIDTH - KNIGHT_EAR_WIDTH), KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, KNIGHT_UPPER_WIDTH + KNIGHT_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f((KNIGHT_UPPER_WIDTH - KNIGHT_EAR_WIDTH), KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT + KNIGHT_EAR_HEIGHT, KNIGHT_UPPER_WIDTH);
		glTexCoord2f(0.0f, 0.0f); glVertex3f((KNIGHT_UPPER_WIDTH - KNIGHT_EAR_WIDTH), KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, (KNIGHT_UPPER_WIDTH / 2.0f));
	glEnd();

	// ear plates
	glBegin(GL_QUADS);
		// left back
		glNormal3f(0.0f, 1.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, KNIGHT_UPPER_WIDTH + KNIGHT_HEAD_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-(KNIGHT_UPPER_WIDTH - KNIGHT_EAR_WIDTH), KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, KNIGHT_UPPER_WIDTH + KNIGHT_HEAD_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-(KNIGHT_UPPER_WIDTH - KNIGHT_EAR_WIDTH), KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT + KNIGHT_EAR_HEIGHT, KNIGHT_UPPER_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT + KNIGHT_EAR_HEIGHT, KNIGHT_UPPER_WIDTH);

		// right back
		glNormal3f(0.0f, 1.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f((KNIGHT_UPPER_WIDTH - KNIGHT_EAR_WIDTH), KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, KNIGHT_UPPER_WIDTH + KNIGHT_HEAD_WIDTH);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, KNIGHT_UPPER_WIDTH + KNIGHT_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT + KNIGHT_EAR_HEIGHT, KNIGHT_UPPER_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f((KNIGHT_UPPER_WIDTH - KNIGHT_EAR_WIDTH), KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT + KNIGHT_EAR_HEIGHT, KNIGHT_UPPER_WIDTH);

		// left front
		glNormal3f(0.0f, 1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-(KNIGHT_UPPER_WIDTH - KNIGHT_EAR_WIDTH), KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, (KNIGHT_UPPER_WIDTH / 2.0f));
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, (KNIGHT_UPPER_WIDTH / 2.0f));
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT + KNIGHT_EAR_HEIGHT, KNIGHT_UPPER_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-(KNIGHT_UPPER_WIDTH - KNIGHT_EAR_WIDTH), KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT + KNIGHT_EAR_HEIGHT, KNIGHT_UPPER_WIDTH);
		
		// right front
		glNormal3f(0.0f, 1.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, (KNIGHT_UPPER_WIDTH / 2.0f));
		glTexCoord2f(0.0f, 0.0f); glVertex3f((KNIGHT_UPPER_WIDTH - KNIGHT_EAR_WIDTH), KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT, (KNIGHT_UPPER_WIDTH / 2.0f));
		glTexCoord2f(0.0f, 1.0f); glVertex3f((KNIGHT_UPPER_WIDTH - KNIGHT_EAR_WIDTH), KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT + KNIGHT_EAR_HEIGHT, KNIGHT_UPPER_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT + KNIGHT_EAR_HEIGHT, KNIGHT_UPPER_WIDTH);
	glEnd();

	// eyes
	glPushMatrix();
	glTranslatef(-(KNIGHT_UPPER_WIDTH), KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT - KNIGHT_EYE_SIZE * 0.3f, -KNIGHT_UPPER_WIDTH);
	glScalef(0.12f, 0.1f, 0.12f);
	gluSphere(knightEye, KNIGHT_EYE_SIZE, 64, 64);
	glPopMatrix();

	glTranslatef(KNIGHT_UPPER_WIDTH, KNIGHT_HEIGHT_TOP + KNIGHT_HEAD_HEIGHT - KNIGHT_EYE_SIZE * 0.3f, -KNIGHT_UPPER_WIDTH);
	glScalef(0.12f, 0.1f, 0.12f);
	gluSphere(knightEye, KNIGHT_EYE_SIZE, 64, 64);

	glEndList();

	// -------------- //

	// bishop
	g_defModels[3] = glGenLists(1);
	glNewList(g_defModels[3], GL_COMPILE);

	// left side
	glBegin(GL_QUADS);
		glNormal3f(-0.5f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, 0.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-BISHOP_NECK_WIDTH, BISHOP_HEIGHT_MID, BISHOP_NECK_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-BISHOP_NECK_WIDTH, BISHOP_HEIGHT_MID, -BISHOP_NECK_WIDTH);
	glEnd();

	// front side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.5f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 0.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 0.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(BISHOP_NECK_WIDTH, BISHOP_HEIGHT_MID, BISHOP_NECK_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-BISHOP_NECK_WIDTH, BISHOP_HEIGHT_MID, BISHOP_NECK_WIDTH);
	glEnd();

	// right side
	glBegin(GL_QUADS);
		glNormal3f(0.5f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 0.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(BISHOP_NECK_WIDTH, BISHOP_HEIGHT_MID, -BISHOP_NECK_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(BISHOP_NECK_WIDTH, BISHOP_HEIGHT_MID, BISHOP_NECK_WIDTH);
	glEnd();

	// back side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, -0.5f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-BISHOP_NECK_WIDTH, BISHOP_HEIGHT_MID, -BISHOP_NECK_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(BISHOP_NECK_WIDTH, BISHOP_HEIGHT_MID, -BISHOP_NECK_WIDTH);
	glEnd();

	// the diamond head
	// lower left side
	glBegin(GL_QUADS);
		glNormal3f(-1.0f, -1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-BISHOP_NECK_WIDTH, BISHOP_HEIGHT_MID, -BISHOP_NECK_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-BISHOP_NECK_WIDTH, BISHOP_HEIGHT_MID, BISHOP_NECK_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-BISHOP_HEAD_WIDTH, BISHOP_HEAD_HEIGHT_MID, BISHOP_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-BISHOP_HEAD_WIDTH, BISHOP_HEAD_HEIGHT_MID, -BISHOP_HEAD_WIDTH);
	glEnd();

	// upper left side
	glBegin(GL_TRIANGLES);
		glNormal3f(-1.0f, 1.0f, 0.0f);	
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-BISHOP_HEAD_WIDTH, BISHOP_HEAD_HEIGHT_MID, BISHOP_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, BISHOP_HEIGHT_TOP, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-BISHOP_HEAD_WIDTH, BISHOP_HEAD_HEIGHT_MID, -BISHOP_HEAD_WIDTH);	
	glEnd();

	// lower front side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, -1.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-BISHOP_NECK_WIDTH, BISHOP_HEIGHT_MID, BISHOP_NECK_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(BISHOP_NECK_WIDTH, BISHOP_HEIGHT_MID, BISHOP_NECK_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(BISHOP_HEAD_WIDTH, BISHOP_HEAD_HEIGHT_MID, BISHOP_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-BISHOP_HEAD_WIDTH, BISHOP_HEAD_HEIGHT_MID, BISHOP_HEAD_WIDTH);
	glEnd();

	// upper front side
	glBegin(GL_TRIANGLES);
		glNormal3f(0.0f, 1.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(BISHOP_HEAD_WIDTH, BISHOP_HEAD_HEIGHT_MID, BISHOP_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, BISHOP_HEIGHT_TOP, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-BISHOP_HEAD_WIDTH, BISHOP_HEAD_HEIGHT_MID, BISHOP_HEAD_WIDTH);
	glEnd();

	// lower right side
	glBegin(GL_QUADS);
		glNormal3f(1.0f, -1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(BISHOP_NECK_WIDTH, BISHOP_HEIGHT_MID, BISHOP_NECK_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(BISHOP_NECK_WIDTH, BISHOP_HEIGHT_MID, -BISHOP_NECK_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(BISHOP_HEAD_WIDTH, BISHOP_HEAD_HEIGHT_MID, -BISHOP_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(BISHOP_HEAD_WIDTH, BISHOP_HEAD_HEIGHT_MID, BISHOP_HEAD_WIDTH);
	glEnd();

	// upper right side
	glBegin(GL_TRIANGLES);
		glNormal3f(1.0f, 1.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(BISHOP_HEAD_WIDTH, BISHOP_HEAD_HEIGHT_MID, -BISHOP_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, BISHOP_HEIGHT_TOP, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(BISHOP_HEAD_WIDTH, BISHOP_HEAD_HEIGHT_MID, BISHOP_HEAD_WIDTH);
	glEnd();

	// lower back side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(BISHOP_NECK_WIDTH, BISHOP_HEIGHT_MID, -BISHOP_NECK_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-BISHOP_NECK_WIDTH, BISHOP_HEIGHT_MID, -BISHOP_NECK_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-BISHOP_HEAD_WIDTH, BISHOP_HEAD_HEIGHT_MID, -BISHOP_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(BISHOP_HEAD_WIDTH, BISHOP_HEAD_HEIGHT_MID, -BISHOP_HEAD_WIDTH);
	glEnd();

	// upper back side
	glBegin(GL_TRIANGLES);
		glNormal3f(0.0f, 1.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-BISHOP_HEAD_WIDTH, BISHOP_HEAD_HEIGHT_MID, -BISHOP_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f, BISHOP_HEIGHT_TOP, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(BISHOP_HEAD_WIDTH, BISHOP_HEAD_HEIGHT_MID, -BISHOP_HEAD_WIDTH);
	glEnd();

	glEndList();

	// -------------- //

	// queen
	g_defModels[4] = glGenLists(1);
	glNewList(g_defModels[4], GL_COMPILE);
	
	// left side
	glBegin(GL_QUADS);
		glNormal3f(-0.5f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, 0.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-QUEEN_NECK_WIDTH, QUEEN_HEIGHT_MID, QUEEN_NECK_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-QUEEN_NECK_WIDTH, QUEEN_HEIGHT_MID, -QUEEN_NECK_WIDTH);
	glEnd();

	// front side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.5f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 0.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 0.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(QUEEN_NECK_WIDTH, QUEEN_HEIGHT_MID, QUEEN_NECK_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-QUEEN_NECK_WIDTH, QUEEN_HEIGHT_MID, QUEEN_NECK_WIDTH);
	glEnd();

	// right side
	glBegin(GL_QUADS);
		glNormal3f(0.5f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 0.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(QUEEN_NECK_WIDTH, QUEEN_HEIGHT_MID, -QUEEN_NECK_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(QUEEN_NECK_WIDTH, QUEEN_HEIGHT_MID, QUEEN_NECK_WIDTH);
	glEnd();

	// back side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, -0.5f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-QUEEN_NECK_WIDTH, QUEEN_HEIGHT_MID, -QUEEN_NECK_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(QUEEN_NECK_WIDTH, QUEEN_HEIGHT_MID, -QUEEN_NECK_WIDTH);
	glEnd();

	// the head
	// lower left side
	glBegin(GL_QUADS);
		glNormal3f(-1.0f, -1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-QUEEN_NECK_WIDTH, QUEEN_HEIGHT_MID, -QUEEN_NECK_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-QUEEN_NECK_WIDTH, QUEEN_HEIGHT_MID, QUEEN_NECK_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, -QUEEN_HEAD_WIDTH);
	glEnd();

	// lower front side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, -1.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-QUEEN_NECK_WIDTH, QUEEN_HEIGHT_MID, QUEEN_NECK_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(QUEEN_NECK_WIDTH, QUEEN_HEIGHT_MID, QUEEN_NECK_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, QUEEN_HEAD_WIDTH);
	glEnd();

	// lower right side
	glBegin(GL_QUADS);
		glNormal3f(1.0f, -1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(QUEEN_NECK_WIDTH, QUEEN_HEIGHT_MID, QUEEN_NECK_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(QUEEN_NECK_WIDTH, QUEEN_HEIGHT_MID, -QUEEN_NECK_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, -QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, QUEEN_HEAD_WIDTH);
	glEnd();

	// lower back side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(QUEEN_NECK_WIDTH, QUEEN_HEIGHT_MID, -QUEEN_NECK_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-QUEEN_NECK_WIDTH, QUEEN_HEIGHT_MID, -QUEEN_NECK_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, -QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, -QUEEN_HEAD_WIDTH);
	glEnd();

	// top plate
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, QUEEN_HEAD_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, QUEEN_HEAD_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, -QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, -QUEEN_HEAD_WIDTH);
	glEnd();

	// left triangles
	glBegin(GL_TRIANGLES);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEIGHT_TOP, QUEEN_HEAD_WIDTH / 2.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, 0.0f);

		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, 0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEIGHT_TOP, QUEEN_HEAD_WIDTH / 2.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, QUEEN_HEAD_WIDTH);

		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, 0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEIGHT_TOP, -(QUEEN_HEAD_WIDTH / 2.0f));
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, -QUEEN_HEAD_WIDTH);

		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, -QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEIGHT_TOP, -(QUEEN_HEAD_WIDTH / 2.0f));
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, 0.0f);
	glEnd();

	// front triangle
	glBegin(GL_TRIANGLES);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(QUEEN_HEAD_WIDTH / 2.0f, QUEEN_HEIGHT_TOP, QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, QUEEN_HEAD_HEIGHT_MID, QUEEN_HEAD_WIDTH);

		glNormal3f(0.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f, QUEEN_HEAD_HEIGHT_MID, QUEEN_HEAD_WIDTH);	
		glTexCoord2f(0.0f, 1.0f); glVertex3f(QUEEN_HEAD_WIDTH / 2.0f, QUEEN_HEIGHT_TOP, QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, QUEEN_HEAD_WIDTH);

		glNormal3f(0.0f, 0.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f, QUEEN_HEAD_HEIGHT_MID, QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-(QUEEN_HEAD_WIDTH / 2.0f), QUEEN_HEIGHT_TOP, QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, QUEEN_HEAD_WIDTH);

		glNormal3f(0.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-(QUEEN_HEAD_WIDTH / 2.0f), QUEEN_HEIGHT_TOP, QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, QUEEN_HEAD_HEIGHT_MID, QUEEN_HEAD_WIDTH);
	glEnd();

	// right triangle
	glBegin(GL_TRIANGLES);
		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEIGHT_TOP, QUEEN_HEAD_WIDTH / 2.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, 0.0f);

		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, 0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEIGHT_TOP, QUEEN_HEAD_WIDTH / 2.0f);	
		glTexCoord2f(0.0f, 0.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, QUEEN_HEAD_WIDTH);

		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, -QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEIGHT_TOP, -(QUEEN_HEAD_WIDTH / 2.0f));
		glTexCoord2f(0.0f, 0.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, 0.0f);

		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, 0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEIGHT_TOP, -(QUEEN_HEAD_WIDTH / 2.0f));
		glTexCoord2f(0.0f, 0.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, -QUEEN_HEAD_WIDTH);
	glEnd();

	// back triangle
	glBegin(GL_TRIANGLES);
		glNormal3f(0.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, -QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-(QUEEN_HEAD_WIDTH / 2.0f), QUEEN_HEIGHT_TOP, -QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, QUEEN_HEAD_HEIGHT_MID, -QUEEN_HEAD_WIDTH);

		glNormal3f(0.0f, 0.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f, QUEEN_HEAD_HEIGHT_MID, -QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-(QUEEN_HEAD_WIDTH / 2.0f), QUEEN_HEIGHT_TOP, -QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, -QUEEN_HEAD_WIDTH);		

		glNormal3f(0.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0f, QUEEN_HEAD_HEIGHT_MID, -QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(QUEEN_HEAD_WIDTH / 2.0f, QUEEN_HEIGHT_TOP, -QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, -QUEEN_HEAD_WIDTH);

		glNormal3f(0.0f, 0.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(QUEEN_HEAD_WIDTH, QUEEN_HEAD_HEIGHT_MID, -QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(QUEEN_HEAD_WIDTH / 2.0f, QUEEN_HEIGHT_TOP, -QUEEN_HEAD_WIDTH);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f, QUEEN_HEAD_HEIGHT_MID, -QUEEN_HEAD_WIDTH);
	glEnd();

	// sphere
	glTranslatef(0.0f, QUEEN_HEIGHT_MID + (QUEEN_HEAD_HEIGHT / 2.0f + 0.05f), 0.0f);
	glScalef(0.12f, 0.1f, 0.12f);	
	gluSphere(queenSphere, QUEEN_SPHERE_SIZE, 64, 64);

	glEndList();

	// -------------- //

	// king
	g_defModels[5] = glGenLists(1);
	glNewList(g_defModels[5], GL_COMPILE);

	// left side
	glBegin(GL_QUADS);
		glNormal3f(-0.5f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, 0.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-KING_NECK_WIDTH, KING_HEIGHT_MID, KING_NECK_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-KING_NECK_WIDTH, KING_HEIGHT_MID, -KING_NECK_WIDTH);
	glEnd();

	// front side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.5f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 0.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 0.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(KING_NECK_WIDTH, KING_HEIGHT_MID, KING_NECK_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-KING_NECK_WIDTH, KING_HEIGHT_MID, KING_NECK_WIDTH);
	glEnd();

	// right side
	glBegin(GL_QUADS);
		glNormal3f(0.5f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 0.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(KING_NECK_WIDTH, KING_HEIGHT_MID, -KING_NECK_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(KING_NECK_WIDTH, KING_HEIGHT_MID, KING_NECK_WIDTH);
	glEnd();

	// back side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, -0.5f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-KING_NECK_WIDTH, KING_HEIGHT_MID, -KING_NECK_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(KING_NECK_WIDTH, KING_HEIGHT_MID, -KING_NECK_WIDTH);
	glEnd();

	// the head
	// lower left side
	glBegin(GL_QUADS);
		glNormal3f(-1.0f, -1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-KING_NECK_WIDTH, KING_HEIGHT_MID, -KING_NECK_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-KING_NECK_WIDTH, KING_HEIGHT_MID, KING_NECK_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-KING_HEAD_WIDTH, KING_HEAD_HEIGHT_MID, KING_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-KING_HEAD_WIDTH, KING_HEAD_HEIGHT_MID, -KING_HEAD_WIDTH);
	glEnd();

	// lower front side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, -1.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-KING_NECK_WIDTH, KING_HEIGHT_MID, KING_NECK_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(KING_NECK_WIDTH, KING_HEIGHT_MID, KING_NECK_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(KING_HEAD_WIDTH, KING_HEAD_HEIGHT_MID, KING_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-KING_HEAD_WIDTH, KING_HEAD_HEIGHT_MID, KING_HEAD_WIDTH);
	glEnd();

	// lower right side
	glBegin(GL_QUADS);
		glNormal3f(1.0f, -1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(KING_NECK_WIDTH, KING_HEIGHT_MID, KING_NECK_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(KING_NECK_WIDTH, KING_HEIGHT_MID, -KING_NECK_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(KING_HEAD_WIDTH, KING_HEAD_HEIGHT_MID, -KING_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(KING_HEAD_WIDTH, KING_HEAD_HEIGHT_MID, KING_HEAD_WIDTH);
	glEnd();

	// lower back side
	glBegin(GL_QUADS);
		glNormal3f(0.0f, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(KING_NECK_WIDTH, KING_HEIGHT_MID, -KING_NECK_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-KING_NECK_WIDTH, KING_HEIGHT_MID, -KING_NECK_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-KING_HEAD_WIDTH, KING_HEAD_HEIGHT_MID, -KING_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(KING_HEAD_WIDTH, KING_HEAD_HEIGHT_MID, -KING_HEAD_WIDTH);
	glEnd();

	// top plate
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-KING_HEAD_WIDTH, KING_HEAD_HEIGHT_MID, KING_HEAD_WIDTH);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(KING_HEAD_WIDTH, KING_HEAD_HEIGHT_MID, KING_HEAD_WIDTH);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(KING_HEAD_WIDTH, KING_HEAD_HEIGHT_MID, -KING_HEAD_WIDTH);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-KING_HEAD_WIDTH, KING_HEAD_HEIGHT_MID, -KING_HEAD_WIDTH);
	glEnd();

	// cross
	// horizontal beam
	glBegin(GL_QUADS);
		// left plate
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE / 2.0f), -(KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE / 2.0f), (KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + KING_CROSS_SIZE, (KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + KING_CROSS_SIZE, -(KING_CROSS_SIZE / 2.0f));

		// right plate
		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE / 2.0f), (KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 0.0f); glVertex3f(KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE / 2.0f), -(KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 1.0f); glVertex3f(KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + KING_CROSS_SIZE, -(KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(0.0f, 1.0f); glVertex3f(KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + KING_CROSS_SIZE, (KING_CROSS_SIZE / 2.0f));

		// front plate
		glNormal3f(0.0f, 0.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE / 2.0f), (KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 0.0f); glVertex3f(KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE / 2.0f), (KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 1.0f); glVertex3f(KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + KING_CROSS_SIZE, (KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + KING_CROSS_SIZE, (KING_CROSS_SIZE / 2.0f));

		// back plate
		glNormal3f(0.0f, 0.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE / 2.0f), -(KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE / 2.0f), -(KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + KING_CROSS_SIZE, -(KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(0.0f, 1.0f); glVertex3f(KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + KING_CROSS_SIZE, -(KING_CROSS_SIZE / 2.0f));

		// bottom plate
		glNormal3f(0.0f, -1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE / 2.0f), -(KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 0.0f); glVertex3f(KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE / 2.0f), -(KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 1.0f); glVertex3f(KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE / 2.0f), (KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE / 2.0f), (KING_CROSS_SIZE / 2.0f));

		// top plate
		glNormal3f(0.0f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + KING_CROSS_SIZE, -(KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + KING_CROSS_SIZE, -(KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + KING_CROSS_SIZE, (KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(0.0f, 1.0f); glVertex3f(KING_CROSS_SIZE, KING_HEAD_HEIGHT_MID + KING_CROSS_SIZE, (KING_CROSS_SIZE / 2.0f));
	glEnd();

	// vertical beam
	glBegin(GL_QUADS);
		// left plate
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-(KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID, -(KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-(KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID, (KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-(KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE * 1.5f), (KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-(KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE * 1.5f), -(KING_CROSS_SIZE / 2.0f));

		// right plate
		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f((KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID, (KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 0.0f); glVertex3f((KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID, -(KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 1.0f); glVertex3f((KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE * 1.5f), -(KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(0.0f, 1.0f); glVertex3f((KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE * 1.5f), (KING_CROSS_SIZE / 2.0f));

		// front plate
		glNormal3f(0.0f, 0.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-(KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID, (KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 0.0f); glVertex3f((KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID, (KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 1.0f); glVertex3f((KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE * 1.5f), (KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-(KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE * 1.5f), (KING_CROSS_SIZE / 2.0f));

		// back plate
		glNormal3f(0.0f, 0.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f((KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID, -(KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-(KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID, -(KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-(KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE * 1.5f), -(KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(0.0f, 1.0f); glVertex3f((KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE * 1.5f), -(KING_CROSS_SIZE / 2.0f));

		// top plate
		glNormal3f(0.0f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-(KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE * 1.5f), (KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 0.0f); glVertex3f((KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE * 1.5f), (KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(1.0f, 1.0f); glVertex3f((KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE * 1.5f), -(KING_CROSS_SIZE / 2.0f));
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-(KING_CROSS_SIZE / 2.5f), KING_HEAD_HEIGHT_MID + (KING_CROSS_SIZE * 1.5f), -(KING_CROSS_SIZE / 2.0f));
	glEnd();
	
	glEndList();
}