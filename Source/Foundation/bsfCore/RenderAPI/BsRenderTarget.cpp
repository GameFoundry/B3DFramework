//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsRenderTarget.h"
#include "RenderAPI/BsViewport.h"
#include "Private/RTTI/BsRenderTargetRTTI.h"
#include "Error/BsException.h"
#include "RenderAPI/BsRenderAPI.h"
#include "CoreThread/BsCoreThread.h"

using namespace bs;

RenderTarget::RenderTarget()
{
	// We never sync from sim to core, so mark it clean to avoid overwriting core thread changes
	MarkCoreClean();
}

void RenderTarget::SetPriority(i32 priority)
{
	std::function<void(SPtr<ct::RenderTarget>, i32)> windowedFunc =
		[](SPtr<ct::RenderTarget> renderTarget, i32 priority)
	{
		renderTarget->SetPriority(priority);
	};

	gCoreThread().QueueCommand(std::bind(windowedFunc, GetCore(), priority));
}

SPtr<ct::RenderTarget> RenderTarget::GetCore() const
{
	return std::static_pointer_cast<ct::RenderTarget>(mCoreSpecific);
}

const RenderTargetProperties& RenderTarget::GetProperties() const
{
	THROW_IF_CORE_THREAD;

	return GetPropertiesInternal();
}

void RenderTarget::GetCustomAttribute(const String& name, void* pData) const
{
	BS_EXCEPT(InvalidParametersException, "Attribute not found.");
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
	BS_EXCEPT(InvalidParametersException, "Attribute not found.");
}
}}
