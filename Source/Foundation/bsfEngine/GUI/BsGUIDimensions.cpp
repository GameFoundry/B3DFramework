//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUIElementStyle.h"
#include "GUI/BsGUIOptions.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace bs;

GUIDimensions GUIDimensions::Create()
{
	return GUIDimensions();
}

GUIDimensions GUIDimensions::Create(const GUIOptions& options)
{
	GUIDimensions dimensions;

	for(auto& option : options.mOptions)
	{
		switch(option.type)
		{
		case GUIOption::Type::Position:
			dimensions.X = (i32)option.min;
			dimensions.Y = (i32)option.max;
			break;
		case GUIOption::Type::FixedWidth:
			dimensions.Flags |= GUIDF_FixedWidth | GUIDF_WidthOverridenAtRuntime;
			dimensions.MinWidth = dimensions.MaxWidth = option.min;
			break;
		case GUIOption::Type::FixedHeight:
			dimensions.Flags |= GUIDF_FixedHeight | GUIDF_HeightOverridenAtRuntime;
			dimensions.MinHeight = dimensions.MaxHeight = option.min;
			break;
		case GUIOption::Type::FlexibleWidth:
			dimensions.Flags |= GUIDF_WidthOverridenAtRuntime;
			dimensions.Flags &= ~GUIDF_FixedWidth;
			dimensions.MinWidth = option.min;
			dimensions.MaxWidth = option.max;
			break;
		case GUIOption::Type::FlexibleHeight:
			dimensions.Flags |= GUIDF_HeightOverridenAtRuntime;
			dimensions.Flags &= ~GUIDF_FixedHeight;
			dimensions.MinHeight = option.min;
			dimensions.MaxHeight = option.max;
			break;
		}
	}

	return dimensions;
}

void GUIDimensions::UpdateWithStyle(const GUIElementStyle* style)
{
	if(!IsWidthOverridenAtRuntime())
	{
		if(style->FixedWidth)
		{
			Flags |= GUIDF_FixedWidth;
			MinWidth = MaxWidth = style->Width;
		}
		else
		{
			Flags &= ~GUIDF_FixedWidth;
			MinWidth = style->MinWidth;
			MaxWidth = style->MaxWidth;
		}
	}

	if(!IsHeightOverridenAtRuntime())
	{
		if(style->FixedHeight)
		{
			Flags |= GUIDF_FixedHeight;
			MinHeight = MaxHeight = style->Height;
		}
		else
		{
			Flags &= ~GUIDF_FixedHeight;
			MinHeight = style->MinHeight;
			MaxHeight = style->MaxHeight;
		}
	}
}

void GUIDimensions::UpdateWithStyle(const GUIStyleSheetStateRule& style)
{
	if(!IsWidthOverridenAtRuntime())
	{
		if(style.IsPropertySet(GUIStyleSheetPropertyType::Width))
		{
			Flags |= GUIDF_FixedWidth;
			MinWidth = MaxWidth = style.Size.Width;
		}
		else
		{
			Flags &= ~GUIDF_FixedWidth;
			MinWidth = style.MinimumSize.Width;
			MaxWidth = style.MaximumSize.Width;
		}
	}

	if(!IsHeightOverridenAtRuntime())
	{
		if(style.IsPropertySet(GUIStyleSheetPropertyType::Height))
		{
			Flags |= GUIDF_FixedHeight;
			MinHeight = MaxHeight = style.Size.Height;
		}
		else
		{
			Flags &= ~GUIDF_FixedHeight;
			MinHeight = style.MinimumSize.Height;
			MaxHeight = style.MaximumSize.Height;
		}
	}
}

LayoutSizeRange GUIDimensions::CalculateSizeRange(const Vector2I& optimal) const
{
	LayoutSizeRange sizeRange;

	if(FixedHeight())
	{
		sizeRange.Optimal.Y = std::max(0, (i32)MinHeight);
		sizeRange.Min.Y = sizeRange.Optimal.Y;
		sizeRange.Max.Y = sizeRange.Optimal.Y;
	}
	else
	{
		sizeRange.Optimal.Y = optimal.Y;

		if(MinHeight > 0)
		{
			sizeRange.Optimal.Y = std::max(std::max(0, (i32)MinHeight), sizeRange.Optimal.Y);
			sizeRange.Min.Y = std::max(0, (i32)MinHeight);
		}

		if(MaxHeight > 0)
		{
			sizeRange.Optimal.Y = std::min(std::max(0, (i32)MaxHeight), sizeRange.Optimal.Y);
			sizeRange.Max.Y = std::max(0, (i32)MaxHeight);
		}
	}

	if(FixedWidth())
	{
		sizeRange.Optimal.X = std::max(0, (i32)MinWidth);
		sizeRange.Min.X = sizeRange.Optimal.X;
		sizeRange.Max.X = sizeRange.Optimal.X;
	}
	else
	{
		sizeRange.Optimal.X = optimal.X;

		if(MinWidth > 0)
		{
			sizeRange.Optimal.X = std::max(std::max(0, (i32)MinWidth), sizeRange.Optimal.X);
			sizeRange.Min.X = std::max(0, (i32)MinWidth);
		}

		if(MaxWidth > 0)
		{
			sizeRange.Optimal.X = std::min(std::max(0, (i32)MaxWidth), sizeRange.Optimal.X);
			sizeRange.Max.X = std::max(0, (i32)MaxWidth);
		}
	}

	return sizeRange;
}
