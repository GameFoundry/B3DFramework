//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "RenderAPI/B3DRenderWindow.h"

namespace b3d::render
{
	/** @addtogroup Null
	 *  @{
	 */

	/** Null implementation of IRenderWindowSurface. */
	class NullRenderWindowSurface : public IRenderWindowSurface
	{
	public:
		NullRenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation);
		~NullRenderWindowSurface();

		void SwapBuffers(GpuQueue& queue, GpuQueueMask syncMask) override {}
		void RebuildSwapChain(u32 width, u32 height, bool vsync) override;
		void MarkSwapChainAsInvalid() override;
		TAsyncOp<SPtr<PixelData>> ReadAsync(GpuCommandBuffer& commandBuffer) override;
		void Destroy() override;

	private:
		u32 mWidth = 0;
		u32 mHeight = 0;
	};

	/** @} */
} // namespace b3d::render
