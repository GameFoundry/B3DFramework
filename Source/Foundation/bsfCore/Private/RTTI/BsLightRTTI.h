//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsMathRTTI.h"
#include "RTTI/BsColorRTTI.h"
#include "Reflection/BsRTTIPlain.h"
#include "Renderer/BsLight.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_CORE_EXPORT LightRTTI : public RTTIType<Light, IReflectable, LightRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mTransform, 0)
			B3D_RTTI_MEMBER(mActive, 1)
			B3D_RTTI_MEMBER(mMobility, 2)
			B3D_RTTI_MEMBER(mType, 3)
			B3D_RTTI_MEMBER(mCastsShadows, 4)
			B3D_RTTI_MEMBER(mColor, 5)
			B3D_RTTI_MEMBER(mAttRadius, 6)
			B3D_RTTI_MEMBER(mIntensity, 7)
			B3D_RTTI_MEMBER(mSpotAngle, 8)
			B3D_RTTI_MEMBER(mSpotFalloffAngle, 9)
			B3D_RTTI_MEMBER(mAutoAttenuation, 10)
			B3D_RTTI_MEMBER(mSourceRadius, 11)
			B3D_RTTI_MEMBER(mShadowBias, 12)
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

		SPtr<IReflectable> NewRttiObject()
		{
			return Light::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
