//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/BsInput.h"
#include "Private/Linux/BsLinuxInput.h"
#include "Input/BsMouse.h"
#include "Input/BsKeyboard.h"
#include "Input/BsGamepad.h"
#include <fcntl.h>
#include <linux/input.h>

namespace bs
{
	/** Information about events reported from a specific input event device. */
	struct EventInfo
	{
		Vector<INT32> buttons;
		Vector<INT32> relAxes;
		Vector<INT32> absAxes;
		Vector<INT32> hats;
	};

	/** Checks is the bit at the specified location in a byte array is set. */
	bool isBitSet(UINT8 bits[], UINT32 bit)
	{
		return ((bits[bit/8] >> (bit%8)) & 1) != 0;
	}

	/** Returns information about an input event device attached to he provided file handle. */
	bool getEventInfo(int fileHandle, EventInfo& eventInfo)
	{
		UINT8 eventBits[1 + EV_MAX/8];
		bs_zero_out(eventBits);

		if (ioctl(fileHandle, EVIOCGBIT(0, sizeof(eventBits)), eventBits) == -1)
			return false;

		for (UINT32 i = 0; i < EV_MAX; i++)
		{
			if(isBitSet(eventBits, i))
			{
				if(i == EV_ABS)
				{
					UINT8 absAxisBits[1 + ABS_MAX/8];
					bs_zero_out(absAxisBits);

					if (ioctl(fileHandle, EVIOCGBIT(i, sizeof(absAxisBits)), absAxisBits) == -1)
					{
						BS_LOG(Error, Platform, "Could not read device absolute axis features.");
						continue;
					}

					for (UINT32 j = 0; j < ABS_MAX; j++)
					{
						if(isBitSet(absAxisBits, j))
						{
							if(j >= ABS_HAT0X && j <= ABS_HAT3Y)
								eventInfo.hats.push_back(j);
							else
								eventInfo.absAxes.push_back(j);
						}
					}
				}
				else if(i == EV_REL)
				{
					UINT8 relAxisBits[1 + REL_MAX/8];
					bs_zero_out(relAxisBits);

					if (ioctl(fileHandle, EVIOCGBIT(i, sizeof(relAxisBits)), relAxisBits) == -1)
					{
						BS_LOG(Error, Platform, "Could not read device relative axis features.");
						continue;
					}

					for (UINT32 j = 0; j < REL_MAX; j++)
					{
						if(isBitSet(relAxisBits, j))
							eventInfo.relAxes.push_back(j);
					}
				}
				else if(i == EV_KEY)
				{
					UINT8 keyBits[1 + KEY_MAX/8];
					bs_zero_out(keyBits);

					if (ioctl(fileHandle, EVIOCGBIT(i, sizeof(keyBits)), keyBits) == -1)
					{
						BS_LOG(Error, Platform, "Could not read device key features.");
						continue;
					}

					for (UINT32 j = 0; j < KEY_MAX; j++)
					{
						if(isBitSet(keyBits, j))
							eventInfo.buttons.push_back(j);
					}
				}
			}
		}

		return true;
	}

	/** Converts a Linux button code to Banshee ButtonCode. */
	ButtonCode gamepadMapCommonButton(INT32 code)
	{
		// Note: Assuming XBox controller layout here
		switch (code)
		{
		case BTN_TRIGGER_HAPPY1:
			return BC_GAMEPAD_DPAD_LEFT;
		case BTN_TRIGGER_HAPPY2:
			return BC_GAMEPAD_DPAD_RIGHT;
		case BTN_TRIGGER_HAPPY3:
			return BC_GAMEPAD_DPAD_UP;
		case BTN_TRIGGER_HAPPY4:
			return BC_GAMEPAD_DPAD_DOWN;
		case BTN_START:
			return BC_GAMEPAD_START;
		case BTN_SELECT:
			return BC_GAMEPAD_BACK;
		case BTN_THUMBL:
			return BC_GAMEPAD_LS;
		case BTN_THUMBR:
			return BC_GAMEPAD_RS;
		case BTN_TL:
			return BC_GAMEPAD_LB;
		case BTN_TR:
			return BC_GAMEPAD_RB;
		case BTN_A:
			return BC_GAMEPAD_A;
		case BTN_B:
			return BC_GAMEPAD_B;
		case BTN_X:
			return BC_GAMEPAD_X;
		case BTN_Y:
			return BC_GAMEPAD_Y;
		}

		return BC_UNASSIGNED;
	}

	/**
	 * Maps an absolute axis as reported by the Linux system, to a Banshee axis. This will be one of the InputAxis enum
	 * members, or -1 if it cannot be mapped.
	 */
	INT32 gamepadMapCommonAxis(INT32 axis)
	{
		switch(axis)
		{
		case ABS_X: return (INT32)InputAxis::LeftStickX;
		case ABS_Y: return (INT32)InputAxis::LeftStickY;
		case ABS_RX: return (INT32)InputAxis::RightStickX;
		case ABS_RY: return (INT32)InputAxis::RightStickY;
		case ABS_Z: return (INT32)InputAxis::LeftTrigger;
		case ABS_RZ: return (INT32)InputAxis::RightTrigger;
		}

		return -1;
	}

