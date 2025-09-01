//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsMathRTTI.h"
#include "Components/BsAnimation.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_CORE_EXPORT AnimationRTTI : public TRTTIType<Animation, Component, AnimationRTTI>
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

		SPtr<IReflectable> NewRttiObject()
		{
			return SceneObject::CreateEmptyComponent<Animation>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
