//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "RTTI/BsStdRTTI.h"

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

	/** @} */
}
