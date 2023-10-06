//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/StyleSheet/BsGUIStyleSheet.h"
#include "BsGUIStyleSheetParser.h"
#include "FileSystem/BsFileSystem.h"
#include "FileSystem/BsDataStream.h"
#include "Private/RTTI/BsGUIStyleSheetRTTI.h"
#include "Resources/BsBuiltinResources.h"
#include "Resources/BsResources.h"

using namespace bs;

bool GUIStyleSheetSelector::IsMatching(const GUIElementBase& element) const
{
	if(Name.empty())
		return false;

	switch(SelectorType)
	{
	case GUIStyleSheetSelectorType::Element:
		if(!element.GetStyleSheetElement())
			return false;

		return Name == element.GetStyleSheetElement();
	case GUIStyleSheetSelectorType::Class:
		if(element.GetStyleSheetClass().empty())
			return false;

		return Name == element.GetStyleSheetClass();
	case GUIStyleSheetSelectorType::Id:
		if(element.GetStyleSheetId().empty())
			return false;

		return Name == element.GetStyleSheetId();
	default:
		return false;
	}
}

bool GUIStyleSheetSelectorList::IsMatching(const GUIElementBase& element) const
{
	if(Selectors.Empty())
		return false;

	// Last selector must be an exact match for the element we're looking for
	if(!Selectors.back().IsMatching(element))
		return false;

	// The rest of the selectors should be combinators
	GUIElementBase* currentParent = element.GetParent();
	for(auto it = Selectors.rbegin() + 1; it != Selectors.rend(); ++it)
	{
		const GUIStyleSheetSelector& selector = *it;
		if(!B3D_ENSURE(selector.CombinatorType == GUIStyleSheetCombinatorType::AncestorOf))
			continue;

		bool isFound = false;
		while(currentParent != nullptr)
		{
			if(!selector.IsMatching(*currentParent))
			{
				currentParent = currentParent->GetParent();
				continue;
			}

			isFound = true;
			break;
		}

		if(!isFound)
			return false;
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
			}

			stringStream << entry.Name;
		}

		mCachedUniqueName = stringStream.str();
	}

	return mCachedUniqueName;
}

SPtr<GUIStyleSheetStateRule> GUIStyleSheetStateRule::kDefault = B3DMakeShared<GUIStyleSheetStateRule>();

GUIStyleSheetStateRule::GUIStyleSheetStateRule()
{
	OverridenProperties.Resize((u32)GUIStyleSheetPropertyType::Count);
}

