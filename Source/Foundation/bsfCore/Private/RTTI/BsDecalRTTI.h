//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Renderer/BsDecal.h"
#include "Renderer/BsRenderer.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_CORE_EXPORT DecalRTTI : public RTTIType <Decal, IReflectable, DecalRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFL(mTransform, 0)
			BS_RTTI_MEMBER_PLAIN(mActive, 1)
			BS_RTTI_MEMBER_PLAIN(mMobility, 2)
			BS_RTTI_MEMBER_PLAIN(mSize, 3)
			BS_RTTI_MEMBER_PLAIN(mMaxDistance, 4)
			BS_RTTI_MEMBER_REFL(mMaterial, 5)
			BS_RTTI_MEMBER_PLAIN(mLayer, 6)
			BS_RTTI_MEMBER_PLAIN(mLayerMask, 7)
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
			static String name = "Decal";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_Decal;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return Decal::createEmpty();
		}
	};

	/** @} */
	/** @endcond */
}
