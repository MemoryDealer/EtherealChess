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

#include <Windows.h>
#include <cassert>
#include <cmath>
#include <cstring>
#include <limits>
#include <sstream>
#include <string>

#include <GL/GL.h>

#include "mathlib.h"

enum models{
	MODEL_PAWN = 0,
	MODEL_ROOK,
	MODEL_KNIGHT,
	MODEL_BISHOP,
	MODEL_QUEEN,
	MODEL_KING
};

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

#include <fstream>
#include <map>
#include <string>
#include <vector>

//-----------------------------------------------------------------------------
// Alias|Wavefront OBJ file loader.
//
// This OBJ file loader contains the following restrictions:
// 1. Group information is ignored. Faces are grouped based on the material
//    that each face uses.
// 2. Object information is ignored. This loader will merge everything into a
//    single object.
// 3. The MTL file must be located in the same directory as the OBJ file. If
//    it isn't then the MTL file will fail to load and a default material is
//    used instead.
// 4. This loader triangulates all polygonal faces during importing.
//-----------------------------------------------------------------------------

class ModelOBJ
{
public:
    struct Material
    {
        float ambient[4];
        float diffuse[4];
        float specular[4];
        float shininess;        // [0 = min shininess, 1 = max shininess]
        float alpha;            // [0 = fully transparent, 1 = fully opaque]

        std::string colorMapFilename;
    };

    struct Vertex
    {
        float position[3];
        float texCoord[2];
        float normal[3];
    };

    struct Mesh
    {
        int startIndex;
        int triangleCount;
        int materialIndex;
    };

    ModelOBJ();
    ~ModelOBJ();

    bool import(const char *pszFilename);
    void normalize(float scaleTo = 1.0f, bool center = true);
	void unload(void);
    void reverseWinding();

    // Getter methods.

    void getCenter(float &x, float &y, float &z) const;
    float getWidth() const;
    float getHeight() const;
    float getLength() const;

    const int *getIndexBuffer() const;
    const Material &getMaterial(int i) const;
    const Mesh &getMesh(int i) const;

    int getNumberOfIndices() const;
    int getNumberOfMaterials() const;
    int getNumberOfMeshes() const;
    int getNumberOfTriangles() const;
    int getNumberOfVertices() const;

	Vec3 getPos(void) const;

    const Vertex &getVertex(int i) const;
    const Vertex *getVertexBuffer() const;
    int getVertexSize() const;

	GLuint  getTexture() const;
    bool hasTextureCoords() const;
    bool hasVertexNormals() const;

	bool render(void) const;

	int isLoaded(void) const;

	// setter methods
	void setTexture(GLuint id);
	void setLoaded(bool b);
	void setPos(Vec3 pos);
	void setPos(float dx, float dy, float dz);

private:
    void addVertex(int hash, const Vertex *pVertex);
    void bounds(float center[3], float &radius) const;
    void bounds(float center[3], float &width, float &height, float &length) const;
    void buildMeshes();
    void generateNormals();
    void importGeometryFirstPass(std::ifstream &stream);
    void importGeometrySecondPass(std::ifstream &stream);
    bool importMaterials(const std::string &filename);
    void scale(float scaleFactor, float offset[3]);
    int triangulateLastInsertedFace(int verticesPerFace);

    static const int FACE_INDEX_CACHE_SIZE = 32;
    static int m_faceIndexCache[FACE_INDEX_CACHE_SIZE];

    bool m_hasTextureCoords;
    bool m_hasVertexNormals;

    int m_numberOfVertexCoords;
    int m_numberOfTextureCoords;
    int m_numberOfNormals;
    int m_numberOfFaces;

    float m_center[3];
    float m_width;
    float m_height;
    float m_length;

	bool loaded;
	GLuint texture;

	Vec3 pos;

    std::string m_directoryPath;

    std::vector<Mesh> m_meshes;
    std::vector<Material> m_materials;
    std::vector<Vertex> m_vertexBuffer;
    std::vector<int> m_indexBuffer;
    std::vector<int> m_attributeBuffer;

    std::map<std::string, int> m_materialCache;
    std::map<int, std::vector<int> > m_vertexCache;
};

//-----------------------------------------------------------------------------

inline void ModelOBJ::getCenter(float &x, float &y, float &z) const
{ x = m_center[0]; y = m_center[1]; z = m_center[2]; }

inline float ModelOBJ::getWidth() const
{ return m_width; }

inline float ModelOBJ::getHeight() const
{ return m_height; }

inline float ModelOBJ::getLength() const
{ return m_length; }

inline const int *ModelOBJ::getIndexBuffer() const
{ return &m_indexBuffer[0]; }

inline const ModelOBJ::Material &ModelOBJ::getMaterial(int i) const
{ return m_materials[i]; }

inline const ModelOBJ::Mesh &ModelOBJ::getMesh(int i) const
{ return m_meshes[i]; }

inline int ModelOBJ::getNumberOfIndices() const
{ return static_cast<int>(m_indexBuffer.size()); }

inline int ModelOBJ::getNumberOfMaterials() const
{ return static_cast<int>(m_materials.size()); }

inline int ModelOBJ::getNumberOfMeshes() const
{ return static_cast<int>(m_meshes.size()); }

inline int ModelOBJ::getNumberOfTriangles() const
{ return static_cast<int>(m_indexBuffer.size() / 3); }

inline int ModelOBJ::getNumberOfVertices() const
{ return static_cast<int>(m_vertexBuffer.size()); }

inline Vec3 ModelOBJ::getPos(void) const
{ return pos; }

inline const ModelOBJ::Vertex &ModelOBJ::getVertex(int i) const
{ return m_vertexBuffer[i]; }

inline const ModelOBJ::Vertex *ModelOBJ::getVertexBuffer() const
{ return &m_vertexBuffer[0]; }

inline int ModelOBJ::getVertexSize() const
{ return static_cast<int>(sizeof(Vertex)); }

inline GLuint ModelOBJ::getTexture() const
{ return texture; }

inline int ModelOBJ::isLoaded(void) const
{ return loaded; }

inline bool ModelOBJ::hasTextureCoords() const
{ return m_hasTextureCoords; }

inline bool ModelOBJ::hasVertexNormals() const
{ return m_hasVertexNormals; }

inline void ModelOBJ::setTexture(GLuint id)
{ texture = id; }

inline void ModelOBJ::setLoaded(bool b)
{ loaded = b; }

inline void ModelOBJ::setPos(Vec3 newPos)
{ pos = newPos; }

inline void ModelOBJ::setPos(float dx, float dy, float dz)
{ pos.set(dx, dy, dz); }
