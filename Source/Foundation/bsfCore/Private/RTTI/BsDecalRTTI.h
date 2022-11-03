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

	class B3D_CORE_EXPORT DecalRTTI : public RTTIType<Decal, IReflectable, DecalRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFL(mTransform, 0)
			B3D_RTTI_MEMBER_PLAIN(mActive, 1)
			B3D_RTTI_MEMBER_PLAIN(mMobility, 2)
			B3D_RTTI_MEMBER_PLAIN(mSize, 3)
			B3D_RTTI_MEMBER_PLAIN(mMaxDistance, 4)
			B3D_RTTI_MEMBER_REFL(mMaterial, 5)
			B3D_RTTI_MEMBER_PLAIN(mLayer, 6)
			B3D_RTTI_MEMBER_PLAIN(mLayerMask, 7)
		B3D_RTTI_END_MEMBERS
	public:
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context)
		{
			// Note: Since this is a CoreObject I should call Initialize() right after deserialization,
			// but since this specific type is used in Components we delay initialization until Component
			// itself does it. Keep this is mind in case this ever needs to be deserialized for non-Component
			// purposes (you'll need to call initialize manually).
		}

		const String& GetRttiName() override
		{
			static String name = "Decal";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_Decal;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return Decal::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
