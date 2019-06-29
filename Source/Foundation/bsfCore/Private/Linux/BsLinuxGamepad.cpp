//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/BsGamepad.h"
#include "Input/BsInput.h"
#include "Private/Linux/BsLinuxInput.h"
#include <fcntl.h>
#include <linux/input.h>

namespace bs
{
	/** Contains private data for the Linux Gamepad implementation. */
	struct Gamepad::Pimpl
	{
		GamepadInfo info;
		INT32 fileHandle;
		ButtonCode povState;
		bool hasInputFocus;
	};

	Gamepad::Gamepad(const String& name, const GamepadInfo& gamepadInfo, Input* owner)
			: mName(name), mOwner(owner)
	{
		m = bs_new<Pimpl>();
		m->info = gamepadInfo;
		m->povState = BC_UNASSIGNED;
		m->hasInputFocus = true;

		String eventPath = "/dev/input/event" + toString(gamepadInfo.eventHandlerIdx);
		m->fileHandle = open(eventPath.c_str(), O_RDWR | O_NONBLOCK);

		if(m->fileHandle == -1)
			BS_LOG(Error, Platform, "Failed to open input event file handle for device: {0}", gamepadInfo.name);
	}

	Gamepad::~Gamepad()
	{
		if(m->fileHandle != -1)
			close(m->fileHandle);

		bs_delete(m);
	}

	void Gamepad::capture()
	{
		if(m->fileHandle == -1)
			return;

		struct AxisState
		{
			bool moved;
			INT32 value;
		};

		AxisState axisState[24];
		bs_zero_out(axisState);

		input_event events[BUFFER_SIZE_GAMEPAD];
		while(true)
		{
			ssize_t numReadBytes = read(m->fileHandle, &events, sizeof(events));
			if(numReadBytes < 0)
				break;

			if(!m->hasInputFocus)
				continue;

			UINT32 numEvents = numReadBytes / sizeof(input_event);
			for(UINT32 i = 0; i < numEvents; ++i)
			{
				switch(events[i].type)
				{
				case EV_KEY:
				{
					auto findIter = m->info.buttonMap.find(events[i].code);
					if(findIter == m->info.buttonMap.end())
						continue;

					if(events[i].value)
						mOwner->_notifyButtonPressed(m->info.id, findIter->second, (UINT64)events[i].time.tv_usec);
					else
						mOwner->_notifyButtonReleased(m->info.id, findIter->second, (UINT64)events[i].time.tv_usec);
				}
					break;
				case EV_ABS:
				{
					// Stick or trigger
					if(events[i].code <= ABS_BRAKE)
					{
						const AxisInfo& axisInfo = m->info.axisMap[events[i].code];

						if(axisInfo.axisIdx >= 24)
							break;

						axisState[axisInfo.axisIdx].moved = true;

						// Scale range if needed
						if(axisInfo.min == Gamepad::MIN_AXIS && axisInfo.max != Gamepad::MAX_AXIS )
							axisState[axisInfo.axisIdx].value = events[i].value;
						else
						{
							float range = (float)(axisInfo.max - axisInfo.min);
							float normalizedValue = (axisInfo.max - events[i].value) / range;

							range = (float)(Gamepad::MAX_AXIS - Gamepad::MIN_AXIS);
							axisState[axisInfo.axisIdx].value = Gamepad::MIN_AXIS + (INT32)(normalizedValue * range);
						}
					}
					else if(events[i].code <= ABS_HAT3Y) // POV
					{
						// Note: We only support a single POV and report events from all POVs as if they were from the
						// same source
						INT32 povIdx = events[i].code - ABS_HAT0X;

						ButtonCode povButton = BC_UNASSIGNED;
						if((povIdx & 0x1) == 0) // Even, x axis
						{
							if(events[i].value == -1)
								povButton = BC_GAMEPAD_DPAD_LEFT;
							else if(events[i].value == 1)
								povButton = BC_GAMEPAD_DPAD_RIGHT;
						}
						else // Odd, y axis
						{
							if(events[i].value == -1)
								povButton = BC_GAMEPAD_DPAD_UP;
							else if(events[i].value == 1)
								povButton = BC_GAMEPAD_DPAD_DOWN;
						}

						if(m->povState != povButton)
						{
							if(m->povState != BC_UNASSIGNED)
								mOwner->_notifyButtonReleased(m->info.id, m->povState, (UINT64)events[i].time.tv_usec);

							if(povButton != BC_UNASSIGNED)
								mOwner->_notifyButtonPressed(m->info.id, povButton, (UINT64)events[i].time.tv_usec);


							m->povState = povButton;
						}
					}
					break;
				}
				default: break;
				}
			}
		}

		for(UINT32 i = 0; i < 24; i++)
		{
			if(axisState[i].moved)
				mOwner->_notifyAxisMoved(m->info.id, i, axisState[i].value);
		}
	}

	void Gamepad::changeCaptureContext(UINT64 windowHandle)
	{
		m->hasInputFocus = windowHandle != (UINT64)-1;
	}
}

