//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsBlendState.h"
#include "BsRenderStateManager.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	template<> struct RTTIPlainType<BLEND_STATE_DESC>
	{	
		enum { id = TID_BLEND_STATE_DESC }; enum { hasDynamicSize = 1 };

		static void toMemory(const BLEND_STATE_DESC& data, char* memory)
		{ 
			UINT32 size = getDynamicSize(data);

			memcpy(memory, &size, sizeof(UINT32));
			memory += sizeof(UINT32);
			size -= sizeof(UINT32);
			memcpy(memory, &data, size); 
		}

		static UINT32 fromMemory(BLEND_STATE_DESC& data, char* memory)
		{ 
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32)); 
			memory += sizeof(UINT32);

			UINT32 dataSize = size - sizeof(UINT32);
			memcpy((void*)&data, memory, dataSize); 

			return size;
		}

		static UINT32 getDynamicSize(const BLEND_STATE_DESC& data)	
		{ 
			UINT64 dataSize = sizeof(data) + sizeof(UINT32);

#if BS_DEBUG_MODE
			if(dataSize > std::numeric_limits<UINT32>::max())
			{
				BS_EXCEPT(InternalErrorException, "Data overflow! Size doesn't fit into 32 bits.");
			}
#endif

			return (UINT32)dataSize;
		}	
	}; 

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

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
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

	/** @} */
	/** @endcond */
}