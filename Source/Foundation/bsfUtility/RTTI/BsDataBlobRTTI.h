//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIPlain.h"
#include "Utility/BsDataBlob.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	template<> struct RTTIPlainType<DataBlob>
	{
		enum { id = TID_DataBlob }; enum { hasDynamicSize = 1 };

		static void toMemory(const DataBlob& data, char* memory)
		{
			UINT32 size = getDynamicSize(data);

			memcpy(memory, &size, sizeof(UINT32));
			memory += sizeof(UINT32);

			memcpy(memory, data.data, data.size);
		}

		static UINT32 fromMemory(DataBlob& data, char* memory)
		{
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			if (data.data != nullptr)
				bs_free(data.data);

			data.size = size - sizeof(UINT32);
			data.data = (UINT8*)bs_alloc(data.size);
			memcpy(data.data, memory, data.size);

			return size;
		}

		static UINT32 getDynamicSize(const DataBlob& data)
		{
			UINT64 dataSize = data.size + sizeof(UINT32);

#if BS_DEBUG_MODE
			if (dataSize > std::numeric_limits<UINT32>::max())
			{
				BS_EXCEPT(InternalErrorException, "Data overflow! Size doesn't fit into 32 bits.");
			}
#endif

			return (UINT32)dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
