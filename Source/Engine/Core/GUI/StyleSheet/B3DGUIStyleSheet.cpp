//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/StyleSheet/B3DGUIStyleSheet.h"
#include "B3DGUIStyleSheetParser.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"
#include "RTTI/B3DGUIStyleSheetRTTI.h"
#include "Resources/B3DBuiltinResources.h"
#include "Resources/B3DResources.h"

using namespace b3d;

bool GUIStyleSheetSelector::IsMatching(const GUIRenderable& element, StringView pseudoElement, StringView pseudoClass, bool ignorePseudoClass) const
{
	return IsMatching(element.GetStyleSheetElement(), element.GetStyleSheetClass(), element.GetStyleSheetId(), pseudoElement, pseudoClass, ignorePseudoClass);
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
	for(auto it = Selectors.rbegin(); it != Selectors.rend();)
	{
		const GUIStyleSheetSelector& selector = *it;

		if(selector.CombinatorType == GUIStyleSheetCombinatorType::AncestorOf || selector.CombinatorType == GUIStyleSheetCombinatorType::ParentOf)
		{
			// Find next parent renderable
			const GUIElement* parent = currentElement;
			do
			{
				parent = parent->GetParent();
				currentElement = B3DRTTICast<GUIRenderable>(parent);

				if(currentElement != nullptr)
					break;

			} while(parent != nullptr);

			isMatchingAnyAncestor = selector.CombinatorType == GUIStyleSheetCombinatorType::AncestorOf;
			itLastAncestorSelectorStart = it;

			// No parent/ancestor, so not chance for a match
			if(currentElement == nullptr)
				return false;
		}

		if(!selector.IsMatching(*currentElement, pseudoElement, pseudoClass, ignorePseudoClass))
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
			}

			stringStream << entry.Name;
		}

		mCachedUniqueName = stringStream.str();
	}

	return mCachedUniqueName;
}

SPtr<GUIStyleSheetRules> GUIStyleSheetRules::kDefault = B3DMakeShared<GUIStyleSheetRules>();

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
	OVERRIDE_PROPERTY(FontFamily, Font)
	OVERRIDE_PROPERTY(FontSize, FontSize)
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

SPtr<const GUIStyleSheetRuleset> GUIStyleSheetRuleset::kDefault = B3DMakeShared<GUIStyleSheetRuleset>();

RTTIType* GUIStyleSheetRuleset::GetRttiStatic()
{
	return GUIStyleSheetRulesetRTTI::Instance();
}

RTTIType* GUIStyleSheetRuleset::GetRtti() const
{
	return GetRttiStatic();
}

SPtr<const GUIStyleSheetStateRulesets> GUIStyleSheetStateRulesets::kDefault = B3DMakeShared<GUIStyleSheetStateRulesets>();

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
		const SPtr<const GUIStyleSheet> myStyleSheet = StyleSheets[styleSheetIndex].StyleSheet.lock();
		const SPtr<const GUIStyleSheet> otherStyleSheet = other.StyleSheets[styleSheetIndex].StyleSheet.lock();

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
		const SPtr<const GUIStyleSheet> styleSheet = entry.StyleSheet.lock();

		B3DCombineHash(hash, (u64)styleSheet.get());

		for(u32 index : entry.RulesetIndices)
			B3DCombineHash(hash, index);
	}

	return hash;
}

