//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Components/B3DCapsuleCollider.h"
#include "RTTI/B3DGameObjectRTTI.h"
#include "RTTI/B3DMathRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT CapsuleColliderRTTI : public TRTTIType<CapsuleCollider, Collider, CapsuleColliderRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mNormal, 0)
			B3D_RTTI_MEMBER(mRadius, 1)
			B3D_RTTI_MEMBER(mHalfHeight, 2)
			B3D_RTTI_MEMBER(mShapeLocalPosition, 3)
			B3D_RTTI_MEMBER(mShapeLocalRotation, 4)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "CapsuleCollider";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_CapsuleCollider;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<CapsuleCollider>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
