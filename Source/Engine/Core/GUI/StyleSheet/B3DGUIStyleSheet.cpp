//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/StyleSheet/B3DGUIStyleSheet.h"
#include "B3DGUIStyleSheetParser.h"
#include "Debug/B3DDebug.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"
#include "RTTI/B3DGUIStyleSheetRTTI.h"
#include "Resources/B3DBuiltinResources.h"
#include "Resources/B3DResources.h"
#include "Text/B3DFontFamily.h"

using namespace b3d;

const HFont& GUIStyleSheetRules::GetFont() const
{
	static const HFont kNoFont;

	if(FontFamily == nullptr || !FontFamily.IsLoaded(false))
		return kNoFont;

	return FontFamily->GetFace(FontFaceStyle(FontWeight, FontSlant));
}

bool GUIStyleSheetSelector::IsMatching(const GUIRenderable& element, StringView pseudoElement, StringView pseudoClass, bool ignorePseudoClass) const
{
	// Elements can carry multiple classes, so the class selector has to be checked against all of them rather than going
	// through the single-value overload
	if(SelectorType == GUIStyleSheetSelectorType::Class)
	{
		if(Name.empty())
			return false;

		for(const auto& elementClass : element.GetStyleSheetClasses())
		{
			if(Name == elementClass)
				return true;
		}

		return false;
	}

	return IsMatching(element.GetStyleSheetElement(), StringView(), element.GetStyleSheetId(), pseudoElement, pseudoClass, ignorePseudoClass);
}

bool GUIStyleSheetSelector::TryGetPseudoClassState(StringView name, GUIElementStates& outState)
{
#define B3D_MATCH_PSEUDO_CLASS(SelectorName, EnumValue) if(name == #SelectorName) { outState = GUIElementState::EnumValue; return true; }

	B3D_MATCH_PSEUDO_CLASS(normal, Normal)
	B3D_MATCH_PSEUDO_CLASS(hover, Hover)
	B3D_MATCH_PSEUDO_CLASS(active, Active)
	B3D_MATCH_PSEUDO_CLASS(focus, Focus)
	B3D_MATCH_PSEUDO_CLASS(disabled, Disabled)
	B3D_MATCH_PSEUDO_CLASS(checked, Checked)

#undef B3D_MATCH_PSEUDO_CLASS

	outState = GUIElementState::Normal;
	return false;
}

bool GUIStyleSheetSelector::IsMatching(StringView elementType, StringView elementClass, StringView elementId, StringView pseudoElement, StringView pseudoClass, bool ignorePseudoClass) const
{
	if(Name.empty())
		return false;

	switch(SelectorType)
	{
	case GUIStyleSheetSelectorType::Element:
		if(elementType.empty())
			return false;

		return Name == elementType;
	case GUIStyleSheetSelectorType::Class:
		return Name == elementClass;
	case GUIStyleSheetSelectorType::Id:
		return Name == elementId;
	case GUIStyleSheetSelectorType::PseudoElement:
		return Name == pseudoElement;
	case GUIStyleSheetSelectorType::PseudoClass:
		return Name == pseudoClass || ignorePseudoClass;
	default:
		return false;
	}
}

bool GUIStyleSheetSelectorList::IsMatching(const GUIRenderable& element, StringView pseudoElement, StringView pseudoClass, bool ignorePseudoClass) const
{
	if(Selectors.Empty())
		return true; // Empty list matches everything

	const GUIRenderable* currentElement = &element;
	auto itLastAncestorSelectorStart = Selectors.rbegin();
	bool isMatchingAnyAncestor = false;
	bool isMatchingAncestor = false;
	for(auto it = Selectors.rbegin(); it != Selectors.rend();)
	{
		const GUIStyleSheetSelector& selector = *it;

		if(selector.CombinatorType == GUIStyleSheetCombinatorType::AncestorOf || selector.CombinatorType == GUIStyleSheetCombinatorType::ParentOf)
		{
			currentElement = currentElement->GetStyleParent();

			isMatchingAnyAncestor = selector.CombinatorType == GUIStyleSheetCombinatorType::AncestorOf;
			isMatchingAncestor = true;
			itLastAncestorSelectorStart = it;

			// No parent/ancestor, so not chance for a match
			if(currentElement == nullptr)
				return false;
		}

		// Pseudo-classes on the element the ruleset applies to are resolved later, per state, so that one match can serve
		// every state the element passes through. Pseudo-classes on an ancestor have no such per-state lookup, so they are
		// resolved here, against the state of the ancestor that was actually matched.
		const bool isAncestorStateSelector = isMatchingAncestor && selector.SelectorType == GUIStyleSheetSelectorType::PseudoClass;

		bool isSelectorMatching;
		if(isAncestorStateSelector)
		{
			GUIElementStates requiredState;
			isSelectorMatching = GUIStyleSheetSelector::TryGetPseudoClassState(selector.Name, requiredState) &&
				GUIStyleSheetStateRulesets::NormalizeState(currentElement->GetStateFlags()).IsSetAll(requiredState);
		}
		else
			isSelectorMatching = selector.IsMatching(*currentElement, pseudoElement, pseudoClass, ignorePseudoClass);

		if(!isSelectorMatching)
		{
			// If we're doing an ancestor match, we reset the search to the first selector and try with an ancestor one level higher
			if(isMatchingAnyAncestor)
			{
				it = itLastAncestorSelectorStart;
				continue;
			}
			else // Otherwise, no match
				return false;
		}

		++it;
	}

	return true;
}

