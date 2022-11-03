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

	B3D_ALLOW_MEMCPY_SERIALIZATION(RASTERIZER_STATE_DESC);

	class B3D_CORE_EXPORT RasterizerStateRTTI : public RTTIType<RasterizerState, IReflectable, RasterizerStateRTTI>
	{
	private:
		RASTERIZER_STATE_DESC& GetData(RasterizerState* obj) { return obj->mProperties.mData; }

		void SetData(RasterizerState* obj, RASTERIZER_STATE_DESC& val) { obj->mProperties.mData = val; }

	public:
		RasterizerStateRTTI()
		{
			AddPlainField("mData", 0, &RasterizerStateRTTI::GetData, &RasterizerStateRTTI::SetData);
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			RasterizerState* rasterizerState = static_cast<RasterizerState*>(obj);
			rasterizerState->Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "RasterizerState";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_RasterizerState;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return RenderStateManager::Instance().CreateRasterizerStatePtrInternal(RASTERIZER_STATE_DESC());
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
