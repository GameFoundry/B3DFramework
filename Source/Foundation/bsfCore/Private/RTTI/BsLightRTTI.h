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

	class BS_CORE_EXPORT LightRTTI : public RTTIType <Light, IReflectable, LightRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFL(mTransform, 0)
			BS_RTTI_MEMBER_PLAIN(mActive, 1)
			BS_RTTI_MEMBER_PLAIN(mMobility, 2)
			BS_RTTI_MEMBER_PLAIN(mType, 3)
			BS_RTTI_MEMBER_PLAIN(mCastsShadows, 4)
			BS_RTTI_MEMBER_PLAIN(mColor, 5)
			BS_RTTI_MEMBER_PLAIN(mAttRadius, 6)
			BS_RTTI_MEMBER_PLAIN(mIntensity, 7)
			BS_RTTI_MEMBER_PLAIN(mSpotAngle, 8)
			BS_RTTI_MEMBER_PLAIN(mSpotFalloffAngle, 9)
			BS_RTTI_MEMBER_PLAIN(mAutoAttenuation, 10)
			BS_RTTI_MEMBER_PLAIN(mSourceRadius, 11)
			BS_RTTI_MEMBER_PLAIN(mShadowBias, 12)
		BS_END_RTTI_MEMBERS
	public:
		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			// Note: Since this is a CoreObject I should call initialize() right after deserialization,
			// but since this specific type is used in Components we delay initialization until Component
			// itself does it. Keep this is mind in case this ever needs to be deserialized for non-Component
			// purposes (you'll need to call initialize manually).
		}

		const String& getRTTIName() override
		{
			static String name = "Light";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_Light;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return Light::createEmpty();
		}
	};

	/** @} */
	/** @endcond */
}