bool GUIStyleSheetSelectorList::IsMatching(StringView elementType, StringView elementClass, StringView elementId, StringView pseudoElement, StringView pseudoClass, bool ignorePseudoClass) const
{
	if(Selectors.Empty())
		return true; // Empty list matches everything

	for(auto it = Selectors.rbegin(); it != Selectors.rend();)
	{
		const GUIStyleSheetSelector& selector = *it;

		// No parent/ancestor, so not chance for a match
		if(selector.CombinatorType == GUIStyleSheetCombinatorType::AncestorOf || selector.CombinatorType == GUIStyleSheetCombinatorType::ParentOf)
			return false;

		if(!selector.IsMatching(elementType, elementClass, elementId, pseudoElement, pseudoClass, ignorePseudoClass))
			return false;

		++it;
	}

	return true;
}

u32 GUIStyleSheetSelectorList::CalculateSpecificity() const
{
	// Following the rules from https://developer.mozilla.org/en-US/docs/Web/CSS/Specificity
	u32 specificity = 0;
	for(const auto& selector : Selectors)
	{
		switch(selector.SelectorType)
		{
		case GUIStyleSheetSelectorType::Element:
		case GUIStyleSheetSelectorType::PseudoElement:
			specificity += 1;
			break;
		case GUIStyleSheetSelectorType::Class:
		case GUIStyleSheetSelectorType::PseudoClass:
			specificity += 100;
			break;
		case GUIStyleSheetSelectorType::Id:
			specificity += 10000;
			break;
		default:
			break;
		}
	}

	return specificity;
}

bool GUIStyleSheetSelectorList::TryGetSubjectStates(GUIElementStates& outStates) const
{
	outStates = GUIElementState::Normal;
	for(auto it = Selectors.rbegin(); it != Selectors.rend(); ++it)
	{
		const GUIStyleSheetSelector& selector = *it;

		// The combinator is stored on the right-most selector of the ancestor compound, so that selector and everything to
		// its left already belong to an ancestor
		if(selector.CombinatorType != GUIStyleSheetCombinatorType::None)
			break;

		if(selector.SelectorType != GUIStyleSheetSelectorType::PseudoClass)
			continue;

		GUIElementStates state;
		if(!GUIStyleSheetSelector::TryGetPseudoClassState(selector.Name, state))
			return false;

		outStates |= state;
	}

	return true;
}

const String& GUIStyleSheetSelectorList::GetUniqueName() const
{
	if(mCachedUniqueName.empty())
	{
		StringStream stringStream;
		for(const auto& entry : Selectors)
		{
			switch(entry.SelectorType)
			{
			case GUIStyleSheetSelectorType::Class:
				stringStream << ".";
				break;
			case GUIStyleSheetSelectorType::Id:
				stringStream << "#";
				break;
			case GUIStyleSheetSelectorType::PseudoElement:
				stringStream << "::";
				break;
			case GUIStyleSheetSelectorType::PseudoClass:
				stringStream << ":";
				break;
			default:
				break;
			}

			stringStream << entry.Name;
		}

		mCachedUniqueName = stringStream.str();
	}

	return mCachedUniqueName;
}

TShared<GUIStyleSheetRules> GUIStyleSheetRules::kDefault = B3DMakeShared<GUIStyleSheetRules>();

GUIStyleSheetRules::GUIStyleSheetRules()
{
	OverridenProperties.Resize((u32)GUIStyleSheetPropertyType::Count);
}

