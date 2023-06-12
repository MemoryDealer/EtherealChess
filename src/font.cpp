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

#include "font.h"

namespace
{
    #pragma pack(push, 1)
    
    // TGA file header structure. This *must* be byte aligned.
    struct TgaHeader
    {
        BYTE idLength;
        BYTE colormapType;
        BYTE imageType;
        WORD firstEntryIndex;
        WORD colormapLength;
        BYTE colormapEntrySize;
        WORD xOrigin;
        WORD yOrigin;
        WORD width;
        WORD height;
        BYTE pixelDepth;
        BYTE imageDescriptor;
    };

    // TGA file footer structure. This *must* be byte aligned.
    struct TgaFooter
    {
        LONG extensionAreaOffet;
        LONG developerDirectoryOffset;
        BYTE signature[18];
    };

    #pragma pack(pop)
}

int Bitmap::m_logpixelsx = 0;
int Bitmap::m_logpixelsy = 0;

Bitmap::Bitmap()
{
    dc = 0;
    hBitmap = 0;
    width = 0;
    height = 0;
    pitch = 0;
    m_hPrevObj = 0;
    m_pBits = 0;
}

Bitmap::Bitmap(const Bitmap &bitmap)
{
    dc = 0;
    hBitmap = 0;
    width = 0;
    height = 0;
    pitch = 0;
    m_hPrevObj = 0;
    m_pBits = 0;
    
    clone(bitmap);
}

Bitmap::~Bitmap()
{
    destroy();
}

Bitmap &Bitmap::operator=(const Bitmap &bitmap)
{
    if (this != &bitmap)
        clone(bitmap);

    return *this;
}

void Bitmap::blt(HDC hdcDest)
{
    StretchBlt(hdcDest, 0, 0, width, height, dc, 0, 0, width, height, SRCCOPY);
}

void Bitmap::blt(HDC hdcDest, int x, int y)
{
    StretchBlt(hdcDest, x, y, width, height, dc, 0, 0, width, height, SRCCOPY);
}

void Bitmap::blt(HDC hdcDest, int x, int y, int w, int h)
{
    StretchBlt(hdcDest, x, y, w, h, dc, 0, 0, width, height, SRCCOPY);
}

void Bitmap::blt(HDC hdcDest, const RECT &rcDest, const RECT &rcSrc)
{
    StretchBlt(hdcDest, rcDest.left, rcDest.top, rcDest.right - rcDest.left,
        rcDest.bottom - rcDest.top, dc, rcSrc.left, rcSrc.top,
        rcSrc.right - rcSrc.left, rcSrc.bottom - rcSrc.top, SRCCOPY);
}

bool Bitmap::clone(const Bitmap &bitmap)
{
    if (create(bitmap.width, bitmap.height))
    {
        memcpy(m_pBits, bitmap.m_pBits, bitmap.pitch * bitmap.height);
        return true;
    }

    return false;
}

bool Bitmap::create(int widthPixels, int heightPixels)
{
    destroy();

    width = widthPixels;
    height = heightPixels;
    pitch = ((width * 32 + 31) & ~31) >> 3;
    dc = CreateCompatibleDC(0);

    if (!dc)
        return false;

    memset(&info, 0, sizeof(info));

    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = -height;
    info.bmiHeader.biCompression = BI_RGB;
    info.bmiHeader.biPlanes = 1;

    hBitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, 
        reinterpret_cast<void**>(&m_pBits), 0, 0);

    if (!hBitmap)
    {
        destroy();
        return false;
    }

    GdiFlush();
    return true;
}

void Bitmap::destroy()
{
    deselectObject();

    if (hBitmap)
    {
        DeleteObject(hBitmap);
        hBitmap = 0;
    }

    if (dc)
    {
        DeleteDC(dc);
        dc = 0;
    }

    width = height = pitch = 0;
    m_hPrevObj = 0;
    m_pBits = 0;
}

void Bitmap::fill(int r, int g, int b, int a)
{
    int pixelPitch = pitch / sizeof(DWORD);
    DWORD pixel = createPixel(r, g, b, a);
    DWORD *pPixels = reinterpret_cast<DWORD*>(m_pBits);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
            pPixels[y * pixelPitch + x] = pixel;
    }
}

void Bitmap::fill(float r, float g, float b, float a)
{
    int pixelPitch = pitch / sizeof(DWORD);
    DWORD pixel = createPixel(r, g, b, a);
    DWORD *pPixels = reinterpret_cast<DWORD*>(m_pBits);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
            pPixels[y * pixelPitch + x] = pixel;
    }
}

bool Bitmap::loadDesktop()
{
    // Takes a screen capture of the current Windows desktop and stores
    // the image in the Bitmap object.

    HWND hDesktop = GetDesktopWindow();

    if (!hDesktop)
        return false;

    int desktopWidth = GetSystemMetrics(SM_CXSCREEN);
    int desktopHeight = GetSystemMetrics(SM_CYSCREEN);
    HDC hDesktopDC = GetDCEx(hDesktop, 0, DCX_CACHE | DCX_WINDOW);

    if (!hDesktopDC)
        return false;

    if (!create(desktopWidth, desktopHeight))
    {
        ReleaseDC(hDesktop, hDesktopDC);
        return false;
    }

    selectObject();

    if (!BitBlt(dc, 0, 0, width, height, hDesktopDC, 0, 0, SRCCOPY))
    {
        destroy();
        ReleaseDC(hDesktop, hDesktopDC);
        return false;
    }

    deselectObject();
    ReleaseDC(hDesktop, hDesktopDC);
    return true;
}

