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

#include "model.h"

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
//
// The methods bounds(), normalize(), and scale() are based on source code
// from http://www.mvps.org/directx/articles/scalemesh9.htm
//
// The methods importGeometrySecondPass(), importMaterials(), and addVertex()
// are based on source code from the Direct3D MeshFromOBJ sample found in the
// DirectX SDK.
//
// When PERFORM_TWO_PASS_LOADING is enabled the import() method will load the
// OBJ model in two passes. The first pass will count the number of vertex
// positions, texture coordinates, vertex normals, and triangle faces. Then the
// second pass will allocate the correct amount of memory and load the model.
// With PERFORM_TWO_PASS_LOADING disabled the model is loaded in a single pass
// and the model data is loaded into vector containers that are dynamically
// grown in size.
//
// The OBJ loader will generate vertex normals only if the OBJ file doesn't
// already contain any vertex normals. You can force the OBJ loader to rebuild
// all the vertex normals every time the OBJ file is imported by enabling
// REBUILD_NORMALS_DURING_IMPORT.
//
//-----------------------------------------------------------------------------

#define PERFORM_TWO_PASS_LOADING        1
//#define REBUILD_NORMALS_DURING_IMPORT   1
#define NOMINMAX

int ModelOBJ::m_faceIndexCache[FACE_INDEX_CACHE_SIZE];

ModelOBJ::ModelOBJ()
{
    m_hasTextureCoords = false;
    m_hasVertexNormals = false;

    m_numberOfVertexCoords = 0;
    m_numberOfTextureCoords = 0;
    m_numberOfNormals = 0;
    m_numberOfFaces = 0;
}

ModelOBJ::~ModelOBJ()
{
}

void ModelOBJ::bounds(float center[3], float &radius) const
{
    center[0] = 0.0f;
    center[1] = 0.0f;
    center[2] = 0.0f;

    int numVerts = static_cast<int>(m_vertexBuffer.size());

    // Calculate the center of the bounding sphere.
    // Done by averaging the vertex coordinates.

    for (int i = 0; i < numVerts; ++i)
    {
        center[0] += m_vertexBuffer[i].position[0];
        center[1] += m_vertexBuffer[i].position[1];
        center[2] += m_vertexBuffer[i].position[2];
    }

    center[0] /= static_cast<float>(numVerts);
    center[1] /= static_cast<float>(numVerts);
    center[2] /= static_cast<float>(numVerts);

    // Calculate the radius. This is the max distance from the center.

    radius = 0.0f;

    const float *pPos = 0;
    float distanceSq = 0.0f;

    for (int i = 0; i < numVerts; ++i)
    {
        pPos = m_vertexBuffer[i].position;
        distanceSq = (pPos[0] * pPos[0]) + (pPos[1] * pPos[1]) + (pPos[2] * pPos[2]);

        if (distanceSq > radius)
            radius = distanceSq;
    }

    radius = sqrtf(radius);
}

void ModelOBJ::bounds(float center[3], float &width, float &height, float &length) const
{
    float xMax = (std::numeric_limits<float>::min)();
    float yMax = (std::numeric_limits<float>::min)();
    float zMax = (std::numeric_limits<float>::min)();

    float xMin = (std::numeric_limits<float>::max)();
    float yMin = (std::numeric_limits<float>::max)();
    float zMin = (std::numeric_limits<float>::max)();

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    int numVerts = static_cast<int>(m_vertexBuffer.size());

    for (int i = 0; i < numVerts; ++i)
    {
        x = m_vertexBuffer[i].position[0];
        y = m_vertexBuffer[i].position[1];
        z = m_vertexBuffer[i].position[2];

        if (x < xMin)
            xMin = x;

        if (x > xMax)
            xMax = x;

        if (y < yMin)
            yMin = y;

        if (y > yMax)
            yMax = y;

        if (z < zMin)
            zMin = z;

        if (z > zMax)
            zMax = z;
    }

    center[0] = (xMin + xMax) / 2.0f;
    center[1] = (yMin + yMax) / 2.0f;
    center[2] = (zMin + zMax) / 2.0f;

    width = xMax - xMin;
    height = yMax - yMin;
    length = zMax - zMin;
}

