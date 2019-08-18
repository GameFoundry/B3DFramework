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

		static void toMemory(const StringID& data, char* memory)
		{
			UINT32 size = getDynamicSize(data);

			UINT32 curSize = sizeof(UINT32);
			memcpy(memory, &size, curSize);
			memory += curSize;

			bool isEmpty = data.empty();
			memory = rttiWriteElem(isEmpty, memory);

			if (!isEmpty)
			{
				UINT32 length = (UINT32)strlen(data.c_str());
				memcpy(memory, data.c_str(), length * sizeof(char));
			}
		}

		static UINT32 fromMemory(StringID& data, char* memory)
		{
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			bool empty = false;
			memory = rttiReadElem(empty, memory);

			if (!empty)
			{
				UINT32 length = (size - sizeof(UINT32) - sizeof(bool)) / sizeof(char);

				auto name = (char*)bs_stack_alloc(length + 1);
				memcpy(name, memory, length);
				name[length] = '\0';

				data = StringID(name);
				bs_stack_free(name);
			}

			return size;
		}

		static UINT32 getDynamicSize(const StringID& data)
		{
			UINT32 dataSize = sizeof(bool) + sizeof(UINT32);

			bool isEmpty = data.empty();
			if (!isEmpty)
			{
				UINT32 length = (UINT32)strlen(data.c_str());
				dataSize += length * sizeof(char);
			}

			return (UINT32)dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