bool Bitmap::loadBitmap(LPCTSTR pszFilename)
{
    // Loads a BMP image and stores it in the Bitmap object.

    HANDLE hImage = LoadImage(GetModuleHandle(0), pszFilename, IMAGE_BITMAP, 0,
                        0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);

    if (!hImage)
        return false;

    BITMAP bitmap = {0};

    if (!GetObject(hImage, sizeof(bitmap), &bitmap))
    {
        DeleteObject(hImage);
        return false;
    }

    HDC hImageDC = CreateCompatibleDC(0);

    if (!hImageDC)
    {
        DeleteObject(hImage);
        return false;
    }

    SelectObject(hImageDC, hImage);

    int h = (bitmap.bmHeight < 0) ? -bitmap.bmHeight : bitmap.bmHeight;

    if (create(bitmap.bmWidth, h))
    {
        selectObject();

        if (!BitBlt(dc, 0, 0, width, height, hImageDC, 0, 0, SRCCOPY))
        {
            destroy();
            DeleteDC(hImageDC);
            DeleteObject(hImage);
            return false;
        }

        deselectObject();
    }

    DeleteDC(hImageDC);
    DeleteObject(hImage);
    return true;
}

bool Bitmap::loadPicture(LPCTSTR pszFilename)
{
    // Loads an image using the IPicture COM interface.
    // Supported image formats: BMP, EMF, GIF, ICO, JPG, WMF, TGA.
    //
    // Based on code from MSDN Magazine, October 2001.
    // http://msdn.microsoft.com/msdnmag/issues/01/10/c/default.aspx

    // IPicture interface doesn't support TGA files.
    if (_tcsstr(pszFilename, _T(".TGA")) || _tcsstr(pszFilename, _T(".tga")))
        return loadTarga(pszFilename);

    HRESULT hr = 0;
    HANDLE hFile = 0;
    HGLOBAL hGlobal = 0;
    IStream *pIStream = 0;
    IPicture *pIPicture = 0;
    BYTE *pBuffer = 0;
    DWORD dwFileSize = 0;
    DWORD dwBytesRead = 0;
    LONG lWidth = 0;
    LONG lHeight = 0;

    if (!m_logpixelsx && !m_logpixelsy)
    {
        HDC hScreenDC = CreateCompatibleDC(GetDC(0));

        if (!hScreenDC)
            return false;

        m_logpixelsx = GetDeviceCaps(hScreenDC, LOGPIXELSX);
        m_logpixelsy = GetDeviceCaps(hScreenDC, LOGPIXELSY);
        DeleteDC(hScreenDC);
    }

    hFile = CreateFile(pszFilename, FILE_READ_DATA, FILE_SHARE_READ, 0,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    if (!(dwFileSize = GetFileSize(hFile, 0)))
    {
        CloseHandle(hFile);
        return false;
    }

    if (!(hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_NODISCARD, dwFileSize)))
    {
        CloseHandle(hFile);
        return false;
    }

    if (!(pBuffer = reinterpret_cast<BYTE*>(GlobalLock(hGlobal))))
    {
        GlobalFree(hGlobal);
        CloseHandle(hFile);
        return false;
    }

    if (!ReadFile(hFile, pBuffer, dwFileSize, &dwBytesRead, 0))
    {
        GlobalUnlock(hGlobal);
        GlobalFree(hGlobal);
        CloseHandle(hFile);
        return false;
    }

    GlobalUnlock(hGlobal);
    CloseHandle(hFile);

    if (FAILED(CreateStreamOnHGlobal(hGlobal, FALSE, &pIStream)))
    {
        GlobalFree(hGlobal);
        return false;
    }

    if (FAILED(OleLoadPicture(pIStream, 0, FALSE, IID_IPicture,
            reinterpret_cast<LPVOID*>(&pIPicture))))
    {
        pIStream->Release();
        GlobalFree(hGlobal);
        return false;
    }

    pIStream->Release();
    GlobalFree(hGlobal);

    pIPicture->get_Width(&lWidth);
    pIPicture->get_Height(&lHeight);

    width = MulDiv(lWidth, m_logpixelsx, HIMETRIC_INCH);
    height = MulDiv(lHeight, m_logpixelsy, HIMETRIC_INCH);

    if (!create(width, height))
    {
        pIPicture->Release();
        return false;
    }

    selectObject();
    hr = pIPicture->Render(dc, 0, 0, width, height, 0, lHeight, lWidth, -lHeight, 0);
    deselectObject();

    pIPicture->Release();
    return (SUCCEEDED(hr)) ? true : false;
}

