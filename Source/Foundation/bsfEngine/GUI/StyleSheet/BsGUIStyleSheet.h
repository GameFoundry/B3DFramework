//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIContent.h"
#include "GUI/BsGUIInteractable.h"
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
		Id, /**< Selector applies only to a particular GUI element with the specific ID. */
		PseudoClass, /**< Similar to class, except the GUI element class is not set explicitly by the user, but is instead set by the runtime. */
		PseudoElement, /**< Similar to element, except the GUI element is not created explicitly by the user, but is instead created by the runtime. */
	};

	/** Determines if and how text wraps to a new line if it doesn't fit on a single line. */
	enum class GUIWordWrapMode
	{
		None,
		WrapWord
		// For later: BreakWord, flag for Ellipis, etc.
	};

	/** Possible values of the style-sheet 'visibility' property. */
	enum class GUIElementVisibility
	{
		Visible,
		Hidden
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
		Visibility,

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

		bool operator==(const GUIStyleSheetBorderElement& other) const
		{
			return Width == other.Width && Color == other.Color && Style == other.Style;
		}

		/** Returns the width of the border if visible, or zero otherwise. */
		u32 GetVisibleWidth() const { return Style != GUIBorderElementStyle::None ? Width : 0; }
	};

	/** If multiple selectors are provided for a style sheet, this is used for determining their relationship. */
	enum class GUIStyleSheetCombinatorType
	{
		None, /*< No combinator, the selector applies to the GUI element directly matching the selector. */
		AncestorOf, /**< GUI element we're looking up the selector for, must have an ancestor matching this selector. */
		ParentOf, /**< GUI element we're looking up the selector for, must have a direct parent matching this selector. */
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
		bool IsMatching(const GUIRenderable& element, StringView pseudoElement = "", StringView pseudoClass = "", bool ignorePseudoClass = false) const;

		/** Checks does the selector matches a GUI element with the provided type/class/id/pseudo-element/pseudo-class. */
		bool IsMatching(StringView elementType, StringView elementClass = "", StringView elementId = "", StringView pseudoElement = "", StringView pseudoClass = "", bool ignorePseudoClass = false) const;
	};

	/** List of all selectors on a particular GUI style sheet. */
	struct GUIStyleSheetSelectorList
	{
		TInlineArray<GUIStyleSheetSelector, 4> Selectors;

		/** Checks does the selector match the provided GUI element. */
		bool IsMatching(const GUIRenderable& element, StringView pseudoElement = "", StringView pseudoClass = "", bool ignorePseudoClass = false) const;

		/** Checks does the selector match a GUI element with provided type/class/id/pseudo-element/pseudo-class. It is assumed the GUI element has no parents. */
		bool IsMatching(StringView elementType, StringView elementClass = "", StringView elementId = "", StringView pseudoElement = "", StringView pseudoClass = "", bool ignorePseudoClass = false) const;

		/** Returns a unique name that represents all the selectors in the list. */
		const String& GetUniqueName() const;

		/** Calculates the weight that determines selector specificity, with higher values resulting in more specific selectors (e.g. id selector is more specific than class). */
		u32 CalculateSpecificity() const;

	private:
		mutable String mCachedUniqueName;
	};

	/** Style rule for a particular state of a GUI element (e.g. normal, hover, focused, disabled, etc.). */
	struct B3D_EXPORT GUIStyleSheetRules : public IReflectable
	{
		String PseudoClass; /**< Pseudo-class the rule is applied to, if any. */

		RectOffset Margins; /**< Empty space around the GUI element outside of the border. In pixels.*/
		RectOffset Padding; /**< Empty space within the GUI element inside the border. In pixels. */

		Size2UI Size = Size2UI::kZero; /**< Size of the GUI element contents in pixels. Total size of the GUI element will be determined by content size, padding, border width and margins. */
		Size2UI MinimumSize = Size2UI::kZero; /**< If non-zero, GUI element size will expand to fill the available area, respecting the minimum and (optionally) maximum size. In pixels. */
		Size2UI MaximumSize = Size2UI::kZero; /**< If non-zero, GUI element size will expand to fill the available area, respecting the maximum and (optionally) minimum size. In pixels. */

		Color BackgroundColor; /**< Color of the GUI element background. */
		Color Color; /**< Color of the GUI element contents (usually text or icon). */
		float Opacity = 1.0f; /**< Opacity of the GUI element. This value will affect all aspects of the GUI element (border, background and contents). In range [0, 1]. */

		HSpriteImage BackgroundImage; /**< Image to render as the background. */
		GUIElementVisibility Visibility = GUIElementVisibility::Visible; /**< Determines if the element should be displayed or not. */

		GUIStyleSheetBorderElement BorderLeft; /**< Style information for the left border. */
		GUIStyleSheetBorderElement BorderRight; /**< Style information for the right border. */
		GUIStyleSheetBorderElement BorderTop; /**< Style information for the top border. */
		GUIStyleSheetBorderElement BorderBottom; /**< Style information for the bottom border. */

		u32 BorderTopLeftRadius = 0; /**< Radius of the top left border corner, if rounded corners are desired. In pixels. */
		u32 BorderTopRightRadius = 0; /**< Radius of the top right border corner, if rounded corners are desired. In pixels. */
		u32 BorderBottomLeftRadius = 0; /**< Radius of the bottom left border corner, if rounded corners are desired. In pixels. */
		u32 BorderBottomRightRadius = 0; /**< Radius of the bottom right border corner, if rounded corners are desired. In pixels. */

		HFont Font; /**< Font family to render the text contents of the GUI element with. */
		float FontSize = 8.0f; /**< Font size to render the text contents of the GUI element with. */
		GUIHorizontalTextAlignment HorizontalTextAlignment = GUIHorizontalTextAlignment::Left; /**< Determines horizontal alignment of text within the GUI element. */
		GUIVerticalTextAlignment VerticalTextAlignment = GUIVerticalTextAlignment::Middle; /**< Determines vertical alignment of text within the GUI element. */
		GUIWordWrapMode WordWrap = GUIWordWrapMode::None; /**< Determines if text wraps when it doesn't fit in a single line. */

		static constexpr u32 kPropertyDWordCount = Math::DivideAndRoundUp((u32)GUIStyleSheetPropertyType::Count, (u32)sizeof(u32) * 8);
		TBitfield<InlineContainerAllocator<kPropertyDWordCount>> OverridenProperties; /**< Bit for each property that is different than the default will be set. Used for determining which properties to override from parent rule. */

		/**	Default style that may be used when no other is available. */
		static SPtr<GUIStyleSheetRules> kDefault;

		GUIStyleSheetRules();

		/** Overrides all the properties of this style with the set properties from @p other style. */
		void Override(const GUIStyleSheetRules& other);

		/** Returns true if that property has been assigned. If false the property is using the default value. */
		bool IsPropertySet(GUIStyleSheetPropertyType property) const { return OverridenProperties[(u32)property]; }

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class GUIStyleSheetRuleRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** Contains a set of rules for a particular selector. */
	struct B3D_EXPORT GUIStyleSheetRuleset : public IReflectable
	{
		GUIStyleSheetSelectorList SelectorList; /**< List of selectors that determines which GUI elements this ruleset applies to. */
		GUIStyleSheetRules Rules; /**< Properties and their values. */

		GUIStyleSheetRuleset() = default;

		/**	Default value that may be used when no other is available. */
		static SPtr<const GUIStyleSheetRuleset> kDefault;
	private:

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class GUIStyleSheetRulesetRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** Contains a set of rulesets for all states that are commonly changing on a GUI element (e.g. hover, focused, checked, active, etc.). */
	struct B3D_EXPORT GUIStyleSheetStateRulesets
	{
		bool operator==(const GUIStyleSheetStateRulesets& other) const;
		u64 GenerateHash() const;

		/**
		 * Builds a ruleset matching the provided GUI element state flags. Rulesets are cached internally, so future calls with the same parameters
		 * will just perform a lookup for an exiting ruleset.
		 *
		 * @param	state				State flags to build the ruleset for.
		 * @param	inheritedRules		Optional rules to inherit the initial set of values from.
		 * @return						Build ruleset, or one returned from the internal cache.
		 */
		SPtr<const GUIStyleSheetRuleset> BuildStateRuleset(GUIElementStateFlags state, const GUIStyleSheetRules* inheritedRules = nullptr) const;

		WeakSPtr<const GUIStyleSheet> StyleSheet; /**< Style sheet holding the rules that are being referenced. */
		TArray<u32> RulesetIndices; /**< Maps into the mRulesets array in GUIStyleSheet. */

		/**	Default value that may be used when no other is available. */
		static SPtr<const GUIStyleSheetStateRulesets> kDefault;

	private:
		/** Key used for looking up cached rulesets. */
		struct RulesetKey
		{
			RulesetKey(GUIElementStateFlags stateFlags = GUIElementStateFlag::Normal, u64 inheritedStateId = 0)
				: StateFlags(stateFlags), InheritedStateId(inheritedStateId)
			{ }

			GUIElementStateFlags StateFlags;
			u64 InheritedStateId;

			bool operator==(const RulesetKey& other) const { return StateFlags == other.StateFlags && InheritedStateId == other.InheritedStateId; }
			u64 GenerateHash() const;
		};

		mutable TUnorderedMap<RulesetKey, SPtr<GUIStyleSheetRuleset>> mCachedRulesets;
	};

	/**
	 * Contains rulesets that determine how are GUI elements displayed. GUI elements will perform lookup into the style sheet
	 * based on the element type, element ID, current element state and other properties.
	 */
	class B3D_EXPORT GUIStyleSheet : public Resource, public std::enable_shared_from_this<GUIStyleSheet>
	{
	public:
		GUIStyleSheet(TArray<GUIStyleSheetRuleset> rulesets = {});
		~GUIStyleSheet() override = default;

		/**
		 * Builds the appropriate ruleset to use for a particular GUI element. This looks up relevant rulesets
		 * based on GUI elements type and optionally its class, id, parent elements and provided pseudo-element name.
		 */
		SPtr<const GUIStyleSheetRuleset> BuildRuleset(const GUIRenderable& guiElement, StringView pseudoElement = "", StringView pseudoClass = "", const GUIStyleSheetRules* inheritedRules = nullptr) const;

		/**
		 * Builds the appropriate ruleset that matches a GUI element with the provided type/class/id/pseud-class/pseudo-element. Any of the provided
		 * selectors may be empty, in which case they will be ignored* in the lookup.
		 */
		SPtr<const GUIStyleSheetRuleset> BuildRuleset(StringView elementType, StringView elementClass = "", StringView elementId = "", StringView pseudoElement = "", StringView pseudoClass = "", const GUIStyleSheetRules* inheritedRules = nullptr) const;

		/**
		 * Similar to BuildRuleset(), except it matches all pseudo-classes that match dynamic GUI element states (e.g. hover, active, focused, etc.).
		 * Rulesets specific for a particular state can then be retrieved from GUIStyleSheetStateRulesets in a separate step. This allows you to
		 * skip calling expensive BuildRuleset() whenever GUI element state changes.
		 */
		SPtr<const GUIStyleSheetStateRulesets> BuildStateRulesets(const GUIRenderable& guiElement, StringView pseudoElement = "") const;

		/** Returns all rulesets stored in the stylesheet. */
		const TArray<GUIStyleSheetRuleset>& GetRulesets() const { return mRulesets; }

		/**
		 * Checks if the style sheet has a ruleset for this particular class.
		 *
		 * @param elementClass		Class name to check.
		 * @param elementType		Optional name of the GUI element type. If not empty, only classes matching this element will be considered.
		 * @return					True if there is at least one ruleset for the class.
		 */
		bool HasRulesetForClass(StringView elementClass, StringView elementType = "") const;

		/** Attempts to parse the provided style sheet file and outputs the parsed style sheet, if successful. */
		static HGUIStyleSheet Parse(const Path& file);
		// TODO - Add LoadOrParse() method that attempts to lookup an existing style sheet from PersistentCache first

		/** Creates a new style sheet. */
		static HGUIStyleSheet Create(TArray<GUIStyleSheetRuleset> rulesets = {});

		/** Creates a new style sheet. */
		static SPtr<GUIStyleSheet> CreateShared(TArray<GUIStyleSheetRuleset> rulesets = {});

		/** Creates a new style sheet without calling Initialize(). Caller must manually call Initialize(). */
		static SPtr<GUIStyleSheet> CreateUninitialized(TArray<GUIStyleSheetRuleset> rulesets = {});
	private:
		friend class GUIStyleSheetParser;

		void Initialize() override;

		/**
		 * Builds a list of all potential ruleset indices for the specified GUI element, using the ruleset lookup map.
		 * Note it's guaranteed that rulesets not in the set will not be a match for the provided GUI element, but
		 * rulesets in the set don't necessarily need to match - the caller needs to check for a match explicitly.
		 */
		void PopulatePotentialRulesetIndices(const GUIRenderable& guiElement, FrameSet<u32>& outOrderedRulesetIndices) const;

		/**
		 * Builds a list of all potential ruleset indices for a GUI element with specified type, class and/or id selectors,
		 * using the ruleset lookup map.* Note it's guaranteed that rulesets not in the set will not be a match for the provided
		 * type/class/id, but rulesets in the set don't necessarily need to match - the caller needs to check for a match explicitly.
		 */
		void PopulatePotentialRulesetIndices(const StringView& elementSelector, const StringView& classSelector, const StringView& idSelector, FrameSet<u32>& outOrderedRulesetIndices) const;

		/** Builds a string that can be used for looking up narrowed list of rulesets matching every one of the provided selectors. */
		static String BuildCacheLookupName(StringView idSelector, StringView classSelector, StringView elementSelector);

		/** List of rules in a GUI style sheet. */
		struct GUIStyleSheetRulesetList
		{
			TArray<u32> RulesetIndices; /**< Maps into the mRulesets array in GUIStyleSheet. */ 
		};

		TArray<GUIStyleSheetRuleset> mRulesets;

		mutable UnorderedMap<String, GUIStyleSheetRulesetList> mRulesetLookupMap; /**< Map to avoid iterating over entire mRuleset array. */
		mutable TSharedUnorderedSet<GUIStyleSheetStateRulesets> mCachedStateRulesets; /**< Cached state rulesets to avoid re-building them on every call. */ 

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