void GUIStyleSheetStateRule::Override(const GUIStyleSheetStateRule& other)
{
#define OVERRIDE_PROPERTY(PropertyName, FieldName)                              \
	if(other.OverridenProperties[(u32)GUIStyleSheetPropertyType::PropertyName]) \
		(FieldName) = other.FieldName;

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

	OVERRIDE_PROPERTY(TextAlign, HorizontalTextAlignment)
	OVERRIDE_PROPERTY(VerticalAlign, VerticalTextAlignment)
	OVERRIDE_PROPERTY(FontFamily, FontFamily)
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

HFont GUIStyleSheetStateRule::GetOrLoadFont() const
{
	// TODO - Style should be containing the HFont reference directly. Otherwise we cannot use the reference search mechanism.
	// - And we'll need the same for styles referencing textures
	// - So these should be resolved directly by the parser. We can provide an interface class that does the lookup, either in builtin resources or project library

	if(!mCachedFont.IsLoaded(false))
		mCachedFont = GetBuiltinResources().GetFont(FontFamily);

	return mCachedFont;
}

const GUIStyleSheetStateRule* GUIStyleSheetRule::FindStateStyle(const StringView& name) const
{
	if(StringUtil::Compare(name, "normal", false))
		return &Normal;

	if(StringUtil::Compare(name, "hover", false))
		return Hover.has_value() ? &Hover.value() : nullptr;

	if(StringUtil::Compare(name, "active", false))
		return Active.has_value() ? &Active.value() : nullptr;

	if(StringUtil::Compare(name, "focus", false))
		return Focus.has_value() ? &Focus.value() : nullptr;

	if(StringUtil::Compare(name, "disabled", false))
		return Disabled.has_value() ? &Disabled.value() : nullptr;

	if(StringUtil::Compare(name, "checked", false))
		return Checked.has_value() ? &Checked.value() : nullptr;

	return nullptr;
}

SPtr<GUIStyleSheetRule> GUIStyleSheetRule::kDefault = B3DMakeShared<GUIStyleSheetRule>();

bool GUIStyleSheetRule::FindAndSetStateStyle(const StringView& name, const GUIStyleSheetStateRule& stateStyle)
{
	if(StringUtil::Compare(name, "normal", false))
	{
		Normal = stateStyle;
		return true;
	}

	if(StringUtil::Compare(name, "hover", false))
	{
		Hover = stateStyle;
		return true;
	}

	if(StringUtil::Compare(name, "active", false))
	{
		Active = stateStyle;
		return true;
	}

	if(StringUtil::Compare(name, "focus", false))
	{
		Focus = stateStyle;
		return true;
	}

	if(StringUtil::Compare(name, "disabled", false))
	{
		Disabled = stateStyle;
		return true;
	}

	if(StringUtil::Compare(name, "checked", false))
	{
		Checked = stateStyle;
		return true;
	}

	return false;
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

SPtr<GUIStyleSheetStateRule> GUIStyleSheetRule::FindStateStyle(GUIElementStateFlags stateFlags) const
{
	if(auto found = mCachedStateStyles.find(stateFlags); found != mCachedStateStyles.end())
		return found->second;

	SPtr<GUIStyleSheetStateRule> stateStyle = B3DMakeShared<GUIStyleSheetStateRule>();
	*stateStyle = Normal;

	if(stateFlags.IsSet(GUIElementStateFlag::Checked))
	{
		if(Checked.has_value())
			stateStyle->Override(*Checked);
	}

	if(stateFlags.IsSet(GUIElementStateFlag::Disabled))
	{
		if(Checked.has_value())
			stateStyle->Override(*Checked);
	}
	else
	{
		if(stateFlags.IsSet(GUIElementStateFlag::Focused))
		{
			if(Focus.has_value())
				stateStyle->Override(*Focus);
		}

		if(stateFlags.IsSet(GUIElementStateFlag::Hover))
		{
			if(Hover.has_value())
				stateStyle->Override(*Hover);
		}
		
		if(stateFlags.IsSet(GUIElementStateFlag::Active))
		{
			if(Active.has_value())
				stateStyle->Override(*Active);
		}
	}

	return mCachedStateStyles.insert(std::make_pair(stateFlags, stateStyle)).first->second;
}

void GUIStyleSheetRule::Override(const GUIStyleSheetRule& other)
{
	Normal.Override(other.Normal);

	if(Hover.has_value())
	{
		if(other.Hover.has_value())
			Hover->Override(*other.Hover);
	}
	else
		Hover = other.Hover;
	
	if(Active.has_value())
	{
		if(other.Active.has_value())
			Active->Override(*other.Active);
	}
	else
		Active = other.Active;

	if(Focus.has_value())
	{
		if(other.Focus.has_value())
			Focus->Override(*other.Focus);
	}
	else
		Focus = other.Focus;

	if(Checked.has_value())
	{
		if(other.Checked.has_value())
			Checked->Override(*other.Checked);
	}
	else
		Checked = other.Checked;

	if(Disabled.has_value())
	{
		if(other.Disabled.has_value())
			Disabled->Override(*other.Disabled);
	}
	else
		Disabled = other.Disabled;
}

GUIStyleSheet::GUIStyleSheet(TArray<GUIStyleSheetRule> rules)
	: Resource(false, "StyleSheet"), mRules(std::move(rules))
{ }

void GUIStyleSheet::Initialize()
{
	RebuildCache();
}

void GUIStyleSheet::RebuildCache()
{
	for(u32 ruleIndex = 0; ruleIndex < (u32)mRules.size(); ++ruleIndex)
	{
		GUIStyleSheetRule& rule = mRules[ruleIndex];

		if(!B3D_ENSURE(!rule.SelectorList.Selectors.Empty()))
			continue;

		GUIStyleSheetSelector& lastSelector = rule.SelectorList.Selectors.back();
		switch(lastSelector.SelectorType)
		{
		case GUIStyleSheetSelectorType::Element:
			mCachedRulesByElement[lastSelector.Name].RuleIndices.Add(ruleIndex);
			break;
		case GUIStyleSheetSelectorType::Class:
			mCachedRulesByClass[lastSelector.Name].RuleIndices.Add(ruleIndex);
			break;
		case GUIStyleSheetSelectorType::Id:
			mCachedRulesById[lastSelector.Name].RuleIndices.Add(ruleIndex);
			break;
		}
	}

	auto fnSortEntries = [this](UnorderedMap<String, GUIStyleSheetRuleList>& map) {
		for(auto& pair : map)
		{
			std::sort(pair.second.RuleIndices.begin(), pair.second.RuleIndices.end(), [this](u32 lhs, u32 rhs) {
				const GUIStyleSheetRule& ruleLHS = mRules[lhs];
				const GUIStyleSheetRule& ruleRHS = mRules[rhs];

				const u32 specificityLHS = (u32)ruleLHS.SelectorList.Selectors.size();
				const u32 specificityRHS = (u32)ruleRHS.SelectorList.Selectors.size();

				return specificityLHS < specificityRHS;
			});
		}
	};

	// Sort from least specific to most specific
	fnSortEntries(mCachedRulesByElement);
	fnSortEntries(mCachedRulesByClass);
	fnSortEntries(mCachedRulesById);
}
 
SPtr<GUIStyleSheetRule> GUIStyleSheet::BuildRule(const GUIElement& guiElement) const
{
	SPtr<GUIStyleSheetRule> outputRule = B3DMakeShared<GUIStyleSheetRule>();

	// Go over rules based on specificity: rules matching element name, then rules matching class name, then rules matching id.
	// Rules within each category are already sorted from least to most specific.
	// More specific rules keep overriding the properties from less specific rules, until we build our output rule.

	if(auto it = mCachedRulesByElement.find(guiElement.GetStyleSheetElement()); it != mCachedRulesByElement.end())
	{
		for(const auto& ruleIndex : it->second.RuleIndices)
		{
			const GUIStyleSheetRule& rule = mRules[ruleIndex];
			if(!rule.SelectorList.IsMatching(guiElement))
				continue;

			outputRule->Override(rule);
		}
	}

	if(auto it = mCachedRulesByClass.find(guiElement.GetStyleSheetClass()); it != mCachedRulesByClass.end())
	{
		for(const auto& ruleIndex : it->second.RuleIndices)
		{
			const GUIStyleSheetRule& rule = mRules[ruleIndex];
			if(!rule.SelectorList.IsMatching(guiElement))
				continue;

			outputRule->Override(rule);
		}
	}

	if(auto it = mCachedRulesById.find(guiElement.GetStyleSheetId()); it != mCachedRulesById.end())
	{
		for(const auto& ruleIndex : it->second.RuleIndices)
		{
			const GUIStyleSheetRule& rule = mRules[ruleIndex];
			if(!rule.SelectorList.IsMatching(guiElement))
				continue;

			outputRule->Override(rule);
		}
	}

	return outputRule;
}

HGUIStyleSheet GUIStyleSheet::Create(TArray<GUIStyleSheetRule> rules)
{
	const SPtr<GUIStyleSheet> newStyleSheet = CreateShared(std::move(rules));

	return B3DStaticResourceCast<GUIStyleSheet>(GetResources().CreateResourceHandle(newStyleSheet));
}

SPtr<GUIStyleSheet> GUIStyleSheet::CreateShared(TArray<GUIStyleSheetRule> rules)
{
	SPtr<GUIStyleSheet> newStyleSheet = B3DMakeCoreFromExisting<GUIStyleSheet>(new(B3DAllocate<GUIStyleSheet>()) GUIStyleSheet(std::move(rules)));
	newStyleSheet->SetShared(newStyleSheet);
	newStyleSheet->Initialize();

	return newStyleSheet;
}

RTTITypeBase* GUIStyleSheet::GetRttiStatic()
{
	return GUIStyleSheetRTTI::Instance();
}

RTTITypeBase* GUIStyleSheet::GetRtti() const
{
	return GUIStyleSheet::GetRttiStatic();
}
