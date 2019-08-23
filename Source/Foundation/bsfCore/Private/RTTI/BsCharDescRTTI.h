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

		static uint32_t toMemory(const CharDesc& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			return rtti_write_with_size_header(stream, [&data, &stream]()
			{
				uint32_t size = 0;
				size += rttiWriteElem(data.charId, stream);
				size += rttiWriteElem(data.page, stream);
				size += rttiWriteElem(data.uvX, stream);
				size += rttiWriteElem(data.uvY, stream);
				size += rttiWriteElem(data.uvWidth, stream);
				size += rttiWriteElem(data.uvHeight, stream);
				size += rttiWriteElem(data.width, stream);
				size += rttiWriteElem(data.height, stream);
				size += rttiWriteElem(data.xOffset, stream);
				size += rttiWriteElem(data.yOffset, stream);
				size += rttiWriteElem(data.xAdvance, stream);
				size += rttiWriteElem(data.yAdvance, stream);
				size += rttiWriteElem(data.kerningPairs, stream);

				return size;
			});
		}

		static uint32_t fromMemory(CharDesc& data, Bitstream& stream, const RTTIFieldInfo& fieldInfo)
		{
			uint32_t size;
			rttiReadElem(size, stream);
			rttiReadElem(data.charId, stream);
			rttiReadElem(data.page, stream);
			rttiReadElem(data.uvX, stream);
			rttiReadElem(data.uvY, stream);
			rttiReadElem(data.uvWidth, stream);
			rttiReadElem(data.uvHeight, stream);
			rttiReadElem(data.width, stream);
			rttiReadElem(data.height, stream);
			rttiReadElem(data.xOffset, stream);
			rttiReadElem(data.yOffset, stream);
			rttiReadElem(data.xAdvance, stream);
			rttiReadElem(data.yAdvance, stream);
			rttiReadElem(data.kerningPairs, stream);

			return size;
		}

		static uint32_t getDynamicSize(const CharDesc& data)
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
				+ RTTIPlainType<Vector<KerningPair>>::getDynamicSize(data.kerningPairs);

			dataSize += sizeof(uint32_t);

			return (uint32_t)dataSize;
		}
	};

	/** @} */
	/** @endcond */
}
