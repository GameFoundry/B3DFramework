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
		RASTERIZER_STATE_DESC& getData(RasterizerState* obj) { return obj->mProperties.mData; }
		void setData(RasterizerState* obj, RASTERIZER_STATE_DESC& val) { obj->mProperties.mData = val; }

	public:
		RasterizerStateRTTI()
		{
			addPlainField("mData", 0, &RasterizerStateRTTI::getData, &RasterizerStateRTTI::setData);
		}

		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			RasterizerState* rasterizerState = static_cast<RasterizerState*>(obj);
			rasterizerState->initialize();
		}

		const String& getRTTIName() override
		{
			static String name = "RasterizerState";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_RasterizerState;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return RenderStateManager::instance()._createRasterizerStatePtr(RASTERIZER_STATE_DESC());
		}
	};

	/** @} */
	/** @endcond */
}
