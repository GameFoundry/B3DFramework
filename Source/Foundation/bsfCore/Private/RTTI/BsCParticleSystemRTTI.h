//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Components/BsCParticleSystem.h"
#include "Private/RTTI/BsGameObjectRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT CParticleSystemRTTI : public RTTIType<CParticleSystem, Component, CParticleSystemRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mSettings, 0)
			B3D_RTTI_MEMBER_CONTAINER(mEmitters, 1)
			B3D_RTTI_MEMBER_CONTAINER(mEvolvers, 2)
			B3D_RTTI_MEMBER(mGpuSimulationSettings, 3)
			B3D_RTTI_MEMBER(mLayer, 4)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "CParticleSystem";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_CParticleSystem;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return SceneObject::CreateEmptyComponent<CParticleSystem>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
