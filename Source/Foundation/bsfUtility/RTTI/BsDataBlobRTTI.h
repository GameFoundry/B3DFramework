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

		static BitLength toMemory(const DataBlob& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, compress, [&data, &stream]()
			{
				return stream.writeBytes(data.data, data.size);
			});
		}

		static BitLength fromMemory(DataBlob& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			if (data.data != nullptr)
				bs_free(data.data);

			data.size = size.bytes - sizeof(uint32_t);
			data.data = (uint8_t*)bs_alloc(data.size);

			stream.readBytes(data.data, data.size);

			return size;
		}

		static BitLength getSize(const DataBlob& data, bool compress)
		{
			uint64_t dataSize = data.size + sizeof(uint32_t);

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
