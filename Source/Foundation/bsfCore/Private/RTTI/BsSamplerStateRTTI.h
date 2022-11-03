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

	B3D_ALLOW_MEMCPY_SERIALIZATION(SAMPLER_STATE_DESC);

	class B3D_CORE_EXPORT SamplerStateRTTI : public RTTIType<SamplerState, IReflectable, SamplerStateRTTI>
	{
	private:
		SAMPLER_STATE_DESC& GetData(SamplerState* obj) { return obj->mProperties.mData; }

		void SetData(SamplerState* obj, SAMPLER_STATE_DESC& val) { obj->mProperties.mData = val; }

	public:
		SamplerStateRTTI()
		{
			AddPlainField("mData", 0, &SamplerStateRTTI::GetData, &SamplerStateRTTI::SetData);
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context)
		{
			SamplerState* samplerState = static_cast<SamplerState*>(obj);
			samplerState->Initialize();
		}

		const String& GetRttiName()
		{
			static String name = "SamplerState";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_SamplerState;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return RenderStateManager::Instance().CreateSamplerStatePtrInternal(SAMPLER_STATE_DESC());
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