bool Bitmap::loadTarga(LPCTSTR pszFilename)
{
    // Loads a TGA image and stores it in the Bitmap object.

    HANDLE hFile = CreateFile(pszFilename, FILE_READ_DATA, FILE_SHARE_READ, 0,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    DWORD dwBytesRead = 0;
    TgaHeader header = {0};
    
    // Read in the TGA file header.
    ReadFile(hFile, &header, sizeof(header), &dwBytesRead, 0);

    // Skip over the TGA file's ID field.
    if (header.idLength > 0)
        SetFilePointer(hFile, header.idLength, 0, FILE_CURRENT);

    // Check for compatible color depth.
    if (!(header.pixelDepth == 32 || header.pixelDepth == 24 || header.pixelDepth == 8))
    {
        CloseHandle(hFile);
        return false;
    }

    // Only support uncompressed true color and grayscale images.
    if (!(header.imageType == 0x02 || header.imageType == 0x01))
    {
        CloseHandle(hFile);
        return false;
    }

    // Read the TGA file into a temporary buffer.

    DWORD dwPitch = header.width * (header.pixelDepth / 8);
    DWORD dwBufferSize = dwPitch * header.height;
    std::vector<BYTE> buffer(dwBufferSize);

    // Load the pixel data from the TGA file. Flip image if it's not top down.
    if ((header.imageDescriptor & 0x30) == 0x20)
    {
        // TGA image is stored top down in file.
        ReadFile(hFile, &buffer[0], dwBufferSize, &dwBytesRead, 0);
    }
    else
    {
        // TGA image is stored bottom up in file. Need to flip it.
        
        BYTE *pRow = 0;

        for (int i = 0; i < header.height; ++i)
        {
            pRow = &buffer[(header.height - 1 - i) * dwPitch];
            ReadFile(hFile, pRow, dwPitch, &dwBytesRead, 0);
        }
    }

    CloseHandle(hFile);

    if (!create(header.width, header.height))
        return false;

    setPixels(&buffer[0], header.width, header.height, header.pixelDepth / 8);
    return true;
}

void Bitmap::setPixels(const BYTE *pPixels, int w, int h, int bytesPerPixel)
{
    // Copies the specified input pixels to the Bitmap object.
    // This method performs color conversion on the source pixels so that
    // the pixels stored in the Bitmap object have a 32 bit color depth.

    if (!pPixels)
        return;

    int srcPitch = w * bytesPerPixel;

    if (bytesPerPixel == 4)
    {
        const BYTE *pSrcRow = 0;
        BYTE *pDestRow = 0;
                
        for (int i = 0; i < h; ++i)
        {
            pSrcRow = &pPixels[i * srcPitch];
            pDestRow = &m_pBits[i * pitch];
            memcpy(pDestRow, pSrcRow, srcPitch);
        }
    }
    else if (bytesPerPixel == 3)
    {
        const BYTE *pSrcPixel = 0;
        BYTE *pDestPixel = 0;

        for (int i = 0; i < h; ++i)
        {
            for (int j = 0; j < w; ++j)
            {
                pSrcPixel = &pPixels[(i * srcPitch) + (j * 3)];
                pDestPixel = &m_pBits[(i * pitch) + (j * 4)];

                pDestPixel[0] = pSrcPixel[0];
                pDestPixel[1] = pSrcPixel[1];
                pDestPixel[2] = pSrcPixel[2];
                pDestPixel[3] = 255;
            }
        }
    }
    else if (bytesPerPixel == 1)
    {
        BYTE srcPixel = 0;
        BYTE *pDestPixel = 0;

        for (int i = 0; i < h; ++i)
        {
            for (int j = 0; j < w; ++j)
            {
                srcPixel = pPixels[i * srcPitch + j];
                pDestPixel = &m_pBits[(i * pitch) + (j * 4)];

                pDestPixel[0] = srcPixel;
                pDestPixel[1] = srcPixel;
                pDestPixel[2] = srcPixel;
                pDestPixel[3] = 255;
            }
        }
    }
}

bool Bitmap::saveBitmap(LPCTSTR pszFilename) const
{
    HANDLE hFile = CreateFile(pszFilename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, 0);

    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    BITMAPFILEHEADER bfh = {0};
    BITMAPINFOHEADER bih = {0};

    // Fill in file header.
    bfh.bfType = 0x4d42;
    bfh.bfSize = sizeof(bfh) + sizeof(bih) + pitch * height;
    bfh.bfOffBits = sizeof(bfh) + sizeof(bih);

    // Fill in info header.
    bih.biSize = sizeof(bih);
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biPlanes = 1;
    bih.biBitCount = 32;

    DWORD dwNumberOfBytesWritten = 0;

    // Write the BMP headers.
    WriteFile(hFile, &bfh, sizeof(bfh), &dwNumberOfBytesWritten, 0);
    WriteFile(hFile, &bih, sizeof(bih), &dwNumberOfBytesWritten, 0);
    
    // Write the pixel data.
    // Need to store the bitmap pixels bottom-up.
    for (int i = 0; i < height; ++i)
    {
        WriteFile(hFile, &m_pBits[((height - 1) - i) * pitch], pitch,
            &dwNumberOfBytesWritten, 0);
    }

    CloseHandle(hFile);
    return true;
}

bool Bitmap::saveTarga(LPCTSTR pszFilename) const
{
    HANDLE hFile = CreateFile(pszFilename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, 0);

    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    TgaHeader header = {0};
    TgaFooter footer = {0, 0, "TRUEVISION-XFILE."};

    // Fill in file header.
    header.width = width;
    header.height = height;
    header.pixelDepth = 32;
    header.imageType = 2;               // uncompressed true-color
    header.imageDescriptor = 0x20;      // top-down orientation
        
    DWORD dwNumberOfBytesWritten = 0;

    // Write file header.
    WriteFile(hFile, &header, sizeof(header), &dwNumberOfBytesWritten, 0);

    // Write the pixel data. Pixel data needs to be byte aligned.
    for (int i = 0; i < height; ++i)
    {
        WriteFile(hFile, &m_pBits[i * pitch], width * 4,
            &dwNumberOfBytesWritten, 0);
    }

    // Write the file footer.
    WriteFile(hFile, &footer, sizeof(footer), &dwNumberOfBytesWritten, 0);

    CloseHandle(hFile);
    return true;
}

void Bitmap::selectObject()
{
    if (dc)
        m_hPrevObj = SelectObject(dc, hBitmap);
}

void Bitmap::deselectObject()
{
    if (dc && m_hPrevObj)
    {
        SelectObject(dc, m_hPrevObj);
        m_hPrevObj = 0;
    }
}

void Bitmap::copyBytes24Bit(BYTE *pDest) const
{
    // 'pDest' must already point to a chunk of allocated memory of the correct
    // size (i.e., width pixels X height pixels X 24 bits).
    //
    // The returned image is byte aligned and the pixel format is BGR.

    if (!pDest)
        return;

    BYTE *pSrc = 0;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            pSrc = &m_pBits[(pitch * y) + (x * 4)];

            *pDest++ = *pSrc;
            *pDest++ = *(pSrc + 1);
            *pDest++ = *(pSrc + 2);
        }
    }
}

