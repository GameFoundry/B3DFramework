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

	class B3D_CORE_EXPORT CColliderRTTI : public RTTIType<CCollider, Component, CColliderRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(mLayer, 0)
			B3D_RTTI_MEMBER_PLAIN(mRestOffset, 1)
			B3D_RTTI_MEMBER_PLAIN(mContactOffset, 2)
			B3D_RTTI_MEMBER_REFL(mMaterial, 3)
			B3D_RTTI_MEMBER_PLAIN(mMass, 4)
			B3D_RTTI_MEMBER_PLAIN(mIsTrigger, 5)
			B3D_RTTI_MEMBER_PLAIN(mLocalPosition, 6)
			B3D_RTTI_MEMBER_PLAIN(mLocalRotation, 7)
			B3D_RTTI_MEMBER_PLAIN(mCollisionReportMode, 8)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName()
		{
			static String name = "CCollider";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_CCollider;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			B3D_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
