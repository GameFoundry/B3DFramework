//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Input/BsMouse.h"
#include "Input/BsInput.h"
#include "Linux/BsLinuxInput.h"
#include <fcntl.h>
#include <linux/input.h>

namespace bs
{
	constexpr UINT32 MAX_DEVICES = 8;

	/** Contains private data for the Linux Mouse implementation. */
	struct Mouse::Pimpl
	{
		INT32 fileHandles[MAX_DEVICES];
	};

	Mouse::Mouse(const String& name, Input* owner)
		: mName(name), mOwner(owner)
	{
		InputPrivateData* pvtData = owner->_getPrivateData();

		m = bs_new<Pimpl>();

		for(UINT32 i = 0; i < MAX_DEVICES; i++)
			m->fileHandles[i] = -1;

		UINT32 idx = 0;
		for(auto& entry : pvtData->mice)
		{
			String eventPath = "/dev/input/event" + toString(entry);
			m->fileHandles[idx] = open(eventPath.c_str(), O_RDWR | O_NONBLOCK);

			if(m->fileHandles[idx] == -1)
				LOGERR("Failed to open input event file handle for device: " + mName);

			idx++;

			if(idx >= MAX_DEVICES)
				break;
		}
	}

	Mouse::~Mouse()
	{
		for(UINT32 i = 0; i < MAX_DEVICES; i++)
		{
			if(m->fileHandles[i] != -1)
				close(m->fileHandles[i]);
		}

		bs_delete(m);
	}

	void Mouse::capture()
	{
		INT32 relX = 0;
		INT32 relY = 0;
		INT32 relZ = 0;

		input_event events[BUFFER_SIZE_MOUSE];
		for(UINT32 i = 0; i < MAX_DEVICES; i++)
		{
			if(m->fileHandles[i] == -1)
				continue;

			while(true)
			{
				ssize_t numReadBytes = read(m->fileHandles[i], &events, sizeof(events));
				if(numReadBytes < 0)
					break;

				UINT32 numEvents = numReadBytes / sizeof(input_event);
				for(UINT32 j = 0; j < numEvents; ++j)
				{
					switch(events[j].type)
					{
					case EV_KEY:
					{
						ButtonCode bc = BC_UNASSIGNED;

						// Unnamed mouse buttons
						if(events[j].code >= BTN_MISC && events[j].code < BTN_MOUSE)
							bc = (ButtonCode)((INT32)BC_MOUSE_BTN4 + (events[j].code - BTN_MISC));
						// Named mouse buttons
						else if(events[j].code >= BTN_MOUSE && events[j].code < BTN_JOYSTICK)
						{
							switch (events[j].code)
							{
							case BTN_LEFT:
								bc = BC_MOUSE_LEFT;
								break;
							case BTN_MIDDLE:
								bc = BC_MOUSE_MIDDLE;
								break;
							case BTN_RIGHT:
								bc = BC_MOUSE_RIGHT;
								break;
							case BTN_SIDE:
								bc = BC_MOUSE_BTN15;
								break;
							case BTN_EXTRA:
								bc = BC_MOUSE_BTN16;
								break;
							case BTN_FORWARD:
								bc = BC_MOUSE_BTN17;
								break;
							case BTN_BACK:
								bc = BC_MOUSE_BTN18;
								break;
							case BTN_TASK:
								bc = BC_MOUSE_BTN19;
								break;
							default:
								break;
							}
						}

						if(bc != BC_UNASSIGNED)
						{
							if (events[j].value)
								mOwner->_notifyButtonPressed(0, bc, (UINT64) events[j].time.tv_usec);
							else
								mOwner->_notifyButtonReleased(0, bc, (UINT64) events[j].time.tv_usec);
						}
					}
						break;
					case EV_REL:
					{
						switch(events[j].code)
						{
						case REL_X:
							relX = events[j].value;
							break;
						case REL_Y:
							relY = events[i].value;
							break;
						case REL_WHEEL:
							relZ = events[i].value;
							break;
						default:
							break;
						}
						break;
					}
					default: break;
					}
				}
			}
		}

		if(relX != 0 || relY != 0 || relZ != 0)
			mOwner->_notifyMouseMoved(relX, relY, relZ);
	}

	void Mouse::changeCaptureContext(UINT64 windowHandle)
	{
		// Do nothing
	}
}

