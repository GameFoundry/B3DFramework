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
		SAMPLER_STATE_DESC& GetData(SamplerState* obj) { return obj->mProperties.mData; }
		void SetData(SamplerState* obj, SAMPLER_STATE_DESC& val) { obj->mProperties.mData = val; }

	public:
		SamplerStateRTTI()
		{
			addPlainField("mData", 0, &SamplerStateRTTI::getData, &SamplerStateRTTI::setData);
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			SamplerState* samplerState = static_cast<SamplerState*>(obj);
			samplerState->initialize();
		}

		const String& GetRTTIName() override
		{
			static String name = "SamplerState";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_SamplerState;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return RenderStateManager::Instance()._createSamplerStatePtr(SAMPLER_STATE_DESC());
		}
	};

	/** @} */
	/** @endcond */
}