void GUIStyleSheetRules::Override(const GUIStyleSheetRules& other)
{
#define OVERRIDE_PROPERTY(PropertyName, FieldName)                                \
	if(other.OverridenProperties[(u32)GUIStyleSheetPropertyType::PropertyName])   \
	{                                                                             \
		(FieldName) = other.FieldName;                                            \
		OverridenProperties[(u32)GUIStyleSheetPropertyType::PropertyName] = true; \
	}

	OVERRIDE_PROPERTY(Width, Size.Width)
	OVERRIDE_PROPERTY(Height, Size.Height)
	OVERRIDE_PROPERTY(MinWidth, MinimumSize.Width)
	OVERRIDE_PROPERTY(MinHeight, MinimumSize.Height)
	OVERRIDE_PROPERTY(MaxWidth, MaximumSize.Width)
	OVERRIDE_PROPERTY(MaxHeight, MaximumSize.Height)

	OVERRIDE_PROPERTY(MarginTop, Margins.Top)
	OVERRIDE_PROPERTY(MarginBottom, Margins.Bottom)
	OVERRIDE_PROPERTY(MarginLeft, Margins.Left)
	OVERRIDE_PROPERTY(MarginRight, Margins.Right)

	OVERRIDE_PROPERTY(PaddingTop, Padding.Top)
	OVERRIDE_PROPERTY(PaddingBottom, Padding.Bottom)
	OVERRIDE_PROPERTY(PaddingLeft, Padding.Left)
	OVERRIDE_PROPERTY(PaddingRight, Padding.Right)

	OVERRIDE_PROPERTY(Color, Color)
	OVERRIDE_PROPERTY(Opacity, Opacity)
	OVERRIDE_PROPERTY(BackgroundColor, BackgroundColor)

	OVERRIDE_PROPERTY(BackgroundImage, BackgroundImage)

	OVERRIDE_PROPERTY(Visibility, Visibility)

	OVERRIDE_PROPERTY(TextAlign, HorizontalTextAlignment)
	OVERRIDE_PROPERTY(VerticalAlign, VerticalTextAlignment)
	OVERRIDE_PROPERTY(FontFamily, FontFamily)
	OVERRIDE_PROPERTY(FontSize, FontSize)
	OVERRIDE_PROPERTY(FontWeight, FontWeight)
	OVERRIDE_PROPERTY(FontStyle, FontSlant)
	OVERRIDE_PROPERTY(LineHeight, LineHeight)
	OVERRIDE_PROPERTY(LetterSpacing, LetterSpacing)
	OVERRIDE_PROPERTY(WordWrap, WordWrap)

	OVERRIDE_PROPERTY(BorderTopStyle, BorderTop.Style)
	OVERRIDE_PROPERTY(BorderTopWidth, BorderTop.Width)
	OVERRIDE_PROPERTY(BorderTopColor, BorderTop.Color)

	OVERRIDE_PROPERTY(BorderBottomStyle, BorderBottom.Style)
	OVERRIDE_PROPERTY(BorderBottomWidth, BorderBottom.Width)
	OVERRIDE_PROPERTY(BorderBottomColor, BorderBottom.Color)

	OVERRIDE_PROPERTY(BorderLeftStyle, BorderLeft.Style)
	OVERRIDE_PROPERTY(BorderLeftWidth, BorderLeft.Width)
	OVERRIDE_PROPERTY(BorderLeftColor, BorderLeft.Color)

	OVERRIDE_PROPERTY(BorderRightStyle, BorderRight.Style)
	OVERRIDE_PROPERTY(BorderRightWidth, BorderRight.Width)
	OVERRIDE_PROPERTY(BorderRightColor, BorderRight.Color)

	OVERRIDE_PROPERTY(BorderTopLeftRadius, BorderTopLeftRadius);
	OVERRIDE_PROPERTY(BorderTopRightRadius, BorderTopRightRadius);
	OVERRIDE_PROPERTY(BorderBottomLeftRadius, BorderBottomLeftRadius);
	OVERRIDE_PROPERTY(BorderBottomRightRadius, BorderBottomRightRadius);

#undef OVERRIDE_PROPERTY
}

RTTIType* GUIStyleSheetRules::GetRttiStatic()
{
	return GUIStyleSheetRuleRTTI::Instance();
}

RTTIType* GUIStyleSheetRules::GetRtti() const
{
	return GetRttiStatic();
}

TShared<const GUIStyleSheetRuleset> GUIStyleSheetRuleset::kDefault = B3DMakeShared<GUIStyleSheetRuleset>();

RTTIType* GUIStyleSheetRuleset::GetRttiStatic()
{
	return GUIStyleSheetRulesetRTTI::Instance();
}

RTTIType* GUIStyleSheetRuleset::GetRtti() const
{
	return GetRttiStatic();
}

TShared<const GUIStyleSheetStateRulesets> GUIStyleSheetStateRulesets::kDefault = B3DMakeShared<GUIStyleSheetStateRulesets>();

u64 GUIStyleSheetStateRulesets::RulesetKey::GenerateHash() const
{
	u64 hash = 0;
	B3DCombineHash(hash, StateFlags);
	B3DCombineHash(hash, InheritedStateId);

	return hash;
}

bool GUIStyleSheetStateRulesets::operator==(const GUIStyleSheetStateRulesets& other) const
{
	if(StyleSheets.Size() != other.StyleSheets.Size())
		return false;

	for(u32 styleSheetIndex = 0; styleSheetIndex < (u32)StyleSheets.Size(); ++styleSheetIndex)
	{
		const TShared<const GUIStyleSheet> myStyleSheet = StyleSheets[styleSheetIndex].StyleSheet.lock();
		const TShared<const GUIStyleSheet> otherStyleSheet = other.StyleSheets[styleSheetIndex].StyleSheet.lock();

		const TArray<u32>& myRulesetIndices = StyleSheets[styleSheetIndex].RulesetIndices;
		const TArray<u32>& otherRulesetIndices = other.StyleSheets[styleSheetIndex].RulesetIndices;

		if(myStyleSheet != otherStyleSheet)
			return false;

		if(myRulesetIndices.Size() != otherRulesetIndices.Size())
			return false;

		for(u32 rulesetIndex = 0; rulesetIndex < (u32)myRulesetIndices.Size(); ++rulesetIndex)
		{
			if(myRulesetIndices[rulesetIndex] != otherRulesetIndices[rulesetIndex])
				return false;
		}
	}

	return true;
}

