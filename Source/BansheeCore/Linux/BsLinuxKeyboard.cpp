//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Input/BsKeyboard.h"
#include "Input/BsInput.h"
#include "Linux/BsLinuxInput.h"
#include <fcntl.h>
#include <linux/input.h>

namespace bs
{
	constexpr UINT32 MAX_DEVICES = 8;

	/** Contains private data for the Linux Keyboard implementation. */
	struct Keyboard::Pimpl
	{
		INT32 fileHandles[MAX_DEVICES];
	};

	Keyboard::Keyboard(const String& name, Input* owner)
		: mName(name), mOwner(owner)
	{
		InputPrivateData* pvtData = owner->_getPrivateData();

		m = bs_new<Pimpl>();

		for(UINT32 i = 0; i < MAX_DEVICES; i++)
			m->fileHandles[i] = -1;

		UINT32 idx = 0;
		for(auto& entry : pvtData->keyboards)
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

	Keyboard::~Keyboard()
	{
		for(UINT32 i = 0; i < MAX_DEVICES; i++)
		{
			if(m->fileHandles[i] != -1)
				close(m->fileHandles[i]);
		}

		bs_delete(m);
	}

	void Keyboard::capture()
	{
		input_event events[BUFFER_SIZE_KEYBOARD];
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
						ButtonCode bc = (ButtonCode)events[j].code;
						if(bc < BC_MOUSE_LEFT)
						{
							if(events[j].value)
								mOwner->_notifyButtonPressed(0, bc, (UINT64)events[j].time.tv_usec);
							else
								mOwner->_notifyButtonReleased(0, bc, (UINT64)events[j].time.tv_usec);
						}
					}
						break;
					default: break;
					}
				}
			}
		}
	}

	void Keyboard::changeCaptureContext(UINT64 windowHandle)
	{
		// Do nothing
	}
}

