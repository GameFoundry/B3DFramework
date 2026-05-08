//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Components/B3DCollider.h"
#include "RTTI/B3DMathRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT ColliderRTTI : public TRTTIType<Collider, Component, ColliderRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mLayer, 0)
			B3D_RTTI_MEMBER(mRestOffset, 1)
			B3D_RTTI_MEMBER(mContactOffset, 2)
			B3D_RTTI_MEMBER(mMaterial, 3)
			B3D_RTTI_MEMBER(mMass, 4)
			B3D_RTTI_MEMBER(mIsTrigger, 5)
			B3D_RTTI_MEMBER(mCollisionReportMode, 8)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName()
		{
			static String name = "Collider";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Collider;
		}

		TShared<IReflectable> NewRttiObject()
		{
			B3D_ASSERT(false && "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