void Bitmap::copyBytes32Bit(BYTE *pDest) const
{
    // 'pDest' must already point to a chunk of allocated memory of the correct
    // size (i.e., width pixels x height pixels x 32 bits).
    //
    // The returned image is byte aligned and the pixel format is BGRA.

    if (!pDest)
        return;

    const int widthBytes = width * 4;

    for (int y = 0; y < height; ++y)
        memcpy(&pDest[widthBytes * y], &m_pBits[pitch * y], widthBytes);
}

void Bitmap::copyBytesAlpha8Bit(BYTE *pDest) const
{
    // 'pDest' must already point to a chunk of allocated memory of the correct
    // size (i.e., width pixels X height pixels X 8 bits).
    //
    // The returned image is byte aligned and the pixel format is grayscale.
    //
    // The luminance conversion used is the one that Real-Time Rendering
    // 2nd Edition (Moller and Haines, 2002) recommends. It is based on modern
    // CRT and HDTV phosphors.
    //      Y = 0.2125R + 0.7154G + 0.0721B

    if (!pDest)
        return;

    BYTE *pSrc = 0;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            pSrc = &m_pBits[(pitch * y) + (x * 4)];
            b = (*pSrc / 255.0f) * 0.0721f;
            g = (*(pSrc + 1) / 255.0f) * 0.7154f;
            r = (*(pSrc + 2) / 255.0f) * 0.2125f;
            *pDest++ = static_cast<BYTE>(255.0f * (b + g + r));
        }
    }
}

void Bitmap::copyBytesAlpha32Bit(BYTE *pDest) const
{
    // 'pDest' must already point to a chunk of allocated memory of the correct
    // size (i.e., width pixels X height pixels X 32 bits).
    //
    // This is similar to copyBytesAlpha8Bit() only the alpha channel will
    // contain the grayscale luminance map generated by copyBytesAlpha8Bit().
    // The RGB channels are filled with pure white (255, 255, 255).
    //
    // The returned image is byte aligned and the pixel format is BGRA.
    //
    // The luminance conversion used is the one that Real-Time Rendering
    // 2nd Edition (Moller and Haines, 2002) recommends. It is based on modern
    // CRT and HDTV phosphors.
    //      Y = 0.2125R + 0.7154G + 0.0721B

    if (!pDest)
        return;

    BYTE *pSrc = 0;
    float fRed = 0.0f;
    float fGreen = 0.0f;
    float fBlue = 0.0f;
    
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            pSrc = &m_pBits[(pitch * y) + (x * 4)];

            fBlue = (*pSrc / 255.0f) * 0.0721f;
            fGreen = (*(pSrc + 1) / 255.0f) * 0.7154f;
            fRed = (*(pSrc + 2) / 255.0f) * 0.2125f;

            *pDest++ = 255;
            *pDest++ = 255;
            *pDest++ = 255;
            *pDest++ = static_cast<BYTE>(255.0f * (fBlue + fGreen + fRed));
        }
    }
}

void Bitmap::flipHorizontal()
{
    BYTE *pFront = 0;
    BYTE *pBack = 0;
    BYTE pixel[4] = {0};

    for (int i = 0; i < height; ++i)
    {
        pFront = &m_pBits[i * pitch];
        pBack = &pFront[pitch - 4];

        while (pFront < pBack)
        {
            // Save current pixel at position pFront.
            pixel[0] = pFront[0];
            pixel[1] = pFront[1];
            pixel[2] = pFront[2];
            pixel[3] = pFront[3];

            // Copy new pixel from position pBack into pFront.
            pFront[0] = pBack[0];
            pFront[1] = pBack[1];
            pFront[2] = pBack[2];
            pFront[3] = pBack[3];

            // Copy old pixel at position pFront into pBack.
            pBack[0] = pixel[0];
            pBack[1] = pixel[1];
            pBack[2] = pixel[2];
            pBack[3] = pixel[3];

            pFront += 4;
            pBack -= 4;
        }
    }
}

