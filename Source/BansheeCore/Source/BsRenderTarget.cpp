//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderTarget.h"
#include "BsViewport.h"
#include "BsException.h"
#include "BsRenderAPI.h"
#include "BsCoreThread.h"
#include "BsFrameAlloc.h"

namespace bs
{
	RenderTargetCore::RenderTargetCore()
	{

	}

	void RenderTargetCore::setPriority(INT32 priority) 
	{ 
		RenderTargetProperties& props = const_cast<RenderTargetProperties&>(getProperties());

		props.mPriority = priority;
	}

	const RenderTargetProperties& RenderTargetCore::getProperties() const
	{ 
		return getPropertiesInternal(); 
	}

	void RenderTargetCore::getCustomAttribute(const String& name, void* pData) const
	{
		BS_EXCEPT(InvalidParametersException, "Attribute not found.");
	}

	RenderTarget::RenderTarget()
	{
		// We never sync from sim to core, so mark it clean to avoid overwriting core thread changes
		markCoreClean();
	}

	void RenderTarget::setPriority(INT32 priority)
	{
		std::function<void(SPtr<RenderTargetCore>, INT32)> windowedFunc =
			[](SPtr<RenderTargetCore> renderTarget, INT32 priority)
		{
			renderTarget->setPriority(priority);
		};

		gCoreThread().queueCommand(std::bind(windowedFunc, getCore(), priority));
	}

	SPtr<RenderTargetCore> RenderTarget::getCore() const
	{
		return std::static_pointer_cast<RenderTargetCore>(mCoreSpecific);
	}

	const RenderTargetProperties& RenderTarget::getProperties() const
	{
		THROW_IF_CORE_THREAD;

		return getPropertiesInternal();
	}

	void RenderTarget::getCustomAttribute(const String& name, void* pData) const
	{
		BS_EXCEPT(InvalidParametersException, "Attribute not found.");
	}

}        
