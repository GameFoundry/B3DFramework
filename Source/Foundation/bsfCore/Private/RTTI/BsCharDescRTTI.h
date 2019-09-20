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

	template<> struct RTTIPlainType<CharDesc>
	{
		enum { id = TID_CHAR_DESC }; enum { hasDynamicSize = 1 };

		static BitLength toMemory(const CharDesc& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			return rtti_write_with_size_header(stream, compress, [&data, &stream]()
			{
				BitLength size = 0;
				size += rtti_write(data.charId, stream);
				size += rtti_write(data.page, stream);
				size += rtti_write(data.uvX, stream);
				size += rtti_write(data.uvY, stream);
				size += rtti_write(data.uvWidth, stream);
				size += rtti_write(data.uvHeight, stream);
				size += rtti_write(data.width, stream);
				size += rtti_write(data.height, stream);
				size += rtti_write(data.xOffset, stream);
				size += rtti_write(data.yOffset, stream);
				size += rtti_write(data.xAdvance, stream);
				size += rtti_write(data.yAdvance, stream);
				size += rtti_write(data.kerningPairs, stream);

				return size;
			});
		}

		static BitLength fromMemory(CharDesc& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo, bool compress)
		{
			BitLength size;
			rtti_read_size_header(stream, compress, size);
			rtti_read(data.charId, stream);
			rtti_read(data.page, stream);
			rtti_read(data.uvX, stream);
			rtti_read(data.uvY, stream);
			rtti_read(data.uvWidth, stream);
			rtti_read(data.uvHeight, stream);
			rtti_read(data.width, stream);
			rtti_read(data.height, stream);
			rtti_read(data.xOffset, stream);
			rtti_read(data.yOffset, stream);
			rtti_read(data.xAdvance, stream);
			rtti_read(data.yAdvance, stream);
			rtti_read(data.kerningPairs, stream);

			return size;
		}

		static BitLength getSize(const CharDesc& data, bool compress)
		{
			uint64_t dataSize = sizeof(data.charId)
				+ sizeof(data.page)
				+ sizeof(data.uvX)
				+ sizeof(data.uvY)
				+ sizeof(data.uvWidth)
				+ sizeof(data.uvHeight)
				+ sizeof(data.width)
				+ sizeof(data.height)
				+ sizeof(data.xOffset)
				+ sizeof(data.yOffset)
				+ sizeof(data.xAdvance)
				+ sizeof(data.yAdvance)
				+ RTTIPlainType<Vector<KerningPair>>::getSize(data.kerningPairs);

			dataSize += sizeof(uint32_t);

			return (uint32_t)dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