bool ModelOBJ::import(const char *pszFilename)
{
    std::ifstream stream(pszFilename);

    if (!stream.is_open())
        return false;

    // Extract the directory the OBJ file is in from the file name.
    // This directory path will be used to load the OBJ's associated MTL file.

    m_directoryPath.clear();

    std::string filename = pszFilename;
    std::string::size_type offset = filename.find_last_of('\\');

    if (offset != std::string::npos)
    {
        m_directoryPath = filename.substr(0, ++offset);
    }
    else
    {
        offset = filename.find_last_of('/');

        if (offset != std::string::npos)
            m_directoryPath = filename.substr(0, ++offset);
    }

    // Import the geometry and materials.
    // This is done with either two passes or a single pass. Two pass loading
    // goes through the file once to determine the amount of memory required.
    // Single pass loading relies on the STL vector and map classes dynamically
    // growing itself as more elements are added.

#if PERFORM_TWO_PASS_LOADING
    importGeometryFirstPass(stream);
    importGeometrySecondPass(stream);
#else
    importGeometrySecondPass(stream);
#endif

    buildMeshes();
    bounds(m_center, m_width, m_height, m_length);

#if REBUILD_NORMALS_DURING_IMPORT
    generateNormals();
#else
    if (!hasVertexNormals())
        generateNormals();
#endif

    return true;
}

void ModelOBJ::normalize(float scaleTo, bool center)
{
    float radius;
    float centerPos[3];

    bounds(centerPos, radius);

    float scalingFactor = scaleTo / radius;
    float offset[3];

    if (center)
    {
        offset[0] = -centerPos[0];
        offset[1] = -centerPos[1];
        offset[2] = -centerPos[2];
    }
    else
    {
        offset[0] = 0.0f;
        offset[1] = 0.0f;
        offset[2] = 0.0f;
    }

    scale(scalingFactor, offset);
    bounds(m_center, m_width, m_height, m_length);
}

void ModelOBJ::unload(void)
{
	m_hasTextureCoords = false;
    m_hasVertexNormals = false;

    m_numberOfVertexCoords = 0;
    m_numberOfTextureCoords = 0;
    m_numberOfNormals = 0;
    m_numberOfFaces = 0;

	m_meshes.clear();
	m_materials.clear();
	m_vertexBuffer.clear();
	m_indexBuffer.clear();
	m_attributeBuffer.clear();
	m_materialCache.clear();
	m_vertexCache.clear();

	loaded = false;
}

void ModelOBJ::reverseWinding()
{
    int swap = 0;

    // Reverse face winding.
    for (int i = 0; i < static_cast<int>(m_indexBuffer.size()); i += 3)
    {
        swap = m_indexBuffer[i + 1];
        m_indexBuffer[i + 1] = m_indexBuffer[i + 2];
        m_indexBuffer[i + 2] = swap;
    }

    float *pNormal = 0;

    // Invert normals.
    for (int i = 0; i < static_cast<int>(m_vertexBuffer.size()); ++i)
    {
        pNormal = m_vertexBuffer[i].normal;
        pNormal[0] = -pNormal[0];
        pNormal[1] = -pNormal[1];
        pNormal[2] = -pNormal[2];
    }
}

void ModelOBJ::scale(float scaleFactor, float offset[3])
{
    float *pPosition = 0;

    for (int i = 0; i < static_cast<int>(m_vertexBuffer.size()); ++i)
    {
        pPosition = m_vertexBuffer[i].position;

        pPosition[0] += offset[0];
        pPosition[1] += offset[1];
        pPosition[2] += offset[2];

        pPosition[0] *= scaleFactor;
        pPosition[1] *= scaleFactor;
        pPosition[2] *= scaleFactor;
    }
}

