//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Utility/B3DTArray.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	template <class T>
	struct RTTIPlainType<TArray<T>>
	{
		enum
		{
			id = TID_TArray
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const TArray<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
		    {
				BitLength size = 0;

				auto numElements = (uint32_t)data.Size();
				size += B3DRTTIWrite(numElements, stream);

				for (const auto& item : data)
					size += B3DRTTIWrite(item, stream);

				return size; });
		}

		static BitLength FromMemory(TArray<T>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t numElements;
			B3DRTTIRead(numElements, stream);

			data.Clear();
			for(uint32_t i = 0; i < numElements; i++)
			{
				T element;
				B3DRTTIRead(element, stream);

				data.Add(element);
			}

			return size;
		}

		static BitLength GetSize(const TArray<T>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);

			for(const auto& item : data)
				dataSize += B3DRTTISize(item);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	template <class T, u32 N>
	struct RTTIPlainType<TInlineArray<T, N>>
	{
		enum
		{
			id = TID_TInlineArray
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const TInlineArray<T, N>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;

				auto numElements = (uint32_t)data.Size();
				size += B3DRTTIWrite(numElements, stream);

				for (const auto& item : data)
					size += B3DRTTIWrite(item, stream);

				return size; });
		}

		static BitLength FromMemory(TInlineArray<T, N>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			uint32_t numElements;
			B3DRTTIRead(numElements, stream);

			data.Clear();
			for(uint32_t i = 0; i < numElements; i++)
			{
				T element;
				B3DRTTIRead(element, stream);

				data.Add(element);
			}

			return size;
		}

		static BitLength GetSize(const TInlineArray<T, N>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = sizeof(uint32_t);

			for(const auto& item : data)
				dataSize += B3DRTTISize(item);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
