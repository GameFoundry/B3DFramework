//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DRTTIPlain.h"
#include "String/B3DStringID.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	template <>
	struct RTTIPlainType<StringID>
	{
		enum
		{
			id = TID_StringID
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const StringID& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;

				bool isEmpty = data.Empty();
				size += B3DRTTIWrite(isEmpty, stream);

				if (!isEmpty)
				{
					auto length = (uint32_t)strlen(data.CStr());
					size += stream.WriteBytes((uint8_t*)data.CStr(), length * sizeof(char));
				}

				return size; });
		}

		static BitLength FromMemory(StringID& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			bool empty = false;
			B3DRTTIRead(empty, stream);

			if(!empty)
			{
				u32 length = (size.Bytes - sizeof(u32) - sizeof(bool)) / sizeof(char);

				auto name = (uint8_t*)B3DStackAllocate(length + 1);
				stream.ReadBytes(name, length);
				name[length] = '\0';

				data = StringID((char*)name);
				B3DStackFree(name);
			}

			return size;
		}

		static BitLength GetSize(const StringID& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(bool);

			bool isEmpty = data.Empty();
			if(!isEmpty)
			{
				auto length = (uint32_t)strlen(data.CStr());
				dataSize += length * sizeof(char);
			}

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
