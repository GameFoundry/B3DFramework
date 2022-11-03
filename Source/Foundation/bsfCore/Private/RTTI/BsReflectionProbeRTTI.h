//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Renderer/BsReflectionProbe.h"
#include "Renderer/BsRenderer.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_CORE_EXPORT ReflectionProbeRTTI : public RTTIType<ReflectionProbe, IReflectable, ReflectionProbeRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFL(mTransform, 0)
			B3D_RTTI_MEMBER_PLAIN(mActive, 1)
			B3D_RTTI_MEMBER_PLAIN(mMobility, 2)
			B3D_RTTI_MEMBER_PLAIN(mType, 3)
			B3D_RTTI_MEMBER_PLAIN(mRadius, 4)
			B3D_RTTI_MEMBER_PLAIN(mExtents, 5)
			B3D_RTTI_MEMBER_PLAIN(mTransitionDistance, 6)
			B3D_RTTI_MEMBER_REFL(mCustomTexture, 7)
			B3D_RTTI_MEMBER_REFLPTR(mFilteredTexture, 8)
		B3D_RTTI_END_MEMBERS
	public:
		void OnSerializationStarted(IReflectable* obj, SerializationContext* context)
		{
			ReflectionProbe* probe = static_cast<ReflectionProbe*>(obj);

			// Force the renderer task to complete, so the filtered texture is up to date
			if(probe->mRendererTask != nullptr)
				probe->mRendererTask->Wait();
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context)
		{
			// Note: Since this is a CoreObject I should call Initialize() right after deserialization,
			// but since this specific type is used in Components we delay initialization until Component
			// itself does it. Keep this is mind in case this ever needs to be deserialized for non-Component
			// purposes (you'll need to call initialize manually).
		}

		const String& GetRttiName()
		{
			static String name = "ReflectionProbe";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_ReflectionProbe;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return ReflectionProbe::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