u64 GUIStyleSheetStateRulesets::GenerateHash() const
{
	u64 hash = 0;

	for(const auto& entry : StyleSheets)
	{
		const TShared<const GUIStyleSheet> styleSheet = entry.StyleSheet.lock();

		B3DCombineHash(hash, (u64)styleSheet.get());

		for(u32 index : entry.RulesetIndices)
			B3DCombineHash(hash, index);
	}

	return hash;
}

GUIElementStates GUIStyleSheetStateRulesets::NormalizeState(GUIElementStates state)
{
	// A disabled element cannot be interacted with, so any lingering interaction state must not contribute to its look
	if(state.IsSet(GUIElementState::Disabled))
		state &= ~kGUIInteractionStates;

	return state;
}

TShared<const GUIStyleSheetRuleset> GUIStyleSheetStateRulesets::BuildStateRuleset(GUIElementStates state, const GUIStyleSheetRules* inheritedRules) const
{
	state = NormalizeState(state);

	RulesetKey key(state, (u64)inheritedRules);

	auto found = mCachedRulesets.find(key);
	if(found != mCachedRulesets.end())
		return found->second;

	TShared<GUIStyleSheetRuleset> outputRuleset = B3DMakeShared<GUIStyleSheetRuleset>();
	if(inheritedRules)
		outputRuleset->Rules = *inheritedRules;

	for(const auto& entry : StyleSheets)
	{
		TShared<const GUIStyleSheet> styleSheet = entry.StyleSheet.lock();
		if(styleSheet == nullptr)
			continue;

		const TArray<GUIStyleSheetRuleset>& rulesets = styleSheet->GetRulesets();
		for(u32 rulesetIndex : entry.RulesetIndices)
		{
			const GUIStyleSheetRuleset& ruleset = rulesets[rulesetIndex];

			// Only the pseudo-classes on the element the ruleset applies to are tested here. Pseudo-classes on ancestor
			// selectors were already resolved while matching, so they must not be re-tested against this element's state.
			GUIElementStates subjectStates;
			if(!ruleset.SelectorList.TryGetSubjectStates(subjectStates) || !state.IsSetAll(subjectStates))
				continue;

			outputRuleset->Rules.Override(ruleset.Rules);
		}
	}

	mCachedRulesets[key] = outputRuleset;
	return outputRuleset;
}

GUIStyleSheet::GUIStyleSheet(TArray<GUIStyleSheetRuleset> rulesets)
	: Resource(false, "StyleSheet"), mRulesets(std::move(rulesets))
{ }

void GUIStyleSheet::Initialize()
{
	// Sort from least specific to more specific (so more specific properties override the less specific ones)
	std::stable_sort(mRulesets.begin(), mRulesets.end(), [](const GUIStyleSheetRuleset& lhs, const GUIStyleSheetRuleset& rhs) {
		const u32 specifityLHS = lhs.SelectorList.CalculateSpecificity();
		const u32 specifityRHS = rhs.SelectorList.CalculateSpecificity();

		return specifityLHS < specifityRHS;
	});

	// Generate ruleset lookup based on the right-most element, class and id, to avoid iterating over all the rulesets
	for(u32 rulesetIndex = 0; rulesetIndex < (u32)mRulesets.size(); ++rulesetIndex)
	{
		const GUIStyleSheetRuleset& ruleset = mRulesets[rulesetIndex];

		StringView idSelector;
		StringView classSelector;
		StringView elementSelector;
		for(auto it = ruleset.SelectorList.Selectors.rbegin(); it != ruleset.SelectorList.Selectors.rend(); ++it)
		{
			const GUIStyleSheetSelector& selector = *it;

			// Reached a combinator, we can't do caching based on those selectors
			if(selector.CombinatorType != GUIStyleSheetCombinatorType::None)
				break;
			
			switch(selector.SelectorType)
			{
			case GUIStyleSheetSelectorType::Element:
				if(elementSelector.empty())
					elementSelector = selector.Name;
				break;
			case GUIStyleSheetSelectorType::Class:
				if(classSelector.empty())
					classSelector = selector.Name;
				break;
			case GUIStyleSheetSelectorType::Id:
				if(idSelector.empty())
					idSelector = selector.Name;
				break;
			default:
				break;
			}
		}

		const String cacheLookupName = BuildCacheLookupName(idSelector, classSelector, elementSelector);
		mRulesetLookupMap[cacheLookupName].RulesetIndices.Add(rulesetIndex);
	}
}

HGUIStyleSheet GUIStyleSheet::Parse(const Path& file)
{
	const TShared<DataStream> fileStream = FileSystem::OpenFile(file);
	if(!fileStream)
		return nullptr;

	GUIStyleSheetParser parser;
	TShared<GUIStyleSheet> styleSheet = parser.Parse(B3DMakeShared<SourceCode>(fileStream->GetAsString()));

	const String warnings = parser.GetWarnings();
	if(!warnings.empty())
		B3D_LOG(Warning, LogGUI, "Style sheet \"{0}\":\n{1}", file.ToString(), warnings);

	if(styleSheet == nullptr)
	{
		B3D_LOG(Error, LogGUI, "Failed parsing style sheet \"{0}\": {1}", file.ToString(), parser.GetErrors());
		return nullptr;
	}

	return B3DStaticResourceCast<GUIStyleSheet>(GetResources().CreateResourceHandle(styleSheet));
}

