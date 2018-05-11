//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup Text
	 *  @{
	 */

	/**	Kerning pair representing larger or smaller offset between a specific pair of characters. */
	struct BS_SCRIPT_EXPORT(pl:true,m:GUI_Engine) KerningPair
	{
		UINT32 otherCharId;
		INT32 amount;
	};

	/**	Describes a single character in a font of a specific size. */
	struct BS_SCRIPT_EXPORT(pl:true,m:GUI_Engine) CharDesc
	{
		UINT32 charId; /**< Character ID, corresponding to a Unicode key. */
		UINT32 page; /**< Index of the texture the character is located on. */
		float uvX, uvY; /**< Texture coordinates of the character in the page texture. */
		float uvWidth, uvHeight; /**< Width/height of the character in texture coordinates. */
		UINT32 width, height; /**< Width/height of the character in pixels. */
		INT32 xOffset, yOffset; /**< Offset for the visible portion of the character in pixels. */
		INT32 xAdvance, yAdvance; /**< Determines how much to advance the pen after writing this character, in pixels. */

		/**
		 * Pairs that determine if certain character pairs should be closer or father together. for example "AV"
		 * combination.
		 */
		Vector<KerningPair> kerningPairs;
	};

	/** @cond SPECIALIZATIONS */

	// Make CHAR_DESC serializable
	template<> struct RTTIPlainType<CharDesc>
	{
		enum { id = TID_CHAR_DESC }; enum { hasDynamicSize = 1 };

		static void toMemory(const CharDesc& data, char* memory)
		{
			UINT32 size = getDynamicSize(data);

			memcpy(memory, &size, sizeof(UINT32));
			memory += sizeof(UINT32);

			memory = rttiWriteElem(data.charId, memory);
			memory = rttiWriteElem(data.page, memory);
			memory = rttiWriteElem(data.uvX, memory);
			memory = rttiWriteElem(data.uvY, memory);
			memory = rttiWriteElem(data.uvWidth, memory);
			memory = rttiWriteElem(data.uvHeight, memory);
			memory = rttiWriteElem(data.width, memory);
			memory = rttiWriteElem(data.height, memory);
			memory = rttiWriteElem(data.xOffset, memory);
			memory = rttiWriteElem(data.yOffset, memory);
			memory = rttiWriteElem(data.xAdvance, memory);
			memory = rttiWriteElem(data.yAdvance, memory);
			rttiWriteElem(data.kerningPairs, memory);
		}

		static UINT32 fromMemory(CharDesc& data, char* memory)
		{
			UINT32 size;
			memcpy(&size, memory, sizeof(UINT32));
			memory += sizeof(UINT32);

			memory = rttiReadElem(data.charId, memory);
			memory = rttiReadElem(data.page, memory);
			memory = rttiReadElem(data.uvX, memory);
			memory = rttiReadElem(data.uvY, memory);
			memory = rttiReadElem(data.uvWidth, memory);
			memory = rttiReadElem(data.uvHeight, memory);
			memory = rttiReadElem(data.width, memory);
			memory = rttiReadElem(data.height, memory);
			memory = rttiReadElem(data.xOffset, memory);
			memory = rttiReadElem(data.yOffset, memory);
			memory = rttiReadElem(data.xAdvance, memory);
			memory = rttiReadElem(data.yAdvance, memory);
			rttiReadElem(data.kerningPairs, memory);

			return size;
		}

		static UINT32 getDynamicSize(const CharDesc& data)
		{
			UINT64 dataSize = sizeof(data.charId)
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

			dataSize += sizeof(UINT32);

			return (UINT32)dataSize;
		}
	};

	/** @endcond */
	/** @} */
}
