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
#include "2D/BsTextSprite.h"

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
		Element, /**< Selector applies to entire type of GUI elements (e.g. button, input box, etc.), */
		Class, /**< Selector applies to a set of GUI elements given a particular class name. */
		Id /**< Selector applies only to a particular GUI element with the specific ID. */
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
		BackgroundImage,

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

	/** All possible states in a GUI style sheet rule. */
	enum class GUIStyleSheetStateType
	{
		Undefined,

		Normal,
		Hover,
		Active,
		Focus,
		Checked,
		Disabled,

		Count,
	};

	/** Style information for a single border side (left, right, top or bottom). */
	struct GUIStyleSheetBorderElement
	{
		u32 Width = 0; /**< Size of the border in pixels. Zero means no border. */
		Color Color; /**< Color of the border. */
		GUIBorderElementStyle Style = GUIBorderElementStyle::Solid; /**< Style how to render the border. */

		/** Returns the width of the border if visible, or zero otherwise. */
		u32 GetVisibleWidth() const { return Style != GUIBorderElementStyle::None ? Width : 0; }
	};

	/** If multiple selectors are provided for a style sheet, this is used for determining their relationship. */
	enum class GUIStyleSheetCombinatorType
	{
		None, /*< No combinator, the selector applies to the GUI element directly matching the selector. */
		AncestorOf, /**< GUI element we're looking up the selector for, must have an ancestor matching this selector. */
	};

	/** Determines to which GUI elements a particular style will be applied to. */
	struct GUIStyleSheetSelector
	{
		GUIStyleSheetSelector() = default;
		GUIStyleSheetSelector(const String& name, GUIStyleSheetSelectorType selectorType, GUIStyleSheetCombinatorType combinatorType)
			: Name(name), SelectorType(selectorType), CombinatorType(combinatorType)
		{ }

		String Name;
		GUIStyleSheetSelectorType SelectorType = GUIStyleSheetSelectorType::Id;
		GUIStyleSheetCombinatorType CombinatorType = GUIStyleSheetCombinatorType::None;

		/** Checks does the selector match the provided GUI element. */
		bool IsMatching(const GUIElementBase& element) const;
	};

	/** List of all selectors on a particular GUI style sheet. */
	struct GUIStyleSheetSelectorList
	{
		TInlineArray<GUIStyleSheetSelector, 4> Selectors;

		/** Checks does the selector match the provided GUI element. */
		bool IsMatching(const GUIElementBase& element) const;

		/** Returns a unique name that represents all the selectors in the list. */
		const String& GetUniqueName() const;

	private:
		mutable String mCachedUniqueName;
	};

	/** Style rule for a particular state of a GUI element (e.g. normal, hover, focused, disabled, etc.). */
	struct B3D_EXPORT GUIStyleSheetStateRule : public IReflectable
	{
		RectOffset Margins; /**< Empty space around the GUI element outside of the border. In pixels.*/
		RectOffset Padding; /**< Empty space within the GUI element inside the border. In pixels. */

		Size2UI Size = Size2UI::kZero; /**< Size of the GUI element contents in pixels. Total size of the GUI element will be determined by content size, padding, border width and margins. */
		Size2UI MinimumSize = Size2UI::kZero; /**< If non-zero, GUI element size will expand to fill the available area, respecting the minimum and (optionally) maximum size. In pixels. */
		Size2UI MaximumSize = Size2UI::kZero; /**< If non-zero, GUI element size will expand to fill the available area, respecting the maximum and (optionally) minimum size. In pixels. */

		Color BackgroundColor; /**< Color of the GUI element background. */
		Color Color; /**< Color of the GUI element contents (usually text or icon). */
		float Opacity = 1.0f; /**< Opacity of the GUI element. This value will affect all aspects of the GUI element (border, background and contents). In range [0, 1]. */

		HTexture BackgroundImage; /**< Image to render as the background. */

		GUIStyleSheetBorderElement BorderLeft; /**< Style information for the left border. */
		GUIStyleSheetBorderElement BorderRight; /**< Style information for the right border. */
		GUIStyleSheetBorderElement BorderTop; /**< Style information for the top border. */
		GUIStyleSheetBorderElement BorderBottom; /**< Style information for the bottom border. */

		u32 BorderTopLeftRadius = 0; /**< Radius of the top left border corner, if rounded corners are desired. In pixels. */
		u32 BorderTopRightRadius = 0; /**< Radius of the top right border corner, if rounded corners are desired. In pixels. */
		u32 BorderBottomLeftRadius = 0; /**< Radius of the bottom left border corner, if rounded corners are desired. In pixels. */
		u32 BorderBottomRightRadius = 0; /**< Radius of the bottom right border corner, if rounded corners are desired. In pixels. */

		HFont Font; /**< Font family to render the text contents of the GUI element with. */
		u32 FontSize = 8; /**< Font size to render the text contents of the GUI element with. */
		GUIHorizontalTextAlignment HorizontalTextAlignment = GUIHorizontalTextAlignment::Left; /**< Determines horizontal alignment of text within the GUI element. */
		GUIVerticalTextAlignment VerticalTextAlignment = GUIVerticalTextAlignment::Middle; /**< Determines vertical alignment of text within the GUI element. */
		GUIWordWrapMode WordWrap = GUIWordWrapMode::None; /**< Determines if text wraps when it doesn't fit in a single line. */

		static constexpr u32 kPropertyDWordCount = Math::DivideAndRoundUp((u32)GUIStyleSheetPropertyType::Count, (u32)sizeof(u32) * 8);
		TBitfield<InlineContainerAllocator<kPropertyDWordCount>> OverridenProperties; /**< Bit for each property that is different than the default will be set. Used for determining which properties to override from parent rule. */

		/**	Default style that may be used when no other is available. */
		static SPtr<GUIStyleSheetStateRule> kDefault;

		GUIStyleSheetStateRule();

		/** Overrides all the properties of this style with the set properties from @p other style. */
		void Override(const GUIStyleSheetStateRule& other);

		/** Returns true if that property has been assigned. If false the property is using the default value. */
		bool IsPropertySet(GUIStyleSheetPropertyType property) const { return OverridenProperties[(u32)property]; }

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class GUIStyleSheetStateRuleRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** Contains a set of style rules for all states in a GUI element. */
	struct B3D_EXPORT GUIStyleSheetRule : public IReflectable
	{
		GUIStyleSheetSelectorList SelectorList; /**< List of selectors that determines which GUI elements this style applies to. */
		String PseudoElement; /**< Name of the pseudo-element, if the rule is specified for one. */

		GUIStyleSheetStateRule Normal; /**< Normal style of the GUI element that is interactable, but isn't currently being interacted with. */
		GUIStyleSheetStateRule Hover; /**< Style of GUI element that is interactable and the mouse pointer is hovering over the GUI element. Inherits from Normal state and optionally from Focused, or Checked state, if those are active. */
		GUIStyleSheetStateRule Active; /**< Style of GUI element that is interactable and the user is currently clicking on the element. Inherits from Normal state and optionally from Focused, Hover or Checked state, if those are active. */
		GUIStyleSheetStateRule Focus; /**< Style of GUI element that is interactable and currently has input focus. Inherits from Normal state. */
		GUIStyleSheetStateRule Disabled; /**< Style of GUI element that is interactable and currently has input focus. Inherits from Normal state. */
		GUIStyleSheetStateRule Checked; /**< Style of GUI element that is interactable, can be toggled on/off and is currently toggled on. Inherits from Normal state. */

		static constexpr u32 kStateDWordCount = Math::DivideAndRoundUp((u32)GUIStyleSheetStateType::Count, (u32)sizeof(u32) * 8);
		TBitfield<InlineContainerAllocator<kStateDWordCount>> OverridenStates; /**< Bit for each state that is different than the default will be set. Used for determining which properties to override from parent rule. */

		GUIStyleSheetRule();

		/** Returns the state with the given name. Returns null if the state with the name doesn't exist, or if the state has default values. */
		const GUIStyleSheetStateRule* FindStateStyle(const StringView& name) const;

		/** Returns the state with the given GUI element state flags. Note this may be a combination of internal states depending on the provided flags. */
		SPtr<GUIStyleSheetStateRule> FindStateStyle(GUIElementStateFlags state) const;

		/** Assigns state information to a state with the specified name. If a state with the specified name the method returns false, otherwise true. */
		bool FindAndSetStateStyle(const StringView& name, const GUIStyleSheetStateRule& stateStyle);

		/** Overrides all the properties of this style with the set properties from @p other style. */
		void Override(const GUIStyleSheetRule& other);

		/** Returns true if that state has been assigned. If false the state is using the default value. */
		bool IsStateSet(GUIStyleSheetStateType state) const { return OverridenStates[(u32)state]; }

		/**	Default style that may be used when no other is available. */
		static SPtr<GUIStyleSheetRule> kDefault;
	private:
		mutable UnorderedMap<GUIElementStateFlags, SPtr<GUIStyleSheetStateRule>> mCachedStateStyles;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class GUIStyleSheetRuleRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**
	 * Contains a set of styles that determine how are GUI elements displayed. GUI elements will perform lookup into the style sheet
	 * based on the element type, element ID and current element state.
	 */
	class B3D_EXPORT GUIStyleSheet : public Resource
	{
	public:
		GUIStyleSheet(TArray<GUIStyleSheetRule> rules = {});
		~GUIStyleSheet() override = default;

		/** Builds the appropriate rule to use for a particular GUI element.  */
		SPtr<GUIStyleSheetRule> BuildRule(const GUIElement& guiElement, const String& pseudoElement = StringUtil::kBlank) const;

		/** Attempts to parse the provided style sheet file and outputs the parsed style sheet, if successful. */
		static HGUIStyleSheet Parse(const Path& file);
		// TODO - Add LoadOrParse() method that attempts to lookup an existing style sheet from PersistentCache first

		/** Creates a new style sheet. */
		static HGUIStyleSheet Create(TArray<GUIStyleSheetRule> rules = {});

		/** Creates a new style sheet. */
		static SPtr<GUIStyleSheet> CreateShared(TArray<GUIStyleSheetRule> rules = {});
	private:
		friend class GUIStyleSheetParser;

		void Initialize() override;

		/** Rebuilds the cache based on mStyles array. Should be called after first created or after deserialization. */
		void RebuildCache();

		/** List of rules in a GUI style sheet. */
		struct GUIStyleSheetRuleList
		{
			TArray<u32> RuleIndices; /**< Maps into the mStyles array in GUIStyleSheet. */ 
		};

		TArray<GUIStyleSheetRule> mRules;

		// Cached by last selector, for faster lookup (avoids iterating over the entire mRules array)
		mutable UnorderedMap<String, GUIStyleSheetRuleList> mCachedRulesByElement;
		mutable UnorderedMap<String, GUIStyleSheetRuleList> mCachedRulesByClass;
		mutable UnorderedMap<String, GUIStyleSheetRuleList> mCachedRulesById;

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