String GUIStyleSheet::BuildCacheLookupName(StringView idSelector, StringView classSelector, StringView elementSelector)
{
	String cacheLookupName;

	// Cache based on the most specific selector
	if(!idSelector.empty())
	{
		cacheLookupName += "#";
		cacheLookupName += idSelector;
	}
	else if(!classSelector.empty())
	{
		cacheLookupName += ".";
		cacheLookupName += classSelector;
	}
	else if(!elementSelector.empty())
	{
		if(!cacheLookupName.empty())
			cacheLookupName += " ";

		cacheLookupName += elementSelector;
	}

	return cacheLookupName;
}
 
GUIStyleSheetRules GUIStyleSheet::BuildRules(const GUIRenderable& guiElement, StringView pseudoElement, StringView pseudoClass, const GUIStyleSheetRules* inheritedRules) const
{
	// Note: Not supporting multiple pseudo classes at the moment

	GUIStyleSheetRules outputRules;
	if(inheritedRules)
		outputRules = *inheritedRules;

	FrameAllocatorScope frameScope;
	FrameSet<u32> sortedRulesetIndices;

	PopulatePotentialRulesetIndices(guiElement, sortedRulesetIndices);

	for(u32 rulesetIndex : sortedRulesetIndices)
	{
		const GUIStyleSheetRuleset& ruleset = mRulesets[rulesetIndex];

		if(ruleset.SelectorList.IsMatching(guiElement, pseudoElement, pseudoClass))
			outputRules.Override(ruleset.Rules);
	}

	return outputRules;
}

GUIStyleSheetRules GUIStyleSheet::BuildRules(StringView elementType, StringView elementClass, StringView elementId, StringView pseudoElement, StringView pseudoClass, const GUIStyleSheetRules* inheritedRules) const
{
	GUIStyleSheetRules outputRules;
	if(inheritedRules)
		outputRules = *inheritedRules;

	FrameAllocatorScope frameScope;
	FrameSet<u32> sortedRulesetIndices;

	PopulatePotentialRulesetIndices(elementType, elementClass, elementId, sortedRulesetIndices);

	for(u32 rulesetIndex : sortedRulesetIndices)
	{
		const GUIStyleSheetRuleset& ruleset = mRulesets[rulesetIndex];

		if(ruleset.SelectorList.IsMatching(elementType, elementClass, elementId, pseudoElement, pseudoClass))
			outputRules.Override(ruleset.Rules);
	}

	return outputRules;
}

bool GUIStyleSheet::HasRulesetForClass(StringView elementClass, StringView elementType) const
{
	bool foundClass = elementClass.empty();
	bool foundElement = elementType.empty();
	for(const auto& ruleset : mRulesets)
	{
		for(const auto& selector : ruleset.SelectorList.Selectors)
		{
			if(selector.SelectorType == GUIStyleSheetSelectorType::Class)
				foundClass |= selector.Name == elementClass;
			else if(selector.SelectorType == GUIStyleSheetSelectorType::Element)
				foundElement |= selector.Name == elementType;
		}
	}

	return foundElement && foundClass;
}

void GUIStyleSheet::GetMatchingRulesetIndices(const GUIRenderable& guiElement, TArray<u32>& outOrderedRulesetIndices, StringView pseudoElement, StringView pseudoClass, bool ignorePseudoClass) const
{
	FrameAllocatorScope frameScope;
	FrameSet<u32> sortedRulesetIndices;

	PopulatePotentialRulesetIndices(guiElement, sortedRulesetIndices);

	for(u32 rulesetIndex : sortedRulesetIndices)
	{
		const GUIStyleSheetRuleset& ruleset = mRulesets[rulesetIndex];

		if(ruleset.SelectorList.IsMatching(guiElement, pseudoElement, pseudoClass, ignorePseudoClass))
			outOrderedRulesetIndices.Add(rulesetIndex);
	}
}

void GUIStyleSheet::PopulatePotentialRulesetIndices(const GUIRenderable& guiElement, FrameSet<u32>& outOrderedRulesetIndices) const
{
	const StringView idSelector = guiElement.GetStyleSheetId();
	const StringView elementSelector = guiElement.GetStyleSheetElement();

	PopulatePotentialRulesetIndices(elementSelector, StringView(), idSelector, outOrderedRulesetIndices);

	// An element can carry any number of classes, each of which may bring in its own rulesets
	for(const auto& classSelector : guiElement.GetStyleSheetClasses())
		PopulatePotentialRulesetIndices(StringView(), classSelector, StringView(), outOrderedRulesetIndices);
}

