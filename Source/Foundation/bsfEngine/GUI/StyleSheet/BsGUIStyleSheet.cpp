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

SPtr<GUIStyleSheetStateStyle> GUIStyleSheetStateStyle::kDefault = B3DMakeShared<GUIStyleSheetStateStyle>();

GUIStyleSheetStateStyle::GUIStyleSheetStateStyle()
{
	OverridenProperties.Resize((u32)GUIStyleSheetPropertyType::Count);
}

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

HFont GUIStyleSheetStateStyle::GetOrLoadFont() const
{
	// TODO - Style should be containing the HFont reference directly. Otherwise we cannot use the reference search mechanism.
	// - And we'll need the same for styles referencing textures
	// - So these should be resolved directly by the parser. We can provide an interface class that does the lookup, either in builtin resources or project library

	if(!mCachedFont.IsLoaded(false))
		mCachedFont = GetBuiltinResources().GetFont(FontFamily);

	return mCachedFont;
}

const GUIStyleSheetStateStyle* GUIStyleSheetStyle::FindStateStyle(const StringView& name) const
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

SPtr<GUIStyleSheetStyle> GUIStyleSheetStyle::kDefault = B3DMakeShared<GUIStyleSheetStyle>();

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

SPtr<GUIStyleSheetStateStyle> GUIStyleSheetStyle::FindStateStyle(GUIElementStateFlags stateFlags) const
{
	if(auto found = mCachedStateStyles.find(stateFlags); found != mCachedStateStyles.end())
		return found->second;

	SPtr<GUIStyleSheetStateStyle> stateStyle = B3DMakeShared<GUIStyleSheetStateStyle>();
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

void GUIStyleSheetStyle::Override(const GUIStyleSheetStyle& other)
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

GUIStyleSheet::GUIStyleSheet()
	: Resource(false, "StyleSheet")
{ }
 
SPtr<GUIStyleSheetStyle> GUIStyleSheet::FindStyle(const String& elementType, const String& elementId) const
{
	CachedStateStyleKey key(elementType, elementId);

	if(auto found = mCachedStyles.find(key); found != mCachedStyles.end())
		return found->second;

	SPtr<GUIStyleSheetStyle> style = B3DMakeShared<GUIStyleSheetStyle>();

	if(auto it = mElementStyles.find(elementType); it != mElementStyles.end())
		*style = it->second;

	if(auto it = mIdStyles.find(elementId); it != mIdStyles.end())
		style->Override(it->second);

	return mCachedStyles.insert(std::make_pair(std::move(key),  style)).first->second;
}

HGUIStyleSheet GUIStyleSheet::Create()
{
	const SPtr<GUIStyleSheet> newStyleSheet = CreateShared();

	return B3DStaticResourceCast<GUIStyleSheet>(GetResources().CreateResourceHandle(newStyleSheet));
}

SPtr<GUIStyleSheet> GUIStyleSheet::CreateShared()
{
	SPtr<GUIStyleSheet> newStyleSheet = B3DMakeCoreFromExisting<GUIStyleSheet>(new(B3DAllocate<GUIStyleSheet>()) GUIStyleSheet());
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
