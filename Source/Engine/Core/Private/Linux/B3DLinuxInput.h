//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Input/B3DInputFwd.h"

namespace b3d
{
	/** Infomation about an analog axis that's part of a gamepad. */
	struct AxisInfo
	{
		i32 axisIdx;
		i32 min;
		i32 max;
	};

	/** Information about a gamepad. */
	struct GamepadInfo
	{
		u32 id;
		u32 eventHandlerIdx;
		String name;

		UnorderedMap<i32, ButtonCode> buttonMap;
		UnorderedMap<i32, AxisInfo> axisMap;
	};

	/**
	 * Data specific to Linux implementation of the input system. Can be passed to platform specific implementations of
	 * the individual device types.
	 */
	struct InputPrivateData
	{
		Vector<GamepadInfo> gamepadInfos;
	};

	/** Data about relative pointer / scroll wheel movement. */
	struct LinuxMouseMotionEvent
	{
		double deltaX; /**< Relative pointer movement in X direction. */
		double deltaY; /**< Relative pointer movement in Y direction. */
		double deltaZ; /**< Relative vertical scroll amount. */
	};

	/** Data about a single button press or release. */
	struct LinuxButtonEvent
	{
		u64 timestamp;
		ButtonCode button;
		bool pressed;
	};

#define BUFFER_SIZE_GAMEPAD 64
} // namespace b3d
