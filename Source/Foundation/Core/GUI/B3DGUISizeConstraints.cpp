//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUISizeConstraints.h"
#include "GUI/B3DGUIOptions.h"
#include "StyleSheet/B3DGUIStyleSheet.h"

using namespace b3d;

GUILogicalSize GUIConstrainedSizeRange::CalculateSizeConstrainedByParentSize(const GUISizeConstraints& sizeConstraints, const GUILogicalSize& parentSize) const
{
	GUILogicalSize size;

	if(sizeConstraints.IsWidthFixed())
		size.Width = Optimal.Width;
	else
	{
		size.Width = parentSize.Width;

		if(size.Width > Optimal.Width)
		{
			if(Maximum.Width > 0)
				size.Width = Math::Min(size.Width, Maximum.Width);
		}
		else if(size.Width < Optimal.Width)
		{
			if(Minimum.Width > 0)
				size.Width = Math::Max(size.Width, Minimum.Width);
		}
	}

	if(sizeConstraints.IsHeightFixed())
		size.Height = Optimal.Height;
	else
	{
		size.Height = parentSize.Height;

		if(size.Height > Optimal.Height)
		{
			if(Maximum.Height > 0)
				size.Height = Math::Min(size.Height, Maximum.Height);
		}
		else if(size.Height < Optimal.Height)
		{
			if(Minimum.Height > 0)
				size.Height = Math::Max(size.Height, Minimum.Height);
		}
	}

	return size;
}


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
		switch(option.mType)
		{
		case GUIOptionType::Position:
			dimensions.ExplicitPosition = GUILogicalPoint((i32)option.mMinimum, (i32)option.mMaximum);
			break;
		case GUIOptionType::FixedWidth:
			dimensions.Flags |= GUISizeConstraintFlag::FixedWidth | GUISizeConstraintFlag::WidthOverridenAtRuntime;
			dimensions.MinimumWidth = dimensions.MaximumWidth = option.mMinimum;
			break;
		case GUIOptionType::FixedHeight:
			dimensions.Flags |= GUISizeConstraintFlag::FixedHeight | GUISizeConstraintFlag::HeightOverridenAtRuntime;
			dimensions.MinimumHeight = dimensions.MaximumHeight = option.mMinimum;
			break;
		case GUIOptionType::FlexibleWidth:
			dimensions.Flags |= GUISizeConstraintFlag::WidthOverridenAtRuntime;
			dimensions.Flags.Unset(GUISizeConstraintFlag::FixedWidth);
			dimensions.Flags.Unset(GUISizeConstraintFlag::ExpandingWidth);
			dimensions.MinimumWidth = option.mMinimum;
			dimensions.MaximumWidth = option.mMaximum;
			break;
		case GUIOptionType::FlexibleHeight:
			dimensions.Flags |= GUISizeConstraintFlag::HeightOverridenAtRuntime;
			dimensions.Flags.Unset(GUISizeConstraintFlag::FixedHeight);
			dimensions.Flags.Unset(GUISizeConstraintFlag::ExpandingHeight);
			dimensions.MinimumHeight = option.mMinimum;
			dimensions.MaximumHeight = option.mMaximum;
			break;
		case GUIOptionType::ExpandingWidth:
			dimensions.Flags |= GUISizeConstraintFlag::ExpandingWidth | GUISizeConstraintFlag::WidthOverridenAtRuntime;
			dimensions.Flags.Unset(GUISizeConstraintFlag::FixedWidth);
			dimensions.MinimumWidth = option.mMinimum;
			dimensions.MaximumWidth = option.mMaximum;
			break;
		case GUIOptionType::ExpandingHeight:
			dimensions.Flags |= GUISizeConstraintFlag::ExpandingHeight | GUISizeConstraintFlag::HeightOverridenAtRuntime;
			dimensions.Flags.Unset(GUISizeConstraintFlag::FixedHeight);
			dimensions.MinimumHeight = option.mMinimum;
			dimensions.MaximumHeight = option.mMaximum;
			break;
		}
	}

	return dimensions;
}

