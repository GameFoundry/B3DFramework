//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DInput.h"
#include "Private/Linux/B3DLinuxInput.h"

#include "B3DApplication.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "Platform/B3DPlatform.h"

#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>

using namespace b3d;

/** Information about events reported from a specific input event device. */
struct EventInfo
{
	Vector<i32> Buttons;
	Vector<i32> RelAxes;
	Vector<i32> AbsAxes;
	Vector<i32> Hats;
};

/** Checks if the bit at the specified location in a byte array is set. */
bool IsBitSet(u8 bits[], u32 bit)
{
	return ((bits[bit / 8] >> (bit % 8)) & 1) != 0;
}

/** Returns information about an input event device attached to the provided file handle. */
bool GetEventInfo(int fileHandle, EventInfo& eventInfo)
{
	u8 eventBits[1 + EV_MAX / 8];
	B3DZeroOut(eventBits);

	if(ioctl(fileHandle, EVIOCGBIT(0, sizeof(eventBits)), eventBits) == -1)
		return false;

	for(u32 eventIndex = 0; eventIndex < EV_MAX; eventIndex++)
	{
		if(!IsBitSet(eventBits, eventIndex))
			continue;

		if(eventIndex == EV_ABS)
		{
			u8 absAxisBits[1 + ABS_MAX / 8];
			B3DZeroOut(absAxisBits);

			if(ioctl(fileHandle, EVIOCGBIT(eventIndex, sizeof(absAxisBits)), absAxisBits) == -1)
			{
				B3D_LOG(Error, LogPlatform, "Could not read device absolute axis features.");
				continue;
			}

			for(u32 axisIndex = 0; axisIndex < ABS_MAX; axisIndex++)
			{
				if(IsBitSet(absAxisBits, axisIndex))
				{
					if(axisIndex >= ABS_HAT0X && axisIndex <= ABS_HAT3Y)
						eventInfo.Hats.push_back(axisIndex);
					else
						eventInfo.AbsAxes.push_back(axisIndex);
				}
			}
		}
		else if(eventIndex == EV_REL)
		{
			u8 relAxisBits[1 + REL_MAX / 8];
			B3DZeroOut(relAxisBits);

			if(ioctl(fileHandle, EVIOCGBIT(eventIndex, sizeof(relAxisBits)), relAxisBits) == -1)
			{
				B3D_LOG(Error, LogPlatform, "Could not read device relative axis features.");
				continue;
			}

			for(u32 axisIndex = 0; axisIndex < REL_MAX; axisIndex++)
			{
				if(IsBitSet(relAxisBits, axisIndex))
					eventInfo.RelAxes.push_back(axisIndex);
			}
		}
		else if(eventIndex == EV_KEY)
		{
			u8 keyBits[1 + KEY_MAX / 8];
			B3DZeroOut(keyBits);

			if(ioctl(fileHandle, EVIOCGBIT(eventIndex, sizeof(keyBits)), keyBits) == -1)
			{
				B3D_LOG(Error, LogPlatform, "Could not read device key features.");
				continue;
			}

			for(u32 keyIndex = 0; keyIndex < KEY_MAX; keyIndex++)
			{
				if(IsBitSet(keyBits, keyIndex))
					eventInfo.Buttons.push_back(keyIndex);
			}
		}
	}

	return true;
}

/** Converts an evdev button code to the engine button code. Assumes an Xbox controller layout. */
ButtonCode GamepadMapCommonButton(i32 code)
{
	switch(code)
	{
	case BTN_TRIGGER_HAPPY1:
		return ButtonCode::GamepadDPadLeft;
	case BTN_TRIGGER_HAPPY2:
		return ButtonCode::GamepadDPatRight;
	case BTN_TRIGGER_HAPPY3:
		return ButtonCode::GamepadDPadUp;
	case BTN_TRIGGER_HAPPY4:
		return ButtonCode::GamepadDPadDown;
	case BTN_START:
		return ButtonCode::GamepadStart;
	case BTN_SELECT:
		return ButtonCode::GamepadBack;
	case BTN_THUMBL:
		return ButtonCode::GamepadLeftStick;
	case BTN_THUMBR:
		return ButtonCode::GamepadRightStick;
	case BTN_TL:
		return ButtonCode::GamepadLeftBumper;
	case BTN_TR:
		return ButtonCode::GamepadRightBumper;
	case BTN_A:
		return ButtonCode::GamepadA;
	case BTN_B:
		return ButtonCode::GamepadB;
	case BTN_X:
		return ButtonCode::GamepadX;
	case BTN_Y:
		return ButtonCode::GamepadY;
	}

	return ButtonCode::Unassigned;
}

/**
 * Maps an absolute axis as reported by the evdev system to an engine axis. This will be one of the InputAxis enum
 * members, or -1 if it cannot be mapped.
 */
