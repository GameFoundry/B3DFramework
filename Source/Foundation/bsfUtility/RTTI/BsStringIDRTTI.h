//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIPlain.h"
#include "String/BsStringID.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	template<> struct RTTIPlainType <StringID>
	{
		enum { id = TID_StringID }; enum { hasDynamicSize = 1 };

		static uint32_t toMemory(const StringID& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				uint32_t size = 0;

				bool isEmpty = data.empty();
				size += rtti_write(isEmpty, stream);

				if (!isEmpty)
				{
					auto length = (uint32_t)strlen(data.c_str());
					size += stream.writeBytes((uint8_t*)data.c_str(), length * sizeof(char));
				}

				return size;
			});
		}

		static uint32_t fromMemory(StringID& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			uint32_t size;
			rtti_read(size, stream);

			bool empty = false;
			rtti_read(empty, stream);

			if (!empty)
			{
				UINT32 length = (size - sizeof(UINT32) - sizeof(bool)) / sizeof(char);

				auto name = (uint8_t*)bs_stack_alloc(length + 1);
				stream.readBytes(name, length);
				name[length] = '\0';

				data = StringID((char*)name);
				bs_stack_free(name);
			}

			return size;
		}

		static uint32_t getDynamicSize(const StringID& data)
		{
			uint32_t dataSize = sizeof(bool) + sizeof(uint32_t);

			bool isEmpty = data.empty();
			if (!isEmpty)
			{
				auto length = (uint32_t)strlen(data.c_str());
				dataSize += length * sizeof(char);
			}

			return (uint32_t)dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
