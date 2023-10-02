//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIContent.h"
#include "GUI/BsGUIElement.h"
#include "Image/BsColor.h"
#include "Resources/BsResource.h"
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

	/** Determines if and how text wraps to a new line if it doesn't fit on a single line. */
	enum class GUIWordWrapMode
	{
		None,
		WrapWord
		// For later: BreakWord, flag for Ellipis, etc.
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
		WordWrap,

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
	struct GUIStyleSheetBorderElement
	{
		u32 Width = 0; /**< Size of the border in pixels. Zero means no border. */
		Color Color; /**< Color of the border. */
		GUIBorderElementStyle Style = GUIBorderElementStyle::Solid; /**< Style how to render the border. */
	};

	/** Style for a particular state of a GUI element (e.g. normal, hover, focused, disabled, etc. */
	struct B3D_EXPORT GUIStyleSheetStateStyle
	{
		String Selector; /**< Selector that describes to which GUI elements this style applies to. */
		String PseudoClass; /**< Psuedo-class determines which state of the GUI element this style applies to (normal, hover, focused, disabled, etc.). */
		GUIStyleSheetSelectorType SelectorType = GUIStyleSheetSelectorType::Element; /**< Type of selector provided in @p Selector. */

		RectOffset Margins; /**< Empty space around the GUI element outside of the border. In pixels.*/
		RectOffset Padding; /**< Empty space within the GUI element inside the border. In pixels. */

		Size2UI Size = Size2UI::kZero; /**< Size of the GUI element in pixels. This is the size at which border will be rendered. Contents will be rendered at this size, minus the padding. */
		Size2UI MinimumSize = Size2UI::kZero; /**< If non-zero, GUI element size will expand to fill the available area, respecting the minimum and (optionally) maximum size. In pixels. */
		Size2UI MaximumSize = Size2UI::kZero; /**< If non-zero, GUI element size will expand to fill the available area, respecting the maximum and (optionally) minimum size. In pixels. */

		Color BackgroundColor; /**< Color of the GUI element background. */
		Color Color; /**< Color of the GUI element contents (usually text or icon). */
		float Opacity = 1.0f; /**< Opacity of the GUI element. This value will affect all aspects of the GUI element (border, background and contents). In range [0, 1]. */

		GUIStyleSheetBorderElement BorderLeft; /**< Style information for the left border. */
		GUIStyleSheetBorderElement BorderRight; /**< Style information for the right border. */
		GUIStyleSheetBorderElement BorderTop; /**< Style information for the top border. */
		GUIStyleSheetBorderElement BorderBottom; /**< Style information for the bottom border. */

		u32 BorderTopLeftRadius = 0; /**< Radius of the top left border corner, if rounded corners are desired. In pixels. */
		u32 BorderTopRightRadius = 0; /**< Radius of the top right border corner, if rounded corners are desired. In pixels. */
		u32 BorderBottomLeftRadius = 0; /**< Radius of the bottom left border corner, if rounded corners are desired. In pixels. */
		u32 BorderBottomRightRadius = 0; /**< Radius of the bottom right border corner, if rounded corners are desired. In pixels. */

		String FontFamily; /**< Font family to render the text contents of the GUI element with. */
		u32 FontSize = 8; /**< Font size to render the text contents of the GUI element with. */
		GUIHorizontalTextAlignment HorizontalTextAlignment = GUIHorizontalTextAlignment::Left; /**< Determines horizontal alignment of text within the GUI element. */
		GUIVerticalTextAlignment VerticalTextAlignment = GUIVerticalTextAlignment::Middle; /**< Determines vertical alignment of text within the GUI element. */
		GUIWordWrapMode WordWrap = GUIWordWrapMode::None; /**< Determines if text wraps when it doesn't fit in a single line. */

		static constexpr u32 kPropertyDWordCount = Math::DivideAndRoundUp((u32)GUIStyleSheetPropertyType::Count, (u32)sizeof(u32) * 8);
		TBitfield<InlineContainerAllocator<kPropertyDWordCount>> OverridenProperties; /**< Bit for each property that is different than the default will be set. Used for determining which properties to override from parent style. */

		/**	Default style that may be used when no other is available. */
		static SPtr<GUIStyleSheetStateStyle> kDefault;

		GUIStyleSheetStateStyle();

		/** Overrides all the properties of this style with the set properties from @p other style. */
		void Override(const GUIStyleSheetStateStyle& other);

		/** Returns true if that property has been assigned. If false the property is using the default value. */
		bool IsPropertySet(GUIStyleSheetPropertyType property) const { return OverridenProperties[(u32)property]; }

		/** Returns the font as specified by FontFamily. If the font isn't already loaded, loads it. If exact font cannot be found, it falls back to a default font. */
		HFont GetOrLoadFont() const;

	private:
		mutable HFont mCachedFont;
	};

	/** Contains a set of state styles for all supported states. */
	struct B3D_EXPORT GUIStyleSheetStyle
	{
		GUIStyleSheetStateStyle Normal; /**< Normal style of the GUI element that is interactable, but isn't currently being interacted with. */
		Optional<GUIStyleSheetStateStyle> Hover; /**< Style of GUI element that is interactable and the mouse pointer is hovering over the GUI element. Inherits from Normal state and optionally from Focused, or Checked state, if those are active. */
		Optional<GUIStyleSheetStateStyle> Active; /**< Style of GUI element that is interactable and the user is currently clicking on the element. Inherits from Normal state and optionally from Focused, Hover or Checked state, if those are active. */
		Optional<GUIStyleSheetStateStyle> Focus; /**< Style of GUI element that is interactable and currently has input focus. Inherits from Normal state. */
		Optional<GUIStyleSheetStateStyle> Disabled; /**< Style of GUI element that is interactable and currently has input focus. Inherits from Normal state. */
		Optional<GUIStyleSheetStateStyle> Checked; /**< Style of GUI element that is interactable, can be toggled on/off and is currently toggled on. Inherits from Normal state. */

		/** Returns the state with the given name. Returns null if the state with the name doesn't exist, or if the state has default values. */
		const GUIStyleSheetStateStyle* FindStateStyle(const StringView& name) const;

		/** Returns the state with the given GUI element state flags. Note this may be a combination of internal states depending on the provided flags. */
		SPtr<GUIStyleSheetStateStyle> FindStateStyle(GUIElementStateFlags state) const;

		/** Assigns state information to a state with the specified name. If a state with the specified name the method returns false, otherwise true. */
		bool FindAndSetStateStyle(const StringView& name, const GUIStyleSheetStateStyle& stateStyle);

		/** Overrides all the properties of this style with the set properties from @p other style. */
		void Override(const GUIStyleSheetStyle& other);

		/**	Default style that may be used when no other is available. */
		static SPtr<GUIStyleSheetStyle> kDefault;
	private:
		mutable UnorderedMap<GUIElementStateFlags, SPtr<GUIStyleSheetStateStyle>> mCachedStateStyles;
	};

	/**
	 * Contains a set of styles that determine how are GUI elements displayed. GUI elements will perform lookup into the style sheet
	 * based on the element type, element ID and current element state.
	 */
	class B3D_EXPORT GUIStyleSheet : public Resource
	{
	public:
		GUIStyleSheet();
		~GUIStyleSheet() override = default;

		/** Attempts to parse the provided style sheet file and outputs the parsed style sheet, if successful. */
		static HGUIStyleSheet Parse(const Path& file);

		/** Creates a new empty style sheet. */
		static HGUIStyleSheet Create();

		/** Creates a new empty style sheet. */
		static SPtr<GUIStyleSheet> CreateShared();

		// TODO - Add LoadOrParse() method that attempts to lookup an existing style sheet from PersistentCache first

		/**
		 * Find the appropriate style to use for a particular GUI element.
		 *
		 * @name	elementType		Name of the GUI element type to retrieve the state for (e.g. button, input, checkbox, etc.)
		 * @name	elementId		Element ID, in case you wish to style a particular GUI element differently from the rest of its type. This will override any
		 *							properties defined in the global element style.
		 * @return					Style that should be used to rendering the provided GUI element.
		 */
		SPtr<GUIStyleSheetStyle> FindStyle(const String& elementType, const String& elementId) const;
	private:
		friend class GUIStyleSheetParser;

		/** Structure used for looking up cached states. */
		struct CachedStateStyleKey
		{
			CachedStateStyleKey(const String& elementType, const String& elementId)
				: ElementType(elementType), ElementId(elementId)
			{ }

			struct Hash
			{
				size_t operator()(const CachedStateStyleKey& value) const
				{
					size_t hash = 0;
					B3DCombineHash(hash, value.ElementType);
					B3DCombineHash(hash, value.ElementId);

					return hash;
				}
			};

			bool operator==(const CachedStateStyleKey& other) const
			{
				return ElementType == other.ElementType && ElementId == other.ElementId;
			}

			String ElementType;
			String ElementId;
		};

		friend struct ::std::hash<CachedStateStyleKey>;

		UnorderedMap<String, GUIStyleSheetStyle> mElementStyles;
		UnorderedMap<String, GUIStyleSheetStyle> mIdStyles;

		mutable UnorderedMap<CachedStateStyleKey, SPtr<GUIStyleSheetStyle>, CachedStateStyleKey::Hash> mCachedStyles;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class GUIStyleSheetRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */
} // namespace bs
