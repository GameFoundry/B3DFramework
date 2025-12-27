//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Components/B3DAudioListener.h"
#include "RTTI/B3DGameObjectRTTI.h"
#include "RTTI/B3DMathRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT AudioListenerRTTI : public TRTTIType<AudioListener, Component, AudioListenerRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mVelocity, 0)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "AudioListener";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_AudioListener;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<AudioListener>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
