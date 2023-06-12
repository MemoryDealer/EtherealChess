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

#include "dialog.h"
#include "resource.h"

extern Music g_music;
extern ConfigParser* g_config;

BOOL CALLBACK OptionsProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			Graphics& graphics = Graphics::inst();
			Game& game = Game::inst();

			// add items to drop down menus
			HWND hBox = GetDlgItem(hwnd, IDC_COMBO_CHESS_SET);
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Native");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Normal");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Fancy");

			hBox = GetDlgItem(hwnd, IDC_COMBO_SKYBOX);
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Space");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Moonlight");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Alien Planet");

			hBox = GetDlgItem(hwnd, IDC_COMBO_BACKGROUND);
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"None");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Earth");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Earth (Night)");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Saturn");

			hBox = GetDlgItem(hwnd, IDC_COMBO_TEXTURE);
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Metallic");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Gold/Silver");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Light Gold/Silver");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Glass");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Marble");

			hBox = GetDlgItem(hwnd, IDC_COMBO_GAME_TIME);
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"10 Minutes");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"15 Minutes");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"20 Minutes");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"30 Minutes");

			hBox = GetDlgItem(hwnd, IDC_COMBO_AI_ENGINE);
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Houdini");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Critter");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Custom");
			SendMessage(hBox, CB_SETCURSEL, (WPARAM)AI::inst().getEngine(), 0);

			hBox = GetDlgItem(hwnd, IDC_COMBO_AI_LEVEL);
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Child");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Walrus");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Lion");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Velociraptor");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Grandmaster");
			
			hBox = GetDlgItem(hwnd, IDC_COMBO_GEOMETRY_QUALITY);
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Mediocre");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Low");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Medium");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"High");
			SendMessage(hBox, CB_ADDSTRING, 0, (LPARAM)"Ultra");

			if(graphics.getAntialiasing())
				SendDlgItemMessage(hwnd, IDC_CHECK_ANTIALIASING, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			if(graphics.useReflection())
				SendDlgItemMessage(hwnd, IDC_CHECK_REFLECTIONS, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			if(graphics.getVerticalSync())
				SendDlgItemMessage(hwnd, IDC_CHECK_VERTICAL_SYNC, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);

			SendDlgItemMessage(hwnd, IDC_COMBO_CHESS_SET, CB_SETCURSEL, (WPARAM)game.getChessSet(), 0);
			SendDlgItemMessage(hwnd, IDC_COMBO_SKYBOX, CB_SETCURSEL, (WPARAM)game.getSkybox(), 0);
			SendDlgItemMessage(hwnd, IDC_COMBO_BACKGROUND, CB_SETCURSEL, (WPARAM)game.getPlanet(), 0);
			SendDlgItemMessage(hwnd, IDC_COMBO_TEXTURE, CB_SETCURSEL, (WPARAM)game.getTextureMode(), 0);
			SendDlgItemMessage(hwnd, IDC_COMBO_AI_LEVEL, CB_SETCURSEL, (WPARAM)game.getAILevel(), 0);
			SendDlgItemMessage(hwnd, IDC_COMBO_GEOMETRY_QUALITY, CB_SETCURSEL, (WPARAM)graphics.getGeometryQuality(), 0);
			
			switch(game.getTime()){
			case 600000:
			default:
				SendDlgItemMessage(hwnd, IDC_COMBO_GAME_TIME, CB_SETCURSEL, (WPARAM)0, 0);
				break;
				
			case 900000:
				SendDlgItemMessage(hwnd, IDC_COMBO_GAME_TIME, CB_SETCURSEL, (WPARAM)1, 0);
				break;

			case 1200000:
				SendDlgItemMessage(hwnd, IDC_COMBO_GAME_TIME, CB_SETCURSEL, (WPARAM)2, 0);
				break;

			case 1800000:
				SendDlgItemMessage(hwnd, IDC_COMBO_GAME_TIME, CB_SETCURSEL, (WPARAM)3, 0);
				break;
			}

			if(g_music.isPlaying())
				SendDlgItemMessage(hwnd, IDC_CHECK_MUSIC, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		}
		return 0;

	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDOK:
			{
				extern GLuint g_textures[];
				extern char g_workingDir[];
				Graphics& graphics = Graphics::inst();
				Game& game = Game::inst();
				int x = 0;

				SetDlgItemText(hwnd, IDC_STATIC_SAVING, "Applying settings, please wait...");

				SetCurrentDirectory(g_workingDir);

				graphics.setAntialiasing(IsDlgButtonChecked(hwnd, IDC_CHECK_ANTIALIASING) ? true : false);
				graphics.setReflection(IsDlgButtonChecked(hwnd, IDC_CHECK_REFLECTIONS) ? true : false);
				graphics.enableVerticalSync(IsDlgButtonChecked(hwnd, IDC_CHECK_VERTICAL_SYNC) ? true : false);

				game.setChessSet(SendDlgItemMessage(hwnd, IDC_COMBO_CHESS_SET, CB_GETCURSEL, 0, 0));
				if(game.getChessSet() != Game::SET_NATIVE){
					if(!game.isSetLoaded(game.getChessSet())){
						Graphics::inst().reloadModels();
					}
				}

				x = SendDlgItemMessage(hwnd, IDC_COMBO_SKYBOX, CB_GETCURSEL, 0, 0);
				if(x != game.getSkybox()){
					game.setSkybox(x);
					Graphics::inst().reloadSkyboxTextures();
				}
				else
					game.setSkybox(x);

				x = SendDlgItemMessage(hwnd, IDC_COMBO_BACKGROUND, CB_GETCURSEL, 0, 0);
				// load texture if needed
				if(x == Game::PLANET_EARTH && game.getPlanet() != Game::PLANET_EARTH){
					g_textures[Game::TEX_EARTH] = 0;
					graphics.clamp(true);
					graphics.loadTexture(g_textures[Game::TEX_EARTH], "Data/Images/earth.jpg");
					graphics.clamp(false);
				}
				else if(x == Game::PLANET_EARTH_NIGHT && game.getPlanet() != Game::PLANET_EARTH_NIGHT){
					g_textures[Game::TEX_EARTH] = 0;
					graphics.clamp(true);
					graphics.loadTexture(g_textures[Game::TEX_EARTH], "Data/Images/earth-night.jpg");
					graphics.clamp(false);
				}
				game.setPlanet(x);				

				x = SendDlgItemMessage(hwnd, IDC_COMBO_TEXTURE, CB_GETCURSEL, 0, 0);
				if(x != game.getTextureMode()){
					game.setTextureMode(x);
					Graphics::inst().reloadPieceTextures();
				}
				else
					game.setTextureMode(x);

				x = SendDlgItemMessage(hwnd, IDC_COMBO_AI_ENGINE, CB_GETCURSEL, 0, 0);
				if(x == (AI::ENGINE_CUSTOM + 1)){
					x -= 1;
				}
				AI::inst().setEngine(x);

				game.setAILevel(SendDlgItemMessage(hwnd, IDC_COMBO_AI_LEVEL, CB_GETCURSEL, 0, 0));

				graphics.setGeometryQuality(SendDlgItemMessage(hwnd, IDC_COMBO_GEOMETRY_QUALITY, CB_GETCURSEL, 0, 0));
				
				switch(SendDlgItemMessage(hwnd, IDC_COMBO_GAME_TIME, CB_GETCURSEL, 0, 0)){
				case 0:
				default:
					game.setTime(600000);
					break;

				case 1:
					game.setTime(900000);
					break;

				case 2:
					game.setTime(1200000);
					break;

				case 3:
					game.setTime(1800000);
					break;
				}

				if(IsDlgButtonChecked(hwnd, IDC_CHECK_MUSIC))
					g_music.play();
				else
					g_music.stop();

				g_config->saveAll();
				EndDialog(hwnd, 0);
				SetFocus(g_hWnd);
			}
			return 0;

		case IDCANCEL:
			EndDialog(hwnd, 0);
			SetFocus(g_hWnd);
			return 0;
		}
		return 0;

	case WM_CLOSE:
		EndDialog(hwnd, 0);
		return 0;

	default:
		return 0;
	}

	return true;
}

