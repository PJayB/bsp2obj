// bsp2obj.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "BSP.h"

#include <vector>
#include <iostream>
#include <string>

using namespace std;

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
		cout << "Usage: bsp2obj <input.bsp> <output.obj>" << endl;
		return 1;
	}

	cout << "Reading " << argv[1] << " ... ";

	if ( !ReadWholeFile( argv[1], bspData ) )
	{
		cout << "FAILED" << endl; 
		return 1;
	}

	cout << "OK! " << bspData.size() << " bytes read." << endl;
	cout << "Parsing ... ";

	BSP* bsp = BSP::Create( &bspData[0], bspData.size() );
	if ( !bsp )
	{
		cout << "FAILED" << endl;
		return 1;
	}

	cout << "OK! Summary:" << endl;

	cout << " * " << bsp->Materials.size() << " Materials" << endl;
	cout << " * " << bsp->Planes.size() << " Planes" << endl;
	cout << " * " << bsp->Nodes.size() << " Nodes" << endl;
	cout << " * " << bsp->Leaves.size() << " Leaves" << endl;
	cout << " * " << bsp->LeafFaces.size() << " Leaf Faces" << endl;
	cout << " * " << bsp->LeafBrushes.size() << " Leaf Brushes" << endl;
	cout << " * " << bsp->Models.size() << " Models" << endl;
	cout << " * " << bsp->Brushes.size() << " Brushes" << endl;
	cout << " * " << bsp->BrushSides.size() << " Brush Sides" << endl;
	cout << " * " << bsp->Vertices.size() << " Vertices" << endl;
	cout << " * " << bsp->Indices.size() << " Indices" << endl;
	cout << " * " << bsp->Fogs.size() << " Fogs" << endl;
	cout << " * " << bsp->Faces.size() << " Faces" << endl;
	cout << " * " << bsp->LightMaps.size() << " Light Maps" << endl;
	cout << " * " << bsp->LightVolumes.size() << " Light Volumes" << endl;
	cout << " * " << bsp->NumClusters << " Clusters" << endl;





	delete bsp;

	return 0;
}

