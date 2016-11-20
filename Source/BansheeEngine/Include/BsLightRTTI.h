//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsRTTIType.h"
#include "BsLight.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_EXPORT LightRTTI : public RTTIType <Light, IReflectable, LightRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mPosition, 0)
			BS_RTTI_MEMBER_PLAIN(mRotation, 1)
			BS_RTTI_MEMBER_PLAIN(mType, 2)
			BS_RTTI_MEMBER_PLAIN(mCastsShadows, 3)
			BS_RTTI_MEMBER_PLAIN(mColor, 4)
			BS_RTTI_MEMBER_PLAIN(mRange, 5)
			BS_RTTI_MEMBER_PLAIN(mIntensity, 6)
			BS_RTTI_MEMBER_PLAIN(mSpotAngle, 7)
			BS_RTTI_MEMBER_PLAIN(mSpotFalloffAngle, 8)
			BS_RTTI_MEMBER_PLAIN(mPhysCorrectAtten, 9)
		BS_END_RTTI_MEMBERS
	public:
		LightRTTI()
			:mInitMembers(this)
		{ }

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
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