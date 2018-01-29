//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "GUI/BsProfilerOverlay.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_EXPORT ProfilerOverlayRTTI : public RTTIType <CProfilerOverlay, Component, ProfilerOverlayRTTI>
	{
	private:

	public:
		ProfilerOverlayRTTI()
		{ }

		const String& getRTTIName() override
		{
			static String name = "ProfilerOverlay";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ProfilerOverlay;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<CProfilerOverlay>();
		}
	};

	/** @} */
	/** @endcond */
}