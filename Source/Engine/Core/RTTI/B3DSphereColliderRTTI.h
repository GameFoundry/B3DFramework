//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Components/B3DSphereCollider.h"
#include "RTTI/B3DGameObjectRTTI.h"
#include "RTTI/B3DMathRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT CSphereColliderRTTI : public TRTTIType<SphereCollider, Collider, CSphereColliderRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mRadius, 0)
			B3D_RTTI_MEMBER(mShapeLocalPosition, 1)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "CSphereCollider";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_SphereCollider;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<SphereCollider>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