void ModelOBJ::addVertex(int hash, const Vertex *pVertex)
{
    std::map<int, std::vector<int> >::const_iterator iter = m_vertexCache.find(hash);

    if (iter == m_vertexCache.end())
    {
        // Vertex hash doesn't exist in the cache.

        int index = static_cast<int>(m_vertexBuffer.size());

        m_vertexBuffer.push_back(*pVertex);
        m_vertexCache.insert(std::make_pair(hash, std::vector<int>(1, index)));

        m_indexBuffer.push_back(index);
    }
    else
    {
        // One or more vertices have been hashed to this entry in the cache.

        const std::vector<int> &vertices = iter->second;
        const Vertex *pCachedVertex = 0;
        int index = 0;
        bool found = false;

        for (std::vector<int>::const_iterator i = vertices.begin(); i != vertices.end(); ++i)
        {
            index = *i;
            pCachedVertex = &m_vertexBuffer[index];

            if (memcmp(pCachedVertex, pVertex, sizeof(Vertex)) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            index = static_cast<int>(m_vertexBuffer.size());
            m_vertexBuffer.push_back(*pVertex);
            m_vertexCache[hash].push_back(index);
        }

        m_indexBuffer.push_back(index);
    }
}

void ModelOBJ::buildMeshes()
{
    // Group the OBJ file triangles based on material type.

    Mesh *pMesh = 0;
    int materialId = -1;

    for (int i = 0; i < static_cast<int>(m_attributeBuffer.size()); ++i)
    {
        if (m_attributeBuffer[i] != materialId)
        {
            materialId = m_attributeBuffer[i];

            m_meshes.push_back(Mesh());
            pMesh = &m_meshes[m_meshes.size() - 1];
            pMesh->materialIndex = materialId;
            pMesh->startIndex = i * 3;
            ++pMesh->triangleCount;
        }
        else
        {
            ++pMesh->triangleCount;
        }
    }
}

void ModelOBJ::generateNormals()
{
    const int *pTriangle = 0;
    Vertex *pVertex0 = 0;
    Vertex *pVertex1 = 0;
    Vertex *pVertex2 = 0;
    float edge1[3] = {0.0f};
    float edge2[3] = {0.0f};
    float normal[3] = {0.0f};
    float length = 0.0f;
    int totalVertices = getNumberOfVertices();
    int totalTriangles = getNumberOfTriangles();

    // Initialize all the vertex normals.
    for (int i = 0; i < totalVertices; ++i)
    {
        pVertex0 = &m_vertexBuffer[i];
        pVertex0->normal[0] = 0.0f;
        pVertex0->normal[1] = 0.0f;
        pVertex0->normal[2] = 0.0f;
    }

    // Calculate the vertex normals.
    for (int i = 0; i < totalTriangles; ++i)
    {
        pTriangle = &m_indexBuffer[i * 3];

        pVertex0 = &m_vertexBuffer[pTriangle[0]];
        pVertex1 = &m_vertexBuffer[pTriangle[1]];
        pVertex2 = &m_vertexBuffer[pTriangle[2]];

        // Calculate triangle face normal.

        edge1[0] = pVertex1->position[0] - pVertex0->position[0];
        edge1[1] = pVertex1->position[1] - pVertex0->position[1];
        edge1[2] = pVertex1->position[2] - pVertex0->position[2];

        edge2[0] = pVertex2->position[0] - pVertex0->position[0];
        edge2[1] = pVertex2->position[1] - pVertex0->position[1];
        edge2[2] = pVertex2->position[2] - pVertex0->position[2];

        normal[0] = (edge1[1] * edge2[2]) - (edge1[2] * edge2[1]);
        normal[1] = (edge1[2] * edge2[0]) - (edge1[0] * edge2[2]);
        normal[2] = (edge1[0] * edge2[1]) - (edge1[1] * edge2[0]);

        // Accumulate the normals.

        pVertex0->normal[0] += normal[0];
        pVertex0->normal[1] += normal[1];
        pVertex0->normal[2] += normal[2];

        pVertex1->normal[0] += normal[0];
        pVertex1->normal[1] += normal[1];
        pVertex1->normal[2] += normal[2];

        pVertex2->normal[0] += normal[0];
        pVertex2->normal[1] += normal[1];
        pVertex2->normal[2] += normal[2];
    }

    // Normalize the vertex normals.
    for (int i = 0; i < totalVertices; ++i)
    {
        pVertex0 = &m_vertexBuffer[i];

        length = 1.0f / sqrtf(pVertex0->normal[0] * pVertex0->normal[0] +
                              pVertex0->normal[1] * pVertex0->normal[1] +
                              pVertex0->normal[2] * pVertex0->normal[2]);

        pVertex0->normal[0] *= length;
        pVertex0->normal[1] *= length;
        pVertex0->normal[2] *= length;
    }
}

void ModelOBJ::importGeometryFirstPass(std::ifstream &stream)
{
    m_hasTextureCoords = false;
    m_hasVertexNormals = false;

    m_numberOfVertexCoords = 0;
    m_numberOfTextureCoords = 0;
    m_numberOfNormals = 0;
    m_numberOfFaces = 0;

    std::istringstream strStream;
    std::string line;
    std::string token;
    int ch = 0;
    int verticesPerFace = 0;

    while (!stream.eof())
    {
        line.clear();
        std::getline(stream, line);

        if (line.empty())
            continue;

        strStream.clear();
        strStream.str(line);

        ch = strStream.get();

        switch (ch)
        {
        case 'v':
            ch = strStream.get();

            if (ch == ' ')
                ++m_numberOfVertexCoords;
            else if (ch == 't')
                ++m_numberOfTextureCoords;
            else if (ch == 'n')
                ++m_numberOfNormals;

            break;

        case 'f':
            verticesPerFace = 0;

            while (strStream >> token)
                ++verticesPerFace;

            // Triangulate all faces.
            //
            // Consider:
            //  3 vertices = 1 face
            //  4 vertices = 2 faces
            //  5 vertices = 3 faces
            //
            // Therefore the number of triangles per face is:
            //  number of triangles per face = (vertices per face - 3) + 1

            m_numberOfFaces += verticesPerFace - 2;
            break;

        default:
            break;
        }
    }

    stream.clear();
    stream.seekg(0, std::ios_base::beg);
}

void ModelOBJ::importGeometrySecondPass(std::ifstream &stream)
{
    Mesh *pGroup = 0;
    int activeMaterial = 0;
    int posIndex = 0;
    int texCoordIndex = 0;
    int normalIndex = 0;
    int verticesPerFace = 0;
    float position[3] = {0.0f};
    float texCoord[2] = {0.0f};
    float normal[3] = {0.0f};
    Vertex vertex;
    std::string command;
    std::string name;
    std::string line;
    std::vector<float> vertexCoords;
    std::vector<float> textureCoords;
    std::vector<float> normals;
    std::map<std::string, int>::const_iterator iter;
    std::istringstream strStream;

#if PERFORM_TWO_PASS_LOADING
    vertexCoords.reserve(m_numberOfVertexCoords * 3);
    textureCoords.reserve(m_numberOfTextureCoords * 2);
    normals.reserve(m_numberOfNormals * 3);
    m_indexBuffer.reserve(m_numberOfFaces * 3);
    m_attributeBuffer.reserve(m_numberOfFaces);
#endif

    Material defaultMaterial =
    {
        0.2f, 0.2f, 0.2f, 1.0f,
        0.8f, 0.8f, 0.8f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
        0.0f,
        1.0f,
        std::string()
    };

    m_materials.push_back(defaultMaterial);
    m_materialCache["default"] = 0;

    while (!stream.eof())
    {
        line.clear();
        std::getline(stream, line);

        if (line.empty())
            continue;

        strStream.clear();
        strStream.str(line);
        strStream >> command;

        if (command == "#")
        {
            continue;
        }
        else if (command == "mtllib")
        {
            strStream >> name;
            name = m_directoryPath + name;
            importMaterials(name);
        }
        else if (command == "usemtl")
        {
            strStream >> name;
            iter = m_materialCache.find(name);
            activeMaterial = (iter == m_materialCache.end()) ? 0 : iter->second;
        }
        else if (command == "v")
        {
            strStream >> position[0] >> position[1] >> position[2];
            vertexCoords.push_back(position[0]);
            vertexCoords.push_back(position[1]);
            vertexCoords.push_back(position[2]);
        }
        else if (command == "vt")
        {
            strStream >> texCoord[0] >> texCoord[1];
            textureCoords.push_back(texCoord[0]);
            textureCoords.push_back(texCoord[1]);
        }
        else if (command == "vn")
        {
            strStream >> normal[0] >> normal[1] >> normal[2];
            normals.push_back(normal[0]);
            normals.push_back(normal[1]);
            normals.push_back(normal[2]);
        }
        else if (command == "f")
        {
            verticesPerFace = 0;

            while (true)
            {
                vertex.position[0] = vertex.position[1] = vertex.position[2] = 0.0f;
                vertex.texCoord[0] = vertex.texCoord[1] = 0.0f;
                vertex.normal[0] = vertex.normal[1] = vertex.normal[2] = 0.0f;

                strStream >> posIndex;

                if (strStream.fail())
                    break;

                vertex.position[0] = vertexCoords[(posIndex - 1) * 3];
                vertex.position[1] = vertexCoords[(posIndex - 1) * 3 + 1];
                vertex.position[2] = vertexCoords[(posIndex - 1) * 3 + 2];

                if (strStream.peek() == '/')
                {
                    strStream.ignore();

                    if (strStream.peek() != '/')
                    {
                        strStream >> texCoordIndex;
                        vertex.texCoord[0] = textureCoords[(texCoordIndex - 1) * 2];
                        vertex.texCoord[1] = textureCoords[(texCoordIndex - 1) * 2 + 1];
                    }

                    if (strStream.peek() == '/')
                    {
                        strStream.ignore();
                        strStream >> normalIndex;
                        vertex.normal[0] = normals[(normalIndex - 1) * 3];
                        vertex.normal[1] = normals[(normalIndex - 1) * 3 + 1];
                        vertex.normal[2] = normals[(normalIndex - 1) * 3 + 2];
                    }
                }

                addVertex(posIndex, &vertex);
                ++verticesPerFace;
            }

            if (verticesPerFace > 0)
            {
                if (verticesPerFace > 3)
                {
                    int triangles = triangulateLastInsertedFace(verticesPerFace);

                    for (int i = 0; i < triangles; ++i)
                        m_attributeBuffer.push_back(activeMaterial);
                }
                else
                {
                    m_attributeBuffer.push_back(activeMaterial);
                }
            }
        }
    }

    m_hasVertexNormals = !normals.empty();
    m_hasTextureCoords = !textureCoords.empty();
}

bool ModelOBJ::importMaterials(const std::string &filename)
{
    std::ifstream stream(filename.c_str());

    if (!stream.is_open())
        return false;

    Material *pMaterial = 0;
    int materialIndex = 0;
    int illum = 0;
    std::string command;
    std::string materialName;

    while (!stream.eof())
    {
        stream >> command;

        if (command == "newmtl")
        {
            materialIndex = static_cast<int>(m_materials.size());
            m_materials.push_back(Material());
            pMaterial = &m_materials[materialIndex];

            stream >> materialName;
            m_materialCache[materialName] = materialIndex;
        }
        else if (command == "Ka")
        {
            stream >> pMaterial->ambient[0]
            >> pMaterial->ambient[1]
            >> pMaterial->ambient[2];
            pMaterial->ambient[3] = 1.0f;
        }
        else if (command == "Kd")
        {
            stream >> pMaterial->diffuse[0]
            >> pMaterial->diffuse[1]
            >> pMaterial->diffuse[2];
            pMaterial->diffuse[3] = 1.0f;
        }
        else if (command == "Ks")
        {
            stream >> pMaterial->specular[0]
            >> pMaterial->specular[1]
            >> pMaterial->specular[2];
            pMaterial->specular[3] = 1.0f;
        }
        else if (command == "Ns")
        {
            stream >> pMaterial->shininess;

            // Wavefront .MTL file shininess is from [0,1000].
            // Scale back to a generic [0,1] range.

            pMaterial->shininess /= 1000.0f;
        }
        else if (command == "Tr" || command == "d")
        {
            stream >> pMaterial->alpha;
        }
        else if (command == "illum")
        {
            stream >> illum;

            if (illum == 1)
            {
                pMaterial->specular[0] = 0.0f;
                pMaterial->specular[1] = 0.0f;
                pMaterial->specular[2] = 0.0f;
                pMaterial->specular[3] = 1.0f;
            }
        }
        else if (command == "map_Kd")
        {
            stream >> pMaterial->colorMapFilename;
        }

        stream.ignore((std::numeric_limits<int>::max)(), '\n');
    }

    return true;
}

int ModelOBJ::triangulateLastInsertedFace(int verticesPerFace)
{
    // Triangulate the most recently inserted face in the index buffer. It is
    // assumed that the face was added to the end of the index buffer. The
    // first triangle is the first three vertices in the face. Every subsequent
    // triangle is made up of the first vertex in the face, the last vertex in
    // the previously created triangle and the next vertex in the face.

    assert(verticesPerFace <= FACE_INDEX_CACHE_SIZE);

    int offset = getNumberOfIndices() - verticesPerFace;
    int triangles = verticesPerFace - 2;
    int requiredIndices = triangles * 3;

    memcpy(m_faceIndexCache, &m_indexBuffer[offset], verticesPerFace * sizeof(int));

    // Make sure there's enough room for the extra indices.

    if (requiredIndices > verticesPerFace)
    {
        for (int i = 0; i < requiredIndices - verticesPerFace; ++i)
            m_indexBuffer.push_back(0);
    }   

    // Triangulate the face.

    int firstTriangleVertex = m_indexBuffer[offset];
    int lastTriangleVertex = m_indexBuffer[offset + 2];
    int nextTriangleVertex = 0;

    offset += 3;

    for (int i = 3; i < verticesPerFace; ++i)
    {
        nextTriangleVertex = m_faceIndexCache[i];

        m_indexBuffer[offset++] = firstTriangleVertex;
        m_indexBuffer[offset++] = lastTriangleVertex;
        m_indexBuffer[offset++] = nextTriangleVertex;

        lastTriangleVertex = nextTriangleVertex;
    }

    return triangles;
}

bool ModelOBJ::render(void) const
{
	glPushMatrix();

	glPopMatrix();

	return true;
}