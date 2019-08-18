//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "RenderAPI/BsSamplerState.h"
#include "Managers/BsRenderStateManager.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	BS_ALLOW_MEMCPY_SERIALIZATION(SAMPLER_STATE_DESC);

	class BS_CORE_EXPORT SamplerStateRTTI : public RTTIType<SamplerState, IReflectable, SamplerStateRTTI>
	{
	private:
		SAMPLER_STATE_DESC& getData(SamplerState* obj) { return obj->mProperties.mData; }
		void setData(SamplerState* obj, SAMPLER_STATE_DESC& val) { obj->mProperties.mData = val; }

	public:
		SamplerStateRTTI()
		{
			addPlainField("mData", 0, &SamplerStateRTTI::getData, &SamplerStateRTTI::setData);
		}

		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			SamplerState* samplerState = static_cast<SamplerState*>(obj);
			samplerState->initialize();
		}

		const String& getRTTIName() override
		{
			static String name = "SamplerState";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SamplerState;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return RenderStateManager::instance()._createSamplerStatePtr(SAMPLER_STATE_DESC());
		}
	};

	/** @} */
	/** @endcond */
}
