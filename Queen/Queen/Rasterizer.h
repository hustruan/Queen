#ifndef Rasterizer_h__
#define Rasterizer_h__

#include "GraphicCommon.h"
#include "RenderStage.h"
#include "Shader.h"

#define NumPrimitivePerPackage 64
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

public:
	Rasterizer(RenderDevice& device);
	~Rasterizer(void);

	void Draw(PrimitiveType primitiveType, uint32_t primitiveCount);
	void PreDraw();
	void PostDraw();

private:

	uint32_t Clip(VS_Output* clipped, const VS_Output& v0, const VS_Output& v1, const VS_Output& v2);
	
	void RasterizeTriangle(const VS_Output& vsOut0, const VS_Output& vsOut1, const VS_Output& vsOut2);

	void RasterizeTriangle_Top(const VS_Output& vsOut0, const VS_Output& vsOut1, const VS_Output& vsOut2);
	
	void RasterizeTriangle_Bottom(const VS_Output& vsOut0, const VS_Output& vsOut1, const VS_Output& vsOut2);
	
	void RasterizeScanline(int32_t xStart, int32_t xEnd, int32_t Y, VS_Output* baseVertex, const VS_Output* ddx);

	bool BackFaceCulling(const VS_Output& v0, const VS_Output& v1, const VS_Output& v2);

	void SetupGeometry(std::vector<VS_Output>& outVertices, std::vector<RasterFaceInfo>& outFaces, 
		std::atomic<uint32_t>& workPackage, uint32_t primitiveCount);

	void ProjectVertex(VS_Output* vertex);

private:

	std::vector<VS_Output> mClippedVertices;
	std::vector<RasterFaceInfo> mClippedFaces;

	std::array<float4, 2> mClipPlanes;

	// current vertex shader output register count, set every draw
	uint32_t mCurrVSOutputCount;
};


#endif // Rasterizer_h__


