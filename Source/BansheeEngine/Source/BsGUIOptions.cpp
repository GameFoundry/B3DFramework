//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIOptions.h"

namespace bs
{
	GUIOption::GUIOption()
		:min(0), max(0), type(Type::FixedWidth)
	{

	}

	GUIOption GUIOption::position(INT32 x, INT32 y)
	{
		GUIOption option;

		option.min = (UINT32)x;
		option.max = (UINT32)y;
		option.type = Type::Position;

		return option;
	}

	GUIOption GUIOption::fixedWidth(UINT32 value)
	{
		GUIOption option;

		option.min = option.max = value;
		option.type = Type::FixedWidth;

		return option;
	}

	GUIOption GUIOption::flexibleWidth(UINT32 min, UINT32 max)
	{
		GUIOption option;

		option.min = min;
		option.max = max;
		option.type = Type::FlexibleWidth;

		return option;
	}

	GUIOption GUIOption::fixedHeight(UINT32 value)
	{
		GUIOption option;

		option.min = option.max = value;
		option.type = Type::FixedHeight;

		return option;
	}

	GUIOption GUIOption::flexibleHeight(UINT32 min, UINT32 max)
	{
		GUIOption option;

		option.min = min;
		option.max = max;
		option.type = Type::FlexibleHeight;

		return option;
	}
}