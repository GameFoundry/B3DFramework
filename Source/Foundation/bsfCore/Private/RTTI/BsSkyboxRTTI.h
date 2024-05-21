//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Renderer/BsSkybox.h"
#include "Renderer/BsRenderer.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_CORE_EXPORT SkyboxRTTI : public RTTIType<Skybox, IReflectable, SkyboxRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFL(mTexture, 0)
			B3D_RTTI_MEMBER_PLAIN(mBrightness, 1)
			B3D_RTTI_MEMBER_REFLPTR(mFilteredRadiance, 2)
			B3D_RTTI_MEMBER_REFLPTR(mIrradiance, 3)
		B3D_RTTI_END_MEMBERS
	public:
		void OnOperationStarted(Skybox& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				// Make sure that the renderer finishes generating filtered radiance and irradiance before saving
				if(object.mRendererTask)
					object.mRendererTask->Wait();
			}
		}

		void OnDeserializationEnded(IReflectable* obj, RTTIOperationContext* context)
		{
			// Note: Since this is a CoreObject I should call Initialize() right after deserialization,
			// but since this specific type is used in Components we delay initialization until Component
			// itself does it. Keep this is mind in case this ever needs to be deserialized for non-Component
			// purposes (you'll need to call initialize manually).
		}

		const String& GetRttiName()
		{
			static String name = "Skybox";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Skybox;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return Skybox::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