void GUISizeConstraints::UpdateWithStyleSheetRule(const GUIStyleSheetRules& rule)
{
	if(!Flags.IsSet(GUISizeConstraintFlag::WidthOverridenAtRuntime))
	{
		Flags.Unset(GUISizeConstraintFlag::ExpandingWidth);

		if(rule.IsPropertySet(GUIStyleSheetPropertyType::Width))
		{
			Flags |= GUISizeConstraintFlag::FixedWidth;
			MinimumWidth = MaximumWidth = rule.Size.Width;
		}
		else
		{
			Flags.Unset(GUISizeConstraintFlag::FixedWidth);
			MinimumWidth = rule.MinimumSize.Width;
			MaximumWidth = rule.MaximumSize.Width;
		}
	}

	if(!Flags.IsSet(GUISizeConstraintFlag::HeightOverridenAtRuntime))
	{
		Flags.Unset(GUISizeConstraintFlag::ExpandingHeight);

		if(rule.IsPropertySet(GUIStyleSheetPropertyType::Height))
		{
			Flags |= GUISizeConstraintFlag::FixedHeight;
			MinimumHeight = MaximumHeight = rule.Size.Height;
		}
		else
		{
			Flags.Unset(GUISizeConstraintFlag::FixedHeight);
			MinimumHeight = rule.MinimumSize.Height;
			MaximumHeight = rule.MaximumSize.Height;
		}
	}
}

GUILogicalSize GUISizeConstraints::CalculateConstrainedOptimalSize(const GUILogicalSize& unconstrainedOptimalSize) const
{
	GUILogicalSize constrainedOptimalSize;

	if(IsHeightFixed())
		constrainedOptimalSize.Height = Math::Max(MinimumHeight, 0);
	else
	{
		constrainedOptimalSize.Height = unconstrainedOptimalSize.Height;

		if(MinimumHeight > 0)
			constrainedOptimalSize.Height = Math::Max(Math::Max(MinimumHeight, 0), constrainedOptimalSize.Height);

		if(MaximumHeight > 0)
			constrainedOptimalSize.Height = Math::Min(Math::Max(MaximumHeight, 0), constrainedOptimalSize.Height);
	}

	if(IsWidthFixed())
		constrainedOptimalSize.Width = Math::Max(MinimumWidth, 0);
	else
	{
		constrainedOptimalSize.Width = unconstrainedOptimalSize.Width;

		if(MinimumWidth > 0)
			constrainedOptimalSize.Width = Math::Max(Math::Max(MinimumWidth, 0), constrainedOptimalSize.Width);

		if(MaximumWidth > 0)
			constrainedOptimalSize.Width = Math::Min(Math::Max(MaximumWidth, 0), constrainedOptimalSize.Width);
	}

	return constrainedOptimalSize;
}

GUIConstrainedSizeRange GUISizeConstraints::CalculateConstrainedSizeRange(const GUILogicalSize& unconstrainedOptimalSize) const
{
	GUIConstrainedSizeRange sizeRange;

	if(IsHeightFixed())
	{
		sizeRange.Optimal.Height = Math::Max(MinimumHeight, 0);
		sizeRange.Minimum.Height = sizeRange.Optimal.Height;
		sizeRange.Maximum.Height = sizeRange.Optimal.Height;
	}
	else
	{
		sizeRange.Optimal.Height = unconstrainedOptimalSize.Height;

		if(MinimumHeight > 0)
		{
			sizeRange.Optimal.Height = Math::Max(Math::Max(MinimumHeight, 0), sizeRange.Optimal.Height);
			sizeRange.Minimum.Height = Math::Max(MinimumHeight, 0);
		}

		if(MaximumHeight > 0)
		{
			sizeRange.Optimal.Height = Math::Min(Math::Max(MaximumHeight, 0), sizeRange.Optimal.Height);
			sizeRange.Maximum.Height = Math::Max(MaximumHeight, 0);
		}
	}

	if(IsWidthFixed())
	{
		sizeRange.Optimal.Width = Math::Max(MinimumWidth, 0);
		sizeRange.Minimum.Width = sizeRange.Optimal.Width;
		sizeRange.Maximum.Width = sizeRange.Optimal.Width;
	}
	else
	{
		sizeRange.Optimal.Width = unconstrainedOptimalSize.Width;

		if(MinimumWidth > 0)
		{
			sizeRange.Optimal.Width = Math::Max(Math::Max(MinimumWidth, 0), sizeRange.Optimal.Width);
			sizeRange.Minimum.Width = Math::Max(MinimumWidth, 0);
		}

		if(MaximumWidth > 0)
		{
			sizeRange.Optimal.Width = Math::Min(Math::Max(MaximumWidth, 0), sizeRange.Optimal.Width);
			sizeRange.Maximum.Width = Math::Max(MaximumWidth, 0);
		}
	}

	return sizeRange;
}