i32 GamepadMapCommonAxis(i32 axis)
{
	switch(axis)
	{
	case ABS_X: return (i32)InputAxis::LeftStickX;
	case ABS_Y: return (i32)InputAxis::LeftStickY;
	case ABS_RX: return (i32)InputAxis::RightStickX;
	case ABS_RY: return (i32)InputAxis::RightStickY;
	case ABS_Z: return (i32)InputAxis::LeftTrigger;
	case ABS_RZ: return (i32)InputAxis::RightTrigger;
	}

	return -1;
}

/**
 * Returns true if the input event device attached to the specified file handle is a gamepad, and populates the gamepad
 * info structure. Returns false otherwise.
 */
bool ParseGamepadInfo(int fileHandle, u32 eventHandlerIdx, GamepadInfo& info)
{
	EventInfo eventInfo;
	if(!GetEventInfo(fileHandle, eventInfo))
		return false;

	bool isGamepad = false;

	// Check for gamepad buttons
	u32 unknownButtonIdx = 0;
	for(auto& entry : eventInfo.Buttons)
	{
		if((entry >= BTN_JOYSTICK && entry < BTN_GAMEPAD) || (entry >= BTN_GAMEPAD && entry < BTN_DIGI) ||
			(entry >= BTN_WHEEL && entry < KEY_OK))
		{
			ButtonCode buttonCode = GamepadMapCommonButton(entry);
			if(buttonCode == ButtonCode::Unassigned)
			{
				// Map to unnamed buttons
				if(unknownButtonIdx < 20)
				{
					buttonCode = (ButtonCode)((u32)ButtonCode::GamepadButton1 + unknownButtonIdx);
					info.ButtonMap[entry] = buttonCode;

					unknownButtonIdx++;
				}
			}
			else
				info.ButtonMap[entry] = buttonCode;

			isGamepad = true;
		}
	}

	if(isGamepad)
	{
		info.EventHandlerIdx = eventHandlerIdx;

		// Get device name
		char name[128];
		if(ioctl(fileHandle, EVIOCGNAME(sizeof(name)), name) != -1)
			info.Name = String(name);
		else
			B3D_LOG(Error, LogPlatform, "Could not read device name.");

		// Get axis ranges
		u32 unknownAxisIdx = 0;
		for(auto& entry : eventInfo.AbsAxes)
		{
			AxisInfo& axisInfo = info.AxisMap[entry];
			axisInfo.Min = IInputBackend::kMinAxis;
			axisInfo.Max = IInputBackend::kMaxAxis;

			input_absinfo absinfo;
			if(ioctl(fileHandle, EVIOCGABS(entry), &absinfo) == -1)
			{
				B3D_LOG(Error, LogPlatform, "Could not read absolute axis device features.");
				continue;
			}

			axisInfo.Min = absinfo.minimum;
			axisInfo.Max = absinfo.maximum;

			axisInfo.AxisIdx = GamepadMapCommonAxis(entry);
			if(axisInfo.AxisIdx == -1)
			{
				axisInfo.AxisIdx = (i32)InputAxis::Count + unknownAxisIdx;
				unknownAxisIdx++;
			}
		}
	}

	return isGamepad;
}

LinuxInputBackend::LinuxInputBackend(Input& owner)
	: mOwner(owner)
{
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

	const bool isHeadless = gpuDevice == nullptr || gpuDevice->GetCapabilities().DeviceName == "Null" ||
		owner.GetWindowHandle() == 0;
	if(isHeadless)
		return;

	mKeyboard = B3DNew<LinuxKeyboard>(owner);
	mMouse = B3DNew<LinuxMouse>(owner);

	Vector<GamepadInfo> gamepadInfos;
	EnumerateGamepads(gamepadInfos);

	for(auto& gamepadInfo : gamepadInfos)
	{
		mGamepads.push_back(B3DNew<LinuxGamepad>(gamepadInfo, owner));
		mOwner.NotifyGamepadAdded(gamepadInfo.Id, gamepadInfo.Name);
	}

	// Detect gamepad hot-plug, reported by the platform layer through an inotify watch on /dev/input. The event may
	// trigger from a non-main thread, so only flag the change here and process it on the next Update().
	mDevicesChangedConn = Platform::OnInputDevicesChanged.Connect([this]() { mDevicesDirty.store(true); });
}

LinuxInputBackend::~LinuxInputBackend()
{
	mDevicesChangedConn.Disconnect();

	if(mMouse != nullptr)
		B3DDelete(mMouse);

	if(mKeyboard != nullptr)
		B3DDelete(mKeyboard);

	for(auto& gamepad : mGamepads)
		B3DDelete(gamepad);
}

