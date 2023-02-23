#include "OBJ.h"
#include "BSP.h"

#include <fstream>
#include <string>
#include <iostream>
#include <cstdint>
#include <cassert>

using namespace std;

string SafeF( float f )
{
	char tmp[128];
	snprintf(
		tmp, sizeof(tmp),
		"%f", f );
	return tmp;
}

string SafeMaterial( string mtl )
{
    for ( size_t i = 0; i < mtl.size(); ++i )
    {
        if ( mtl[i] == '/' )
            mtl[i] = '_';
    }
    return move( mtl );
}

void DumpVertexList(
	const std::vector<BSP::Vertex>& vertices,
	ofstream& obj)
{
	obj << "# Begin Vertex Positions for " << vertices.size() << " vertices." << endl;
	for ( auto& v : vertices )
	{
		obj << "v " << SafeF( v.Position[0] )
			<<  " " << SafeF( v.Position[2] )
			<<  " " << SafeF(-v.Position[1] )
			<< endl;
	}
	obj << "# End Vertex Positions" << endl << endl;

	obj << "# Begin Vertex UVs for " << vertices.size() << " vertices." << endl;
	for ( auto& v : vertices )
	{
		obj << "vt " << SafeF( v.TexCoord[0] )
			<<   " " << SafeF( v.TexCoord[1] )
			<<   " " << SafeF( v.TexCoord[2] )
			<< endl;
	}
	obj << "# End Vertex UVs" << endl << endl;

	obj << "# Begin Vertex Normals for " << vertices.size() << " vertices." << endl;
	for ( auto& v : vertices )
	{
		obj << "vn " << SafeF( v.Normal[0] )
			<<   " " << SafeF( v.Normal[1] )
			<<   " " << SafeF( v.Normal[2] )
			<< endl;
	}
	obj << "# End Vertex Normals" << endl << endl;
}

void DumpFaceList(
	const std::vector<BSP::Face>& faces,
	const BSP& bsp,
	ofstream& obj)
{

	obj << "# Begin Face Definitions for " << faces.size() << " faces." << endl;
    int surfCount = 0;
	for ( auto& f : faces )
	{
        if ( f.NumIndices == 0 )
            continue;

        // Skip non-solid surfs
		const BSP::Texture& tex = bsp.Materials[f.TextureID];
        if ( tex.Flags & BSP::kSurfNoDraw )
            continue;

		// Skip patches (done later)
		if (f.FaceType == BSP::kPatch)
			continue;

        obj << "usemtl " << SafeMaterial( tex.Name.data() ) << endl;
		obj << "o surf" << surfCount << endl;

		const uint32_t* indices = &bsp.Indices[f.StartIndex];
		
		surfCount++;

		int numFaces = f.NumIndices / 3;
		for ( int a = 0; a < numFaces; ++a )
		{
			size_t i = 1 + f.StartVertexIndex + indices[a * 3 + 0];
			size_t j = 1 + f.StartVertexIndex + indices[a * 3 + 1];
			size_t k = 1 + f.StartVertexIndex + indices[a * 3 + 2];
			obj << "f "	<< i << "/" << i << "/" << i << " "
						<< k << "/" << k << "/" << k << " "
						<< j << "/" << j << "/" << j << " "
						<< endl;
		}

		obj << endl;
	}
	obj << "# End Face Definitions" << endl << endl;
}

struct Tesselator
{
	Tesselator(
		BSP::TVertexList& vertices,
		BSP::TIndexList& indices,
		int bezierLevel)
	: VertexArray(vertices)
	, IndexArray(indices)
	, BezierLevel(bezierLevel)
	{}

	BSP::TVertexList& VertexArray;
	BSP::TIndexList& IndexArray;
	int BezierLevel;

	void Tesselate(int controlOffset, int controlWidth, int vOffset, int iOffset);
};

// Shamelessly stolen from git@github.com:leezh/bspviewer.git
void Tesselator::Tesselate(
	int controlOffset, int controlWidth, int vOffset, int iOffset)
{
    BSP::Vertex controls[9];
    int cIndex = 0;
    for (int c = 0; c < 3; c++)
    {
        int pos = c * controlWidth;
        controls[cIndex++] = VertexArray[controlOffset + pos];
        controls[cIndex++] = VertexArray[controlOffset + pos + 1];
        controls[cIndex++] = VertexArray[controlOffset + pos + 2];
    }

    int L1 = BezierLevel + 1;

    for (int j = 0; j <= BezierLevel; ++j)
    {
        float a = (float)j / BezierLevel;
        float b = 1.f - a;

		BSP::Vertex v;
		// todo interpolate
		// controls[0] * b * b + controls[3] * 2 * b * a + controls[6] * a * a
        VertexArray.push_back(v);
    }

    for (int i = 1; i <= BezierLevel; ++i)
    {
        float a = (float)i / BezierLevel;
        float b = 1.f - a;

        BSP::Vertex temp[3];

        for (int j = 0; j < 3; ++j)
        {
            int k = 3 * j;
            // todo: interpolate
			//temp[j] = controls[k + 0] * b * b + controls[k + 1] * 2 * b * a + controls[k + 2] * a * a;
        }

        for (int j = 0; j <= BezierLevel; ++j)
        {
            float a = (float)j / BezierLevel;
            float b = 1.f - a;

			BSP::Vertex v;
            // todo: interpolate temp[0] * b * b + temp[1] * 2 * b * a + temp[2] * a * a;
			assert(VertexArray.size() == vOffset + i * L1 + j);
            VertexArray.push_back(v);
        }
    }

    for (int i = 0; i <= BezierLevel; ++i)
    {
        for (int j = 0; j <= BezierLevel; ++j)
        {
            int offset = iOffset + (i * BezierLevel + j) * 6;
            IndexArray.push_back((i    ) * L1 + (j    ) + vOffset);
            IndexArray.push_back((i    ) * L1 + (j + 1) + vOffset);
            IndexArray.push_back((i + 1) * L1 + (j + 1) + vOffset);

            IndexArray.push_back((i + 1) * L1 + (j + 1) + vOffset);
            IndexArray.push_back((i + 1) * L1 + (j    ) + vOffset);
            IndexArray.push_back((i    ) * L1 + (j    ) + vOffset);
        }
    }
}