void GUIStyleSheet::PopulatePotentialRulesetIndices(const StringView& elementSelector, const StringView& classSelector, const StringView& idSelector, FrameSet<u32>& outOrderedRulesetIndices) const
{
	auto fnLookupRulesets = [&outOrderedRulesetIndices, this](const String& lookupValue)
	{
		auto found = mRulesetLookupMap.find(lookupValue);
		if(found != mRulesetLookupMap.end())
		{
			for(u32 rulesetIndex : found->second.RulesetIndices)
				outOrderedRulesetIndices.insert(rulesetIndex);
		}
	};

	if(!elementSelector.empty())
		fnLookupRulesets((String)elementSelector);

	if(!classSelector.empty())
		fnLookupRulesets(String(".") + (String)classSelector);

	if(!idSelector.empty())
		fnLookupRulesets(String("#") + (String)idSelector);
}

HGUIStyleSheet GUIStyleSheet::Create(TArray<GUIStyleSheetRuleset> rulesets)
{
	const TShared<GUIStyleSheet> newStyleSheet = CreateShared(std::move(rulesets));

	return B3DStaticResourceCast<GUIStyleSheet>(GetResources().CreateResourceHandle(newStyleSheet));
}

TShared<GUIStyleSheet> GUIStyleSheet::CreateShared(TArray<GUIStyleSheetRuleset> rulesets)
{
	TShared<GUIStyleSheet> newStyleSheet = CreateUninitialized(rulesets);
	newStyleSheet->Initialize();

	return newStyleSheet;
}

TShared<GUIStyleSheet> GUIStyleSheet::CreateUninitialized(TArray<GUIStyleSheetRuleset> rulesets)
{
	TShared<GUIStyleSheet> newStyleSheet = B3DMakeSharedFromExisting<GUIStyleSheet>(new(B3DAllocate<GUIStyleSheet>()) GUIStyleSheet(std::move(rulesets)));
	newStyleSheet->SetShared(newStyleSheet);

	return newStyleSheet;
}

RTTIType* GUIStyleSheet::GetRttiStatic()
{
	return GUIStyleSheetRTTI::Instance();
}

RTTIType* GUIStyleSheet::GetRtti() const
{
	return GetRttiStatic();
}

/** Returns a cascade revision that was never returned before. See GUIStyleSheetCascade::GetRevision(). */
static u64 AllocateCascadeRevision()
{
	// Zero is reserved for style keys that were never calculated
	static std::atomic<u64> sNextRevision = 1;
	return sNextRevision++;
}

/** Returns the key GUIStyleSheetCascade indexes ancestor selectors on. */
static u64 GetAncestorSelectorLookupKey(GUIStyleSheetSelectorType selectorType, StringView name)
{
	u64 lookupKey = 0;
	B3DCombineHash(lookupKey, selectorType);
	B3DCombineHash(lookupKey, name);

	return lookupKey;
}

const GUIStyleSheetCascade GUIStyleSheetCascade::kEmpty = GUIStyleSheetCascade();

GUIStyleSheetCascade::GUIStyleSheetCascade()
	: mRevision(AllocateCascadeRevision())
{ }

 GUIStyleSheetRules GUIStyleSheetCascade::BuildRules(StringView elementType, StringView elementClass, StringView elementId, StringView pseudoElement, StringView pseudoClass, const GUIStyleSheetRules* inheritedRules) const
{
	GUIStyleSheetRules combinedRules;
	if(inheritedRules)
		combinedRules = *inheritedRules;

	for(const auto& styleSheetWithImportance : mStyleSheets)
	{
		GUIStyleSheetRules rules = styleSheetWithImportance.StyleSheet->BuildRules(elementType, elementClass, elementId, pseudoElement, pseudoClass);
		combinedRules.Override(rules);
	}

	return combinedRules;
}

GUIStyleSheetRules GUIStyleSheetCascade::BuildRules(const GUIRenderable& guiElement, StringView pseudoElement, StringView pseudoClass, const GUIStyleSheetRules* inheritedRules) const
{
	GUIStyleSheetRules combinedRules;
	if(inheritedRules)
		combinedRules = *inheritedRules;

	for(const auto& styleSheetWithImportance : mStyleSheets)
	{
		GUIStyleSheetRules rules = styleSheetWithImportance.StyleSheet->BuildRules(guiElement, pseudoElement, pseudoClass);
		combinedRules.Override(rules);
	}

	return combinedRules;
}

