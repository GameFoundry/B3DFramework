//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsCBoxCollider.h"
#include "BsGameObjectRTTI.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT CBoxColliderRTTI : public RTTIType<CBoxCollider, CCollider, CBoxColliderRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mExtents, 0)
		BS_END_RTTI_MEMBERS
	public:
		CBoxColliderRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "CBoxCollider";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CBoxCollider;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<CBoxCollider>();
		}
	};

	/** @} */
	/** @endcond */
}