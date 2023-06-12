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

//-----------------------------------------------------------------------------
// Copyright (c) 2007 dhpoware. All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#pragma once

#pragma comment (lib, "olepro32.lib")   // for IPicture COM interface support

#include <windows.h>
#include <tchar.h>
#include <GL/gl.h>
#include <cassert>
#include <string>
#include <olectl.h.>    // for OleLoadPicture() and IPicture COM interface
#include <cmath>
#include <cstring>
#include <cstdarg>
#include <vector>

//-----------------------------------------------------------------------------
// 32-bit BGRA WIN32 device independent bitmap (DIB) class.
//
// Supports the loading of BMP, EMF, GIF, ICO, JPG, and WMF files using the
// WIN32 IPicture COM object.
//
// Also supports the loading of true color 24-bit and 32-bit TGA files, and
// grayscale 8-bit TGA files.
//
// Support is also provided for capturing a screen shot of the current Windows
// desktop and loading that as an image into the Bitmap class.
//
// This class stores the DIB in a top-down orientation. The pixel bytes are
// stored in the standard Windows DIB order of BGRA.
//
// All Windows DIBs are aligned to 4-byte (DWORD) memory boundaries. This means
// that each scan line is padded with extra bytes to ensure that the next scan
// line starts on a 4-byte memory boundary.
//
// To get a copy of the DIB that is BYTE (1-byte) aligned with all the extra
// padding bytes removed use the copyBytes() methods.
//-----------------------------------------------------------------------------

class Bitmap
{
public:
    HDC dc;
    HBITMAP hBitmap;
    int width;
    int height;
    int pitch;
    BITMAPINFO info;

    Bitmap();
    Bitmap(const Bitmap &bitmap);
    ~Bitmap();

    Bitmap &operator=(const Bitmap &bitmap);

    BYTE *operator[](int row) const
    { return &m_pBits[pitch * row]; }

    void blt(HDC hdcDest);
    void blt(HDC hdcDest, int x, int y);
    void blt(HDC hdcDest, int x, int y, int w, int h);
    void blt(HDC hdcDest, const RECT &rcDest, const RECT &rcSrc);

    bool clone(const Bitmap &bitmap);
    bool create(int widthPixels, int heightPixels);
    void destroy();

    void fill(int r, int g, int b, int a);
    void fill(float r, float g, float b, float a);
    
    BYTE *getPixels() const
    { return m_pBits; }

    bool loadDesktop();
    bool loadBitmap(LPCTSTR pszFilename);
    bool loadPicture(LPCTSTR pszFilename);
    bool loadTarga(LPCTSTR pszFilename);
    
    bool saveBitmap(LPCTSTR pszFilename) const;
    bool saveTarga(LPCTSTR pszFilename) const;

    void selectObject();
    void deselectObject();

    void copyBytes24Bit(BYTE *pDest) const;
    void copyBytes32Bit(BYTE *pDest) const;

    void copyBytesAlpha8Bit(BYTE *pDest) const;
    void copyBytesAlpha32Bit(BYTE *pDest) const;
    
    void setPixels(const BYTE *pPixels, int w, int h, int bytesPerPixel);

    void flipHorizontal();
    void flipVertical();
    
    void resize(int newWidth, int newHeight);

private:
    DWORD createPixel(int r, int g, int b, int a) const;
    DWORD createPixel(float r, float g, float b, float a) const;
    
    static const int HIMETRIC_INCH = 2540; // matches constant in MFC CDC class

    static int m_logpixelsx;
    static int m_logpixelsy;

    HGDIOBJ m_hPrevObj;
    BYTE *m_pBits;
};

//-----------------------------------------------------------------------------
// This GLFont class draws text as a bunch of textured quads. The GLFont class
// only stores the displayable ASCII characters in the range ASCII 32 to ASCII
// 126 inclusive. This range represents the displayable ASCII characters.
// ClearType text smoothing is used if the code is running under WinXP or
// higher. Otherwise standard text anti-aliasing is used.
//
// To use the GLFont class:
//  GLFont font;
//  font.create("Arial", 12, GLFont::NORMAL);
//  font.begin();
//  font.setColor(1.0f, 0.0f, 0.0f);
//  font.drawTextFormat(1, 1, "%s", "Hello, World!");
//  font.end();
//  font.destroy();
//-----------------------------------------------------------------------------

