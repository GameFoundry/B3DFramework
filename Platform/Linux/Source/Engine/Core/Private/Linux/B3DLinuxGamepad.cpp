//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DInput.h"
#include "Private/Linux/B3DLinuxInput.h"

#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

using namespace b3d;

// Number of axis slots tracked per gamepad. Covers the common InputAxis entries plus a few extra slots that
// unrecognized device axes get mapped to (starting at InputAxis::Count).
static constexpr u32 kMaxGamepadAxes = 24;

/** Converts an evdev event timestamp into milliseconds. */
u64 EventTimeToMs(const timeval& time)
{
	return (u64)time.tv_sec * 1000 + (u64)time.tv_usec / 1000;
}

/** Converts the combined horizontal/vertical hat direction (each in [-1, 1]) into a single 8-way POV button code. */
ButtonCode PovToButtonCode(i32 x, i32 y)
{
	if(y < 0)
	{
		if(x < 0)
			return ButtonCode::GamepadDPadUpLeft;

		if(x > 0)
			return ButtonCode::GamepadDPadUpRight;

		return ButtonCode::GamepadDPadUp;
	}

	if(y > 0)
	{
		if(x < 0)
			return ButtonCode::GamepadDPadDownLeft;

		if(x > 0)
			return ButtonCode::GamepadDPadDownRight;

		return ButtonCode::GamepadDPadDown;
	}

	if(x < 0)
		return ButtonCode::GamepadDPadLeft;

	if(x > 0)
		return ButtonCode::GamepadDPatRight;

	return ButtonCode::Unassigned;
}

/**
 * Converts an absolute axis value from the device-reported [min, max] range into the engine axis range. Trigger axes
 * are mapped to [0, kMaxAxis] so an idle trigger reports zero, matching the other platform backends. All other axes
 * are mapped to the full [kMinAxis, kMaxAxis] range.
 */
i32 RescaleAxisValue(const AxisInfo& axisInfo, i32 value)
{
	const bool isTrigger = axisInfo.AxisIdx == (i32)InputAxis::LeftTrigger ||
		axisInfo.AxisIdx == (i32)InputAxis::RightTrigger;

	const i32 targetMin = isTrigger ? 0 : IInputBackend::kMinAxis;
	if(axisInfo.Min == targetMin && axisInfo.Max == IInputBackend::kMaxAxis)
		return value;

	const float range = (float)(axisInfo.Max - axisInfo.Min);
	if(range <= 0.0f)
		return 0;

	const float normalized = (float)(value - axisInfo.Min) / range;
	return targetMin + (i32)(normalized * (float)(IInputBackend::kMaxAxis - targetMin));
}

LinuxGamepad::LinuxGamepad(const GamepadInfo& gamepadInfo, Input& owner)
	: mOwner(owner), mInfo(gamepadInfo)
{
	mPovState.Code = ButtonCode::Unassigned;
	mPovState.Pressed = false;

	const String eventPath = "/dev/input/event" + ToString(mInfo.EventHandlerIdx);
	mFileHandle = open(eventPath.c_str(), O_RDONLY | O_NONBLOCK);

	if(mFileHandle == -1)
		B3D_LOG(Error, LogPlatform, "Failed to open input event file handle for device: {0}.", mInfo.Name);
}

LinuxGamepad::~LinuxGamepad()
{
	if(mFileHandle != -1)
		close(mFileHandle);
}

void LinuxGamepad::Capture()
{
	if(mFileHandle == -1)
		return;

	struct AxisState
	{
		bool Moved;
		i32 Value;
	};

	AxisState axisState[kMaxGamepadAxes];
	B3DZeroOut(axisState);

	input_event events[BUFFER_SIZE_GAMEPAD];
	while(true)
	{
		const ssize_t numReadBytes = read(mFileHandle, &events, sizeof(events));
		if(numReadBytes < 0)
			break;

		// Drain the events but discard them while another window owns input
		if(!mHasInputFocus)
			continue;

		const u32 numEvents = numReadBytes / sizeof(input_event);
		for(u32 eventIndex = 0; eventIndex < numEvents; ++eventIndex)
		{
			const input_event& event = events[eventIndex];
			switch(event.type)
			{
			case EV_KEY:
				{
					auto findIter = mInfo.ButtonMap.find(event.code);
					if(findIter == mInfo.ButtonMap.end())
						continue;

					if(event.value)
						mOwner.NotifyButtonPressed(mInfo.Id, findIter->second, EventTimeToMs(event.time));
					else
						mOwner.NotifyButtonReleased(mInfo.Id, findIter->second, EventTimeToMs(event.time));
				}
				break;
			case EV_ABS:
				{
					// Stick or trigger
					if(event.code <= ABS_BRAKE)
					{
						auto findIter = mInfo.AxisMap.find(event.code);
						if(findIter == mInfo.AxisMap.end())
							continue;

						const AxisInfo& axisInfo = findIter->second;
						if(axisInfo.AxisIdx < 0 || axisInfo.AxisIdx >= (i32)kMaxGamepadAxes)
							continue;

						axisState[axisInfo.AxisIdx].Moved = true;
						axisState[axisInfo.AxisIdx].Value = RescaleAxisValue(axisInfo, event.value);
					}
					else if(event.code <= ABS_HAT3Y) // POV (DPad)
					{
						// Note: We only support a single POV and report events from all POVs as if they were from the
						// same source
						const i32 povIdx = event.code - ABS_HAT0X;

						if((povIdx & 0x1) == 0) // Even, x axis
							mPovX = event.value;
						else // Odd, y axis
							mPovY = event.value;

						const u64 timestamp = EventTimeToMs(event.time);
						const ButtonCode povButton = PovToButtonCode(mPovX, mPovY);
						if(povButton != ButtonCode::Unassigned) // Pressed
						{
							// Another button was previously pressed
							if(mPovState.Pressed)
							{
								// If its a different button, release the old one and press the new one
								if(mPovState.Code != povButton)
								{
									mOwner.NotifyButtonReleased(mInfo.Id, mPovState.Code, timestamp);
									mOwner.NotifyButtonPressed(mInfo.Id, povButton, timestamp);

									mPovState.Code = povButton;
								}
							}
							else
							{
								mOwner.NotifyButtonPressed(mInfo.Id, povButton, timestamp);
								mPovState.Code = povButton;
								mPovState.Pressed = true;
							}
						}
						else if(mPovState.Pressed)
						{
							mOwner.NotifyButtonReleased(mInfo.Id, mPovState.Code, timestamp);
							mPovState.Pressed = false;
						}
					}
				}
				break;
			default:
				break;
			}
		}
	}

	for(u32 axisIndex = 0; axisIndex < kMaxGamepadAxes; axisIndex++)
	{
		if(axisState[axisIndex].Moved)
			mOwner.NotifyAxisMoved(mInfo.Id, axisIndex, axisState[axisIndex].Value);
	}
}

void LinuxGamepad::ChangeCaptureContext(u64 windowHandle)
{
	mHasInputFocus = windowHandle != 0;
}
