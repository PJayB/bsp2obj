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

#define MAXLIGHTMAPS 4
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
        kLightArray,        // <TODO>
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
		uint32_t				Offset;	// Offset into file.
		uint32_t				Length;	// Length of data block.
	};
	
	struct Header
	{
		uint8_t					IBSP[4];
		int32_t					Version;
	};								
	
	struct Vertex
	{
		float					Position[3];
		float					TexCoord[2];
		float					LMCoord[MAXLIGHTMAPS][2];	// Lightmap coordinate.
		float					Normal[3];
		uint8_t					Colour[MAXLIGHTMAPS][4];
	};
	
	struct Face
	{
		int32_t					TextureID;
		int32_t					FogID;
		uint32_t				FaceType;

		uint32_t				StartVertexIndex;
		uint32_t				NumVertices;

		uint32_t				StartIndex;
		uint32_t				NumIndices;

		uint8_t					LightMapStyles[MAXLIGHTMAPS];
		uint8_t					VertexStyles[MAXLIGHTMAPS];

		uint32_t				LightMapIDs[MAXLIGHTMAPS];
		uint32_t				LMX[MAXLIGHTMAPS]; // "The face's lightmap corner in the image"
		uint32_t				LMY[MAXLIGHTMAPS];
		uint32_t				LMWidth; // Size of the lightmap section
		uint32_t				LMHeight;
		
		float					LMOrigin[3]; // 3D origin of lightmap (???)
		float					LMVecs[3][3]; // 3D space for s and t unit vectors (???)
		
		uint32_t				BezierDimensions[2];
	};
	
	struct Texture
	{
		char					Name[64];
		uint32_t				Flags; // Apparently unused?
		uint32_t				TextureFlags; // See eTextureFlags
	};
	
	struct LightMap
	{
		uint8_t					Data[128][128][3];
	};
	
	struct Node
	{
		uint32_t				PlaneIndex;
		uint32_t				FrontIndex; // front node index
		uint32_t				BackIndex;
		int32_t					Mins[3];
		int32_t					Maxs[3];
	};
	
	struct Leaf
	{
		uint32_t				VisibilityCluster;
		uint32_t				AreaPortal;
		int32_t					Mins[3];
		int32_t					Maxs[3];
		uint32_t				FirstFaceIndex;
		uint32_t				NumLeafFaces;
		uint32_t				FirstBrushIndex;
		uint32_t				NumLeafBrushes;
	};
	
	struct Brush
	{
		uint32_t				FirstBrushSide;
		uint32_t				NumBrushSides;
		int32_t					TextureIndex;
	};
	
	struct BrushSide
	{
		uint32_t				Plane;
		int32_t					TextureIndex;
        int32_t                 DrawSurfNum;
	};
	
	struct Fog
	{
		char					TextureName[64]; 
		int32_t					BrushIndex;
		int32_t					VisibleSide; // -1 for none
	};
	
	struct LightVolume
	{
		uint8_t					Ambient[MAXLIGHTMAPS][3];
		uint8_t					Directional[MAXLIGHTMAPS][3];
        uint8_t                 Syles[MAXLIGHTMAPS];
		uint8_t					Direction[2]; // phi, theta.
	};
	
	struct Model
	{
		float					Mins[3];
		float					Maxs[3];
		uint32_t				FirstFaceIndex;
		uint32_t				NumFaces;
		uint32_t				FirstBrushIndex;
		uint32_t				NumBrushes;
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
	
public:

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
	typedef std::vector<uint32_t>			TIndexList;
	typedef std::vector<Fog>				TFogList;
	typedef std::vector<Face>				TFaceList;
	typedef std::vector<LightMap>			TLightMapList;
	typedef std::vector<LightVolume>		TLightVolumeList;
	typedef std::vector<uint8_t>			TClusterBitList;
	
	TMaterialList		Materials;
	TPlaneList			Planes;
	TNodeList			Nodes;
	TLeafList			Leaves;
	TLeafFaceList		LeafFaces;
	TLeafBrushList		LeafBrushes;
	TModelList			Models;
	TBrushList			Brushes;
	TBrushSideList		BrushSides;
	TVertexList			Vertices;
	TIndexList			Indices;
	TFogList			Fogs;
	TFaceList			Faces;
	TLightMapList		LightMaps;
	TLightVolumeList	LightVolumes;
	
	std::string			EntityString;
	
	TClusterBitList		ClusterBits;
	uint32_t			NumClusters;
	uint32_t			ClusterVisDataSize;
};

#endif
