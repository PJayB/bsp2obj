#include "stdafx.h"
#include "OBJ.h"
#include "BSP.h"

#include <fstream>
#include <string>
#include <iostream>
#include <cstdint>

using namespace std;

string SafeF( float f )
{
	char tmp[128];
	sprintf_s(
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


const int SF_NODRAW = 0x00200000;


bool DumpObj( const char* filename, const char* mtlFilename, const BSP* bsp )
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

	obj << "# Begin Vertex Positions for " << bsp->Vertices.size() << " vertices." << endl;
	for ( auto& v : bsp->Vertices )
	{
		obj << "v " << SafeF( v.Position[0] )
			<<  " " << SafeF( v.Position[2] )
			<<  " " << SafeF(-v.Position[1] )
			<< endl;
	}
	obj << "# End Vertex Positions" << endl << endl;

	obj << "# Begin Vertex UVs for " << bsp->Vertices.size() << " vertices." << endl;
	for ( auto& v : bsp->Vertices )
	{
		obj << "vt " << SafeF( v.TexCoord[0] )
			<<   " " << SafeF( v.TexCoord[1] )
			<<   " " << SafeF( v.TexCoord[2] )
			<< endl;
	}
	obj << "# End Vertex UVs" << endl << endl;

	obj << "# Begin Vertex Normals for " << bsp->Vertices.size() << " vertices." << endl;
	for ( auto& v : bsp->Vertices )
	{
		obj << "vn " << SafeF( v.Normal[0] )
			<<   " " << SafeF( v.Normal[1] )
			<<   " " << SafeF( v.Normal[2] )
			<< endl;
	}
	obj << "# End Vertex Normals" << endl << endl;

	obj << "# Begin Face Definitions for " << bsp->Faces.size() << " faces." << endl;
    int surfCount = 0;
	for ( auto& f : bsp->Faces )
	{
        if ( f.NumIndices == 0 )
            continue;

        // Skip non-solid surfs
		const BSP::Texture& tex = bsp->Materials[f.TextureID];
        if ( tex.Flags & SF_NODRAW )
            continue;

        obj << "usemtl " << SafeMaterial( tex.Name ) << endl;
        obj << "o surf" << surfCount << endl;

		const uint32_t* indices = &bsp->Indices[f.StartIndex];
        
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
        if ( tex.Flags & SF_NODRAW )
            continue;

        mtl << "newmtl " << SafeMaterial( tex.Name ) << endl;
        mtl << "Ka 1 1 1" << endl;
        mtl << "Kd 1 1 1" << endl;
        mtl << "Ks 0 0 0" << endl;
        mtl << "Ns 10" << endl;

		StringMap::const_iterator m = textureRemap.find(tex.Name);
		if ( m != end(textureRemap) )
			mtl << "map_Kd " << m->second << endl;
		else
			mtl << "map_Kd " << tex.Name << endl;

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
