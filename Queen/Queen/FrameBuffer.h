#ifndef FrameBuffer_h__
#define FrameBuffer_h__

#include "Prerequisite.h"
#include "GraphicCommon.h"
#include "PixelFormat.h"
#include "MathLib/Matrix.hpp"
#include "MathLib/ColorRGBA.hpp"

using MathLib::float44;
using MathLib::ColorRGBA;

struct PS_Output;
struct BlendState;

#define MaxRenderTarget 8

class FrameBuffer
{
public:
	FrameBuffer(int32_t width, int32_t height);
	virtual ~FrameBuffer(void);

	uint32_t GetWidth() const			{ return mWidth; }
	uint32_t GetHeight() const			{ return mHeight; }

	PixelFormat GetColorFormat() const		{ return mColorFormat; }
	const Viewport&	GetViewport() const		{ return mViewport; }

	uint32_t GetColorDepth() const		{ return mColorDepth; }
	uint32_t GetDepthBits() const		{ return mDepthBits; }
	uint32_t GetStencilBits() const		{ return mStencilBits;}

	const shared_ptr<Texture2D>& GetRenderTarget(Attachment att) const;

	const float44& GetViewportMatrix() const { return mViewportMatrix; }

	bool IsDepthBuffered() const			{ return mIsDepthBuffered; }
	bool IsDirty() const					{ return mDirty; }

	void Attach(Attachment att, const shared_ptr<Texture2D>& renderTarget);
	void Detach(Attachment att);
	void DetachAll();

	//Called when this frame buffer is binded
	void OnBind();
	void OnUnbind();

	void Clear(uint32_t flags, const ColorRGBA& clr, float depth, uint32_t stencil);

private:
	void WritePixel(int32_t x, int32_t y, const PS_Output* psOutput, float* depth);
	void ReadPixel(int32_t x, int32_t y, PS_Output* oPixel, float* oDepth);
	void ClearColor(uint32_t index, PixelFormat fmt, const ColorRGBA& clr, std::atomic<uint32_t>& workingPackage, uint32_t width, uint32_t height);


protected:

	PixelFormat mColorFormat;
	bool mIsDepthBuffered;
	uint32_t mDepthBits, mStencilBits;
	uint32_t mColorDepth;

	int32_t mWidth, mHeight;
	Viewport mViewport;

	float44 mViewportMatrix;

	std::vector< shared_ptr<Texture2D> > mRenderTargets;
	shared_ptr<Texture2D> mDepthStencilTarget;

	bool mOffscreen;
	bool mDirty;
	bool mActice;

	//keep every render target buffer address, avoid to call the virtual function multiple times
	void* mRTBuffer[MaxRenderTarget];
	uint32_t mRTBufferPitch[MaxRenderTarget];

	friend class Rasterizer;
};

#endif // FrameBuffer_h__