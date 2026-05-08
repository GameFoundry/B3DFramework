//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DMathRTTI.h"
#include "Components/B3DAnimation.h"
#include "RTTI/B3DGameObjectRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT AnimationRTTI : public TRTTIType<Animation, Component, AnimationRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mDefaultClip, 0)
			B3D_RTTI_MEMBER(mWrapMode, 1)
			B3D_RTTI_MEMBER(mSpeed, 2)
			B3D_RTTI_MEMBER(mEnableCull, 3)
			B3D_RTTI_MEMBER(mUseCustomBounds, 4)
			B3D_RTTI_MEMBER(mCustomBounds, 5)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "Animation";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Animation;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return SceneObject::CreateEmptyComponent<Animation>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
