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
			B3D_RTTI_MEMBER_REFL(mTransform, 0)
			B3D_RTTI_MEMBER_PLAIN(mActive, 1)
			B3D_RTTI_MEMBER_PLAIN(mMobility, 2)
			B3D_RTTI_MEMBER_PLAIN(mType, 3)
			B3D_RTTI_MEMBER_PLAIN(mCastsShadows, 4)
			B3D_RTTI_MEMBER_PLAIN(mColor, 5)
			B3D_RTTI_MEMBER_PLAIN(mAttRadius, 6)
			B3D_RTTI_MEMBER_PLAIN(mIntensity, 7)
			B3D_RTTI_MEMBER_PLAIN(mSpotAngle, 8)
			B3D_RTTI_MEMBER_PLAIN(mSpotFalloffAngle, 9)
			B3D_RTTI_MEMBER_PLAIN(mAutoAttenuation, 10)
			B3D_RTTI_MEMBER_PLAIN(mSourceRadius, 11)
			B3D_RTTI_MEMBER_PLAIN(mShadowBias, 12)
		B3D_RTTI_END_MEMBERS
	public:
		void OnDeserializationEnded(IReflectable* obj, RTTIOperationContext* context) override
		{
			// Note: Since this is a CoreObject I should call Initialize() right after deserialization,
			// but since this specific type is used in Components we delay initialization until Component
			// itself does it. Keep this is mind in case this ever needs to be deserialized for non-Component
			// purposes (you'll need to call initialize manually).
		}

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
