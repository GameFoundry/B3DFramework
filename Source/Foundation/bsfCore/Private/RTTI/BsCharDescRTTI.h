//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Text/BsFontDesc.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	template <>
	struct RTTIPlainType<CharDesc>
	{
		enum
		{
			id = TID_CHAR_DESC
		};

		enum
		{
			hasDynamicSize = 1
		};

		static BitLength ToMemory(const CharDesc& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return B3DRTTIWriteWithSizeHeader(stream, data, compress, [&data, &stream]()
											   {
				BitLength size = 0;
				size += B3DRTTIWrite(data.CharId, stream);
				size += B3DRTTIWrite(data.Page, stream);
				size += B3DRTTIWrite(data.UvX, stream);
				size += B3DRTTIWrite(data.UvY, stream);
				size += B3DRTTIWrite(data.UvWidth, stream);
				size += B3DRTTIWrite(data.UvHeight, stream);
				size += B3DRTTIWrite(data.Width, stream);
				size += B3DRTTIWrite(data.Height, stream);
				size += B3DRTTIWrite(data.XOffset, stream);
				size += B3DRTTIWrite(data.YOffset, stream);
				size += B3DRTTIWrite(data.XAdvance, stream);
				size += B3DRTTIWrite(data.YAdvance, stream);
				size += B3DRTTIWrite(data.KerningPairs, stream);

				return size; });
		}

		static BitLength FromMemory(CharDesc& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			B3DRTTIReadSizeHeader(stream, compress, size);
			B3DRTTIRead(data.CharId, stream);
			B3DRTTIRead(data.Page, stream);
			B3DRTTIRead(data.UvX, stream);
			B3DRTTIRead(data.UvY, stream);
			B3DRTTIRead(data.UvWidth, stream);
			B3DRTTIRead(data.UvHeight, stream);
			B3DRTTIRead(data.Width, stream);
			B3DRTTIRead(data.Height, stream);
			B3DRTTIRead(data.XOffset, stream);
			B3DRTTIRead(data.YOffset, stream);
			B3DRTTIRead(data.XAdvance, stream);
			B3DRTTIRead(data.YAdvance, stream);
			B3DRTTIRead(data.KerningPairs, stream);

			return size;
		}

		static BitLength GetSize(const CharDesc& data, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength dataSize = B3DRTTISize(data.CharId) + B3DRTTISize(data.Page) + B3DRTTISize(data.UvX) + B3DRTTISize(data.UvY) + B3DRTTISize(data.UvWidth) + B3DRTTISize(data.UvHeight) + B3DRTTISize(data.Width) + B3DRTTISize(data.Height) + B3DRTTISize(data.XOffset) + B3DRTTISize(data.YOffset) + B3DRTTISize(data.XAdvance) + B3DRTTISize(data.YAdvance) + B3DRTTISize(data.KerningPairs);

			B3DRTTIAddHeaderSize(dataSize, compress);
			return dataSize;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
