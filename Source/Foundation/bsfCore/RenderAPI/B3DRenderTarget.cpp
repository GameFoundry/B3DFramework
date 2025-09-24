//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsRenderTarget.h"
#include "RenderAPI/BsViewport.h"
#include "Private/RTTI/BsRenderTargetRTTI.h"
#include "Error/BsException.h"
#include "CoreObject/BsRenderThread.h"

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

	GetRenderThread().PostCommand(std::bind(windowedFunc, B3DGetRenderProxy(this), priority), "RenderTarget::SetPriority");
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
}}
