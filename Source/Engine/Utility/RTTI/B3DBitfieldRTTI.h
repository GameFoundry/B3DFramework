//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Utility/B3DBitfield.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	template <class Allocator>
	struct RTTIPlainType<TBitfield<Allocator>>
	{
		enum
		{
			id = TID_Bitfield
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const TBitfield<Allocator>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
		    {
				BitLength size = 0;

				const u64 elementCount = (u64)data.Size();
				size += B3DRTTIWrite(elementCount, stream);

				const u32 dwordCount = (u32)Math::DivideAndRoundUp(elementCount, TBitfield<Allocator>::kBitsPerDword);
				size.Bytes += stream.WriteBytes((u8*)data.Data(), dwordCount * sizeof(u32));

				return size;
		    });
		}

		static BitLength FromMemory(TBitfield<Allocator>& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);

			u64 elementCount;
			B3DRTTIRead(elementCount, stream);

			const u32 dwordCount = (u32)Math::DivideAndRoundUp(elementCount, TBitfield<Allocator>::kBitsPerDword);

			data.Clear();
			data.Reserve(dwordCount * TBitfield<Allocator>::kBitsPerDword);
			data.Resize(elementCount);

			stream.ReadBytes((u8*)data.Data(), dwordCount * sizeof(u32));

			return size;
		}

		static BitLength GetSize(const TBitfield<Allocator>& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			const u32 dwordCount = (u32)Math::DivideAndRoundUp(data.Size(), TBitfield<Allocator>::kBitsPerDword);
			BitLength dataSize = sizeof(u64) + sizeof(u32) * dwordCount;

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
