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

	class BS_CORE_EXPORT SkyboxRTTI : public RTTIType <Skybox, IReflectable, SkyboxRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFL(mTexture, 0)
			BS_RTTI_MEMBER_PLAIN(mBrightness, 1)
			BS_RTTI_MEMBER_REFLPTR(mFilteredRadiance, 2)
			BS_RTTI_MEMBER_REFLPTR(mIrradiance, 3)
		BS_END_RTTI_MEMBERS
	public:
		void OnSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			Skybox* skybox = static_cast<Skybox*>(obj);

			// Make sure that the renderer finishes generating filtered radiance and irradiance before saving
			if (skybox->mRendererTask)
				skybox->mRendererTask->wait();
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			// Note: Since this is a CoreObject I should call initialize() right after deserialization,
			// but since this specific type is used in Components we delay initialization until Component
			// itself does it. Keep this is mind in case this ever needs to be deserialized for non-Component
			// purposes (you'll need to call initialize manually).
		}

		const String& GetRTTIName() override
		{
			static String name = "Skybox";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_Skybox;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return Skybox::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
}