TShared<const GUIStyleSheetStateRulesets> GUIStyleSheetCascade::BuildStateRulesets(const GUIRenderable& guiElement, StringView pseudoElement) const
{
	thread_local TShared<GUIStyleSheetStateRulesets> tlLookupValue = B3DMakeShared<GUIStyleSheetStateRulesets>();
	const static TShared<GUIStyleSheetStateRulesets> kEmpty = B3DMakeShared<GUIStyleSheetStateRulesets>();

	mResolutionStatistics.RequestCount++;

	// Everything a selector can match on is folded into the element's style key, so two elements sharing a key always
	// resolve to the same rulesets. Toggling a style class therefore costs a hash lookup rather than a selector match.
	u64 cacheKey = guiElement.GetStyleKey();
	if(!pseudoElement.empty())
	{
		B3DCombineHash(cacheKey, GUIStyleKeyPart::PseudoElement);
		B3DCombineHash(cacheKey, pseudoElement);
	}

	if(const auto foundResolvedEntry = mResolvedStateRulesets.find(cacheKey); foundResolvedEntry != mResolvedStateRulesets.end())
		return foundResolvedEntry->second;

	mResolutionStatistics.FullResolutionCount++;

	tlLookupValue->StyleSheets.Clear();

	for(const auto& entry : mStyleSheets)
	{
		GUIStyleSheetStateRulesets::StyleSheetRulesetIndices styleSheetRulesetIndices;
		styleSheetRulesetIndices.StyleSheet = entry.StyleSheet.GetShared();

		entry.StyleSheet->GetMatchingRulesetIndices(guiElement, styleSheetRulesetIndices.RulesetIndices, pseudoElement, "", true);

		tlLookupValue->StyleSheets.Add(styleSheetRulesetIndices);
	}

	auto foundCacheEntry = mCachedStateRulesets.find(tlLookupValue);
	if(foundCacheEntry != mCachedStateRulesets.end())
	{
		mResolvedStateRulesets[cacheKey] = *foundCacheEntry;
		return *foundCacheEntry;
	}

	TShared<GUIStyleSheetStateRulesets> newStateRulesets = B3DMakeShared<GUIStyleSheetStateRulesets>();
	newStateRulesets->StyleSheets = std::move(tlLookupValue->StyleSheets);

	mCachedStateRulesets.insert(newStateRulesets);
	mResolvedStateRulesets[cacheKey] = newStateRulesets;

	return newStateRulesets;
}

void GUIStyleSheetCascade::ResetResolvedRulesetCache()
{
	mResolvedStateRulesets.clear();
	mCachedStateRulesets.clear();
}

bool GUIStyleSheetCascade::HasRulesetForClass(StringView elementClass, StringView elementType) const
{
	for(const auto& entry : mStyleSheets)
	{
		if(!B3D_ENSURE(entry.StyleSheet.IsLoaded(false)))
			continue;

		if(entry.StyleSheet->HasRulesetForClass(elementClass, elementType))
			return true;
	}

	return false;
}

void GUIStyleSheetCascade::RegisterStyleSheet(const HGUIStyleSheet& styleSheet, i32 importance)
{
	if(!B3D_ENSURE(styleSheet.IsLoaded(false)))
		return;

	// If there are style sheets registered with equal importance, we merge them together. This way when their merged rules can be sorted in a way
	// so they are picked based on specificity (which is in accordance with CSS specification).
	bool foundEqualImportanceEntry = false;
	for(auto it = mStyleSheets.begin(); it != mStyleSheets.end(); ++it)
	{
		StyleSheetWithImportance& entry = *it;
		if(entry.Importance != importance)
			continue;

		if(!B3D_ENSURE(entry.StyleSheet.IsLoaded(false)))
			continue;

		foundEqualImportanceEntry = true;

		TArray<GUIStyleSheetRuleset> combinedRulesets = entry.StyleSheet->GetRulesets();
		combinedRulesets.Append(styleSheet->GetRulesets().begin(), styleSheet->GetRulesets().end());

		HGUIStyleSheet newStyleSheet = GUIStyleSheet::Create(combinedRulesets);
		entry.StyleSheet = newStyleSheet;
	}

	// If no equal importance entry, we register a new entry and then re-sort
	if(!foundEqualImportanceEntry)
	{
		StyleSheetWithImportance entry;
		entry.StyleSheet = styleSheet;
		entry.Importance = importance;

		mStyleSheets.Add(entry);
		std::sort(mStyleSheets.begin(), mStyleSheets.end(), [](const StyleSheetWithImportance& lhs, const StyleSheetWithImportance& rhs) {
			return lhs.Importance < rhs.Importance; // Lower importance comes first, so when we iterate over the list we override higher with lower importance items
		});
	}

	// Every cached resolution and every style key was made against the old set of style sheets
	ResetResolvedRulesetCache();
	mRevision = AllocateCascadeRevision();

	// Collect the ancestor parts of every selector. The combinator is stored on the right-most selector of each ancestor
	// part, so a part spans from the previous combinator up to and including the selector holding the next one, while the
	// selectors after the last combinator describe the styled element itself.
	mAncestorSelectors.Clear();
	mAncestorSelectorLookup.clear();
	mUniversalAncestorSelectors.Clear();
	for(const auto& entry : mStyleSheets)
	{
		if(!B3D_ENSURE(entry.StyleSheet.IsLoaded(false)))
			continue;

		for(const auto& ruleset : entry.StyleSheet->GetRulesets())
		{
			AncestorSelector ancestorSelector;
			bool hasUnknownPseudoClass = false;
			for(const auto& selector : ruleset.SelectorList.Selectors)
			{
				// Pseudo-elements can only apply to the styled element, and the pseudo-element being looked up is part of
				// the style key on its own
				if(selector.SelectorType == GUIStyleSheetSelectorType::PseudoClass)
				{
					GUIElementStates pseudoClassState;
					if(GUIStyleSheetSelector::TryGetPseudoClassState(selector.Name, pseudoClassState))
						ancestorSelector.States |= pseudoClassState;
					else
						hasUnknownPseudoClass = true;
				}
				else if(selector.SelectorType != GUIStyleSheetSelectorType::PseudoElement)
					ancestorSelector.Selectors.Add(selector);

				if(selector.CombinatorType == GUIStyleSheetCombinatorType::None)
					continue;

				// A pseudo-class the runtime does not know never matches, so neither can the ancestor part holding it. Such a part
				// is not stored either, as it would compare equal to the same part without the pseudo-class.
				if(!hasUnknownPseudoClass)
				{
					ancestorSelector.IsFollowedByParentOf = selector.CombinatorType == GUIStyleSheetCombinatorType::ParentOf;

					const auto foundSelector = std::find(mAncestorSelectors.begin(), mAncestorSelectors.end(), ancestorSelector);
					if(foundSelector != mAncestorSelectors.end())
						foundSelector->IsFollowedByParentOf |= ancestorSelector.IsFollowedByParentOf;
					else
					{
						// Index the selector by whichever of its parts is the most selective. Element, class and id selector types
						// are ordered from the least to the most selective.
						const GUIStyleSheetSelector* lookupSelector = nullptr;
						for(const auto& entrySelector : ancestorSelector.Selectors)
						{
							if(lookupSelector == nullptr || (u32)entrySelector.SelectorType > (u32)lookupSelector->SelectorType)
								lookupSelector = &entrySelector;
						}

						const u32 ancestorSelectorIndex = (u32)mAncestorSelectors.Size();
						if(lookupSelector != nullptr)
							mAncestorSelectorLookup[GetAncestorSelectorLookupKey(lookupSelector->SelectorType, lookupSelector->Name)].Add(ancestorSelectorIndex);
						else
							mUniversalAncestorSelectors.Add(ancestorSelectorIndex);

						mAncestorSelectors.Add(ancestorSelector);
					}
				}

				ancestorSelector = AncestorSelector();
				hasUnknownPseudoClass = false;
			}
		}
	}
}

