//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullRenderWindowSurface.h"
#include "Image/B3DPixelData.h"

namespace b3d::render
{
	NullRenderWindowSurface::NullRenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation)
		: mWidth(createInformation.Width), mHeight(createInformation.Height)
	{
	}

	NullRenderWindowSurface::~NullRenderWindowSurface()
	{
		// No-op destructor
	}

	void NullRenderWindowSurface::RebuildSwapChain(u32 width, u32 height, bool vsync)
	{
		// No-op implementation
	}

	void NullRenderWindowSurface::MarkSwapChainAsInvalid()
	{
		// No-op implementation
	}

	TAsyncOp<SPtr<PixelData>> NullRenderWindowSurface::ReadAsync(GpuCommandBuffer& commandBuffer)
	{
		SPtr<PixelData> pixelData = PixelData::Create(mWidth, mHeight, 1, PF_RGBA8);

		TAsyncOp<SPtr<PixelData>> op;
		op.CompleteOperation(pixelData);

		return op;
	}

	void NullRenderWindowSurface::Destroy()
	{
		// No-op implementation
	}
} // namespace b3d::render
