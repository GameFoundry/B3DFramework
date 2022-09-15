//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "RenderAPI/BsBlendState.h"
#include "Managers/BsRenderStateManager.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT BlendStateRTTI : public RTTIType<BlendState, IReflectable, BlendStateRTTI>
	{
	private:
		BLEND_STATE_DESC& GetData(BlendState* obj) { return obj->mProperties.mData; }
		void SetData(BlendState* obj, BLEND_STATE_DESC& val) { obj->mProperties.mData = val; }

	public:
		BlendStateRTTI()
		{
			AddPlainField("mData", 0, &BlendStateRTTI::GetData, &BlendStateRTTI::SetData);
		}

		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			BlendState* blendState = static_cast<BlendState*>(obj);
			blendState->Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "BlendState";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_BlendState;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return RenderStateManager::Instance().CreateBlendStatePtrInternal(BLEND_STATE_DESC());
		}
	};

	template<> struct RTTIPlainType<BLEND_STATE_DESC>
	{
		enum { id = TID_BLEND_STATE_DESC }; enum { hasDynamicSize = 1 };

		static BitLength ToMemory(const BLEND_STATE_DESC& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				return stream.WriteBytes(data);
			});
		}

		static BitLength FromMemory(BLEND_STATE_DESC& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);
			stream.ReadBytes(data);

			return size;
		}

		static BitLength GetSize(const BLEND_STATE_DESC& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(data);
			rtti_add_header_size(dataSize, compress);

			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