#define CHARACTER_HEIGHT_OFFSET 20

class GLFont
{
public:
    enum Style
    {
        NORMAL,
        BOLD
    };

    struct Glyph
    {
        int width;
        float upperLeft[2];
        float lowerLeft[2];
        float upperRight[2];
        float lowerRight[2];
    };

    GLFont();
    ~GLFont();

    bool create(const char *pszName, int ptSize, Style style = NORMAL);
    void destroy();

    void begin();
    void end();

    void drawChar(char c, int x, int y);
    void drawText(int x, int y, const char *pszText);
    void drawTextFormat(int x, int y, const char *pszFmt, ...);

    int getCellHeight() const;
    int getCellWidth() const;
    const Glyph &getChar(char ch) const;
    int getCharHeight() const;
    int getCharWidthAvg() const;
    int getCharWidthMax() const;
    int getCharWidth(char ch) const;
    const char *getName() const;
    int getPointSize() const;
    int getStrWidth(const char *pszText) const;
    int getStrWidth(const char *pChar, int length) const;

    bool isNull() const;

    void setColor(float r, float g, float b);
    void setDrawDropShadows(bool enableShadows);
    void setDropShadowOffset(int offset);

private:
    GLFont(const GLFont &);
    GLFont &operator=(const GLFont &);

    struct Vertex
    {
        int x, y;
        float s, t;
        float r, g, b, a;
    };

    bool createTexture(const Bitmap &bitmap);
    bool createFontBitmap();
    void drawBatchOfChars();
    void drawTextBegin();
    void drawTextEnd();
    bool extractFontMetrics();
    void generateTexCoords(const Bitmap &bitmap);
    int nextPower2(int x) const;

    static const int CHAR_FIRST = 32;
    static const int CHAR_LAST = 126;
    static const int TAB_SPACES = 4;
    static const int TOTAL_CHARS = 95;

    static const int MAX_CHARS_PER_BATCH = 256;
    static const int MAX_STR_SIZE = 1024;
    static const int MAX_VERTICES = MAX_CHARS_PER_BATCH * 4;

    static int m_logPixelsY;
    static BYTE m_lfQuality;
    static char m_szBuffer[MAX_STR_SIZE];

    std::string m_name;
    int m_dropShadowOffset;
    int m_pointSize;
    int m_cellHeight;
    int m_cellWidth;
    int m_charHeight;
    int m_charAvgWidth;
    int m_charMaxWidth;
    int m_numCharsToDraw;
    unsigned int m_textureObject;
    bool m_drawDropShadows;
    float m_color[4];
    Vertex *m_pVertex;
    HFONT m_hFont;
    LOGFONT m_lf;
    Glyph m_glyphs[TOTAL_CHARS];
    Vertex m_vertices[MAX_VERTICES];
};

//-----------------------------------------------------------------------------

inline int GLFont::getCellHeight() const
{ return m_cellHeight; }

inline int GLFont::getCellWidth() const
{ return m_cellWidth; }

inline const GLFont::Glyph &GLFont::getChar(char ch) const
{
    assert(ch >= CHAR_FIRST && ch <= CHAR_LAST);
    return m_glyphs[ch - 32];
}

inline int GLFont::getCharHeight() const
{ return m_charHeight; }

inline int GLFont::getCharWidthAvg() const
{ return m_charAvgWidth; }

inline int GLFont::getCharWidthMax() const
{ return m_charMaxWidth; }

inline int GLFont::getCharWidth(char ch) const
{
    assert(ch >= CHAR_FIRST && ch <= CHAR_LAST);
    return m_glyphs[ch - 32].width;
}

inline const char *GLFont::getName() const
{ return m_name.c_str(); }

inline int GLFont::getPointSize() const
{ return m_pointSize; }

inline bool GLFont::isNull() const
{ return !m_hFont; }
