//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Input/BsInput.h"
#include "Private/MacOS/BsMacOSInput.h"
#include "Input/BsMouse.h"
#include "Input/BsKeyboard.h"
#include "Input/BsGamepad.h"

namespace bs
{
	/**
	 * Helper method that creates a dictionary that is used for matching a specific set of devices (matching the provided
	 * page and usage values, as USB HID values), used for initializing a HIDManager.
	 */
	static CFDictionaryRef createHIDDeviceMatchDictionary(UINT32 page, UINT32 usage)
	{
		CFDictionaryRef output = nullptr;
		CFNumberRef pageNumRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &page);
		CFNumberRef usageNumRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);
		const void* keys[2] = { (void*) CFSTR(kIOHIDDeviceUsagePageKey), (void*) CFSTR(kIOHIDDeviceUsageKey) };
		const void* values[2] = { (void*) pageNumRef, (void*) usageNumRef };

		if (pageNumRef && usageNumRef)
		{
			output = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 2, &kCFTypeDictionaryKeyCallBacks,
					&kCFTypeDictionaryValueCallBacks);
		}

		if (pageNumRef)
			CFRelease(pageNumRef);

		if (usageNumRef)
			CFRelease(usageNumRef);

		return output;
	}

	/** Returns the name of the run loop used for processing events for the specified category of input devices. */
	static CFStringRef getRunLoopMode(HIDType type)
	{
		static CFStringRef KeyboardMode = CFSTR("BSKeyboard");
		static CFStringRef MouseMode = CFSTR("BSMouse");
		static CFStringRef GamepadMode = CFSTR("BSGamepad");

		switch(type)
		{
		case HIDType::Keyboard:
			return KeyboardMode;
		case HIDType::Mouse:
			return MouseMode;
		case HIDType::Gamepad:
			return GamepadMode;
		}

		return nullptr;
	}

	static void HIDAddElements(CFArrayRef array, HIDDevice* device);

	/**
	 * Callback called when enumerating an array of HID elements. Each element's information is parsed and stored in the
	 * owner HIDDevice (passed through @p passthrough parameter).
	 *
	 * @param[in] value 		IOHIDElementRef of the current element.
	 * @param[in] passthrough 	Pointer to element's parent HIDDevice.
	 */
	static void HIDAddElement(const void* value, void* passthrough)
	{
		auto device = (HIDDevice*)passthrough;
		auto elemRef = (IOHIDElementRef) value;

		if(!elemRef)
			return;

		CFTypeID typeID = CFGetTypeID(elemRef);
		if(typeID != IOHIDElementGetTypeID())
			return;

		IOHIDElementType type = IOHIDElementGetType(elemRef);
		switch (type)
		{
		case kIOHIDElementTypeInput_Button:
		case kIOHIDElementTypeInput_Axis:
		case kIOHIDElementTypeInput_Misc:
		case kIOHIDElementTypeInput_ScanCodes:
			break;
		case kIOHIDElementTypeCollection:
		{
			CFArrayRef array = IOHIDElementGetChildren(elemRef);
			if(array)
				HIDAddElements(array, device);
		}
			return;
		default:
			return;
		}

		UINT32 usagePage = IOHIDElementGetUsagePage(elemRef);
		UINT32 usage = IOHIDElementGetUsage(elemRef);

		enum ElemState { IsUnknown, IsButton, IsAxis, IsHat };
		ElemState state = IsUnknown;

		switch(usagePage)
		{
		case kHIDPage_Button:
		case kHIDPage_KeyboardOrKeypad:
			state = IsButton;
			break;
		case kHIDPage_GenericDesktop:
			switch(usage)
			{
			case kHIDUsage_GD_Start:
			case kHIDUsage_GD_Select:
			case kHIDUsage_GD_SystemMainMenu:
			case kHIDUsage_GD_DPadUp:
			case kHIDUsage_GD_DPadDown:
			case kHIDUsage_GD_DPadRight:
			case kHIDUsage_GD_DPadLeft:
				state = IsButton;
				break;
			case kHIDUsage_GD_X:
			case kHIDUsage_GD_Y:
			case kHIDUsage_GD_Z:
			case kHIDUsage_GD_Rx:
			case kHIDUsage_GD_Ry:
			case kHIDUsage_GD_Rz:
			case kHIDUsage_GD_Slider:
			case kHIDUsage_GD_Dial:
			case kHIDUsage_GD_Wheel:
				state = IsAxis;
				break;
			case kHIDUsage_GD_Hatswitch:
				state = IsHat;
				break;
			default:
				break;
			};
			break;
		case kHIDPage_Simulation:
			switch(usage)
			{
			case kHIDUsage_Sim_Rudder:
			case kHIDUsage_Sim_Throttle:
			case kHIDUsage_Sim_Accelerator:
			case kHIDUsage_Sim_Brake:
				state = IsAxis;
			default:
				break;
			}
			break;
		default:
			break;
		};

		Vector<HIDElement>* elements = nullptr;
		switch(state)
		{
		case IsButton:
			elements = &device->buttons;
			break;
		case IsAxis:
			elements = &device->axes;
			break;
		case IsHat:
			elements = &device->hats;
			break;
		default:
			break;
		}

		if(elements != nullptr)
		{
			HIDElement element;
			element.usage = usage;
			element.ref = elemRef;
			element.cookie = IOHIDElementGetCookie(elemRef);
			element.min = element.detectedMin = (INT32)IOHIDElementGetLogicalMin(elemRef);
			element.max = element.detectedMax = (INT32)IOHIDElementGetLogicalMax(elemRef);

			auto iterFind = std::find_if(elements->begin(), elements->end(),
					[&element](const HIDElement& v)
					{
						return v.cookie == element.cookie;
					});

			if(iterFind == elements->end())
				elements->push_back(element);
		}
	}

	/** Parses information about and registers all HID elements in @p array with the @p device. */
	void HIDAddElements(CFArrayRef array, HIDDevice* device)
	{
		CFRange range = { 0, CFArrayGetCount(array) };
		CFArrayApplyFunction(array, range, HIDAddElement, device);
	}

	/**
	 * Callback triggered when a HID manager detects a new device. Also called for existing devices when HID manager is
	 * first initialized.
	 */
	static void HIDDeviceAddedCallback(void* context, IOReturn result, void* sender, IOHIDDeviceRef device)
	{
		auto data = (HIDData*)context;

		for(auto& entry : data->devices)
		{
			if(entry.ref == device)
				return; // Duplicate
		}

		HIDDevice newDevice;
		newDevice.ref = device;

		// Parse device name
		CFTypeRef propertyRef = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
		if(!propertyRef)
			propertyRef = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDManufacturerKey));

		if(propertyRef)
		{
			char buffer[256];
			if(CFStringGetCString((CFStringRef)propertyRef, buffer,	sizeof(buffer), kCFStringEncodingUTF8))
				newDevice.name = String(buffer);
		}

		// Parse device elements
		CFArrayRef elements = IOHIDDeviceCopyMatchingElements(device, nullptr, kIOHIDOptionsTypeNone);
		if(elements)
		{
			HIDAddElements(elements, &newDevice);
			CFRelease(elements);
		}

		// Create a queue
		newDevice.queueRef = IOHIDQueueCreate(kCFAllocatorDefault, device, 128, kIOHIDOptionsTypeNone);

		for(auto& button : newDevice.buttons)
			IOHIDQueueAddElement(newDevice.queueRef, button.ref);

		for(auto& hat : newDevice.hats)
			IOHIDQueueAddElement(newDevice.queueRef, hat.ref);

		IOHIDQueueStart(newDevice.queueRef);

		// Assign a device ID
		if(data->type == HIDType::Gamepad)
		{
			auto freeId = (UINT32)-1;

			auto numDevices = (UINT32)data->devices.size();
			for(UINT32 i = 0; i < numDevices; i++)
			{
				bool validId = true;
				for(auto& entry : data->devices)
				{
					if(entry.id == i)
					{
						validId = false;
						break;
					}
				}

				if(validId)
				{
					freeId = i;
					break;
				}
			}

			if(freeId == (UINT32)-1)
				freeId = numDevices;

			newDevice.id = freeId;
		}
		else // All keyboard/mouse devices are coalesced into a single device
			newDevice.id = 0;

		data->devices.push_back(newDevice);

		// Register the gamepad device with Input manager
		if(data->type == HIDType::Gamepad)
		{
			InputPrivateData* pvtData = data->owner->_getPrivateData();

			GamepadInfo gamepadInfo;
			gamepadInfo.name = newDevice.name;
			gamepadInfo.id = newDevice.id;
			gamepadInfo.deviceRef = newDevice.ref;
			gamepadInfo.hid = nullptr;

			pvtData->gamepadInfos.push_back(gamepadInfo);
		}
	}

	/** Callback triggered when an input device is removed. */
	static void HIDDeviceRemovedCallback(void* context, IOReturn result, void* sender, IOHIDDeviceRef device)
	{
		auto data = (HIDData*)context;

		auto iterFind = std::find_if(data->devices.begin(), data->devices.end(),
				[&device](const HIDDevice& v)
				{
					return v.ref == device;
				});

		if(iterFind != data->devices.end())
		{
			IOHIDQueueStop(iterFind->queueRef);
			CFRelease(iterFind->queueRef);

			// Unregister the gamepad device from the Input manager
			if(data->type == HIDType::Gamepad)
			{
				InputPrivateData* pvtData = data->owner->_getPrivateData();

				UINT32 deviceId = iterFind->id;
				auto iterFind2 = std::find_if(
					pvtData->gamepadInfos.begin(),
					pvtData->gamepadInfos.end(),
					[deviceId](const GamepadInfo& info)
					{
						return info.id == deviceId;
					});

				if(iterFind2 != pvtData->gamepadInfos.end())
					pvtData->gamepadInfos.erase(iterFind2);
			}

			data->devices.erase(iterFind);
		}
	}

	/** Reads the current value of a particular HID element (e.g. button, axis). */
	static INT32 HIDGetElementValue(const HIDDevice &device, const HIDElement &element)
	{
		IOHIDValueRef valueRef;
		if(IOHIDDeviceGetValue(device.ref, element.ref, &valueRef) != kIOReturnSuccess)
			return 0;

		auto value = (INT32) IOHIDValueGetIntegerValue(valueRef);

		if(value < element.detectedMin)
			element.detectedMin = value;

		if(value > element.detectedMax)
			element.detectedMax = value;

		return value;
	}

	/**
	 * Reads the current value of a particular HID element (e.g. button, axis) and converts the value so it fits
	 * the provided [min, max] range.
	 */
	static INT32 HIDGetElementValueScaled(const HIDDevice &device, const HIDElement &element, INT32 min, INT32 max)
	{
		INT32 value = HIDGetElementValue(device, element);

		float deviceRange = element.detectedMax - element.detectedMin;
		if(deviceRange == 0.0f)
			return value;

		float normalizedRange = (value - element.detectedMin) / deviceRange;

		float targetRange = max - min;
		return (INT32)(normalizedRange * targetRange) + min;
	}

	/** Converts a keyboard scan key (as reported by the HID manager) into engine's ButtonCode. */
	static ButtonCode scanCodeToKeyCode(UINT32 scanCode)
	{
		switch(scanCode)
		{
		case 0x04: return BC_A;
		case 0x05: return BC_B;
		case 0x06: return BC_C;
		case 0x07: return BC_D;
		case 0x08: return BC_E;
		case 0x09: return BC_F;
		case 0x0a: return BC_G;
		case 0x0b: return BC_H;
		case 0x0c: return BC_I;
		case 0x0d: return BC_J;
		case 0x0e: return BC_K;
		case 0x0f: return BC_L;
		case 0x10: return BC_M;
		case 0x11: return BC_N;
		case 0x12: return BC_O;
		case 0x13: return BC_P;
		case 0x14: return BC_Q;
		case 0x15: return BC_R;
		case 0x16: return BC_S;
		case 0x17: return BC_T;
		case 0x18: return BC_U;
		case 0x19: return BC_V;
		case 0x1a: return BC_W;
		case 0x1b: return BC_X;
		case 0x1c: return BC_Y;
		case 0x1d: return BC_Z;

		case 0x1e: return BC_1;
		case 0x1f: return BC_2;
		case 0x20: return BC_3;
		case 0x21: return BC_4;
		case 0x22: return BC_5;
		case 0x23: return BC_6;
		case 0x24: return BC_7;
		case 0x25: return BC_8;
		case 0x26: return BC_9;
		case 0x27: return BC_0;

		case 0x28: return BC_RETURN;
		case 0x29: return BC_ESCAPE;
		case 0x2a: return BC_BACK;
		case 0x2b: return BC_TAB;
		case 0x2c: return BC_SPACE;
		case 0x2d: return BC_MINUS;
		case 0x2e: return BC_EQUALS;
		case 0x2f: return BC_LBRACKET;
		case 0x30: return BC_RBRACKET;
		case 0x31: return BC_BACKSLASH;
		case 0x32: return BC_GRAVE;
		case 0x33: return BC_SEMICOLON;
		case 0x34: return BC_APOSTROPHE;
		case 0x35: return BC_GRAVE;
		case 0x36: return BC_COMMA;
		case 0x37: return BC_PERIOD;
		case 0x38: return BC_SLASH;
		case 0x39: return BC_CAPITAL;

		case 0x3a: return BC_F1;
		case 0x3b: return BC_F2;
		case 0x3c: return BC_F3;
		case 0x3d: return BC_F4;
		case 0x3e: return BC_F5;
		case 0x3f: return BC_F6;
		case 0x40: return BC_F7;
		case 0x41: return BC_F8;
		case 0x42: return BC_F9;
		case 0x43: return BC_F10;
		case 0x44: return BC_F11;
		case 0x45: return BC_F12;

		case 0x46: return BC_SYSRQ;
		case 0x47: return BC_SCROLL;
		case 0x48: return BC_PAUSE;
		case 0x49: return BC_INSERT;
		case 0x4a: return BC_HOME;
		case 0x4b: return BC_PGUP;
		case 0x4c: return BC_DELETE;
		case 0x4d: return BC_END;
		case 0x4e: return BC_PGDOWN;
		case 0x4f: return BC_RIGHT;
		case 0x50: return BC_LEFT;
		case 0x51: return BC_DOWN;
		case 0x52: return BC_UP;

		case 0x53: return BC_NUMLOCK;
		case 0x54: return BC_DIVIDE;
		case 0x55: return BC_MULTIPLY;
		case 0x56: return BC_SUBTRACT;
		case 0x57: return BC_ADD;
		case 0x58: return BC_NUMPADENTER;
		case 0x59: return BC_NUMPAD1;
		case 0x5a: return BC_NUMPAD2;
		case 0x5b: return BC_NUMPAD3;
		case 0x5c: return BC_NUMPAD4;
		case 0x5d: return BC_NUMPAD5;
		case 0x5e: return BC_NUMPAD6;
		case 0x5f: return BC_NUMPAD7;
		case 0x60: return BC_NUMPAD8;
		case 0x61: return BC_NUMPAD9;
		case 0x62: return BC_NUMPAD0;
		case 0x63: return BC_NUMPADCOMMA;

		case 0x64: return BC_OEM_102;
		case 0x66: return BC_POWER;
		case 0x67: return BC_NUMPADEQUALS;

		case 0x68: return BC_F13;
		case 0x69: return BC_F14;
		case 0x6a: return BC_F15;

		case 0x78: return BC_STOP;
		case 0x7f: return BC_MUTE;
		case 0x80: return BC_VOLUMEUP;
		case 0x81: return BC_VOLUMEDOWN;
		case 0x85: return BC_NUMPADCOMMA;
		case 0x86: return BC_NUMPADEQUALS;
		case 0x89: return BC_YEN;

		case 0xe0: return BC_LCONTROL;
		case 0xe1: return BC_LSHIFT;
		case 0xe2: return BC_LMENU;
		case 0xe3: return BC_LWIN;
		case 0xe4: return BC_RCONTROL;
		case 0xe5: return BC_RSHIFT;
		case 0xe6: return BC_RMENU;
		case 0xe7: return BC_RWIN;

		case 0xe8: return BC_PLAYPAUSE;
		case 0xe9: return BC_MEDIASTOP;
		case 0xea: return BC_PREVTRACK;
		case 0xeb: return BC_NEXTTRACK;
		case 0xed: return BC_VOLUMEUP;
		case 0xee: return BC_VOLUMEDOWN;
		case 0xef: return BC_MUTE;
		case 0xf0: return BC_WEBSEARCH;
		case 0xf1: return BC_WEBBACK;
		case 0xf2: return BC_WEBFORWARD;
		case 0xf3: return BC_WEBSTOP;
		case 0xf4: return BC_WEBSEARCH;
		case 0xf8: return BC_SLEEP;
		case 0xf9: return BC_WAKE;
		case 0xfb: return BC_CALCULATOR;

		default:
			return BC_UNASSIGNED;
		}
	}

	HIDManager::HIDManager(HIDType type, Input* input)
	{
		mData.type = type;
		mData.owner = input;

		mHIDManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDManagerOptionNone);
		if(mHIDManager == nullptr)
			return;

		if(IOHIDManagerOpen(mHIDManager, kIOHIDOptionsTypeNone) != kIOReturnSuccess)
			return;

		UINT32 numEntries = 0;
		const void* entries[3];

		switch (type)
		{
		case HIDType::Keyboard:
			entries[0] = createHIDDeviceMatchDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_Keyboard);
			numEntries = 1;
			break;
		case HIDType::Mouse:
			entries[0] = createHIDDeviceMatchDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_Mouse);
			numEntries = 1;
			break;
		case HIDType::Gamepad:
			entries[0] = createHIDDeviceMatchDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick);
			entries[1] = createHIDDeviceMatchDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad);
			entries[2] = createHIDDeviceMatchDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_MultiAxisController);
			numEntries = 3;
			break;
		}

		CFArrayRef entryArray = CFArrayCreate(kCFAllocatorDefault, entries, numEntries, &kCFTypeArrayCallBacks);

		IOHIDManagerSetDeviceMatchingMultiple(mHIDManager, entryArray);
		IOHIDManagerRegisterDeviceMatchingCallback(mHIDManager, HIDDeviceAddedCallback, &mData);
		IOHIDManagerRegisterDeviceRemovalCallback(mHIDManager, HIDDeviceRemovedCallback, &mData);

		CFStringRef runLoopMode = getRunLoopMode(type);
		IOHIDManagerScheduleWithRunLoop(mHIDManager, CFRunLoopGetCurrent(), runLoopMode);

		while(CFRunLoopRunInMode(runLoopMode, 0, TRUE) == kCFRunLoopRunHandledSource)
		{ /* Do nothing */ }

		for (UINT32 i = 0; i < numEntries; i++)
		{
			if (entries[i])
				CFRelease((CFTypeRef) entries[i]);
		}

		CFRelease(entryArray);
	}

	HIDManager::~HIDManager()
	{
		for(auto& device : mData.devices)
		{
			IOHIDQueueStop(device.queueRef);
			CFRelease(device.queueRef);
		}

		CFStringRef runLoopMode = getRunLoopMode(mData.type);
		IOHIDManagerUnscheduleFromRunLoop(mHIDManager, CFRunLoopGetCurrent(), runLoopMode);

		IOHIDManagerClose(mHIDManager, kIOHIDOptionsTypeNone);
		CFRelease(mHIDManager);
	}

	void HIDManager::capture(IOHIDDeviceRef device, bool ignoreEvents)
	{
		// First trigger any callbacks
		CFStringRef runLoopMode = getRunLoopMode(mData.type);
		while(CFRunLoopRunInMode(runLoopMode, 0, TRUE) == kCFRunLoopRunHandledSource)
		{ /* Do nothing */ }

		for(auto& entry : mData.devices)
		{
			if(device != nullptr && entry.ref != device)
				continue;

			// Read non-queued elements
			if(!ignoreEvents)
			{
				INT32 relX, relY, relZ;
				relX = relY = relZ = 0;

				struct AxisState
				{
					bool moved;
					INT32 value;
				};

				AxisState axisValues[24];
				bs_zero_out(axisValues);

				for (auto& axis : entry.axes)
				{
					auto axisType = (InputAxis) -1;

					if (mData.type == HIDType::Gamepad)
					{
						INT32 axisValue = HIDGetElementValueScaled(entry, axis, Gamepad::MIN_AXIS, Gamepad::MAX_AXIS);
						INT32 lastInputAxis = (INT32) InputAxis::RightTrigger + 1;
						switch (axis.usage)
						{
						case kHIDUsage_GD_X:
							axisType = InputAxis::LeftStickX;
							break;
						case kHIDUsage_GD_Y:
							axisType = InputAxis::LeftStickY;
							break;
						case kHIDUsage_GD_Rx:
							axisType = InputAxis::RightStickX;
							break;
						case kHIDUsage_GD_Ry:
							axisType = InputAxis::RightStickY;
							break;
						case kHIDUsage_GD_Z:
							axisType = InputAxis::LeftTrigger;
							break;
						case kHIDUsage_GD_Rz:
							axisType = InputAxis::RightTrigger;
							break;
						case kHIDUsage_GD_Slider:
							axisType = (InputAxis) (lastInputAxis + 1);
							break;
						case kHIDUsage_GD_Dial:
							axisType = (InputAxis) (lastInputAxis + 2);
							break;
						case kHIDUsage_GD_Wheel:
							axisType = (InputAxis) (lastInputAxis + 3);
							break;
						case kHIDUsage_Sim_Rudder:
							axisType = (InputAxis) (lastInputAxis + 4);
							break;
						case kHIDUsage_Sim_Throttle:
							axisType = (InputAxis) (lastInputAxis + 5);
							break;
						case kHIDUsage_Sim_Accelerator:
							axisType = (InputAxis) (lastInputAxis + 6);
							break;
						case kHIDUsage_Sim_Brake:
							axisType = (InputAxis) (lastInputAxis + 7);
							break;
						default:
							break;
						}

						if((INT32)axisType < 24)
						{
							axisValues[(INT32)axisType].moved = true;
							axisValues[(INT32)axisType].value = axisValue;
						}

					}
					else if (mData.type == HIDType::Mouse)
					{
						INT32 axisValue = HIDGetElementValue(entry, axis);
						switch (axis.usage)
						{
						case kHIDUsage_GD_X:
							axisType = InputAxis::MouseX;
							relX += axisValue;
							break;
						case kHIDUsage_GD_Y:
							axisType = InputAxis::MouseY;
							relY += axisValue;
							break;
						case kHIDUsage_GD_Z:
							axisType = InputAxis::MouseZ;
							relZ += axisValue;
							break;
						default:
							break;
						}
					}
				}

				static int dbgi = 0;
				if(relX != 0 || relY != 0 || relZ != 0)
				{
					LOGWRN("Mouse move " + toString(relX) + " " + toString(relY) + " " + toString(relZ) + " " +
						toString(dbgi++));
					mData.owner->_notifyMouseMoved(relX, relY, relZ);
				}

				for(UINT32 i = 0; i < 24; i++)
				{
					if(axisValues[i].moved)
						mData.owner->_notifyAxisMoved(entry.id, (UINT32)i, axisValues[i].value);
				}
			}

			// Read queued elements
			while(true)
			{
				IOHIDValueRef valueRef = IOHIDQueueCopyNextValueWithTimeout(entry.queueRef, 0);
				if(!valueRef)
					break;

				if(ignoreEvents)
					continue;

				IOHIDElementRef elemRef = IOHIDValueGetElement(valueRef);
				auto value = (INT32) IOHIDValueGetIntegerValue(valueRef); // For buttons this is 1 when pressed, 0 when released
				UINT64 timestamp = IOHIDValueGetTimeStamp(valueRef);

				UINT32 usage = IOHIDElementGetUsage(elemRef);
				UINT32 usagePage = IOHIDElementGetUsagePage(elemRef);

				ButtonCode button = BC_UNASSIGNED;
				if(usagePage == kHIDPage_GenericDesktop)
				{
					if (usage == kHIDUsage_GD_Hatswitch)
					{
						switch (value)
						{
						case 0:
							button = BC_GAMEPAD_DPAD_UP;
							break;
						case 1:
							button = BC_GAMEPAD_DPAD_UPRIGHT;
							break;
						case 2:
							button = BC_GAMEPAD_DPAD_RIGHT;
							break;
						case 3:
							button = BC_GAMEPAD_DPAD_DOWNRIGHT;
							break;
						case 4:
							button = BC_GAMEPAD_DPAD_DOWN;
							break;
						case 5:
							button = BC_GAMEPAD_DPAD_DOWNLEFT;
							break;
						case 6:
							button = BC_GAMEPAD_DPAD_LEFT;
							break;
						case 7:
							button = BC_GAMEPAD_DPAD_UPLEFT;
							break;
						default:
							break;
						}
					}
				}
				else if(usagePage == kHIDPage_Button)
				{
					if(mData.type == HIDType::Mouse)
					{
						if (usage > 0 && usage <= BC_NumMouse)
							button = (ButtonCode) ((UINT32) BC_MOUSE_LEFT + usage - 1);
					}
					else if(mData.type == HIDType::Gamepad)
					{
						// These are based on the xbox controller:
						switch(usage)
						{
						case 0: break;
						case 1: button = BC_GAMEPAD_A; break;
						case 2: button = BC_GAMEPAD_B; break;
						case 3: button = BC_GAMEPAD_X; break;
						case 4: button = BC_GAMEPAD_Y; break;
						case 5: button = BC_GAMEPAD_LB; break;
						case 6: button = BC_GAMEPAD_RB; break;
						case 7: button = BC_GAMEPAD_LS; break;
						case 8: button = BC_GAMEPAD_RS; break;
						case 9: button = BC_GAMEPAD_START; break;
						case 10: button = BC_GAMEPAD_BACK; break;
						case 11: button = BC_GAMEPAD_BTN1; break;
						case 12: button = BC_GAMEPAD_DPAD_UP; break;
						case 13: button = BC_GAMEPAD_DPAD_DOWN; break;
						case 14: button = BC_GAMEPAD_DPAD_LEFT; break;
						case 15: button = BC_GAMEPAD_DPAD_RIGHT; break;
						default:
						{
							INT32 buttonIdx = usage - 16;
							if(buttonIdx < 19)
								button = (ButtonCode)((INT32)(BC_GAMEPAD_BTN2 + buttonIdx));
						}
							break;
						}
					}
				}
				else if(usagePage == kHIDPage_KeyboardOrKeypad)
				{
					// Usage -1 and 1 are special signals that happen along with every button press/release and should be
					// ignored
					if(usage != -1 && usage != 1)
						button = scanCodeToKeyCode((UINT32)usage);
				}

				if(button != BC_UNASSIGNED)
				{
					if(value != 0)
						mData.owner->_notifyButtonPressed(entry.id, button, timestamp);
					else
						mData.owner->_notifyButtonReleased(entry.id, button, timestamp);
				}

				CFRelease(valueRef);
			}
		}
	}

	void Input::initRawInput()
	{
		mPlatformData = bs_new<InputPrivateData>();

		mKeyboard = bs_new<Keyboard>("Keyboard", this);
		mMouse = bs_new<Mouse>("Mouse", this);

		mPlatformData->gamepadHIDManager = bs_new<HIDManager>(HIDType::Gamepad, this);

		for(auto& entry : mPlatformData->gamepadInfos)
		{
			entry.hid = mPlatformData->gamepadHIDManager;
			mGamepads.push_back(bs_new<Gamepad>(entry.name, entry, this));
		}
	}

	void Input::cleanUpRawInput()
	{
		if (mMouse != nullptr)
			bs_delete(mMouse);

		if (mKeyboard != nullptr)
			bs_delete(mKeyboard);

		for (auto& gamepad : mGamepads)
			bs_delete(gamepad);

		bs_delete(mPlatformData->gamepadHIDManager);

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

