//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIPlain.h"
#include "Utility/BsSmallVector.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	*  @{
	*/

	template<class T, UINT32 N> struct RTTIPlainType<SmallVector<T, N>>
	{
		enum { id = TID_SmallVector }; enum { hasDynamicSize = 1 };

		/** @copydoc RTTIPlainType::toMemory */
		static void toMemory(const SmallVector<T, N>& data, char* memory)
		{
			UINT32 size = sizeof(UINT32);
			char* memoryStart = memory;
			memory += sizeof(UINT32);

			UINT32 numElements = (UINT32)data.size();
			memcpy(memory, &numElements, sizeof(UINT32));
			memory += sizeof(UINT32);
			size += sizeof(UINT32);

			for (const auto& item : data)
			{
				UINT32 elementSize = rttiGetElemSize(item);
				RTTIPlainType<T>::toMemory(item, memory);

				memory += elementSize;
				size += elementSize;
			}

			memcpy(memoryStart, &size, sizeof(UINT32));
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static UINT32 fromMemory(SmallVector<T, N>& data, char* memory)
		{
			UINT32 size = 0;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			UINT32 numElements;
			memcpy(&numElements, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			data.clear();
			for (UINT32 i = 0; i < numElements; i++)
			{
				T element;
				UINT32 elementSize = RTTIPlainType<T>::fromMemory(element, memory);
				data.add(element);

				memory += elementSize;
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static UINT32 getDynamicSize(const SmallVector<T, N>& data)
		{
			UINT64 dataSize = sizeof(UINT32) * 2;

			for (const auto& item : data)
				dataSize += rttiGetElemSize(item);

			assert(dataSize <= std::numeric_limits<UINT32>::max());

			return (UINT32)dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
