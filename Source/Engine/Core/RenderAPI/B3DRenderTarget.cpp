//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/B3DRenderTarget.h"
#include "RenderAPI/B3DViewport.h"
#include "Image/B3DTexture.h"
#include "RTTI/B3DRenderTargetRTTI.h"
#include "CoreObject/B3DRenderThread.h"

using namespace b3d;

RenderTarget::RenderTarget()
{
	// We never sync from main to render thread, so mark it clean to avoid overwriting render thread changes
	MarkRenderProxyDataUpToDate();
}

void RenderTarget::SetPriority(i32 priority)
{
	std::function<void(SPtr<render::RenderTarget>, i32)> windowedFunc =
		[](SPtr<render::RenderTarget> renderTarget, i32 priority)
	{
		renderTarget->SetPriority(priority);
	};

	SPtr<render::RenderTarget> renderProxy = B3DGetRenderProxy(this);
	GetRenderThread().PostCommand([windowedFunc, renderProxy, priority]() { windowedFunc(renderProxy, priority); }, "RenderTarget::SetPriority");
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/

RTTIType* RenderTarget::GetRttiStatic()
{
	return RenderTargetRTTI::Instance();
}

RTTIType* RenderTarget::GetRtti() const
{
	return RenderTarget::GetRttiStatic();
}

namespace b3d { namespace render
{
RenderTarget::RenderTarget()
{
}

void RenderTarget::SetPriority(i32 priority)
{
	mRenderTargetProperties.Priority = priority;
}

TAsyncOp<SPtr<PixelData>> RenderTarget::ReadAsync(GpuCommandBuffer& commandBuffer, u32 colorSurfaceIndex, u32 mipLevel, u32 arrayLayer)
{
	TAsyncOp<SPtr<PixelData>> asyncOp;
	asyncOp.CompleteOperation(nullptr);
	return asyncOp;
}
}}
