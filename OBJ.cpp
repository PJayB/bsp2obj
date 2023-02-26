#include "OBJ.h"

#include <id3bsp/BSP.h>
#include <id3bsp/Entities.h>

#include <fstream>
#include <string>
#include <iostream>
#include <cstdint>
#include <cassert>

using namespace std;
using namespace id3bsp;

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
			<<   " " << SafeF( 1.0f - v.TexCoord[1] )
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
	const BSP::TFaceList& faces,
	const BSP::TIndexList& indices,
	const BSP::TMaterialList& materials,
	ofstream& obj)
{

	obj << "# Begin Face Definitions for " << faces.size() << " faces." << endl;
    int surfCount = 0;
	for (size_t fi = 0; fi < faces.size(); ++fi)
	{
		const auto& f = faces[fi];

        // Skip non-solid surfs
		const BSP::Texture& tex = materials[f.TextureID];
        if ( tex.Flags & BSP::kSurfNoDraw ) {
			obj << "# skipping surface " << fi << " because it has SURF_NODRAW set" << endl << endl;
            continue;
		}

		// Skip patches (need to be tessellated into trimeshes beforehand)
		if (f.FaceType == BSP::kPatch) {
			obj << "# skipping patch surface " << fi << endl << endl;
            continue;
		}

		// Skip empty faces
		if (f.NumIndices == 0 && f.NumVertices == 0) {
			obj << "# skipping empty surface " << fi << endl << endl;
            continue;
		}

		obj << "# surface " << fi << " indices: " << f.StartIndex << "[" << f.NumIndices << "], verts: "
			<< f.StartVertexIndex << "[" << f.NumVertices << "]" << endl;
        obj << "usemtl " << SafeMaterial( tex.Name.data() ) << endl;
		obj << "o surf" << surfCount << endl;

		assert(f.NumIndices > 0);
		assert(f.NumVertices > 0);
		assert(f.StartIndex + f.NumIndices <= indices.size());

		const uint32_t* faceIndices = &indices[f.StartIndex];
		
		surfCount++;

		int numFaces = f.NumIndices / 3;
		for ( int a = 0; a < numFaces; ++a )
		{
			auto v0 = faceIndices[a * 3 + 0];
			auto v1 = faceIndices[a * 3 + 0];
			auto v2 = faceIndices[a * 3 + 0];
			assert(v0 < f.NumVertices);
			assert(v1 < f.NumVertices);
			assert(v2 < f.NumVertices);
			size_t i = 1 + f.StartVertexIndex + faceIndices[a * 3 + 0];
			size_t j = 1 + f.StartVertexIndex + faceIndices[a * 3 + 1];
			size_t k = 1 + f.StartVertexIndex + faceIndices[a * 3 + 2];
			obj << "f "	<< i << "/" << i << "/" << i << " "
						<< k << "/" << k << "/" << k << " "
						<< j << "/" << j << "/" << j << " "
						<< endl;
		}

		obj << endl;
	}
	obj << "# End Face Definitions" << endl << endl;
}

void TessellatePatches(
	BSP::TFaceList& faces,
	BSP::TIndexList& indices,
	BSP::TVertexList& vertices,
	int level)
{
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

	for (auto& f : faces)
	{
        if (f.FaceType != BSP::kPatch)
            continue;

		BSP::Tessellate(f, vertices, indices, level);
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

	obj << "# Generated by BSP2OBJ: github.com/PJayB/bsp2obj" << endl;
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
	TessellatePatches(faces, indices, vertices, tesselationLevel);

	DumpVertexList(vertices, obj);
	DumpFaceList(faces, indices, bsp->Materials, obj);

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
	// Here just to sanity check the parser -- doesn't do anything
	std::vector<id3bsp::Entity> entities;
	bool r = id3bsp::Entity::Parse(bsp->EntityString, filename, entities);
	assert(r);

	ofstream entF;
	entF.open( filename );
	if ( !entF.is_open() )
	{
		cout << "FAILED to open entity file: " << filename << endl;
		return false;
	}

	entF << "// Generated by BSP2OBJ" << endl;
	entF << endl;

	entF << bsp->EntityString;

	entF.close();

	return true;
}
