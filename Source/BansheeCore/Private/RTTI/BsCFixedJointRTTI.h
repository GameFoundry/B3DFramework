//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Components/BsCFixedJoint.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT CFixedJointRTTI : public RTTIType<CFixedJoint, CJoint, CFixedJointRTTI>
	{
	public:
		CFixedJointRTTI()
		{ }

		const String& getRTTIName() override
		{
			static String name = "CFixedJoint";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CFixedJoint;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<CFixedJoint>();
		}
	};

	/** @} */
	/** @endcond */
}