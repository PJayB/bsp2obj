#ifndef __BSP_H__
#define __BSP_H__

#include <cstdlib>
#include <vector>
#include <string>
#include <array>

class BSP
{
public:
	static constexpr uint32_t kMaxLightMaps = 4;

	static constexpr uint32_t kRBSPFormat = (('P'<<24)+('S'<<16)+('B'<<8)+'R');
	static constexpr uint32_t kIBSPFormat = (('P'<<24)+('S'<<16)+('B'<<8)+'I');

	using Bounds2D = std::array<int32_t, 2>;
	using Bounds3D = std::array<int32_t, 3>;
	using Vec2 = std::array<float, 2>;
	using Vec3 = std::array<float, 3>;
	using Color3 = std::array<uint8_t, 3>;
	using Color4 = std::array<uint8_t, 4>;
	using LightMapU8s = std::array<uint8_t, kMaxLightMaps>;
	using LightMapU32s = std::array<uint32_t, kMaxLightMaps>;
	using LightMapVec2s = std::array<Vec2, kMaxLightMaps>;
	using LightMapColor3s = std::array<Color3, kMaxLightMaps>;
	using LightMapColor4s = std::array<Color4, kMaxLightMaps>;
	using LightMapData = std::array<uint8_t, 128 * 128 * 3>;

	// There are plenty more but this is the most commonly relevant one
	enum eSurfaceFlags
	{
		kSurfNoDraw     	= 0x80
	};

	enum eLightMapIndices
	{
		kLightMap2D 		= -4,	// shader is for 2D rendering
		kLightMapByVertex 	= -3,	// pre-lit triangle models
		kLightMapWhiteImage = -2,
		kLightMapNone 		= -1,
	};

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
		uint32_t				Format; // RBSP for Raven BSP files
		int32_t					Version;
	};
	
	struct Vertex
	{
		std::array<float, 3>	Position;
		std::array<float, 2>	TexCoord;
		LightMapVec2s			LMCoord;
		std::array<float, 3>	Normal;
		LightMapColor4s		    Color;
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

		LightMapU8s				LightMapStyles;
		LightMapU8s				VertexStyles;

		LightMapU32s			LightMapIDs;
		LightMapU32s			LMX; // "The face's lightmap corner in the image"
		LightMapU32s			LMY;
		uint32_t				LMWidth; // Size of the lightmap section
		uint32_t				LMHeight;
		
		Vec3					LMOrigin; // 3D origin of lightmap (???)
		std::array<Vec3, 3>		LMVecs; // 3D space for s and t unit vectors (???)
		
		Bounds2D				BezierDimensions;
	};
	
	struct Texture
	{
		std::array<char, 64>	Name;
		uint32_t				Flags; // Apparently unused?
		uint32_t				TextureFlags; // See eTextureFlags
	};
	
	struct LightMap
	{
		LightMapData			Data;
	};
	
	struct Node
	{
		uint32_t				PlaneIndex;
		uint32_t				FrontIndex; // front node index
		uint32_t				BackIndex;
		Bounds3D				Mins;
		Bounds3D				Maxs;
	};
	
	struct Leaf
	{
		uint32_t				VisibilityCluster;
		uint32_t				AreaPortal;
		Bounds3D				Mins;
		Bounds3D				Maxs;
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
        int32_t                 DrawSurfIndex;
	};
	
	struct Fog
	{
		char					TextureName[64]; 
		int32_t					BrushIndex;
		int32_t					VisibleSide; // -1 for none
	};
	
	struct LightVolume
	{
		LightMapColor3s			Ambient;
		LightMapColor3s			Directional;
        LightMapU8s             Styles;
		std::array<uint8_t, 2>	Direction; // phi, theta.
	};
	
	struct Model
	{
		Vec3					Mins;
		Vec3					Maxs;
		uint32_t				FirstFaceIndex;
		uint32_t				NumFaces;
		uint32_t				FirstBrushIndex;
		uint32_t				NumBrushes;
	};

	struct Plane
	{
		float X, Y, Z, W;
	};



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

	uint32_t			Format;
	
	static BSP*	
	Create(
		const void* lpByteBlob,
		size_t cbSize );
	
	~BSP();
	
	static float				
	GetLightMapGamma( );

	static void
	Tessellate(
		Face& f,
		TVertexList& vertices,
		TIndexList& indices,
		int numSubdivions);

	static int
	Tessellate(
		const Vertex* controlPoints,
		TVertexList& vertices,
		TIndexList& indices,
		int numSubdivisions,
		int indexOffset);
	
protected:

	BSP();
	bool				Load( const uint8_t* lpBytes, size_t cbSize );
	void				Unload();
};

// Utilities for interpolating vertices
BSP::Vertex operator + (const BSP::Vertex& v1, const BSP::Vertex& v2);
BSP::Vertex operator * (const BSP::Vertex& v1, const float& d);

#endif