void Bitmap::flipVertical()
{
    std::vector<BYTE> srcPixels(pitch * height);

    memcpy(&srcPixels[0], m_pBits, pitch * height);

    BYTE *pSrcRow = 0;
    BYTE *pDestRow = 0;

    for (int i = 0; i < height; ++i)
    {
        pSrcRow = &srcPixels[(height - 1 - i) * pitch];
        pDestRow = &m_pBits[i * pitch];
        memcpy(pDestRow, pSrcRow, pitch);
    }
}

void Bitmap::resize(int newWidth, int newHeight)
{
    // Resizes the bitmap image using bilinear sampling.

    float ax = 0.0f, ay = 0.0f;
    float bx = 0.0f, by = 0.0f;
    float cx = 0.0f, cy = 0.0f;
    float dx = 0.0f, dy = 0.0f;
    float u = 0.0f, v = 0.0f, uv = 0.0f;
    float oneMinusU = 0.0f, oneMinusV = 0.0f, oneMinusUOneMinusV = 0.0f;
    float uOneMinusV = 0.0f, vOneMinusU = 0.0f;

    float srcX = 0.0f;
    float srcY = 0.0f;
    float srcXStep = static_cast<float>(width) / static_cast<float>(newWidth);
    float srcYStep = static_cast<float>(height) / static_cast<float>(newHeight);

    int destPitch = newWidth * 4;
    std::vector<BYTE> destPixels(destPitch * newHeight);

    BYTE pixel[4] ={0};

    const BYTE *pSrcPixelA = 0;
    const BYTE *pSrcPixelB = 0;
    const BYTE *pSrcPixelC = 0;
    const BYTE *pSrcPixelD = 0;
    BYTE *pDestPixel = 0;

    for (int y = 0; y < newHeight; ++y)
    {
        for (int x = 0; x < newWidth; ++x)
        {
            ax = floor(srcX);
            u = srcX - ax;

            ay = floor(srcY);
            v = srcY - ay;

            dx = ax + 1.0f;
            dy = ay + 1.0f;

            if (dx >= width)
                dx = width - 1.0f;

            if (dy >= height)
                dy = height - 1.0f;

            bx = dx;
            by = ay;

            cx = ax;
            cy = dy;

            uv = u * v;
            oneMinusU = 1.0f - u;
            oneMinusV = 1.0f - v;
            uOneMinusV = u * oneMinusV;
            vOneMinusU = v * oneMinusU;
            oneMinusUOneMinusV = oneMinusU * oneMinusV;

            pSrcPixelA = &m_pBits[(static_cast<int>(ay) * pitch) + (static_cast<int>(ax) * 4)];
            pSrcPixelB = &m_pBits[(static_cast<int>(by) * pitch) + (static_cast<int>(bx) * 4)];
            pSrcPixelC = &m_pBits[(static_cast<int>(cy) * pitch) + (static_cast<int>(cx) * 4)];
            pSrcPixelD = &m_pBits[(static_cast<int>(dy) * pitch) + (static_cast<int>(dx) * 4)];

            pixel[0] = static_cast<BYTE>(pSrcPixelA[0] * oneMinusUOneMinusV + pSrcPixelB[0] * uOneMinusV + pSrcPixelC[0] * vOneMinusU + pSrcPixelD[0] * uv);
            pixel[1] = static_cast<BYTE>(pSrcPixelA[1] * oneMinusUOneMinusV + pSrcPixelB[1] * uOneMinusV + pSrcPixelC[1] * vOneMinusU + pSrcPixelD[1] * uv);
            pixel[2] = static_cast<BYTE>(pSrcPixelA[2] * oneMinusUOneMinusV + pSrcPixelB[2] * uOneMinusV + pSrcPixelC[2] * vOneMinusU + pSrcPixelD[2] * uv);
            pixel[3] = static_cast<BYTE>(pSrcPixelA[3] * oneMinusUOneMinusV + pSrcPixelB[3] * uOneMinusV + pSrcPixelC[3] * vOneMinusU + pSrcPixelD[3] * uv);

            pDestPixel = &destPixels[(y * destPitch) + (x * 4)];

            pDestPixel[0] = pixel[0];
            pDestPixel[1] = pixel[1];
            pDestPixel[2] = pixel[2];
            pDestPixel[3] = pixel[3];

            srcX += srcXStep;
        }

        srcX = 0.0f;
        srcY += srcYStep;
    }

    destroy();

    if (create(newWidth, newHeight))
        setPixels(&destPixels[0], newWidth, newHeight, 4);
}

DWORD Bitmap::createPixel(int r, int g, int b, int a) const
{
    return static_cast<DWORD>(
          (static_cast<DWORD>(a) << 24)
        | (static_cast<DWORD>(r) << 16)
        | (static_cast<DWORD>(g) << 8)
        |  static_cast<DWORD>(b));
}

DWORD Bitmap::createPixel(float r, float g, float b, float a) const
{
    return static_cast<DWORD>(
          (static_cast<DWORD>(a * 255.0f) << 24)
        | (static_cast<DWORD>(r * 255.0f) << 16)
        | (static_cast<DWORD>(g * 255.0f) << 8)
        |  static_cast<DWORD>(b * 255.0f));
}

// Taken from: wingdi.h
#if !defined(CLEARTYPE_QUALITY)
	#define CLEARTYPE_QUALITY 5
#endif

