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

	class B3D_CORE_EXPORT CCapsuleColliderRTTI : public RTTIType<CCapsuleCollider, CCollider, CCapsuleColliderRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(mNormal, 0)
			B3D_RTTI_MEMBER_PLAIN(mRadius, 1)
			B3D_RTTI_MEMBER_PLAIN(mHalfHeight, 2)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "CCapsuleCollider";
			return name;
		}

		u32 GetRttiId() override
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
} // namespace bs
