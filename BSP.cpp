/*
 *  BSP.cpp
 *  CameraTest
 *
 *  Created by Peter J. B. Lewis on 21/09/2010.
 *  Copyright 2010 Roaring Fist Entertainment. All rights reserved.
 *
 */

#include "stdafx.h"
#include "BSP.h"

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
	array.resize( lump.m_Length / sizeof( T ) );
	memcpy( &array[0], &cursor[lump.m_Offset], lump.m_Length );
}

static void ReadLump( const uint8_t* cursor, std::string& str, const BSP::Lump& lump )
{
	str.resize( lump.m_Length + 1 );
	memcpy( &str[0], &cursor[lump.m_Offset], lump.m_Length );
}

bool BSP::Load( const uint8_t* lpBytes, size_t cbSize )
{
	Unload();
	
	std::vector<LightMap> lightMaps;
	
	// Read the header
	const Header* header = (const Header*) lpBytes;
	
	// Read the lumps
	const Lump* lumps = (const Lump*) (lpBytes + sizeof(Header));
	
	// Read basic lumps
	// - entities (done later)
	ReadLump( lpBytes, m_Materials,		lumps[kTextures] );
	ReadLump( lpBytes, m_Planes,		lumps[kPlanes] );
	ReadLump( lpBytes, m_Nodes,			lumps[kNodes] );
	ReadLump( lpBytes, m_Leaves,		lumps[kLeaves] );
	ReadLump( lpBytes, m_LeafFaces,		lumps[kLeafFaces] );
	ReadLump( lpBytes, m_LeafBrushes,	lumps[kLeafBrushes] );
	ReadLump( lpBytes, m_Models,		lumps[kModels] );
	ReadLump( lpBytes, m_Brushes,		lumps[kBrushes] );
	ReadLump( lpBytes, m_BrushSides,	lumps[kBrushSides] );
	ReadLump( lpBytes, m_Vertices,		lumps[kVertices] );
	ReadLump( lpBytes, m_Indices,		lumps[kIndices] );
	ReadLump( lpBytes, m_Fogs,			lumps[kFogs] );
	ReadLump( lpBytes, m_Faces,			lumps[kFaces] );
	ReadLump( lpBytes, m_LightMaps,		lumps[kLightmaps] );
	ReadLump( lpBytes, m_LightVolumes,	lumps[kLightVolumes] );
	// - vis data (done later)
	
	// Read the entities info
	if ( lumps[kEntities].m_Length )
	{
		ReadLump( lpBytes, m_EntityString, lumps[kEntities] );
	}
	
	// Visibility data.
	m_NumClusters = 0;
	m_ClusterVisDataSize = 0;
	if ( lumps[kVisData].m_Length )
	{
		const uint32_t* visInfo = (const uint32_t*) (lpBytes + lumps[kVisData].m_Offset);
		m_NumClusters = visInfo[0];
		m_ClusterVisDataSize = visInfo[1];
		
		m_ClusterBits.resize( m_NumClusters * m_ClusterVisDataSize );
		memcpy( &m_ClusterBits[0], &visInfo[2], m_NumClusters * m_ClusterVisDataSize );
	}
	
	// Done and DONE.
	
#if 0//def RF_DEBUG
	for ( uint32_t leafIdx = 0; leafIdx < m_Leaves.size(); ++leafIdx )
	{
		const BSP::Leaf* leaf = &m_Leaves[leafIdx];

		// For each face in the leaf
		for ( uint32_t faceIdx = 0; faceIdx < leaf->m_NumLeafFaces; ++faceIdx )
		{
			uint32_t face_index = m_LeafFaces[ leaf->m_FirstFaceIndex + faceIdx ];
			
			const BSP::Face& f = m_Faces[ face_index ]; 
			const BSP::Vertex* firstVertex = &m_Vertices[f.m_StartVertexIndex];
			const uint16_t* firstIndex = &m_Indices[f.m_StartIndex];
		
			for ( uint32_t i = 0; i < f.m_NumIndices; ++i )
			{
				const BSP::Vertex& v = firstVertex[ firstIndex[i] ];
				//v.m_Position[0], v.m_Position[1], v.m_Position[2]
				RF_ASSERT( v.m_Position[0] >= leaf->m_Mins[0] );
				RF_ASSERT( v.m_Position[1] >= leaf->m_Mins[1] );
				RF_ASSERT( v.m_Position[2] >= leaf->m_Mins[2] );
				RF_ASSERT( v.m_Position[0] <= leaf->m_Maxs[0] );
				RF_ASSERT( v.m_Position[1] <= leaf->m_Maxs[1] );
				RF_ASSERT( v.m_Position[2] <= leaf->m_Maxs[2] );
			}
		}
	}
#endif
	
	return true;
}

void BSP::Unload()
{
	m_Materials.clear();
	m_Planes.clear();
	m_Nodes.clear();
	m_Leaves.clear();
	m_LeafFaces.clear();
	m_LeafBrushes.clear();
	m_Models.clear();
	m_Brushes.clear();
	m_BrushSides.clear();
	m_Vertices.clear();
	m_Indices.clear();
	m_Fogs.clear();
	m_Faces.clear();
	m_LightMaps.clear();
	m_LightVolumes.clear();
	m_EntityString.clear();
	m_ClusterBits.clear();
	
	m_NumClusters = 0;
	m_ClusterVisDataSize = 0;
}

