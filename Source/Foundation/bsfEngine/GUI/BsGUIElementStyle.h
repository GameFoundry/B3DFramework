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
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIImagePosition
	{
		Left,
		Right
	};

	/**	Specific texture and text color used in a particular GUI element style. */
	struct B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI), ExportAsStruct(true)) GUIElementStateStyle
	{
		B3D_NO_RREF HSpriteTexture Texture;
		Color TextColor;
	};

	/**
	 * GUI element style that determines the look of a GUI element, as well as the element's default layout options.
	 * Different looks can be provided for different element states.
	 */
	struct B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIElementStyle : public IReflectable
	{
		B3D_SCRIPT_EXPORT()
		GUIElementStyle() = default;

		B3D_SCRIPT_EXPORT()
		B3D_NO_RREF HFont Font; /**< Font to use for all text within the GUI element. */

		B3D_SCRIPT_EXPORT()
		u32 FontSize = 8; /**< Font size to use for all text within the GUI element. */

		B3D_SCRIPT_EXPORT()
		TextHorzAlign TextHorzAlign = THA_Left; /**< Horizontal alignment of text within the GUI element. */

		B3D_SCRIPT_EXPORT()
		TextVertAlign TextVertAlign = TVA_Top; /**< Vertical alignment of text within the GUI element. */

		B3D_SCRIPT_EXPORT()
		GUIImagePosition ImagePosition = GUIImagePosition::Left; /**< Position of content image relative to text. */

		B3D_SCRIPT_EXPORT()
		bool WordWrap = false; /**< Should the text word wrap if it doesn't fit. */

		/**
		 * Style used when the element doesn't have focus nor is the user interacting with the element. Used when the
		 * element is in the 'off' state.
		 */
		B3D_SCRIPT_EXPORT()
		GUIElementStateStyle Normal;

		/**
		 * Style used when the user is hovering the pointer over the element, while the element doesn't have focus. Used
		 * when the element is in the 'off' state.
		 */
		B3D_SCRIPT_EXPORT()
		GUIElementStateStyle Hover;

		/**
		 * Style used when the user is actively interacting with the element. Used when the element is in the 'off' state.
		 */
		B3D_SCRIPT_EXPORT()
		GUIElementStateStyle Active;

		/**
		 * Style used when the element has focus but the pointer is not hovering over the element. Used when the element is
		 * in the 'off' state.
		 */
		B3D_SCRIPT_EXPORT()
		GUIElementStateStyle Focused;

		/**
		 * Style used when the element has focus and the pointer is hovering over the element. Used when the element is
		 * in the 'off' state.
		 */
		B3D_SCRIPT_EXPORT()
		GUIElementStateStyle FocusedHover;

		/** Same as GUIElementStyle::normal, except it's used when element is in the 'on' state. */
		B3D_SCRIPT_EXPORT()
		GUIElementStateStyle NormalOn;

		/** Same as GUIElementStyle::hover, except it's used when element is in the 'on' state. */
		B3D_SCRIPT_EXPORT()
		GUIElementStateStyle HoverOn;

		/** Same as GUIElementStyle::active, except it's used when element is in the 'on' state. */
		B3D_SCRIPT_EXPORT()
		GUIElementStateStyle ActiveOn;

		/** Same as GUIElementStyle::focused, except it's used when element is in the 'on' state. */
		B3D_SCRIPT_EXPORT()
		GUIElementStateStyle FocusedOn;

		/** Same as GUIElementStyle::focusedHover, except it's used when element is in the 'on' state. */
		B3D_SCRIPT_EXPORT()
		GUIElementStateStyle FocusedHoverOn;

		B3D_SCRIPT_EXPORT()
		RectOffset Border; /**< Determines how the element is scaled (using the typical Scale9Grid approach). */

		B3D_SCRIPT_EXPORT()
		RectOffset Margins; /**< Determines offset from the background graphics to the content. Input uses bounds offset by this value. */

		B3D_SCRIPT_EXPORT()
		RectOffset ContentOffset; /**< Additional offset to the content, that doesn't effect the bounds. Applied on top of the margins offsets. */

		B3D_SCRIPT_EXPORT()
		RectOffset Padding; /**< Determines extra distance between this and other elements in a layout. */

		B3D_SCRIPT_EXPORT()
		u32 Width = 0; /**< Wanted width of the GUI element in pixels. Only used if fixedWidth is enabled. */

		B3D_SCRIPT_EXPORT()
		u32 Height = 0; /**< Wanted height of the GUI element in pixels. Only used if fixedHeight is enabled. */

		B3D_SCRIPT_EXPORT()
		u32 MinWidth = 0; /**< Minimum width allowed for the GUI element. Used by the layout only when exact width is not specified. */

		B3D_SCRIPT_EXPORT()
		u32 MaxWidth = 0; /**< Maximum width allowed for the GUI element. Used by the layout only when exact width is not specified. */

		B3D_SCRIPT_EXPORT()
		u32 MinHeight = 0; /**< Minimum height allowed for the GUI element. Used by the layout only when exact height is not specified. */

		B3D_SCRIPT_EXPORT()
		u32 MaxHeight = 0; /**< Maximum height allowed for the GUI element. Used by the layout only when exact height is not specified. */

		B3D_SCRIPT_EXPORT()
		bool FixedWidth = false; /**< Determines should the layout resize the element depending on available size. If true no resizing will be done. */

		B3D_SCRIPT_EXPORT()
		bool FixedHeight = false; /**< Determines should the layout resize the element depending on available size. If true no resizing will be done. */

		/**
		 * Registers a new sub-style that is used by complex GUI elements that contain one or multiple sub-elements.
		 * @param[in]	guiType		Name of the sub-element this style is to be used for.  This depends on GUI element the
		 *							style is applied to.
		 * @param[in]	styleName	Name of the style in GUI skin to use for the sub-element.
		 */
		B3D_SCRIPT_EXPORT()

		void AddSubStyle(const String& guiType, const String& styleName)
		{
			SubStyles[guiType] = styleName;
		}

		Map<String, String> SubStyles; /**< Sub-styles used by certain more complex elements. */

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class GUIElementStyleRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const;
	};

	/** @} */
} // namespace bs
