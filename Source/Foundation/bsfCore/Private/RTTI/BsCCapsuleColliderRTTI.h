//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Components/BsCCapsuleCollider.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT CCapsuleColliderRTTI : public RTTIType<CCapsuleCollider, CCollider, CCapsuleColliderRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mNormal, 0)
			BS_RTTI_MEMBER_PLAIN(mRadius, 1)
			BS_RTTI_MEMBER_PLAIN(mHalfHeight, 2)
		BS_END_RTTI_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "CCapsuleCollider";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_CCapsuleCollider;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<CCapsuleCollider>();
		}
	};

	/** @} */
	/** @endcond */
}
