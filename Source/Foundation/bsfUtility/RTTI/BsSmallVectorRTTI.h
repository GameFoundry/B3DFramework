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
		static uint32_t toMemory(const SmallVector<T, N>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				uint32_t size = 0;

				auto numElements = (uint32_t)data.size();
				size += rtti_write(numElements, stream);

				for (const auto& item : data)
					size += rtti_write(item, stream);

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static uint32_t fromMemory(SmallVector<T, N>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			uint32_t size = 0;
			rtti_read(size, stream);

			uint32_t numElements;
			rtti_read(numElements, stream);

			data.clear();
			for (uint32_t i = 0; i < numElements; i++)
			{
				T element;
				rtti_read(element, stream);

				data.add(element);
			}

			return size;
		}

		/** @copydoc RTTIPlainType::getDynamicSize */
		static uint32_t getDynamicSize(const SmallVector<T, N>& data)
		{
			uint64_t dataSize = sizeof(uint32_t) * 2;

			for (const auto& item : data)
				dataSize += rtti_size(item);

			assert(dataSize <= std::numeric_limits<uint32_t>::max());

			return (uint32_t)dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
