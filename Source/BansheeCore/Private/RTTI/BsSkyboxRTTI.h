//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
		SkyboxRTTI()
			:mInitMembers(this)
		{ }

		void onSerializationStarted(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			Skybox* skybox = static_cast<Skybox*>(obj);

			// Make sure that the renderer finishes generating filtered radiance and irradiance before saving
			if (skybox->mRendererTask)
				skybox->mRendererTask->wait();
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			// Note: Since this is a CoreObject I should call initialize() right after deserialization,
			// but since this specific type is used in Components we delay initialization until Component
			// itself does it. Keep this is mind in case this ever needs to be deserialized for non-Component 
			// purposes (you'll need to call initialize manually).
		}

		const String& getRTTIName() override
		{
			static String name = "Skybox";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_Skybox;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return Skybox::createEmpty();
		}
	};

	/** @} */
	/** @endcond */
}
