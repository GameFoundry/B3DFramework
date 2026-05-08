//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Components/B3DSkybox.h"
#include "RTTI/B3DGameObjectRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT SkyboxRTTI : public TRTTIType<Skybox, Component, SkyboxRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mTexture, 0)
			B3D_RTTI_MEMBER(mBrightness, 1)
			B3D_RTTI_MEMBER(mFilteredRadiance, 2)
			B3D_RTTI_MEMBER(mIrradiance, 3)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "Skybox";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Skybox;
		}

		TShared<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<Skybox>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
