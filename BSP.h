/*
 *  BSP.h
 *  CameraTest
 *
 *  Created by Peter J. B. Lewis on 21/09/2010.
 *  Copyright 2010 Roaring Fist Entertainment. All rights reserved.
 *
 */

#ifndef __BSP_H__
#define __BSP_H__

#include <vector>
#include <cstdint>

#define BSP_EPSILON 0.03125f
	
class BSP
{
public:

	enum eLumpID
	{
		kEntities = 0,      // Stores player/object positions, etc...
		kTextures,          // Stores texture information
		kPlanes,            // Stores the splitting planes
		kNodes,             // Stores the BSP nodes
		kLeaves,            // Stores the leafs of the nodes
		kLeafFaces,         // Stores the leaf's indices into the faces
		kLeafBrushes,       // Stores the leaf's indices into the brushes
		kModels,            // Stores the info of world models
		kBrushes,           // Stores the brushes info (for collision)
		kBrushSides,        // Stores the brush surfaces info
		kVertices,          // Stores the level vertices
		kIndices,           // Stores the level indices
		kFogs,				// ???
		kFaces,             // Stores the faces for the level
		kLightmaps,         // Stores the lightmaps for the level
		kLightVolumes,      // Stores extra world lighting information
		kVisData,           // Stores PVS and cluster info (visibility)
		kMaxLumps           // A constant to store the number of lumps	
	};
	
	enum eFaceType
	{
		kBad = 0,
		kPolygon = 1,
		kPatch = 2,
		kMesh = 3,
		kBillboard = 4,
		kFoliage = 5
	};
	
	enum eTextureFlags
	{
		kSolid = 1,
		// TODO
	};

	struct Lump
	{
		uint32_t				m_Offset;	// Offset into file.
		uint32_t				m_Length;	// Length of data block.
	};
	
	struct Header
	{
		uint8_t					m_IBSP[4];
		int32_t					m_Version;
	};								
	
	struct Vertex
	{
		float					m_Position[3];
		float					m_TexCoord[2];
		float					m_LMCoord[2];	// Lightmap coordinate.
		float					m_Normal[3];
		uint8_t					m_Colour[4];
	};
	
	struct Face
	{
		int32_t					m_TextureID;
		int32_t					m_FogID;
		uint32_t				m_FaceType;

		uint32_t				m_StartVertexIndex;
		uint32_t				m_NumVertices;

		uint32_t				m_StartIndex;
		uint32_t				m_NumIndices;

		uint32_t				m_LightMapID;
		uint32_t				m_LMX; // "The face's lightmap corner in the image"
		uint32_t				m_LMY;
		uint32_t				m_LMWidth; // Size of the lightmap section
		uint32_t				m_LMHeight;
		
		float					m_LMOrigin[3]; // 3D origin of lightmap (???)
		float					m_LMVecs[2][3]; // 3D space for s and t unit vectors (???)
		
		float					m_Normal[3];
		
		uint32_t				m_BezierDimensions[2];
	};
	
	struct Texture
	{
		char					m_Name[64];
		uint32_t				m_Flags; // Apparently unused?
		uint32_t				m_TextureFlags; // See eTextureFlags
	};
	
	struct LightMap
	{
		uint8_t					m_Data[128][128][3];
	};
	
	struct Node
	{
		uint32_t				m_PlaneIndex;
		uint32_t				m_FrontIndex; // front node index
		uint32_t				m_BackIndex;
		int32_t					m_Mins[3];
		int32_t					m_Maxs[3];
	};
	
	struct Leaf
	{
		uint32_t				m_VisibilityCluster;
		uint32_t				m_AreaPortal;
		int32_t					m_Mins[3];
		int32_t					m_Maxs[3];
		uint32_t				m_FirstFaceIndex;
		uint32_t				m_NumLeafFaces;
		uint32_t				m_FirstBrushIndex;
		uint32_t				m_NumLeafBrushes;
	};
	
	struct Brush
	{
		uint32_t				m_FirstBrushSide;
		uint32_t				m_NumBrushSides;
		int32_t					m_TextureIndex;
	};
	
	struct BrushSide
	{
		uint32_t				m_Plane;
		int32_t					m_TextureIndex;
	};
	
	struct Fog
	{
		char					m_TextureName[64]; 
		int32_t					m_BrushIndex;
		int32_t					m_VisibleSide; // -1 for none
	};
	
	struct LightVolume
	{
		uint8_t					m_Ambient[3];
		uint8_t					m_Directional[3];
		uint8_t					m_Direction[2]; // phi, theta.
	};
	
	struct Model
	{
		float					m_Mins[3];
		float					m_Maxs[3];
		uint32_t				m_FirstFaceIndex;
		uint32_t				m_NumFaces;
		uint32_t				m_FirstBrushIndex;
		uint32_t				m_NumBrushes;
	};

	struct Plane
	{
		float X, Y, Z, W;
	};
	
	static BSP*	
	Create(
		const void* lpByteBlob,
		size_t cbSize );
	
	~BSP();
	
	static float				
	GetLightMapGamma( );
	
protected:

	BSP();
	bool				Load( const uint8_t* lpBytes, size_t cbSize );
	void				Unload();
	
	typedef std::vector<Texture>			TMaterialList;
	typedef std::vector<Plane>				TPlaneList;
	typedef std::vector<Node>				TNodeList;
	typedef std::vector<Leaf>				TLeafList;
	typedef std::vector<uint32_t>			TLeafFaceList;
	typedef std::vector<uint32_t>			TLeafBrushList;
	typedef std::vector<Model>				TModelList;
	typedef std::vector<Brush>				TBrushList;
	typedef std::vector<BrushSide>			TBrushSideList;
	typedef std::vector<Vertex>				TVertexList;
	typedef std::vector<uint16_t>			TIndexList;
	typedef std::vector<Fog>				TFogList;
	typedef std::vector<Face>				TFaceList;
	typedef std::vector<LightMap>			TLightMapList;
	typedef std::vector<LightVolume>		TLightVolumeList;
	typedef std::vector<uint8_t>			TClusterBitList;
	
	TMaterialList		m_Materials;
	TPlaneList			m_Planes;
	TNodeList			m_Nodes;
	TLeafList			m_Leaves;
	TLeafFaceList		m_LeafFaces;
	TLeafBrushList		m_LeafBrushes;
	TModelList			m_Models;
	TBrushList			m_Brushes;
	TBrushSideList		m_BrushSides;
	TVertexList			m_Vertices;
	TIndexList			m_Indices;
	TFogList			m_Fogs;
	TFaceList			m_Faces;
	TLightMapList		m_LightMaps;
	TLightVolumeList	m_LightVolumes;
	
	std::string			m_EntityString;
	
	TClusterBitList		m_ClusterBits;
	uint32_t			m_NumClusters;
	uint32_t			m_ClusterVisDataSize;
};

#endif
