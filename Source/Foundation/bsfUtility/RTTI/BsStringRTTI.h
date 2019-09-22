//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIPlain.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	*  @{
	*/

	template<> struct RTTIPlainType<String>
	{
		enum { id = 20 }; enum { hasDynamicSize = 1 };

		static BitLength toMemory(const String& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				uint32_t size = (uint32_t)(data.size() * sizeof(String::value_type));
				stream.writeBytes((uint8_t*)data.data(), size);

				return size;
			});
		}

		static BitLength fromMemory(String& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t stringSize = size.bytes - sizeof(size.bytes);
			uint8_t* buffer = (uint8_t*)bs_stack_alloc(stringSize + 1);

			stream.readBytes(buffer, stringSize);
			buffer[stringSize] = '\0';
			data = String((String::value_type*)buffer);

			bs_stack_free(buffer);
			return size;
		}

		static BitLength getSize(const String& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = (uint32_t)(data.size() * sizeof(String::value_type));

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};
	
	template<> struct RTTIPlainType<WString>
	{
		enum { id = TID_WString }; enum { hasDynamicSize = 1 };

		static BitLength toMemory(const WString& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				uint32_t size = (uint32_t)(data.size() * sizeof(WString::value_type));
				stream.writeBytes((uint8_t*)data.data(), size);

				return size;
			});
		}

		static BitLength fromMemory(WString& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

			uint32_t stringSize = size.bytes - sizeof(size.bytes);
			auto buffer = (WString::value_type*)bs_stack_alloc(stringSize + sizeof(WString::value_type));

			stream.readBytes((uint8_t*)buffer, stringSize);

			UINT32 numChars = stringSize / sizeof(WString::value_type);
			buffer[numChars] = L'\0';
			data = WString((WString::value_type*)buffer);

			bs_stack_free(buffer);
			return size;
		}

		static BitLength getSize(const WString& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = (uint32_t)(data.size() * sizeof(WString::value_type));

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
