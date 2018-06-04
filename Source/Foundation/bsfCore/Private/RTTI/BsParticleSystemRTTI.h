//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Particles/BsParticleSystem.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT ParticleSystemRTTI : public RTTIType<ParticleSystem, IReflectable, ParticleSystemRTTI>
	{
	private:

	public:
		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			// Note: Since this is a CoreObject I should call initialize() right after deserialization,
			// but since this specific type is used in Components we delay initialization until Component
			// itself does it. Keep this is mind in case this ever needs to be deserialized for non-Component
			// purposes (you'll need to call initialize manually).
		}

		const String& getRTTIName() override
		{
			static String name = "ParticleSystem";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ParticleSystem;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return ParticleSystem::createEmpty();
		}
	};

	/** @} */
	/** @endcond */
}