int GLFont::m_logPixelsY = 0;
BYTE GLFont::m_lfQuality = 0;
char GLFont::m_szBuffer[MAX_STR_SIZE] = {0};

GLFont::GLFont()
{
    m_name.clear();
    m_dropShadowOffset = 1;
    m_pointSize = 0;
    m_cellHeight = 0;
    m_cellWidth = 0;
    m_charHeight = 0;
    m_charAvgWidth = 0;
    m_charMaxWidth = 0;
    m_numCharsToDraw = 0;
    m_textureObject = 0;
    m_drawDropShadows = false;
    m_color[0] = m_color[1] = m_color[2] = m_color[3] = 1.0f;
    m_pVertex = 0;
    m_hFont = 0;

    memset(m_glyphs, 0, sizeof(m_glyphs));

    if (!m_logPixelsY)
    {
        HWND hWndDesktop = GetDesktopWindow();
        HDC hDC = GetDCEx(hWndDesktop, 0, DCX_CACHE | DCX_WINDOW);

        if (hDC)
        {
            m_logPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);
            ReleaseDC(hWndDesktop, hDC);
        }
    }

    if (!m_lfQuality)
    {
        DWORD dwVersion = GetVersion();
        DWORD dwMajorVersion = static_cast<DWORD>((LOBYTE(LOWORD(dwVersion))));
        DWORD dwMinorVersion = static_cast<DWORD>((HIBYTE(LOWORD(dwVersion))));

        // Windows XP and higher will support ClearType quality fonts.
        if (dwMajorVersion >= 6 || (dwMajorVersion == 5 && dwMinorVersion == 1))
            m_lfQuality = CLEARTYPE_QUALITY;
        else
            m_lfQuality = ANTIALIASED_QUALITY;
    }
}

GLFont::~GLFont()
{
    destroy();
}

bool GLFont::create(const char *pszName, int ptSize, Style style)
{
    m_name = pszName;
    m_pointSize = ptSize;

    m_lf.lfHeight = -MulDiv(ptSize, m_logPixelsY, 72);
    m_lf.lfWidth = 0;
    m_lf.lfEscapement = 0;
    m_lf.lfOrientation = 0;
    m_lf.lfWeight = (style == BOLD) ? FW_BOLD : FW_NORMAL;
    m_lf.lfItalic = FALSE;
    m_lf.lfUnderline = FALSE;
    m_lf.lfStrikeOut = FALSE;
    m_lf.lfCharSet = ANSI_CHARSET;
    m_lf.lfOutPrecision = OUT_STROKE_PRECIS;
    m_lf.lfClipPrecision = CLIP_STROKE_PRECIS;
    m_lf.lfQuality = m_lfQuality;
    m_lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

    if (m_name.length() < LF_FACESIZE)
        strcpy(m_lf.lfFaceName, pszName);
    else
        m_lf.lfFaceName[0] = '\0';

    HFONT hNewFont = CreateFontIndirect(&m_lf);

    if (!hNewFont)
        return false;

    if (m_hFont)
    {
        DeleteObject(m_hFont);
        m_hFont = 0;
    }

    m_hFont = hNewFont;

    if (!createFontBitmap())
    {
        destroy();
        return false;
    }

    return true;
}

void GLFont::destroy()
{
    if (m_hFont)
    {
        DeleteObject(m_hFont);
        m_hFont = 0;
    }

    if (m_textureObject)
    {
        glDeleteTextures(1, &m_textureObject);
        m_textureObject = 0;
    }
}

void GLFont::begin()
{
    HWND hWnd = GetForegroundWindow();
    RECT rcClient;

    GetClientRect(hWnd, &rcClient);

    int w = rcClient.right - rcClient.left;
    int h = rcClient.bottom - rcClient.top;
    
    glPushAttrib(GL_CURRENT_BIT | GL_LIGHTING_BIT);

    glDisable(GL_LIGHTING);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_textureObject);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, w, h, 0.0f, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();   

    drawTextBegin();
}

