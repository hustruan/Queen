#ifndef Rasterizer_h__
#define Rasterizer_h__

#include "GraphicCommon.h"
#include "RenderStage.h"
#include "Shader.h"

// primitive count per package used in set up geometry
#define SetupGeometryPackageSize 64

// primitive count per package used in binning
#define BinningPackageSize 64

// vertex cache size
#define VertexCacheSize 16

class Rasterizer : public RenderStage
{
public:
	typedef std::function<const VS_Output&(uint32_t)> VertexFetchFunc;
	typedef std::function<uint32_t(uint32_t)> IndexFetchFunc;


	struct RasterFaceInfo
	{
		bool FrontFace;
		uint32_t TriCount;
		uint32_t Indices[6];
	};
	
	struct RasterFace
	{
		VS_Output* V[3];
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
	void Draw2(PrimitiveType primitiveType, uint32_t primitiveCount);
	void PreDraw();
	void PostDraw();

private:

	void ClipTriangle(VS_Output* vertices, uint32_t threadIdx);

	uint32_t Clip(VS_Output* clipped, const VS_Output& v0, const VS_Output& v1, const VS_Output& v2);
	
	void RasterizeTriangle(const VS_Output& vsOut0, const VS_Output& vsOut1, const VS_Output& vsOut2);

	void RasterizeTriangle_Top(const VS_Output& vsOut0, const VS_Output& vsOut1, const VS_Output& vsOut2);
	
	void RasterizeTriangle_Bottom(const VS_Output& vsOut0, const VS_Output& vsOut1, const VS_Output& vsOut2);
	
	void RasterizeScanline(int32_t xStart, int32_t xEnd, int32_t Y, VS_Output* baseVertex, const VS_Output* ddx);

	bool BackFaceCulling(const VS_Output& v0, const VS_Output& v1, const VS_Output& v2);

	void SetupGeometry(std::vector<VS_Output>& outVertices, std::vector<RasterFaceInfo>& outFaces, 
		std::atomic<uint32_t>& workPackage, uint32_t primitiveCount);

	void SetupGeometry2(std::vector<VS_Output>& outVertices, std::vector<RasterFace>& outFaces, uint32_t theadIdx, ThreadPackage package);

	void Binning(std::atomic<uint32_t>& workPackage, uint32_t primitiveCount);

	void Bin(const VS_Output& V0, const VS_Output& V1, const VS_Output& V2, uint32_t threadIdx);

	void ProjectVertex(VS_Output* vertex);

	const VS_Output& FetchVertex(uint32_t index, uint32_t threadIdx);

private:

	// dispatch primitive count to each thread
	std::vector<ThreadPackage> mThreadPackage;

	// each thread keep a local clipped vertex buffer
	std::vector< std::vector<VS_Output> > mVerticesThreads;		

	// each thread keep a local clipped faces buffer
	std::vector< std::vector<RasterFace> > mFacesThreads;		

	// each thread keep a vertex cache
	std::vector< std::array<VertexCacheElement, VertexCacheSize> > mVertexCaches;

	std::vector< uint32_t > mNumVerticesThreads;


	std::vector<VS_Output> mClippedVertices;
	std::vector<RasterFaceInfo> mClippedFaces;

	std::array<float4, 2> mClipPlanes;

	// current vertex shader output register count, set every draw
	uint32_t mCurrVSOutputCount;
};


#endif // Rasterizer_h__


