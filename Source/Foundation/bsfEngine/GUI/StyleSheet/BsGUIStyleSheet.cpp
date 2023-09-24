//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/StyleSheet/BsGUIStyleSheet.h"
#include "BsGUIStyleSheetParser.h"
#include "FileSystem/BsFileSystem.h"
#include "FileSystem/BsDataStream.h"

using namespace bs;

void GUIStyleSheetStateStyle::Override(const GUIStyleSheetStateStyle& other)
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

const GUIStyleSheetStateStyle* GUIStyleSheetStyle::FindStateStyle(const StringView& name) const
{
	if(StringUtil::Compare(name, "normal", false))
		return &Normal;

	if(StringUtil::Compare(name, "hover", false))
		return Hover.has_value() ? &Hover.value() : nullptr;

	if(StringUtil::Compare(name, "active", false))
		return Active.has_value() ? &Active.value() : nullptr;

	if(StringUtil::Compare(name, "focused", false))
		return Focused.has_value() ? &Focused.value() : nullptr;

	if(StringUtil::Compare(name, "disabled", false))
		return Disabled.has_value() ? &Disabled.value() : nullptr;

	if(StringUtil::Compare(name, "checked", false))
		return Checked.has_value() ? &Checked.value() : nullptr;

	return nullptr;
}

bool GUIStyleSheetStyle::FindAndSetStateStyle(const StringView& name, const GUIStyleSheetStateStyle& stateStyle)
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

	if(StringUtil::Compare(name, "focused", false))
	{
		Focused = stateStyle;
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

Optional<GUIStyleSheet> GUIStyleSheet::Parse(const Path& file)
{
	const SPtr<DataStream> fileStream = FileSystem::OpenFile(file);
	if(!fileStream)
		return {};

	GUIStyleSheetParser parser;
	return parser.Parse(B3DMakeShared<SourceCode>(fileStream->GetAsString()));
}

GUIStyleSheetStateStyle GUIStyleSheetStyle::FindStateStyle(GUIElementState state) const
{
	// TODO - Replace the current set of states with a new set of flags where each state is a bitmask
	GUIStyleSheetStateStyle stateStyle = Normal;

	const bool isChecked = ((u32)state & (u32)GUIElementState::OnFlag) != 0;
	if(isChecked)
	{
		if(Checked.has_value())
			stateStyle.Override(*Checked);
	}

	switch(state)
	{
	default:
	case GUIElementState::Normal: 
	case GUIElementState::NormalOn:
		break;
	case GUIElementState::Hover:
	case GUIElementState::HoverOn:
		if(Hover.has_value())
			stateStyle.Override(*Hover);
		break;
	case GUIElementState::Active:
	case GUIElementState::ActiveOn:
		if(Hover.has_value())
			stateStyle.Override(*Hover);

		if(Active.has_value())
			stateStyle.Override(*Active);
		break;
	case GUIElementState::Focused:
	case GUIElementState::FocusedOn:
		if(Focused.has_value())
			stateStyle.Override(*Focused);
		break;
	case GUIElementState::FocusedHover:
	case GUIElementState::FocusedHoverOn:
		if(Focused.has_value())
			stateStyle.Override(*Focused);

		if(Hover.has_value())
			stateStyle.Override(*Hover);
		break;
	}

	return stateStyle;
}

const GUIStyleSheetStateStyle& GUIStyleSheet::FindStyle(const String& elementType, const String& elementId, GUIElementState state)
{
	CachedStateStyleKey key(elementType, elementId, state);

	if(auto found = mCachedStateStyles.find(key); found != mCachedStateStyles.end())
		return found->second;

	GUIStyleSheetStateStyle style;

	if(auto it = mElementStyles.find(elementType); it != mElementStyles.end())
		style = it->second.FindStateStyle(state);

	if(auto it = mIdStyles.find(elementId); it != mElementStyles.end())
		style.Override(it->second.FindStateStyle(state));

	return mCachedStateStyles.insert(std::make_pair(std::move(key),  style)).first->second;
}

