//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIContent.h"
#include "Image/BsColor.h"
#include "Utility/BsBitfield.h"
#include "Utility/BsRectOffset.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** GUI element border style. */
	enum class GUIBorderElementStyle
	{
		None, /**< No border. */
		Solid /**< Border with a solid color. */
	};

	/** Type of selector in a style sheet. Selectors determine to which GUI elements does a style apply to. */
	enum class GUIStyleSheetSelectorType
	{
		Element, /**< Selector applies to entire type of GUI elements (e.g. button, input box, etc.) */
		Id /**< Selector applies only to a particular GUI element with a specific ID. */
	};

	/** Determines how is text horizontally aligned in a GUI element. */
	enum class GUIHorizontalTextAlignment
	{
		Left, Center, Right
	};

	/** Determines how is text vertically aligned in a GUI element. */
	enum class GUIVerticalTextAlignment
	{
		Top, Middle, Bottom
	};

	/** All possible properties in a GUI style sheet. See GUIStyleSheetStateStyle for their descriptions. */
	enum class GUIStyleSheetPropertyType
	{
		Undefined,

		Width,
		Height,
		MinWidth,
		MinHeight,
		MaxWidth,
		MaxHeight,

		Margin,
		MarginTop,
		MarginBottom,
		MarginLeft,
		MarginRight,

		Padding,
		PaddingTop,
		PaddingBottom,
		PaddingLeft,
		PaddingRight,

		Color,
		Opacity,
		BackgroundColor,

		TextAlign,
		VerticalAlign,
		FontFamily,
		FontSize,

		Border,
		BorderStyle,
		BorderWidth,
		BorderColor,

		BorderTop,
		BorderTopStyle,
		BorderTopWidth,
		BorderTopColor,

		BorderBottom,
		BorderBottomStyle,
		BorderBottomWidth,
		BorderBottomColor,

		BorderLeft,
		BorderLeftStyle,
		BorderLeftWidth,
		BorderLeftColor,

		BorderRight,
		BorderRightStyle,
		BorderRightWidth,
		BorderRightColor,

		BorderRadius,
		BorderTopLeftRadius,
		BorderTopRightRadius,
		BorderBottomLeftRadius,
		BorderBottomRightRadius,

		Count,
	};

	/** Style information for a single border side (left, right, top or bottom). */
	struct GUIBorderElement
	{
		u32 Width = 0; /**< Size of the border in pixels. Zero means no border. */
		Color Color; /**< Color of the border. */
		GUIBorderElementStyle Style = GUIBorderElementStyle::None; /**< Style how to render the border. */
	};

	/** Style for a particular state of a GUI element (e.g. normal, hover, focused, disabled, etc. */
	struct GUIStyleSheetStateStyle
	{
		String Selector; /**< Selector that describes to which GUI elements this style applies to. */
		String PseudoClass; /**< Psuedo-class determines which state of the GUI element this style applies to (normal, hover, focused, disabled, etc.). */
		GUIStyleSheetSelectorType SelectorType = GUIStyleSheetSelectorType::Element; /**< Type of selector provided in @p Selector. */

		RectOffset Margins; /**< Empty space around the GUI element outside of the border. In pixels.*/
		RectOffset Padding; /**< Empty space within the GUI element inside the border. In pixels. */

		Size2UI Size; /**< Size of the GUI element in pixels. This is the size at which border will be rendered. Contents will be rendered at this size, minus the padding. */
		Size2UI MinimumSize; /**< If non-zero, GUI element size will expand to fill the available area, respecting the minimum and (optionally) maximum size. In pixels. */
		Size2UI MaximumSize; /**< If non-zero, GUI element size will expand to fill the available area, respecting the maximum and (optionally) minimum size. In pixels. */

		Color BackgroundColor; /**< Color of the GUI element background. */
		Color Color; /**< Color of the GUI element contents (usually text or icon). */
		float Opacity = 1.0f; /**< Opacity of the GUI element. This value will affect all aspects of the GUI element (border, background and contents). In range [0, 1]. */

		GUIBorderElement BorderLeft; /**< Style information for the left border. */
		GUIBorderElement BorderRight; /**< Style information for the right border. */
		GUIBorderElement BorderTop; /**< Style information for the top border. */
		GUIBorderElement BorderBottom; /**< Style information for the bottom border. */

		u32 BorderTopLeftRadius = 0; /**< Radius of the top left border corner, if rounded corners are desired. In pixels. */
		u32 BorderTopRightRadius = 0; /**< Radius of the top right border corner, if rounded corners are desired. In pixels. */
		u32 BorderBottomLeftRadius = 0; /**< Radius of the bottom left border corner, if rounded corners are desired. In pixels. */
		u32 BorderBottomRightRadius = 0; /**< Radius of the bottom right border corner, if rounded corners are desired. In pixels. */

		String FontFamily; /**< Font family to render the text contents of the GUI element with. */
		u32 FontSize = 8; /**< Font size to render the text contents of the GUI element with. */
		GUIHorizontalTextAlignment HorizontalTextAlignment = GUIHorizontalTextAlignment::Left; /**< Determines horizontal alignment of text within the GUI element. */
		GUIVerticalTextAlignment VerticalTextAlignment = GUIVerticalTextAlignment::Middle; /**< Determines vertical alignment of text within the GUI element. */

		static constexpr u32 kPropertyDWordCount = Math::DivideAndRoundUp((u32)GUIStyleSheetPropertyType::Count, (u32)sizeof(u32));
		TBitfield<InlineContainerAllocator<kPropertyDWordCount>> OverridenProperties; /**< Bit for each property that is different than the default will be set. Used for determining which properties to override from parent style. */

		void Override(const GUIStyleSheetStateStyle& other);
	};

	/** Contains a set of state styles for all supported states. */
	struct GUIStyleSheetStyle
	{
		GUIStyleSheetStateStyle Normal; /**< Normal style of the GUI element that is interactable, but isn't currently being interacted with. */
		Optional<GUIStyleSheetStateStyle> Hover; /**< Style of GUI element that is interable and the mouse pointer is hovering over the GUI element. Inherits from Normal state and optionally from Focused, or Checked state, if those are active. */
		Optional<GUIStyleSheetStateStyle> Active; /**< Style of GUI element that is interactable and the user is currently clicking on the element. Inherits from Normal state and optionally from Focused, Hover or Checked state, if those are active. */
		Optional<GUIStyleSheetStateStyle> Focused; /**< Style of GUI element that is interactable and currently has input focus. Inherits from Normal state. */
		Optional<GUIStyleSheetStateStyle> Disabled; /**< Style of GUI element that is interactable and currently has input focus. Inherits from Normal state. */
		Optional<GUIStyleSheetStateStyle> Checked; /**< Style of GUI element that is interactable, can be toggled on/off and is currently toggled on. Inherits from Normal state. */
	};

	// TODO - Doc
	class B3D_EXPORT GUIStyleSheet
	{
	public:
		static Optional<GUIStyleSheet> Parse(const Path& file);

		GUIStyleSheetStateStyle FindStyle(const String& elementType, const String& elementId, GUIElementState state);

	private:
		UnorderedMap<String, GUIStyleSheetStyle> mElementStyles;
		UnorderedMap<String, GUIStyleSheetStyle> mIdStyles;
	};


	/** @} */
} // namespace bs