void TesselatePatches(
	BSP::TFaceList& faces,
	BSP::TIndexList& indices,
	BSP::TVertexList& vertices,
	int level)
{
	return; // todo

	int patchVertexCount = (level + 1) * (level + 1);
	int patchIndexCount = 6 * level * level;

	// Do a quick count of how much space we'll need
	int extraVertCount = 0;
	int extraIndexCount = 0;
	for (auto& f : faces)
	{
        if (f.FaceType == BSP::kPatch) {
			extraVertCount += patchVertexCount;
			extraIndexCount += patchIndexCount;
		}   
	}

	vertices.reserve(vertices.size() + extraVertCount);
	indices.reserve(indices.size() + extraIndexCount);

	Tesselator t(
		vertices,
		indices,
		level);

	for (auto& f : faces)
	{
        if (f.FaceType != BSP::kPatch)
            continue;

		int sizeX = (f.BezierDimensions[0] - 1) / 2;
		int sizeY = (f.BezierDimensions[1] - 1) / 2;

		f.StartIndex = static_cast<int>(indices.size());

		for (int x = 0, i = 0; i < sizeX; ++i, x = i * 2) {
			for (int y = 0, j = 0; j < sizeY; ++j, y = j * 2) {
				t.Tesselate(
					f.StartIndex + x + f.BezierDimensions[0] * y,
					f.BezierDimensions[0],
					static_cast<int>(vertices.size()),
					static_cast<int>(indices.size()));
			}
		}

		f.NumIndices = static_cast<int>(indices.size()) - f.StartIndex;
		f.FaceType = BSP::kPolygon;
	}
}

bool DumpObj( const char* filename, const char* mtlFilename, const BSP* bsp, int tesselationLevel )
{
	// Open the output file
	ofstream obj;
	obj.open( filename );
	if ( !obj.is_open() )
	{
		cout << "FAILED to write!" << endl;
		return false;
	}

	obj.unsetf(ostream::floatfield);

	obj << "# Generated by BSP2OBJ" << endl;
	obj << "# BSP2OBJ was created by Peter J. B. Lewis, 2013" << endl;
	obj << "# If your OBJ doesn't load, please email the " << endl;
	obj << "# dedicated support email: bsp2obj@gibbering.net" << endl;
	obj << endl;

    string localMtlFN = mtlFilename;
    string::size_type slashPos = localMtlFN.find('/');
    if ( slashPos != string::npos )
    {
        localMtlFN = localMtlFN.substr(slashPos + 1);
    }
    obj << "mtllib " << localMtlFN.c_str() << endl << endl;

	// Copy the vertex and face data
	auto vertices = bsp->Vertices;
	auto indices = bsp->Indices;
	auto faces = bsp->Faces;

	// Junk all the patch faces and replace them with trimeshes
	TesselatePatches(faces, indices, vertices, tesselationLevel);

	DumpVertexList(bsp->Vertices, obj);
	DumpFaceList(bsp->Faces, *bsp, obj);

	obj.close();

    return true;
}

bool DumpMtl( const char* filename, const BSP* bsp, const StringMap& textureRemap )
{
	ofstream mtl;
	mtl.open( filename );
	if ( !mtl.is_open() )
	{
		cout << "FAILED to open material file: " << filename << endl;
		return false;
	}

	mtl.unsetf(ostream::floatfield);

	mtl << "# Generated by BSP2OBJ" << endl;
	mtl << "# BSP2OBJ was created by Peter J. B. Lewis, 2013" << endl;
	mtl << "# If your OBJ doesn't load, please email the " << endl;
	mtl << "# dedicated support email: bsp2obj@gibbering.net" << endl;
	mtl << endl;

    for ( auto& tex : bsp->Materials )
    {
        // Skip non-solid surfs
        if ( tex.Flags & BSP::kSurfNoDraw )
            continue;

        mtl << "newmtl " << SafeMaterial( tex.Name.data() ) << endl;
        mtl << "Ka 1 1 1" << endl;
        mtl << "Kd 1 1 1" << endl;
        mtl << "Ks 0 0 0" << endl;
        mtl << "Ns 10" << endl;

		StringMap::const_iterator m = textureRemap.find(tex.Name.data());
		if ( m != end(textureRemap) )
			mtl << "map_Kd " << m->second << endl;
		else
			mtl << "map_Kd " << tex.Name.data() << endl;

        mtl << endl;
    }

	mtl.close();

    return true;
}

bool DumpEnts( const char* filename, const BSP* bsp )
{
	ofstream entF;
	entF.open( filename );
	if ( !entF.is_open() )
	{
		cout << "FAILED to open entity file: " << filename << endl;
		return false;
	}

	entF << "// Generated by BSP2OBJ" << endl;
	entF << "// BSP2OBJ was created by Peter J. B. Lewis, 2013" << endl;
	entF << "// If your OBJ doesn't load, please email the " << endl;
	entF << "// dedicated support email: bsp2obj@gibbering.net" << endl;
	entF << endl;

	entF << bsp->EntityString;

	entF.close();

	return true;
}
