//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "Utility/BsRectOffset.h"
#include "2D/BsTextSprite.h"
#include "Image/BsColor.h"
#include "Math/BsVector2I.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	Possible positions used for positioning content image within a GUI element. */
	enum class GUIImagePosition
	{
		Left, Right
	};

	/**
	 * GUI element style that determines the look of a GUI element, as well as the element's default layout options. 
	 * Different looks can be provided for different element states.
	 */
	struct BS_EXPORT GUIElementStyle : public IReflectable
	{
		/**	Specific texture and text color used in a particular GUI element style. */
		struct BS_EXPORT GUIElementStateStyle
		{
			HSpriteTexture texture;
			Color textColor;
		};

		HFont font; /**< Font to use for all text within the GUI element. */
		UINT32 fontSize = 8; /**< Font size to use for all text within the GUI element. */
		TextHorzAlign textHorzAlign = THA_Left; /**< Horizontal alignment of text within the GUI element. */
		TextVertAlign textVertAlign = TVA_Top; /**< Vertical alignment of text within the GUI element. */
		GUIImagePosition imagePosition = GUIImagePosition::Left; /**< Position of content image relative to text. */
		bool wordWrap = false; /**< Should the text word wrap if it doesn't fit. */

		GUIElementStateStyle normal; /**< Style used when element is in normal state and off. */
		GUIElementStateStyle hover; /**< Style used when element is in hover state and off. */
		GUIElementStateStyle active; /**< Style used when element is in active state and off. */
		GUIElementStateStyle focused; /**< Style used when element is in focused state and off. */

		// For controls that can be turned on-off
		GUIElementStateStyle normalOn; /**< Style used when element is in normal state and on. */
		GUIElementStateStyle hoverOn; /**< Style used when element is in hover state and on. */
		GUIElementStateStyle activeOn; /**< Style used when element is in active state and on. */
		GUIElementStateStyle focusedOn; /**< Style used when element is in focused state and on. */

		RectOffset border; /**< Determines how the element is scaled (using the typical Scale9Grid approach). */
		RectOffset margins; /**< Determines offset from the background graphics to the content. Input uses bounds offset by this value. */
		RectOffset contentOffset; /**< Additional offset to the content, that doesn't effect the bounds. Applied on top of the margins offsets. */
		RectOffset padding; /**< Determines extra distance between this and other elements in a layout. */

		UINT32 width = 0; /**< Wanted width of the GUI element in pixels. Only used if fixedWidth is enabled. */
		UINT32 height = 0; /**< Wanted height of the GUI element in pixels. Only used if fixedHeight is enabled. */
		UINT32 minWidth = 0; /**< Minimum width allowed for the GUI element. Used by the layout only when exact width is not specified. */
		UINT32 maxWidth = 0; /**< Maximum width allowed for the GUI element. Used by the layout only when exact width is not specified. */
		UINT32 minHeight = 0; /**< Minimum height allowed for the GUI element. Used by the layout only when exact height is not specified. */
		UINT32 maxHeight = 0; /**< Maximum height allowed for the GUI element. Used by the layout only when exact height is not specified. */
		bool fixedWidth = false; /**< Determines should the layout resize the element depending on available size. If true no resizing will be done. */
		bool fixedHeight = false; /**< Determines should the layout resize the element depending on available size. If true no resizing will be done. */

		Map<String, String> subStyles; /**< Sub-styles used by certain more complex elements. */

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class GUIElementStyleRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}