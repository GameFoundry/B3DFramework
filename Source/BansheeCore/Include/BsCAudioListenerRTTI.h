//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsCAudioListener.h"
#include "BsGameObjectRTTI.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT CAudioListenerRTTI : public RTTIType<CAudioListener, Component, CAudioListenerRTTI>
	{
	public:
		CAudioListenerRTTI()
		{ }

		const String& getRTTIName() override
		{
			static String name = "CAudioListener";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CAudioListener;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<CAudioListener>();
		}
	};

	/** @} */
	/** @endcond */
}