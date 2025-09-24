//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIOptions.h"

using namespace b3d;

GUIOption GUIOption::Position(GUILogicalUnit x, GUILogicalUnit y)
{
	GUIOption option;

	option.mMinimum = x;
	option.mMaximum = y;
	option.mType = GUIOptionType::Position;

	return option;
}

GUIOption GUIOption::FixedWidth(GUILogicalUnit value)
{
	GUIOption option;

	option.mMinimum = option.mMaximum = Math::Max(value, 0);
	option.mType = GUIOptionType::FixedWidth;

	return option;
}

GUIOption GUIOption::FlexibleWidth(GUILogicalUnit minimum, GUILogicalUnit maximum)
{
	GUIOption option;

	option.mMinimum = Math::Max(minimum, 0);
	option.mMaximum = Math::Max(maximum, 0);
	option.mType = GUIOptionType::FlexibleWidth;

	return option;
}

GUIOption GUIOption::ExpandingWidth(GUILogicalUnit minimum, GUILogicalUnit maximum)
{
	GUIOption option;

	option.mMinimum = Math::Max(minimum, 0);
	option.mMaximum = Math::Max(maximum, 0);
	option.mType = GUIOptionType::ExpandingWidth;

	return option;
}

GUIOption GUIOption::FixedHeight(GUILogicalUnit value)
{
	GUIOption option;

	option.mMinimum = option.mMaximum = Math::Max(value, 0);
	option.mType = GUIOptionType::FixedHeight;

	return option;
}

GUIOption GUIOption::FlexibleHeight(GUILogicalUnit minimum, GUILogicalUnit maximum)
{
	GUIOption option;

	option.mMinimum = Math::Max(minimum, 0);
	option.mMaximum = Math::Max(maximum, 0);
	option.mType = GUIOptionType::FlexibleHeight;

	return option;
}

GUIOption GUIOption::ExpandingHeight(GUILogicalUnit minimum, GUILogicalUnit maximum)
{
	GUIOption option;

	option.mMinimum = Math::Max(minimum, 0);
	option.mMaximum = Math::Max(maximum, 0);
	option.mType = GUIOptionType::ExpandingHeight;

	return option;
}
