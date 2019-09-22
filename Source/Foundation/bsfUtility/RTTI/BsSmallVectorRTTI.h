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
		static BitLength toMemory(const SmallVector<T, N>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, data, compress, [&data, &stream]()
			{
				BitLength size = 0;

				auto numElements = (uint32_t)data.size();
				size += rtti_write(numElements, stream);

				for (const auto& item : data)
					size += rtti_write(item, stream);

				return size;
			});
		}

		/** @copydoc RTTIPlainType::fromMemory */
		static BitLength fromMemory(SmallVector<T, N>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);

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

		/** @copydoc RTTIPlainType::getSize */
		static BitLength getSize(const SmallVector<T, N>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);

			for (const auto& item : data)
				dataSize += rtti_size(item);

			rtti_add_header_size(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
