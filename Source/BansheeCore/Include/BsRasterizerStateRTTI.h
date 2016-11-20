//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsRasterizerState.h"
#include "BsRenderStateManager.h"

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

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
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