BOOL CALLBACK HelpProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg){
	case WM_INITDIALOG:
		{
			bool success = false;
			HANDLE hFile = CreateFile("Data/help.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
			if(hFile != INVALID_HANDLE_VALUE){
				DWORD size = GetFileSize(hFile, NULL);
				if(size != 0xFFFFFFFF){
					char* buf = new char[size + 1];

					DWORD read;

					if(ReadFile(hFile, buf, size, &read, NULL)){
						buf[size] = 0;
						SetDlgItemText(hwnd, IDC_EDIT, buf);
						success = true;
					}

					delete[] buf;
				}

				CloseHandle(hFile);
			}

			if(!success){
				MessageBox(hwnd, "Failed to open help file located at Data/help.txt\n\n"
					"Please visit https://sourceforge.net/projects/etherealchess/ to retrieve all the files.", "Error", MB_OK | MB_ICONERROR);
			}
		}
		return 0;

	

	case WM_COMMAND:
		if(LOWORD(wParam) == IDOK){
			EndDialog(hwnd, 0);
			SetFocus(g_hWnd);
		}
		else if(LOWORD(wParam) == IDC_LINK){
			if((int)ShellExecute(NULL, "open", "https://sourceforge.net/projects/etherealchess/", 0, 0, SW_SHOWNORMAL) <= 32){
					MessageBox(hwnd, "Failed to open the specified URL.", "Error", MB_ICONWARNING);
			}
		}
		return 0;

	case WM_CLOSE:
		EndDialog(hwnd, 0);
		SetFocus(g_hWnd);
		return 0;

	default:
		return 0;
	}

	return true;
}