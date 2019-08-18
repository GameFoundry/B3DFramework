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

		static void toMemory(const String& data, char* memory)
		{
			UINT32 size = getDynamicSize(data);

			memcpy(memory, &size, sizeof(UINT32));
			memory += sizeof(UINT32);
			size -= sizeof(UINT32);
			memcpy(memory, data.data(), size);
		}

		static UINT32 fromMemory(String& data, char* memory)
		{
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			UINT32 stringSize = size - sizeof(UINT32);
			char* buffer = (char*)bs_alloc(stringSize + 1);
			memcpy(buffer, memory, stringSize);
			buffer[stringSize] = '\0';
			data = String(buffer);

			bs_free(buffer);

			return size;
		}

		static UINT32 getDynamicSize(const String& data)
		{
			UINT64 dataSize = data.size() * sizeof(String::value_type) + sizeof(UINT32);

#if BS_DEBUG_MODE
			if (dataSize > std::numeric_limits<UINT32>::max())
			{
				__string_throwDataOverflowException();
			}
#endif

			return (UINT32)dataSize;
		}
	};

	template<> struct RTTIPlainType<WString>
	{
		enum { id = TID_WString }; enum { hasDynamicSize = 1 };

		static void toMemory(const WString& data, char* memory)
		{
			UINT32 size = getDynamicSize(data);

			memcpy(memory, &size, sizeof(UINT32));
			memory += sizeof(UINT32);
			size -= sizeof(UINT32);
			memcpy(memory, data.data(), size);
		}

		static UINT32 fromMemory(WString& data, char* memory)
		{
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			UINT32 stringSize = size - sizeof(UINT32);
			WString::value_type* buffer = (WString::value_type*)bs_alloc(stringSize + sizeof(WString::value_type));
			memcpy(buffer, memory, stringSize);

			UINT32 numChars = stringSize / sizeof(WString::value_type);
			buffer[numChars] = L'\0';

			data = WString(buffer);

			bs_free(buffer);

			return size;
		}

		static UINT32 getDynamicSize(const WString& data)
		{
			UINT64 dataSize = data.size() * sizeof(WString::value_type) + sizeof(UINT32);

#if BS_DEBUG_MODE
			if (dataSize > std::numeric_limits<UINT32>::max())
			{
				__string_throwDataOverflowException();
			}
#endif

			return (UINT32)dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