	/**
	 * Returns true if the input event attached to the specified file handle is a gamepad,
	 * and populates the gamepad info structure. Returns false otherwise.
	 */
	bool parseGamepadInfo(int fileHandle, int eventHandlerIdx, GamepadInfo& info)
	{
		EventInfo eventInfo;
		if(!getEventInfo(fileHandle, eventInfo))
			return false;

		bool isGamepad = false;

		// Check for gamepad buttons
		UINT32 unknownButtonIdx = 0;
		for(auto& entry : eventInfo.buttons)
		{
			if((entry >= BTN_JOYSTICK && entry < BTN_GAMEPAD)
				|| (entry >= BTN_GAMEPAD && entry < BTN_DIGI)
				|| (entry >= BTN_WHEEL && entry < KEY_OK))
			{
				ButtonCode bc = gamepadMapCommonButton(entry);
				if(bc == BC_UNASSIGNED)
				{
					// Map to unnamed buttons
					if(unknownButtonIdx < 20)
					{
						bc = (ButtonCode)((INT32)BC_GAMEPAD_BTN1 + unknownButtonIdx);
						info.buttonMap[entry] = bc;

						unknownButtonIdx++;
					}
				}
				else
					info.buttonMap[entry] = bc;

				isGamepad = true;
			}
		}

		if(isGamepad)
		{
			info.eventHandlerIdx = eventHandlerIdx;

			// Get device name
			char name[128];
			if (ioctl(fileHandle, EVIOCGNAME(sizeof(name)), name) != -1)
				info.name = String(name);
			else
				BS_LOG(Error, Platform, "Could not read device name.");

			// Get axis ranges
			UINT32 unknownAxisIdx = 0;
			for(auto& entry : eventInfo.absAxes)
			{
				AxisInfo& axisInfo = info.axisMap[entry];
				axisInfo.min = Gamepad::MIN_AXIS;
				axisInfo.max = Gamepad::MAX_AXIS;

				input_absinfo absinfo;
				if (ioctl(fileHandle, EVIOCGABS(entry), &absinfo) == -1)
				{
					BS_LOG(Error, Platform, "Could not read absolute axis device features.");
					continue;
				}

				axisInfo.min = absinfo.minimum;
				axisInfo.max = absinfo.maximum;

				axisInfo.axisIdx = gamepadMapCommonAxis(entry);
				if(axisInfo.axisIdx == -1)
				{
					axisInfo.axisIdx = (INT32)InputAxis::Count + unknownAxisIdx;
					unknownAxisIdx++;
				}
			}
		}

		return isGamepad;
	}

	void Input::initRawInput()
	{
		mPlatformData = bs_new<InputPrivateData>();

		// Scan for valid gamepad devices
		for(int i = 0; i < 64; ++i )
		{
			String eventPath = "/dev/input/event" + toString(i);
			int file = open(eventPath.c_str(), O_RDONLY |O_NONBLOCK);
			if(file == -1)
			{
				// Note: We're ignoring failures due to permissions. The assumption is that gamepads won't have special
				// permissions. If this assumption proves wrong, then using udev might be required to read gamepad input.
				continue;
			}

			GamepadInfo info;
			if(parseGamepadInfo(file, i, info))
			{
				info.id = (UINT32)mPlatformData->gamepadInfos.size();
				mPlatformData->gamepadInfos.push_back(info);
			}

			close(file);
		}

		mKeyboard = bs_new<Keyboard>("Keyboard", this);
		mMouse = bs_new<Mouse>("Mouse", this);

		UINT32 numGamepads = getDeviceCount(InputDevice::Gamepad);
		for (UINT32 i = 0; i < numGamepads; i++)
			mGamepads.push_back(bs_new<Gamepad>(mPlatformData->gamepadInfos[i].name, mPlatformData->gamepadInfos[i], this));
	}

	void Input::cleanUpRawInput()
	{
		if (mMouse != nullptr)
			bs_delete(mMouse);

		if (mKeyboard != nullptr)
			bs_delete(mKeyboard);

		for (auto& gamepad : mGamepads)
			bs_delete(gamepad);

		bs_delete(mPlatformData);
	}

	UINT32 Input::getDeviceCount(InputDevice device) const
	{
		switch(device)
		{
		case InputDevice::Keyboard: return 1;
		case InputDevice::Mouse: return 1;
		case InputDevice::Gamepad: return (UINT32)mPlatformData->gamepadInfos.size();
		case InputDevice::Count: return 0;
		}

		return 0;
	}
}

