// bsp2obj.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "BSP.h"

#include <vector>
#include <iostream>
#include <string>

using namespace std;

#define VERBOSE( x )	

bool ReadWholeFile( const char* file, vector<uint8_t>& data )
{
	FILE* f;
	fopen_s( &f, file, "rb" );
	if ( !f )
		return false;

	fseek( f, 0, SEEK_END );
	data.resize( ftell( f ) );
	fseek( f, 0, SEEK_SET );

	size_t totalRead = 0;
	while ( !feof( f ) && totalRead < data.size() )
	{
		size_t r = fread( &data[totalRead], 1, data.size() - totalRead, f );
		if ( !r )
			break;
		totalRead += r;
	}

	fclose(f);

	data.resize( totalRead );

	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	vector<uint8_t> bspData;

	if ( argc < 3 )
	{
		VERBOSE( cout << "Usage: bsp2obj <input.bsp> <output.obj>" << endl );
		return 1;
	}

	cout << "BSP2OBJ " << argv[1] << " ... ";

	if ( !ReadWholeFile( argv[1], bspData ) )
	{
		cout << "FAILED" << endl; 
		return 1;
	}

	VERBOSE( cout << "OK! " << bspData.size() << " bytes read." << endl );
	VERBOSE( cout << "Parsing ... " );

	BSP* bsp = BSP::Create( &bspData[0], bspData.size() );
	if ( !bsp )
	{
		cout << "FAILED" << endl;
		return 1;
	}

	cout << "OK!" << endl;
	VERBOSE( cout << "Summary:" << endl );

	VERBOSE( cout << " * " << bsp->Materials.size() << " Materials" << endl );
	VERBOSE( cout << " * " << bsp->Planes.size() << " Planes" << endl );
	VERBOSE( cout << " * " << bsp->Nodes.size() << " Nodes" << endl );
	VERBOSE( cout << " * " << bsp->Leaves.size() << " Leaves" << endl );
	VERBOSE( cout << " * " << bsp->LeafFaces.size() << " Leaf Faces" << endl );
	VERBOSE( cout << " * " << bsp->LeafBrushes.size() << " Leaf Brushes" << endl );
	VERBOSE( cout << " * " << bsp->Models.size() << " Models" << endl );
	VERBOSE( cout << " * " << bsp->Brushes.size() << " Brushes" << endl );
	VERBOSE( cout << " * " << bsp->BrushSides.size() << " Brush Sides" << endl );
	VERBOSE( cout << " * " << bsp->Vertices.size() << " Vertices" << endl );
	VERBOSE( cout << " * " << bsp->Indices.size() << " Indices" << endl );
	VERBOSE( cout << " * " << bsp->Fogs.size() << " Fogs" << endl );
	VERBOSE( cout << " * " << bsp->Faces.size() << " Faces" << endl );
	VERBOSE( cout << " * " << bsp->LightMaps.size() << " Light Maps" << endl );
	VERBOSE( cout << " * " << bsp->LightVolumes.size() << " Light Volumes" << endl );
	VERBOSE( cout << " * " << bsp->NumClusters << " Clusters" << endl );
	
	delete bsp;

	return 0;
}

