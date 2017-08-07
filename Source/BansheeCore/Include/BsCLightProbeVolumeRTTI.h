//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsCLightProbeVolume.h"
#include "BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_CORE_EXPORT CLightProbeVolumeRTTI : public RTTIType <CLightProbeVolume, Component, CLightProbeVolumeRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFLPTR(mInternal, 0)
		BS_END_RTTI_MEMBERS

	public:
		CLightProbeVolumeRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "CLightProbeVolume";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CLightProbeVolume;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<CLightProbeVolume>();
		}
	};

	/** @} */
	/** @endcond */
}