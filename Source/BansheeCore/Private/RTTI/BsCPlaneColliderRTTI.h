//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Components/BsCPlaneCollider.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT CPlaneColliderRTTI : public RTTIType<CPlaneCollider, CCollider, CPlaneColliderRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mNormal, 0)
			BS_RTTI_MEMBER_PLAIN(mDistance, 1)
		BS_END_RTTI_MEMBERS
	public:
		CPlaneColliderRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "CPlaneCollider";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CPlaneCollider;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<CPlaneCollider>();
		}
	};

	/** @} */
	/** @endcond */
}