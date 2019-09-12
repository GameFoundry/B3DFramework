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
		BLEND_STATE_DESC& getData(BlendState* obj) { return obj->mProperties.mData; }
		void setData(BlendState* obj, BLEND_STATE_DESC& val) { obj->mProperties.mData = val; }

	public:
		BlendStateRTTI()
		{
			addPlainField("mData", 0, &BlendStateRTTI::getData, &BlendStateRTTI::setData);
		}

		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			BlendState* blendState = static_cast<BlendState*>(obj);
			blendState->initialize();
		}

		const String& getRTTIName() override
		{
			static String name = "BlendState";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_BlendState;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return RenderStateManager::instance()._createBlendStatePtr(BLEND_STATE_DESC());
		}
	};

	template<> struct RTTIPlainType<BLEND_STATE_DESC>
	{
		enum { id = TID_BLEND_STATE_DESC }; enum { hasDynamicSize = 1 };

		static uint32_t toMemory(const BLEND_STATE_DESC& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				return stream.writeBytes(data);
			});
		}

		static uint32_t fromMemory(BLEND_STATE_DESC& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			uint32_t size;
			rtti_read(size, stream);
			stream.readBytes(data);

			return size;
		}

		static uint32_t getSize(const BLEND_STATE_DESC& data)
		{
			uint64_t dataSize = sizeof(data) + sizeof(uint32_t);

#if BS_DEBUG_MODE
			if (dataSize > std::numeric_limits<uint32_t>::max())
			{
				BS_EXCEPT(InternalErrorException, "Data overflow! Size doesn't fit into 32 bits.");
			}
#endif

			return (uint32_t)dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