void GLFont::drawChar(char c, int x, int y)
{
    //  1------4
    //  |      |            1 = (x, y)
    //  |      |            2 = (x, y + charHeight)
    //  |      |            3 = (x + charWidth, y + charHeight)
    //  |      |            4 = (x + charWidth, y)
    //  |      |
    //  |      |
    //  2------3
    //

    const Glyph &glyph = getChar(c);
    int charWidth = glyph.width;
    int charHeight = m_charHeight;

    if (m_drawDropShadows)
    {
        // 1
        m_pVertex->x = x + m_dropShadowOffset;
        m_pVertex->y = y + m_dropShadowOffset;
        m_pVertex->s = glyph.upperLeft[0];
        m_pVertex->t = glyph.upperLeft[1];
        m_pVertex->r = 0.0f;
        m_pVertex->g = 0.0f;
        m_pVertex->b = 0.0f;
        m_pVertex->a = 1.0f;
        ++m_pVertex;

        // 2
        m_pVertex->x = x + m_dropShadowOffset;
        m_pVertex->y = y + charHeight + m_dropShadowOffset;
        m_pVertex->s = glyph.lowerLeft[0];
        m_pVertex->t = glyph.lowerLeft[1];
        m_pVertex->r = 0.0f;
        m_pVertex->g = 0.0f;
        m_pVertex->b = 0.0f;
        m_pVertex->a = 1.0f;
        ++m_pVertex;

        // 3
        m_pVertex->x = x + charWidth + m_dropShadowOffset;
        m_pVertex->y = y + charHeight + m_dropShadowOffset;
        m_pVertex->s = glyph.lowerRight[0];
        m_pVertex->t = glyph.lowerRight[1];
        m_pVertex->r = 0.0f;
        m_pVertex->g = 0.0f;
        m_pVertex->b = 0.0f;
        m_pVertex->a = 1.0f;
        ++m_pVertex;

        // 4
        m_pVertex->x = x + charWidth + m_dropShadowOffset;
        m_pVertex->y = y + m_dropShadowOffset;
        m_pVertex->s = glyph.upperRight[0];
        m_pVertex->t = glyph.upperRight[1];
        m_pVertex->r = 0.0f;
        m_pVertex->g = 0.0f;
        m_pVertex->b = 0.0f;
        m_pVertex->a = 1.0f;
        ++m_pVertex;

        if (++m_numCharsToDraw == MAX_CHARS_PER_BATCH)
        {
            drawTextEnd();
            drawBatchOfChars();
            drawTextBegin();
        }
    }

    // 1
    m_pVertex->x = x;
    m_pVertex->y = y;
    m_pVertex->s = glyph.upperLeft[0];
    m_pVertex->t = glyph.upperLeft[1];
    m_pVertex->r = m_color[0];
    m_pVertex->g = m_color[1];
    m_pVertex->b = m_color[2];
    m_pVertex->a = m_color[3];
    ++m_pVertex;

    // 2
    m_pVertex->x = x;
    m_pVertex->y = y + charHeight;
    m_pVertex->s = glyph.lowerLeft[0];
    m_pVertex->t = glyph.lowerLeft[1];
    m_pVertex->r = m_color[0];
    m_pVertex->g = m_color[1];
    m_pVertex->b = m_color[2];
    m_pVertex->a = m_color[3];
    ++m_pVertex;

    // 3
    m_pVertex->x = x + charWidth;
    m_pVertex->y = y + charHeight;
    m_pVertex->s = glyph.lowerRight[0];
    m_pVertex->t = glyph.lowerRight[1];
    m_pVertex->r = m_color[0];
    m_pVertex->g = m_color[1];
    m_pVertex->b = m_color[2];
    m_pVertex->a = m_color[3];
    ++m_pVertex;

    // 4
    m_pVertex->x = x + charWidth;
    m_pVertex->y = y;
    m_pVertex->s = glyph.upperRight[0];
    m_pVertex->t = glyph.upperRight[1];
    m_pVertex->r = m_color[0];
    m_pVertex->g = m_color[1];
    m_pVertex->b = m_color[2];
    m_pVertex->a = m_color[3];
    ++m_pVertex;

    if (++m_numCharsToDraw == MAX_CHARS_PER_BATCH)
    {
        drawTextEnd();
        drawBatchOfChars();
        drawTextBegin();
    }
}

void GLFont::end()
{
    drawTextEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    glDisable(GL_BLEND);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glPopAttrib();    
}

void GLFont::drawText(int x, int y, const char *pszText)
{
    char prevCh = 0;
    char ch = 0;
    int dx = x;
    int dy = y;
    int charHeight = getCellHeight();
    int whitespaceWidth = getChar(' ').width;

    while (*pszText != '\0')
    {
        prevCh = ch;
        ch = *pszText++;

        if (ch == ' ')
        {
            if (prevCh != '\r')
                dx += whitespaceWidth;
        }
        else if (ch == '\n' || ch == '\r')
        {
            dx = x;
            dy += charHeight;
        }
        else if (ch == '\t')
        {
            dx += whitespaceWidth * TAB_SPACES;
        }
        else if (ch >= CHAR_FIRST && ch <= CHAR_LAST)
        {
            drawChar(ch, dx, dy);
            dx += getChar(ch).width;
        }
    }
}

void GLFont::drawTextFormat(int x, int y, const char *pszFmt, ...)
{
    va_list vlist;

    va_start(vlist, pszFmt);
    _vsnprintf(m_szBuffer, MAX_STR_SIZE, pszFmt, vlist);
    va_end(vlist);

    drawText(x, y, m_szBuffer);
}

int GLFont::getStrWidth(const char *pszText) const
{
    if (!pszText)
        return 0;

    int width = 0;

    while (*pszText != '\0')
        width += getCharWidth(*pszText++);    

    return width;
}

int GLFont::getStrWidth(const char *pChar, int length) const
{
    if (!pChar || length <= 0)
        return 0;

    int width = 0;

    for (int i = 0; i < length; ++i)
        width += getCharWidth(pChar[i]);

    return width;
}

void GLFont::setColor(float r, float g, float b)
{
    m_color[0] = r;
    m_color[1] = g;
    m_color[2] = b;
    m_color[3] = 1.0f;
}

void GLFont::setDrawDropShadows(bool enableShadows)
{
    m_drawDropShadows = enableShadows;
}

void GLFont::setDropShadowOffset(int offset)
{
    m_dropShadowOffset = offset;
}

