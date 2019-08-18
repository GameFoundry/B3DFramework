//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Components/BsCCollider.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT CColliderRTTI : public RTTIType<CCollider, Component, CColliderRTTI>
	{
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mLayer, 0)
			BS_RTTI_MEMBER_PLAIN(mRestOffset, 1)
			BS_RTTI_MEMBER_PLAIN(mContactOffset, 2)
			BS_RTTI_MEMBER_REFL(mMaterial, 3)
			BS_RTTI_MEMBER_PLAIN(mMass, 4)
			BS_RTTI_MEMBER_PLAIN(mIsTrigger, 5)
			BS_RTTI_MEMBER_PLAIN(mLocalPosition, 6)
			BS_RTTI_MEMBER_PLAIN(mLocalRotation, 7)
			BS_RTTI_MEMBER_PLAIN(mCollisionReportMode, 8)
		BS_END_RTTI_MEMBERS
	public:
		const String& getRTTIName() override
		{
			static String name = "CCollider";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CCollider;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			BS_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
}
