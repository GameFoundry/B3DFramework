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
	struct BS_SCRIPT_EXPORT(ExportAsStruct(true),DocumentationGroup(GUI_Engine)) KerningPair
	{
		u32 OtherCharId;
		i32 Amount;
	};

	/**	Describes a single character in a font of a specific size. */
	struct BS_SCRIPT_EXPORT(ExportAsStruct(true),DocumentationGroup(GUI_Engine)) CharDesc
	{
		u32 CharId; /**< Character ID, corresponding to a Unicode key. */
		u32 Page; /**< Index of the texture the character is located on. */
		float UvX, UvY; /**< Texture coordinates of the character in the page texture. */
		float UvWidth, UvHeight; /**< Width/height of the character in texture coordinates. */
		u32 Width, Height; /**< Width/height of the character in pixels. */
		i32 XOffset, YOffset; /**< Offset for the visible portion of the character in pixels. */
		i32 XAdvance, YAdvance; /**< Determines how much to advance the pen after writing this character, in pixels. */

		/**
		 * Pairs that determine if certain character pairs should be closer or father together. for example "AV"
		 * combination.
		 */
		Vector<KerningPair> KerningPairs;
	};

	/** @} */
}