GUIStyleSheetAncestorMatch GUIStyleSheetCascade::MatchAncestorSelectors(const GUIRenderable& element) const
{
	GUIStyleSheetAncestorMatch match;
	if(mAncestorSelectors.Empty())
		return match;

	const GUIElementStates elementStates = GUIStyleSheetStateRulesets::NormalizeState(element.GetStateFlags());

	TInlineArray<u32, 8> matchingSelectorIndices;
	auto fnTestAncestorSelector = [&](u32 ancestorSelectorIndex)
	{
		const AncestorSelector& ancestorSelector = mAncestorSelectors[ancestorSelectorIndex];
		for(const auto& selector : ancestorSelector.Selectors)
		{
			if(!selector.IsMatching(element))
				return;
		}

		match.StatesAffectingDescendants |= ancestorSelector.States;

		if(elementStates.IsSetAll(ancestorSelector.States))
			matchingSelectorIndices.Add(ancestorSelectorIndex);
	};

	auto fnTestIndexedAncestorSelectors = [&](GUIStyleSheetSelectorType selectorType, StringView name)
	{
		const auto found = mAncestorSelectorLookup.find(GetAncestorSelectorLookupKey(selectorType, name));
		if(found == mAncestorSelectorLookup.end())
			return;

		for(u32 ancestorSelectorIndex : found->second)
			fnTestAncestorSelector(ancestorSelectorIndex);
	};

	for(u32 ancestorSelectorIndex : mUniversalAncestorSelectors)
		fnTestAncestorSelector(ancestorSelectorIndex);

	fnTestIndexedAncestorSelectors(GUIStyleSheetSelectorType::Element, element.GetStyleSheetElement());

	for(const auto& styleClass : element.GetStyleSheetClasses())
		fnTestIndexedAncestorSelectors(GUIStyleSheetSelectorType::Class, styleClass);

	const String& styleId = element.GetStyleSheetId();
	if(!styleId.empty())
		fnTestIndexedAncestorSelectors(GUIStyleSheetSelectorType::Id, styleId);

	if(matchingSelectorIndices.Empty())
		return match;

	// Lookup keys of different names can collide, in which case the same selector was tested more than once
	std::sort(matchingSelectorIndices.begin(), matchingSelectorIndices.end());
	const auto uniqueEnd = std::unique(matchingSelectorIndices.begin(), matchingSelectorIndices.end());

	for(auto it = matchingSelectorIndices.begin(); it != uniqueEnd; ++it)
	{
		B3DCombineHash(match.MatchingSelectorsHash, *it);
		match.HasParentOfMatch |= mAncestorSelectors[*it].IsFollowedByParentOf;
	}

	match.IsObservableByDescendants = true;
	return match;
}

bool GUIStyleSheetCascade::AncestorSelector::operator==(const AncestorSelector& other) const
{
	if(States != other.States || Selectors.Size() != other.Selectors.Size())
		return false;

	for(u32 selectorIndex = 0; selectorIndex < Selectors.Size(); selectorIndex++)
	{
		if(Selectors[selectorIndex].SelectorType != other.Selectors[selectorIndex].SelectorType || Selectors[selectorIndex].Name != other.Selectors[selectorIndex].Name)
			return false;
	}

	return true;
}