bool GLFont::createTexture(const Bitmap &bitmap)
{
    int w = bitmap.width;
    int h = bitmap.height;

    std::vector<unsigned char> pixels;
    pixels.resize(w * h);

    bitmap.copyBytesAlpha8Bit(&pixels[0]);

    glGenTextures(1, &m_textureObject);
    glBindTexture(GL_TEXTURE_2D, m_textureObject);

    // Only use GL_NEAREST filtering for the min and mag filter. Using anything
    // else will cause the font glyphs to be blurred. Using only GL_NEAREST
    // filtering ensures that edges of the font glyphs remain crisp and sharp.

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA,
        GL_UNSIGNED_BYTE, &pixels[0]);

    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

bool GLFont::createFontBitmap()
{
    // The font is drawn as a 10 x 10 grid of characters.

    if (!extractFontMetrics())
        return false;

    int w = 10 * m_cellWidth;
    int h = 10 * m_cellHeight;
    Bitmap bitmap;

    if (!bitmap.create(nextPower2(10 * m_cellWidth), nextPower2(10 * m_cellHeight)))
        return false;

    int x = 0;
    int y = 0;
    int ch = 32;
    HFONT hPrevFont = 0;
    COLORREF prevColor = 0;
    RECT rc = {0, 0, bitmap.width, bitmap.height};

    bitmap.selectObject();
    hPrevFont = reinterpret_cast<HFONT>(SelectObject(bitmap.dc, m_hFont));
    prevColor = SetTextColor(bitmap.dc, RGB(255,255,255));
    SetBkMode(bitmap.dc, TRANSPARENT);
    FillRect(bitmap.dc, &rc, reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));

    for (int i = 0; i < 10; ++i)
    {
        y = m_cellHeight * i;

        for (int j = 0; j < 10; ++j)
        {
            x = m_cellWidth * j;

            if (ch > 31 && ch < 127)
                TextOut(bitmap.dc, x, y, reinterpret_cast<LPCSTR>(&ch), 1);

            ++ch;
        }
    }

    SetTextColor(bitmap.dc, prevColor);
    SelectObject(bitmap.dc, hPrevFont);
    bitmap.deselectObject();

    generateTexCoords(bitmap);
    return createTexture(bitmap);
}

void GLFont::drawBatchOfChars()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(2, GL_INT, sizeof(Vertex), &m_vertices->x);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &m_vertices->s);
    glColorPointer(4, GL_FLOAT, sizeof(Vertex), &m_vertices->r);

    glDrawArrays(GL_QUADS, 0, m_numCharsToDraw * 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void GLFont::drawTextBegin()
{
    m_numCharsToDraw = 0;
    m_pVertex = m_vertices;
}

void GLFont::drawTextEnd()
{
    if (m_numCharsToDraw > 0)
        drawBatchOfChars();
}

bool GLFont::extractFontMetrics()
{
    HWND hWndDesktop = GetDesktopWindow();
    HDC hDC = GetDCEx(hWndDesktop, 0, DCX_CACHE | DCX_WINDOW);

    if (!hDC)
        return false;

    HFONT hPrevFont = reinterpret_cast<HFONT>(SelectObject(hDC, m_hFont));
    TEXTMETRIC tm;
    SIZE charSize = {0};
    char szString[2] = {0};
    char szName[128] = {0};

    GetTextFace(hDC, 128, szName);
    m_name = szName;

    GetTextMetrics(hDC, &tm);
    m_charHeight = m_cellHeight = tm.tmHeight + tm.tmExternalLeading;
    m_charMaxWidth = 0;
    m_charAvgWidth = 0;

    for (int c = 32; c < 127; ++c)
    {
        szString[0] = c;
        GetTextExtentPoint32(hDC, szString, 1, &charSize);

        if (charSize.cx > m_charMaxWidth)
            m_charMaxWidth = charSize.cx;

        m_charAvgWidth += charSize.cx;
        m_glyphs[c - 32].width = charSize.cx;
    }

    m_charAvgWidth /= TOTAL_CHARS;
    m_cellWidth = m_charMaxWidth + (m_charAvgWidth / 2);

    SelectObject(hDC, hPrevFont);
    ReleaseDC(hWndDesktop, hDC);
    return true;
}

void GLFont::generateTexCoords(const Bitmap &bitmap)
{
    Glyph *pGlyph = 0;
    int col = 0;
    int row = 0;
    int charWidth = 0;
    float bmpWidth = static_cast<float>(bitmap.width);
    float bmpHeight = static_cast<float>(bitmap.height);

    for (int c = 32; c < 127; ++c)
    {
        col = (c - 32) % 10;
        row = (c - 32) / 10;

        pGlyph = &m_glyphs[c - 32];
        charWidth = pGlyph->width;

        pGlyph->upperLeft[0] = (col * m_cellWidth) / bmpWidth;
        pGlyph->upperLeft[1] = (row * m_cellHeight) / bmpHeight;

        pGlyph->lowerLeft[0] = (col * m_cellWidth) / bmpWidth;
        pGlyph->lowerLeft[1] = ((row + 1) * m_cellHeight) / bmpHeight;

        pGlyph->lowerRight[0] = ((col * m_cellWidth) + charWidth) / bmpWidth;
        pGlyph->lowerRight[1] = ((row + 1) * m_cellHeight) / bmpHeight;

        pGlyph->upperRight[0] = ((col * m_cellWidth) + charWidth) / bmpWidth;
        pGlyph->upperRight[1] = (row * m_cellHeight) / bmpHeight;
    }
}

int GLFont::nextPower2(int x) const
{
    int i = x & (~x + 1);

    while (i < x)
        i <<= 1;	

    return i;
}