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
			B3D_RTTI_MEMBER(mTexture, 0)
			B3D_RTTI_MEMBER(mBrightness, 1)
			B3D_RTTI_MEMBER(mFilteredRadiance, 2)
			B3D_RTTI_MEMBER(mIrradiance, 3)
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
