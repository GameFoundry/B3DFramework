//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Utility/B3DDataBlob.h"

namespace b3d
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
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   { return stream.WriteBytes(data.Data, data.Size); });
		}

		static BitLength FromMemory(DataBlob& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

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

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
