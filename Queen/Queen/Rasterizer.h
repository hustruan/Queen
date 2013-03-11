#ifndef Rasterizer_h__
#define Rasterizer_h__

#include "GraphicCommon.h"
#include "RenderStage.h"
#include "Shader.h"
#include "Profiler.h"

// primitive count per package used in set up geometry
#define SetupGeometryPackageSize 64

// primitive count per package used in binning
#define BinningPackageSize 64

#define RasterizeTilePackageSize 16

// vertex cache size
#define VertexCacheSize 16

#define TileSize 64
#define TileSizeShift 6

class Rasterizer : public RenderStage
{
public:
	typedef std::function<const VS_Output&(uint32_t)> VertexFetchFunc;
	typedef std::function<uint32_t(uint32_t)> IndexFetchFunc;

	struct Tile
	{
		uint32_t X, Y;
		uint32_t Width, Height;

		vector<vector<uint32_t> > TriQueue;
		vector<uint32_t> TriQueueSize;
	};

	struct RasterFaceInfo
	{
		bool FrontFace;
		uint32_t TriCount;
		uint32_t Indices[6];
	};
	
	struct RasterFace
	{
		VS_Output* V[3];
		VS_Output ddxVarying, ddyVarying;

		// fixed point position
		int32_t X[3], Y[3];

		// half space constant
		int32_t C1, C2, C3;

		int32_t MinX, MinY, MaxX, MaxY;

	};

	struct ThreadPackage
	{
		uint32_t Start, End;
	};

	struct VertexCacheElement
	{
		uint32_t Index;
		VS_Output Vertex;
	};

public:
	Rasterizer(RenderDevice& device);
	~Rasterizer(void);

	void Draw(PrimitiveType primitiveType, uint32_t primitiveCount);
	void DrawTiled(PrimitiveType primitiveType, uint32_t primitiveCount);
	void PreDraw();
	void PostDraw();

	void OnBindFrameBuffer(const shared_ptr<FrameBuffer>& fb);

private:

	void ProjectVertex(VS_Output* vertex);

	const VS_Output& FetchVertex(uint32_t index, uint32_t threadIdx);

	uint32_t ClipTriangle(VS_Output* clipped, const VS_Output& v0, const VS_Output& v1, const VS_Output& v2);
	
	void RasterizeTriangle(const VS_Output& vsOut0, const VS_Output& vsOut1, const VS_Output& vsOut2);
	
	void RasterizeScanline(int32_t xStart, int32_t xEnd, int32_t Y, VS_Output* baseVertex, const VS_Output* ddx);
 
	bool BackFaceCulling(const VS_Output& v0, const VS_Output& v1, const VS_Output& v2, bool* oriented = nullptr);

	void SetupGeometry(std::vector<VS_Output>& outVertices, std::vector<RasterFaceInfo>& outFaces, 
		std::atomic<uint32_t>& workPackage, uint32_t primitiveCount);
	
	void RasterizeFaces(std::vector<RasterFaceInfo>& faces, std::atomic<uint32_t>& workPackage, uint32_t faceCount);

	void ClipTriangleTiled(VS_Output* vertices, uint32_t threadIdx);

	void SetupGeometryTiled(std::vector<VS_Output>& outVertices, std::vector<RasterFace>& outFaces, uint32_t theadIdx, ThreadPackage package);

	void Binning(const VS_Output& V0, const VS_Output& V1, const VS_Output& V2, uint32_t threadIdx);

	void RasterizeTiles(std::vector<uint32_t>& tilesQueue, std::atomic<uint32_t>& workingPackage, uint32_t numTiles);

	// the whole tile is inside an triagnle
	void DrawPartialTile(const RasterFace& face, int32_t tileX, int32_t tileY, int32_t tileWidth, int32_t tileHeight);

	void DrawPixels(const RasterFace& face, int32_t xStart, int32_t yStart, int32_t xEnd, int32_t yEnd);

	void DrawMaskedPixels(const RasterFace& face, int32_t mask, int32_t xStart, int32_t xEnd, int32_t iY);

	void DrawPixel(uint32_t iX, uint32_t iY, const VS_Output& vsOutput);
	

private:

	// dispatch primitive count to each thread
	std::vector<ThreadPackage> mThreadPackage;

	// each thread keep a local clipped vertex buffer
	std::vector< std::vector<VS_Output> > mVerticesThreads;	
	std::vector<uint32_t> mNumVerticesThreads;

	// each thread keep a local clipped faces buffer
	std::vector< std::vector<RasterFace> > mFacesThreads;		

	// each thread keep a vertex cache
	std::vector< std::array<VertexCacheElement, VertexCacheSize> > mVertexCaches;

	// non-empty tile job queue
	std::vector<uint32_t> mTilesQueue;
	uint32_t mTilesQueueSize;

	int32_t mNumTileX, mNumTileY;
	std::vector<Tile> mTiles;
	
	std::vector<VS_Output> mClippedVertices;
	std::vector<RasterFaceInfo> mClippedFaces;

	std::array<float4, 2> mClipPlanes;

	// current vertex shader output register count, set every draw
	uint32_t mCurrVSOutputCount;

	// set every draw
	shared_ptr<FrameBuffer> mCurrFrameBuffer;

	Profiler mProfiler;
};


#endif // Rasterizer_h__