SPtr<const GUIStyleSheetRuleset> GUIStyleSheetStateRulesets::BuildStateRuleset(GUIElementStateFlags state, const GUIStyleSheetRules* inheritedRules) const
{
	RulesetKey key(state, (u64)inheritedRules);

	auto found = mCachedRulesets.find(key);
	if(found != mCachedRulesets.end())
		return found->second;

	SPtr<GUIStyleSheetRuleset> outputRuleset = B3DMakeShared<GUIStyleSheetRuleset>();
	if(inheritedRules)
		outputRuleset->Rules = *inheritedRules;

	for(const auto& entry : StyleSheets)
	{
		SPtr<const GUIStyleSheet> styleSheet = entry.StyleSheet.lock();
		if(styleSheet == nullptr)
			continue;

		const TArray<GUIStyleSheetRuleset>& rulesets = styleSheet->GetRulesets();
		for(u32 rulesetIndex : entry.RulesetIndices)
		{
			const GUIStyleSheetRuleset& ruleset = rulesets[rulesetIndex];

			bool allPseudoClassSelectorsMatching = true;
			for(const auto& selector : ruleset.SelectorList.Selectors)
			{
				// Non-pseudo class selectors are already matching
				if(selector.SelectorType != GUIStyleSheetSelectorType::PseudoClass)
					continue;

				bool foundMatchingPseudoClass = false;
	#define B3D_MATCH_SELECTOR(EnumValue, SelectorName) if(state.IsSet(GUIElementStateFlag::EnumValue)) \
				{\
					if(selector.Name == #SelectorName)\
					{\
						foundMatchingPseudoClass = true;\
					}\
				}

				B3D_MATCH_SELECTOR(Normal, normal)
				B3D_MATCH_SELECTOR(Hover, hover)
				B3D_MATCH_SELECTOR(Active, active)
				B3D_MATCH_SELECTOR(Focus, focus)
				B3D_MATCH_SELECTOR(Disabled, disabled)
				B3D_MATCH_SELECTOR(Checked, checked)
	#undef B3D_MATCH_SELECTOR

				if(!foundMatchingPseudoClass)
				{
					allPseudoClassSelectorsMatching = false;
					break;
				}
			}

			if(allPseudoClassSelectorsMatching)
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
	std::stable_sort(mRulesets.begin(), mRulesets.end(), [this](const GUIStyleSheetRuleset& lhs, const GUIStyleSheetRuleset& rhs) {
		const u32 specifityLHS = lhs.SelectorList.CalculateSpecificity();
		const u32 specifityRHS = rhs.SelectorList.CalculateSpecificity();

		return specifityLHS < specifityRHS;
	});

	// Generate ruleset lookup based on the right-most element, class and id, to avoid iterating over all the rulesets
	for(u32 rulesetIndex = 0; rulesetIndex < (u32)mRulesets.size(); ++rulesetIndex)
	{
		GUIStyleSheetRuleset& ruleset = mRulesets[rulesetIndex];

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
			}
		}

		const String cacheLookupName = BuildCacheLookupName(idSelector, classSelector, elementSelector);
		mRulesetLookupMap[cacheLookupName].RulesetIndices.Add(rulesetIndex);
	}
}

HGUIStyleSheet GUIStyleSheet::Parse(const Path& file)
{
	const SPtr<DataStream> fileStream = FileSystem::OpenFile(file);
	if(!fileStream)
		return nullptr;

	GUIStyleSheetParser parser;
	SPtr<GUIStyleSheet> styleSheet = parser.Parse(B3DMakeShared<SourceCode>(fileStream->GetAsString()));

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
	StringView idSelector = guiElement.GetStyleSheetId();
	StringView classSelector = guiElement.GetStyleSheetClass();
	StringView elementSelector = guiElement.GetStyleSheetElement();

	PopulatePotentialRulesetIndices(elementSelector, classSelector, idSelector, outOrderedRulesetIndices);
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
	const SPtr<GUIStyleSheet> newStyleSheet = CreateShared(std::move(rulesets));

	return B3DStaticResourceCast<GUIStyleSheet>(GetResources().CreateResourceHandle(newStyleSheet));
}

SPtr<GUIStyleSheet> GUIStyleSheet::CreateShared(TArray<GUIStyleSheetRuleset> rulesets)
{
	SPtr<GUIStyleSheet> newStyleSheet = CreateUninitialized(rulesets);
	newStyleSheet->Initialize();

	return newStyleSheet;
}

SPtr<GUIStyleSheet> GUIStyleSheet::CreateUninitialized(TArray<GUIStyleSheetRuleset> rulesets)
{
	SPtr<GUIStyleSheet> newStyleSheet = B3DMakeSharedFromExisting<GUIStyleSheet>(new(B3DAllocate<GUIStyleSheet>()) GUIStyleSheet(std::move(rulesets)));
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

const GUIStyleSheetCascade GUIStyleSheetCascade::kEmpty = GUIStyleSheetCascade();

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

SPtr<const GUIStyleSheetStateRulesets> GUIStyleSheetCascade::BuildStateRulesets(const GUIRenderable& guiElement, StringView pseudoElement) const
{
	thread_local SPtr<GUIStyleSheetStateRulesets> tlLookupValue = B3DMakeShared<GUIStyleSheetStateRulesets>();
	const static SPtr<GUIStyleSheetStateRulesets> kEmpty = B3DMakeShared<GUIStyleSheetStateRulesets>();

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
		return *foundCacheEntry;

	SPtr<GUIStyleSheetStateRulesets> newStateRulesets = B3DMakeShared<GUIStyleSheetStateRulesets>();
	newStateRulesets->StyleSheets = std::move(tlLookupValue->StyleSheets);

	mCachedStateRulesets.insert(newStateRulesets);
	return newStateRulesets;
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
	if(foundEqualImportanceEntry)
		return;

	StyleSheetWithImportance entry;
	entry.StyleSheet = styleSheet;
	entry.Importance = importance;

	mStyleSheets.Add(entry);
	std::sort(mStyleSheets.begin(), mStyleSheets.end(), [](const StyleSheetWithImportance& lhs, const StyleSheetWithImportance& rhs) {
		return lhs.Importance < rhs.Importance; // Lower importance comes first, so when we iterate over the list we override higher with lower importance items
	});
}

