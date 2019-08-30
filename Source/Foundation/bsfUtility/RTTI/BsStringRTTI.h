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

		static uint32_t toMemory(const String& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			uint32_t stringBytes = (uint32_t)(data.size() * sizeof(String::value_type));
			uint32_t size = stringBytes + sizeof(uint32_t);

			stream.writeBytes(size);
			stream.writeBytes((uint8_t*)data.data(), stringBytes);

			return size;
		}

		static uint32_t fromMemory(String& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			uint32_t size;
			stream.readBytes(size);

			uint32_t stringSize = size - sizeof(size);
			uint8_t* buffer = (uint8_t*)bs_stack_alloc(stringSize + 1);

			stream.readBytes(buffer, stringSize);
			buffer[stringSize] = '\0';
			data = String((String::value_type*)buffer);

			bs_stack_free(buffer);
			return size;
		}

		static uint32_t getDynamicSize(const String& data)
		{
			uint64_t dataSize = data.size() * sizeof(String::value_type) + sizeof(uint32_t);

#if BS_DEBUG_MODE
			if (dataSize > std::numeric_limits<uint32_t>::max())
			{
				__string_throwDataOverflowException();
			}
#endif

			return (uint32_t)dataSize;
		}
	};
	
	template<> struct RTTIPlainType<WString>
	{
		enum { id = TID_WString }; enum { hasDynamicSize = 1 };

		static uint32_t toMemory(const WString& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			uint32_t stringBytes = (uint32_t)(data.size() * sizeof(WString::value_type));
			uint32_t size = stringBytes + sizeof(uint32_t);
			
			stream.writeBytes(size);
			stream.writeBytes((uint8_t*)data.data(), stringBytes);

			return size;
		}

		static uint32_t fromMemory(WString& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			uint32_t size;
			stream.readBytes(size);

			uint32_t stringSize = size - sizeof(size);
			auto buffer = (WString::value_type*)bs_stack_alloc(stringSize + sizeof(WString::value_type));

			stream.readBytes((uint8_t*)buffer, stringSize);

			UINT32 numChars = stringSize / sizeof(WString::value_type);
			buffer[numChars] = L'\0';
			data = WString((WString::value_type*)buffer);

			bs_stack_free(buffer);
			return size;
		}

		static uint32_t getDynamicSize(const WString& data)
		{
			uint64_t dataSize = data.size() * sizeof(WString::value_type) + sizeof(uint32_t);

#if BS_DEBUG_MODE
			if (dataSize > std::numeric_limits<uint32_t>::max())
			{
				__string_throwDataOverflowException();
			}
#endif

			return (uint32_t)dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
