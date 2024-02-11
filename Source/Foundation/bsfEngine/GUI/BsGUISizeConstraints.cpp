//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUISizeConstraints.h"
#include "GUI/BsGUIOptions.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace bs;

GUISizeConstraints GUISizeConstraints::Create()
{
	return GUISizeConstraints();
}

GUISizeConstraints GUISizeConstraints::Create(const GUIOptions& options)
{
	return Create(options.mOptions);
}

GUISizeConstraints GUISizeConstraints::Create(const TInlineArray<GUIOption, 4>& options)
{
	GUISizeConstraints dimensions;

	for(auto& option : options)
	{
		switch(option.type)
		{
		case GUIOptionType::Position:
			dimensions.X = (i32)option.min;
			dimensions.Y = (i32)option.max;
			break;
		case GUIOptionType::FixedWidth:
			dimensions.Flags |= GUISizeConstraintFlag::FixedWidth | GUISizeConstraintFlag::WidthOverridenAtRuntime;
			dimensions.MinWidth = dimensions.MaxWidth = option.min;
			break;
		case GUIOptionType::FixedHeight:
			dimensions.Flags |= GUISizeConstraintFlag::FixedHeight | GUISizeConstraintFlag::HeightOverridenAtRuntime;
			dimensions.MinHeight = dimensions.MaxHeight = option.min;
			break;
		case GUIOptionType::FlexibleWidth:
			dimensions.Flags |= GUISizeConstraintFlag::WidthOverridenAtRuntime;
			dimensions.Flags.Unset(GUISizeConstraintFlag::FixedWidth);
			dimensions.MinWidth = option.min;
			dimensions.MaxWidth = option.max;
			break;
		case GUIOptionType::FlexibleHeight:
			dimensions.Flags |= GUISizeConstraintFlag::HeightOverridenAtRuntime;
			dimensions.Flags.Unset(GUISizeConstraintFlag::FixedHeight);
			dimensions.MinHeight = option.min;
			dimensions.MaxHeight = option.max;
			break;
		}
	}

	return dimensions;
}

void GUISizeConstraints::UpdateWithStyleSheetRule(const GUIStyleSheetRules& rule)
{
	if(!IsWidthOverridenAtRuntime())
	{
		if(rule.IsPropertySet(GUIStyleSheetPropertyType::Width))
		{
			Flags |= GUISizeConstraintFlag::FixedWidth;
			MinWidth = MaxWidth = rule.Size.Width;
		}
		else
		{
			Flags.Unset(GUISizeConstraintFlag::FixedWidth);
			MinWidth = rule.MinimumSize.Width;
			MaxWidth = rule.MaximumSize.Width;
		}
	}

	if(!IsHeightOverridenAtRuntime())
	{
		if(rule.IsPropertySet(GUIStyleSheetPropertyType::Height))
		{
			Flags |= GUISizeConstraintFlag::FixedHeight;
			MinHeight = MaxHeight = rule.Size.Height;
		}
		else
		{
			Flags.Unset(GUISizeConstraintFlag::FixedHeight);
			MinHeight = rule.MinimumSize.Height;
			MaxHeight = rule.MaximumSize.Height;
		}
	}
}

GUIConstrainedSize GUISizeConstraints::CalculateConstrainedSize(const Vector2I& optimalSize) const
{
	GUIConstrainedSize sizeRange;

	if(IsHeightFixed())
	{
		sizeRange.Optimal.Y = std::max(0, (i32)MinHeight);
		sizeRange.Min.Y = sizeRange.Optimal.Y;
		sizeRange.Max.Y = sizeRange.Optimal.Y;
	}
	else
	{
		sizeRange.Optimal.Y = optimalSize.Y;

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

	if(IsWidthFixed())
	{
		sizeRange.Optimal.X = std::max(0, (i32)MinWidth);
		sizeRange.Min.X = sizeRange.Optimal.X;
		sizeRange.Max.X = sizeRange.Optimal.X;
	}
	else
	{
		sizeRange.Optimal.X = optimalSize.X;

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

GUIConstrainedSize GUISizeConstraints::CalculateConstrainedSize(const Size2UI& optimalSize) const
{
	return CalculateConstrainedSize(Vector2I((i32)optimalSize.Width, (i32)optimalSize.Height));
}

