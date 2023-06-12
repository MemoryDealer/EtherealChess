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

#include "arcane_lib.h"

void swap(int& n1, int& n2)
{
	int buf = n1;
	n1 = n2;
	n2 = buf;
}

float fRand(float min, float max)
{
	return ((float(rand()) / float(RAND_MAX)) * (max - min)) + min;
}

bool isOS64Bit(void)
{
	typedef BOOL (WINAPI* LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);

	LPFN_ISWOW64PROCESS _IsWow64Process = NULL;
	BOOL				b64bit			= FALSE;

	_IsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(LoadLibrary("Kernel32.dll"), "IsWow64Process");
	if(_IsWow64Process == NULL)
		return false;
	
	if(_IsWow64Process(GetCurrentProcess(), &b64bit))
		return b64bit;

	return false;
}

bool ExtensionSupported(const char *pszExtensionName)
{
	extern HDC g_hDC;
    static const char *pszGLExtensions = 0;
    static const char *pszWGLExtensions = 0;

    if (!pszGLExtensions)
        pszGLExtensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));

    if (!pszWGLExtensions)
    {
        // WGL_ARB_extensions_string.

        typedef const char *(WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC)(HDC);

        PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB =
            reinterpret_cast<PFNWGLGETEXTENSIONSSTRINGARBPROC>(
            wglGetProcAddress("wglGetExtensionsStringARB"));

        if (wglGetExtensionsStringARB)
            pszWGLExtensions = wglGetExtensionsStringARB(g_hDC);
    }

    if (!strstr(pszGLExtensions, pszExtensionName))
    {
        if (!strstr(pszWGLExtensions, pszExtensionName))
            return false;
    }

    return true;
}

float GetElapsedTimeInSeconds(void)
{
    // Returns the elapsed time (in seconds) since the last time this function
    // was called. This elaborate setup is to guard against large spikes in
    // the time returned by QueryPerformanceCounter().

    static const int MAX_SAMPLE_COUNT = 50;

    static float frameTimes[MAX_SAMPLE_COUNT];
    static float timeScale = 0.0f;
    static float actualElapsedTimeSec = 0.0f;
    static INT64 freq = 0;
    static INT64 lastTime = 0;
    static int sampleCount = 0;
    static bool initialized = false;

    INT64 time = 0;
    float elapsedTimeSec = 0.0f;

    if (!initialized)
    {
        initialized = true;
        QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&freq));
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&lastTime));
        timeScale = 1.0f / freq;
    }

    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&time));
    elapsedTimeSec = (time - lastTime) * timeScale;
    lastTime = time;

    if (fabsf(elapsedTimeSec - actualElapsedTimeSec) < 1.0f)
    {
        memmove(&frameTimes[1], frameTimes, sizeof(frameTimes) - sizeof(frameTimes[0]));
        frameTimes[0] = elapsedTimeSec;

        if (sampleCount < MAX_SAMPLE_COUNT)
            ++sampleCount;
    }

    actualElapsedTimeSec = 0.0f;

    for (int i = 0; i < sampleCount; ++i)
        actualElapsedTimeSec += frameTimes[i];

    if (sampleCount > 0)
        actualElapsedTimeSec /= sampleCount;

    return actualElapsedTimeSec;
}

/* check string for extension */
bool ext(const char* str, const char* ext)
{
    /* Example:
     * ext("a.exe", "exe") -> returns true
     * ext("a.exe", "txt") -> returns false
     */

    char    buf[MAX_PATH]   = {0};
    char    buf2[32]        = {0};
    int     len             = strlen(str) - 1;
    int     ext_c           = strlen(ext) - 1;
    int     i,
            p;

    strcpy(buf, str);
    strcpy(buf2, ext);

    // check for a period
    for(i=len; buf[i] != '.' || i > 0; --i)
    {
        if(buf[i] == '.')
        {
            p = 1;
            break;
        }
    }
    if(p == 0)
        return 0; // no extension
    if(len - i > ext_c + 1)
        return false; // file extension is bigger than comparison string

    // convert both to uppercase for comparing
    for(i=0; buf[i] != '\0'; ++i)
        buf[i] = toupper(buf[i]);

    for(i=0; buf2[i] != '\0'; ++i)
        buf2[i] = toupper(buf2[i]);

    // check the file extension
    for(i=0; i<=ext_c; ++i)
    {
        if(buf[len - i] != buf2[ext_c - i])
            return false; // mismatch
    }

    return true;
}