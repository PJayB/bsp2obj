#include "BSP.h"
#include <cassert>
#include <memory.h>

BSP* BSP::Create( const void* lpData, size_t cbSize )
{
	BSP* bsp = new BSP;
	
	if ( !bsp->Load( (const uint8_t*) lpData, cbSize ) )
	{
		delete bsp;
		return NULL;
	}
	
	return bsp;
}

BSP::BSP()
{
}

BSP::~BSP()
{
	Unload();
}

void GammaCorrect(uint8_t* image, uint32_t size, float factor)
{
	for ( uint32_t i = 0; i < size; ++i, image += 3 )
	{
		float r = ( (float)image[0] ) * factor;
		float g = ( (float)image[1] ) * factor;
		float b = ( (float)image[2] ) * factor;
	
		image[0] = std::min( (uint32_t)( r ), (uint32_t)0xFFU );
		image[1] = std::min( (uint32_t)( g ), (uint32_t)0xFFU );
		image[2] = std::min( (uint32_t)( b ), (uint32_t)0xFFU );
	}
	
/*
	// Go through every pixel in the lightmap
	for(int i = 0; i < size; i++, image += 3) 
	{
		float scale = 1.0f, temp = 0.0f;
		float r = 0, g = 0, b = 0;
		
		// extract the current RGB values
		r = (float)image[0];
		g = (float)image[1];
		b = (float)image[2];
		
		// Multiply the factor by the RGB values, while keeping it to a 255 ratio
		r = r * factor / 255.0f;
		g = g * factor / 255.0f;
		b = b * factor / 255.0f;
		
		// Check if the the values went past the highest value
		if(r > 1.0f && (temp = (1.0f/r)) < scale) scale=temp;
		if(g > 1.0f && (temp = (1.0f/g)) < scale) scale=temp;
		if(b > 1.0f && (temp = (1.0f/b)) < scale) scale=temp;
		
		// Get the scale for this pixel and multiply it by our pixel values
		scale*=255.0f;		
		r*=scale;	g*=scale;	b*=scale;
		
		// Assign the new gamma'nized RGB values to our image
		image[0] = (uint8_t)r;
		image[1] = (uint8_t)g;
		image[2] = (uint8_t)b;
	}
*/	
}

float BSP::GetLightMapGamma()
{
	return  5.0f;
}
	
template<class T>
static void ReadLump( const uint8_t* cursor, std::vector<T>& array, const BSP::Lump& lump )
{
	if ( lump.Length > 0 )
	{
		array.resize( lump.Length / sizeof( T ) );

		assert( array.size() * sizeof( T ) == lump.Length );

		memcpy( 
			&array[0], 
			&cursor[lump.Offset], 
			array.size() * sizeof( T ));
	}
}

static void ReadLump( const uint8_t* cursor, std::string& str, const BSP::Lump& lump )
{
	if ( lump.Length > 0 )
	{
		str.reserve( lump.Length + 1 );
		str = (const char*) &cursor[lump.Offset];
	}
}

bool BSP::Load( const uint8_t* lpBytes, size_t cbSize )
{
	Unload();
		
	// Read the header
	const Header* header = (const Header*) lpBytes;
	
	// Read the lumps
	const Lump* lumps = (const Lump*) (lpBytes + sizeof(Header));
	
	// Read basic lumps
	// - entities (done later)
	ReadLump( lpBytes, Materials,		lumps[kTextures] );
	ReadLump( lpBytes, Planes,			lumps[kPlanes] );
	ReadLump( lpBytes, Nodes,			lumps[kNodes] );
	ReadLump( lpBytes, Leaves,			lumps[kLeaves] );
	ReadLump( lpBytes, LeafFaces,		lumps[kLeafFaces] );
	ReadLump( lpBytes, LeafBrushes,		lumps[kLeafBrushes] );
	ReadLump( lpBytes, Models,			lumps[kModels] );
	ReadLump( lpBytes, Brushes,			lumps[kBrushes] );
	ReadLump( lpBytes, BrushSides,		lumps[kBrushSides] );
	ReadLump( lpBytes, Vertices,		lumps[kVertices] );
	ReadLump( lpBytes, Indices,			lumps[kIndices] );
	ReadLump( lpBytes, Fogs,			lumps[kFogs] );
	ReadLump( lpBytes, Faces,			lumps[kFaces] );
	ReadLump( lpBytes, LightMaps,		lumps[kLightmaps] );
	ReadLump( lpBytes, LightVolumes,	lumps[kLightVolumes] );
	// - vis data (done later)
	
	// Read the entities info
	if ( lumps[kEntities].Length )
	{
		ReadLump( lpBytes, EntityString, lumps[kEntities] );
	}

	// Visibility data.
	NumClusters = 0;
	ClusterVisDataSize = 0;
	if ( lumps[kVisData].Length )
	{
		const uint32_t* visInfo = (const uint32_t*) (lpBytes + lumps[kVisData].Offset);
		NumClusters = visInfo[0];
		ClusterVisDataSize = visInfo[1];
		
		ClusterBits.resize( NumClusters * ClusterVisDataSize );
		memcpy( &ClusterBits[0], &visInfo[2], NumClusters * ClusterVisDataSize );
	}
	
	// Done and DONE.

	return true;
}

void BSP::Unload()
{
	Materials.clear();
	Planes.clear();
	Nodes.clear();
	Leaves.clear();
	LeafFaces.clear();
	LeafBrushes.clear();
	Models.clear();
	Brushes.clear();
	BrushSides.clear();
	Vertices.clear();
	Indices.clear();
	Fogs.clear();
	Faces.clear();
	LightMaps.clear();
	LightVolumes.clear();
	EntityString.clear();
	ClusterBits.clear();
	
	NumClusters = 0;
	ClusterVisDataSize = 0;
}

