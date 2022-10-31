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

	template <>
	struct RTTIPlainType<DataBlob>
	{
		enum
		{
			id = TID_DataBlob
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const DataBlob& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
											   { return stream.WriteBytes(data.Data, data.Size); });
		}

		static BitLength FromMemory(DataBlob& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			if(data.Data != nullptr)
				B3DFree(data.Data);

			data.Size = size.Bytes - sizeof(uint32_t);
			data.Data = (uint8_t*)B3DAllocate(data.Size);

			stream.ReadBytes(data.Data, data.Size);

			return size;
		}

		static BitLength GetSize(const DataBlob& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = data.Size;

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
