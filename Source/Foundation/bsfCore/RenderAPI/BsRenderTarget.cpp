//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsRenderTarget.h"
#include "RenderAPI/BsViewport.h"
#include "Private/RTTI/BsRenderTargetRTTI.h"
#include "Error/BsException.h"
#include "CoreObject/BsRenderThread.h"

using namespace bs;

RenderTarget::RenderTarget()
{
	// We never sync from main to render thread, so mark it clean to avoid overwriting render thread changes
	MarkRenderProxyDataUpToDate();
}

void RenderTarget::SetPriority(i32 priority)
{
	std::function<void(SPtr<ct::RenderTarget>, i32)> windowedFunc =
		[](SPtr<ct::RenderTarget> renderTarget, i32 priority)
	{
		renderTarget->SetPriority(priority);
	};

	GetRenderThread().PostCommand(std::bind(windowedFunc, B3DGetRenderProxy(this), priority));
}

const RenderTargetProperties& RenderTarget::GetProperties() const
{
	ASSERT_IF_RENDER_THREAD;

	return GetPropertiesInternal();
}

void RenderTarget::GetCustomAttribute(const String& name, void* pData) const
{
	B3D_EXCEPT(InvalidParametersException, "Attribute not found.");
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/

RTTITypeBase* RenderTarget::GetRttiStatic()
{
	return RenderTargetRTTI::Instance();
}

RTTITypeBase* RenderTarget::GetRtti() const
{
	return RenderTarget::GetRttiStatic();
}

namespace bs { namespace ct
{
RenderTarget::RenderTarget()
{
}

void RenderTarget::SetPriority(i32 priority)
{
	RenderTargetProperties& props = const_cast<RenderTargetProperties&>(GetProperties());

	props.Priority = priority;
}

const RenderTargetProperties& RenderTarget::GetProperties() const
{
	return GetPropertiesInternal();
}

void RenderTarget::GetCustomAttribute(const String& name, void* pData) const
{
	B3D_EXCEPT(InvalidParametersException, "Attribute not found.");
}
}}
