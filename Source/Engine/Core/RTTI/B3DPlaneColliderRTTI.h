//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Components/B3DPlaneCollider.h"
#include "RTTI/B3DGameObjectRTTI.h"
#include "RTTI/B3DMathRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT PlaneColliderRTTI : public TRTTIType<PlaneCollider, Collider, PlaneColliderRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mNormal, 0)
			B3D_RTTI_MEMBER(mDistance, 1)
			B3D_RTTI_MEMBER(mShapeLocalPosition, 2)
			B3D_RTTI_MEMBER(mShapeLocalRotation, 3)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "PlaneCollider";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_PlaneCollider;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<PlaneCollider>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
