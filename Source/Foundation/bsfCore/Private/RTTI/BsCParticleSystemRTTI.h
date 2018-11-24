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

	class BS_CORE_EXPORT CParticleSystemRTTI : public RTTIType<CParticleSystem, Component, CParticleSystemRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFL(mSettings, 0)
			BS_RTTI_MEMBER_REFLPTR_ARRAY(mEmitters, 1)
			BS_RTTI_MEMBER_REFLPTR_ARRAY(mEvolvers, 2)
			BS_RTTI_MEMBER_REFL(mGpuSimulationSettings, 3)
			BS_RTTI_MEMBER_PLAIN(mLayer, 4)
		BS_END_RTTI_MEMBERS

	public:
		const String& getRTTIName() override
		{
			static String name = "CParticleSystem";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CParticleSystem;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<CParticleSystem>();
		}
	};

	/** @} */
	/** @endcond */
}