void LinuxInputBackend::Update()
{
	if(mDevicesDirty.exchange(false))
		RebuildGamepads();

	if(mMouse != nullptr)
		mMouse->Capture();

	if(mKeyboard != nullptr)
		mKeyboard->Capture();

	for(auto& gamepad : mGamepads)
		gamepad->Capture();
}

u32 LinuxInputBackend::GetDeviceCount(InputDevice device) const
{
	switch(device)
	{
	case InputDevice::Keyboard: return mKeyboard != nullptr ? 1 : 0;
	case InputDevice::Mouse: return mMouse != nullptr ? 1 : 0;
	case InputDevice::Gamepad: return (u32)mGamepads.size();
	default:
	case InputDevice::Count: return 0;
	}
}

String LinuxInputBackend::GetDeviceName(InputDevice type, u32 deviceIndex) const
{
	switch(type)
	{
	case InputDevice::Keyboard:
		if(mKeyboard != nullptr && deviceIndex == 0)
			return "Keyboard";

		return StringUtility::kBlank;
	case InputDevice::Mouse:
		if(mMouse != nullptr && deviceIndex == 0)
			return "Mouse";

		return StringUtility::kBlank;
	case InputDevice::Gamepad:
		if(deviceIndex < (u32)mGamepads.size())
			return mGamepads[deviceIndex]->GetName();

		return StringUtility::kBlank;
	default:
		return StringUtility::kBlank;
	}
}

void LinuxInputBackend::ChangeCaptureContext(u64 windowHandle)
{
	if(mKeyboard != nullptr)
		mKeyboard->ChangeCaptureContext(windowHandle);

	if(mMouse != nullptr)
		mMouse->ChangeCaptureContext(windowHandle);

	for(auto& gamepad : mGamepads)
		gamepad->ChangeCaptureContext(windowHandle);
}

void LinuxInputBackend::EnumerateGamepads(Vector<GamepadInfo>& outGamepadInfos)
{
	// Scan for valid gamepad devices
	for(u32 eventIndex = 0; eventIndex < 64; ++eventIndex)
	{
		const String eventPath = "/dev/input/event" + ToString(eventIndex);
		const int file = open(eventPath.c_str(), O_RDONLY | O_NONBLOCK);
		if(file == -1)
		{
			// Note: We're ignoring failures due to permissions. The assumption is that gamepads won't have special
			// permissions. If this assumption proves wrong, then using udev might be required to read gamepad input.
			continue;
		}

		GamepadInfo info;
		if(ParseGamepadInfo(file, eventIndex, info))
		{
			info.Id = (u32)outGamepadInfos.size();
			outGamepadInfos.push_back(info);
		}

		close(file);
	}
}

/**
 * Checks if two gamepad infos refer to the same physical device. The event handler index alone isn't enough, as the
 * kernel reuses freed indices for newly attached devices.
 */
static bool IsSameGamepad(const GamepadInfo& a, const GamepadInfo& b)
{
	return a.EventHandlerIdx == b.EventHandlerIdx && a.Name == b.Name;
}

void LinuxInputBackend::RebuildGamepads()
{
	Vector<GamepadInfo> attachedInfos;
	EnumerateGamepads(attachedInfos);

	// Destroy gamepads that are no longer attached
	for(auto iter = mGamepads.begin(); iter != mGamepads.end();)
	{
		LinuxGamepad* gamepad = *iter;

		bool isAttached = false;
		for(auto& attachedInfo : attachedInfos)
			isAttached |= IsSameGamepad(attachedInfo, gamepad->GetInfo());

		if(isAttached)
		{
			++iter;
			continue;
		}

		mOwner.NotifyGamepadRemoved(gamepad->GetInfo().Id, gamepad->GetName());
		B3DDelete(gamepad);
		iter = mGamepads.erase(iter);
	}

	// Create newly attached gamepads
	for(auto& attachedInfo : attachedInfos)
	{
		bool exists = false;
		for(auto& gamepad : mGamepads)
			exists |= IsSameGamepad(attachedInfo, gamepad->GetInfo());

		if(exists)
			continue;

		// Assign the lowest free id, so device ids (and with them the per-device state slots in Input) get reused
		// instead of growing unbounded across plug/unplug cycles
		u32 id = 0;
		while(true)
		{
			bool isUsed = false;
			for(auto& gamepad : mGamepads)
				isUsed |= gamepad->GetInfo().Id == id;

			if(!isUsed)
				break;

			id++;
		}

		attachedInfo.Id = id;

		mGamepads.push_back(B3DNew<LinuxGamepad>(attachedInfo, mOwner));
		mOwner.NotifyGamepadAdded(attachedInfo.Id, attachedInfo.Name);
	}
}

namespace b3d
{
IInputBackend* CreateInputBackend(Input& owner)
{
	return B3DNew<LinuxInputBackend>(owner);
}
} // namespace b3d
