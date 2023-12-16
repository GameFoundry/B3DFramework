//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIOptions.h"

using namespace bs;

GUIOption GUIOption::Position(i32 x, i32 y)
{
	GUIOption option;

	option.min = (u32)x;
	option.max = (u32)y;
	option.type = GUIOptionType::Position;

	return option;
}

GUIOption GUIOption::FixedWidth(u32 value)
{
	GUIOption option;

	option.min = option.max = value;
	option.type = GUIOptionType::FixedWidth;

	return option;
}

GUIOption GUIOption::FlexibleWidth(u32 min, u32 max)
{
	GUIOption option;

	option.min = min;
	option.max = max;
	option.type = GUIOptionType::FlexibleWidth;

	return option;
}

GUIOption GUIOption::FixedHeight(u32 value)
{
	GUIOption option;

	option.min = option.max = value;
	option.type = GUIOptionType::FixedHeight;

	return option;
}

GUIOption GUIOption::FlexibleHeight(u32 min, u32 max)
{
	GUIOption option;

	option.min = min;
	option.max = max;
	option.type = GUIOptionType::FlexibleHeight;

	return option;
}
