//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "RenderAPI/BsRasterizerState.h"
#include "Managers/BsRenderStateManager.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	BS_ALLOW_MEMCPY_SERIALIZATION(RASTERIZER_STATE_DESC);

	class BS_CORE_EXPORT RasterizerStateRTTI : public RTTIType<RasterizerState, IReflectable, RasterizerStateRTTI>
	{
	private:
		RASTERIZER_STATE_DESC& GetData(RasterizerState* obj) { return obj->mProperties.mData; }
		void SetData(RasterizerState* obj, RASTERIZER_STATE_DESC& val) { obj->mProperties.mData = val; }

	public:
		RasterizerStateRTTI()
		{
			addPlainField("mData", 0, &RasterizerStateRTTI::getData, &RasterizerStateRTTI::setData);
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			RasterizerState* rasterizerState = static_cast<RasterizerState*>(obj);
			rasterizerState->initialize();
		}

		const String& GetRTTIName() override
		{
			static String name = "RasterizerState";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_RasterizerState;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return RenderStateManager::Instance()._createRasterizerStatePtr(RASTERIZER_STATE_DESC());
		}
	};

	/** @} */
	/** @endcond */
}
