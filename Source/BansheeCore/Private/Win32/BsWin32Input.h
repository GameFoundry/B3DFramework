//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

#define WIN32_LEAN_AND_MEAN
#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <dinput.h>
#include <Xinput.h>

#include <wbemidl.h>
#include <oleauto.h>

namespace bs
{
	/** Information about a gamepad from DirectInput. */
	struct GamepadInfo
	{
		UINT32 id;
		GUID guidInstance;
		GUID guidProduct;
		String name;

		bool isXInput;
		int xInputDev;
	};

	/**
	 * Data specific to Win32 implementation of the input system. Can be passed to platform specific implementations of
	 * the individual device types.
	 */
	struct InputPrivateData
	{
		IDirectInput8* directInput;
		Vector<GamepadInfo> gamepadInfos;

		DWORD kbSettings;
		DWORD mouseSettings;
	};

	// Max number of elements to collect from buffered DirectInput
	#define DI_BUFFER_SIZE_KEYBOARD 17
	#define DI_BUFFER_SIZE_MOUSE 128
	#define DI_BUFFER_SIZE_GAMEPAD 129
}