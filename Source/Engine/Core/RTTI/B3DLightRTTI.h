//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Components/B3DLight.h"
#include "RTTI/B3DGameObjectRTTI.h"
#include "RTTI/B3DMathRTTI.h"
#include "RTTI/B3DColorRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT LightRTTI : public TRTTIType<Light, Component, LightRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mType, 0)
			B3D_RTTI_MEMBER(mCastsShadows, 1)
			B3D_RTTI_MEMBER(mColor, 2)
			B3D_RTTI_MEMBER(mAttRadius, 3)
			B3D_RTTI_MEMBER(mIntensity, 4)
			B3D_RTTI_MEMBER(mSpotAngle, 5)
			B3D_RTTI_MEMBER(mSpotFalloffAngle, 6)
			B3D_RTTI_MEMBER(mAutoAttenuation, 7)
			B3D_RTTI_MEMBER(mSourceRadius, 8)
			B3D_RTTI_MEMBER(mShadowBias, 9)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "Light";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Light;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<Light>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
