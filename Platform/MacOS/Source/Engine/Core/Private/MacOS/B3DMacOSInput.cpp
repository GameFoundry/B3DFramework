//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DInput.h"
#include "Private/MacOS/B3DMacOSInput.h"

#include "B3DApplication.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"

#include <mach/mach_time.h>

using namespace b3d;

/** Converts a mach absolute time value (as reported by HID event timestamps) into milliseconds. */
static u64 MachTimeToMs(u64 machTime)
{
	static mach_timebase_info_data_t timebase = []()
	{
		mach_timebase_info_data_t info;
		mach_timebase_info(&info);
		return info;
	}();

	return machTime * timebase.numer / timebase.denom / 1000000;
}

/**
 * Helper method that creates a dictionary that is used for matching a specific set of devices (matching the provided
 * page and usage values, as USB HID values), used for initializing a HIDManager.
 */
static CFDictionaryRef CreateHIDDeviceMatchDictionary(u32 page, u32 usage)
{
	CFDictionaryRef output = nullptr;
	CFNumberRef pageNumRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &page);
	CFNumberRef usageNumRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);
	const void* keys[2] = { (void*)CFSTR(kIOHIDDeviceUsagePageKey), (void*)CFSTR(kIOHIDDeviceUsageKey) };
	const void* values[2] = { (void*)pageNumRef, (void*)usageNumRef };

	if(pageNumRef && usageNumRef)
	{
		output = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 2, &kCFTypeDictionaryKeyCallBacks,
			&kCFTypeDictionaryValueCallBacks);
	}

	if(pageNumRef)
		CFRelease(pageNumRef);

	if(usageNumRef)
		CFRelease(usageNumRef);

	return output;
}

/** Returns the name of the run loop used for processing events for the specified category of input devices. */
static CFStringRef GetRunLoopMode(HIDType type)
{
	static CFStringRef KeyboardMode = CFSTR("B3DKeyboard");
	static CFStringRef MouseMode = CFSTR("B3DMouse");
	static CFStringRef GamepadMode = CFSTR("B3DGamepad");

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
	auto elemRef = (IOHIDElementRef)value;

	if(!elemRef)
		return;

	CFTypeID typeID = CFGetTypeID(elemRef);
	if(typeID != IOHIDElementGetTypeID())
		return;

	IOHIDElementType type = IOHIDElementGetType(elemRef);
	switch(type)
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

	u32 usagePage = IOHIDElementGetUsagePage(elemRef);
	u32 usage = IOHIDElementGetUsage(elemRef);

	enum ElemState
	{
		IsUnknown,
		IsButton,
		IsAxis,
		IsHat
	};

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
		elements = &device->Buttons;
		break;
	case IsAxis:
		elements = &device->Axes;
		break;
	case IsHat:
		elements = &device->Hats;
		break;
	default:
		break;
	}

	if(elements != nullptr)
	{
		HIDElement element;
		element.Usage = usage;
		element.Ref = elemRef;
		element.Cookie = IOHIDElementGetCookie(elemRef);
		element.Min = element.DetectedMin = (i32)IOHIDElementGetLogicalMin(elemRef);
		element.Max = element.DetectedMax = (i32)IOHIDElementGetLogicalMax(elemRef);

		auto iterFind = std::find_if(elements->begin(), elements->end(), [&element](const HIDElement& v)
									 { return v.Cookie == element.Cookie; });

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

	for(auto& entry : data->Devices)
	{
		if(entry.Ref == device)
			return; // Duplicate
	}

	HIDDevice newDevice;
	newDevice.Ref = device;
	newDevice.PovState = ButtonCode::Unassigned;
	B3DZeroOut(newDevice.GamepadAxisTimestamps);

	// Parse device name
	CFTypeRef propertyRef = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
	if(!propertyRef)
		propertyRef = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDManufacturerKey));

	if(propertyRef)
	{
		char buffer[256];
		if(CFStringGetCString((CFStringRef)propertyRef, buffer, sizeof(buffer), kCFStringEncodingUTF8))
			newDevice.Name = String(buffer);
	}

	// Parse device elements
	CFArrayRef elements = IOHIDDeviceCopyMatchingElements(device, nullptr, kIOHIDOptionsTypeNone);
	if(elements)
	{
		HIDAddElements(elements, &newDevice);
		CFRelease(elements);
	}

	// Create a queue
	newDevice.QueueRef = IOHIDQueueCreate(kCFAllocatorDefault, device, 128, kIOHIDOptionsTypeNone);

	for(auto& button : newDevice.Buttons)
		IOHIDQueueAddElement(newDevice.QueueRef, button.Ref);

	for(auto& hat : newDevice.Hats)
		IOHIDQueueAddElement(newDevice.QueueRef, hat.Ref);

	IOHIDQueueStart(newDevice.QueueRef);

	// Assign a device ID
	if(data->Type == HIDType::Gamepad)
	{
		// Assign the lowest free id, so device ids (and with them the per-device state slots in Input) get reused
		// instead of growing unbounded across plug/unplug cycles
		u32 id = 0;
		while(true)
		{
			bool isUsed = false;
			for(auto& entry : data->Devices)
				isUsed |= entry.Id == id;

			if(!isUsed)
				break;

			id++;
		}

		newDevice.Id = id;
	}
	else // All keyboard/mouse devices are coalesced into a single device
		newDevice.Id = 0;

	data->Devices.push_back(newDevice);

	if(data->Type == HIDType::Gamepad)
		data->Owner->NotifyGamepadAdded(newDevice.Id, newDevice.Name);
}

/** Callback triggered when an input device is removed. */
static void HIDDeviceRemovedCallback(void* context, IOReturn result, void* sender, IOHIDDeviceRef device)
{
	auto data = (HIDData*)context;

	auto iterFind = std::find_if(data->Devices.begin(), data->Devices.end(), [&device](const HIDDevice& v)
								 { return v.Ref == device; });

	if(iterFind != data->Devices.end())
	{
		IOHIDQueueStop(iterFind->QueueRef);
		CFRelease(iterFind->QueueRef);

		// Release any input the device was holding at the moment of removal
		if(data->Type == HIDType::Gamepad)
			data->Owner->NotifyGamepadRemoved(iterFind->Id, iterFind->Name);

		data->Devices.erase(iterFind);
	}
}

/**
 * Converts a raw HID axis value into the engine axis range [targetMin, kMaxAxis], scaling by the observed device
 * range. The observed range starts out as the device-reported logical range and expands as larger values get detected.
 */
static i32 ScaleHIDAxisValue(const HIDElement& element, i32 value, i32 targetMin)
{
	if(value < element.DetectedMin)
		element.DetectedMin = value;

	if(value > element.DetectedMax)
		element.DetectedMax = value;

	const float range = (float)(element.DetectedMax - element.DetectedMin);
	if(range <= 0.0f)
		return value;

	const float normalized = (value - element.DetectedMin) / range;
	return targetMin + (i32)(normalized * (float)(IInputBackend::kMaxAxis - targetMin));
}

/** Callback triggered when an input value changes. Only registered for mice, where relative motion is accumulated. */
static void HIDValueChangedCallback(void* context, IOReturn result, void* sender, IOHIDValueRef valueRef)
{
	auto data = (HIDData*)context;

	IOHIDElementRef elementRef = IOHIDValueGetElement(valueRef);
	auto usage = (u32)IOHIDElementGetUsage(elementRef);
	auto axisValue = (i32)IOHIDValueGetIntegerValue(valueRef);
	switch(usage)
	{
	case kHIDUsage_GD_X:
		data->MouseAxisValues[0] += axisValue;
		break;
	case kHIDUsage_GD_Y:
		data->MouseAxisValues[1] += axisValue;
		break;
	case kHIDUsage_GD_Z:
		data->MouseAxisValues[2] += axisValue;
		break;
	default:
		break;
	}
}

/** Converts a keyboard scan key (as reported by the HID manager) into the engine's ButtonCode. */
static ButtonCode ScanCodeToButtonCode(u32 scanCode)
{
	switch(scanCode)
	{
	case 0x04: return ButtonCode::A;
	case 0x05: return ButtonCode::B;
	case 0x06: return ButtonCode::C;
	case 0x07: return ButtonCode::D;
	case 0x08: return ButtonCode::E;
	case 0x09: return ButtonCode::F;
	case 0x0a: return ButtonCode::G;
	case 0x0b: return ButtonCode::H;
	case 0x0c: return ButtonCode::I;
	case 0x0d: return ButtonCode::J;
	case 0x0e: return ButtonCode::K;
	case 0x0f: return ButtonCode::L;
	case 0x10: return ButtonCode::M;
	case 0x11: return ButtonCode::N;
	case 0x12: return ButtonCode::O;
	case 0x13: return ButtonCode::P;
	case 0x14: return ButtonCode::Q;
	case 0x15: return ButtonCode::R;
	case 0x16: return ButtonCode::S;
	case 0x17: return ButtonCode::T;
	case 0x18: return ButtonCode::U;
	case 0x19: return ButtonCode::V;
	case 0x1a: return ButtonCode::W;
	case 0x1b: return ButtonCode::X;
	case 0x1c: return ButtonCode::Y;
	case 0x1d: return ButtonCode::Z;

	case 0x1e: return ButtonCode::Key1;
	case 0x1f: return ButtonCode::Key2;
	case 0x20: return ButtonCode::Key3;
	case 0x21: return ButtonCode::Key4;
	case 0x22: return ButtonCode::Key5;
	case 0x23: return ButtonCode::Key6;
	case 0x24: return ButtonCode::Key7;
	case 0x25: return ButtonCode::Key8;
	case 0x26: return ButtonCode::Key9;
	case 0x27: return ButtonCode::Key0;

	case 0x28: return ButtonCode::Enter;
	case 0x29: return ButtonCode::Escape;
	case 0x2a: return ButtonCode::Backspace;
	case 0x2b: return ButtonCode::Tab;
	case 0x2c: return ButtonCode::Space;
	case 0x2d: return ButtonCode::Minus;
	case 0x2e: return ButtonCode::Equals;
	case 0x2f: return ButtonCode::LeftBracket;
	case 0x30: return ButtonCode::RightBracket;
	case 0x31: return ButtonCode::Backslash;
	case 0x32: return ButtonCode::Grave;
	case 0x33: return ButtonCode::Semicolon;
	case 0x34: return ButtonCode::Apostrophe;
	case 0x35: return ButtonCode::Grave;
	case 0x36: return ButtonCode::Comma;
	case 0x37: return ButtonCode::Period;
	case 0x38: return ButtonCode::Slash;
	case 0x39: return ButtonCode::CapsLock;

	case 0x3a: return ButtonCode::F1;
	case 0x3b: return ButtonCode::F2;
	case 0x3c: return ButtonCode::F3;
	case 0x3d: return ButtonCode::F4;
	case 0x3e: return ButtonCode::F5;
	case 0x3f: return ButtonCode::F6;
	case 0x40: return ButtonCode::F7;
	case 0x41: return ButtonCode::F8;
	case 0x42: return ButtonCode::F9;
	case 0x43: return ButtonCode::F10;
	case 0x44: return ButtonCode::F11;
	case 0x45: return ButtonCode::F12;

	case 0x46: return ButtonCode::SysRq;
	case 0x47: return ButtonCode::ScrollLock;
	case 0x48: return ButtonCode::Pause;
	case 0x49: return ButtonCode::Insert;
	case 0x4a: return ButtonCode::Home;
	case 0x4b: return ButtonCode::PageUp;
	case 0x4c: return ButtonCode::Delete;
	case 0x4d: return ButtonCode::End;
	case 0x4e: return ButtonCode::PageDown;
	case 0x4f: return ButtonCode::ArrowRight;
	case 0x50: return ButtonCode::ArrowLeft;
	case 0x51: return ButtonCode::ArrowDown;
	case 0x52: return ButtonCode::ArrowUp;

	case 0x53: return ButtonCode::NumLock;
	case 0x54: return ButtonCode::NumpadDivide;
	case 0x55: return ButtonCode::NumpadMultiply;
	case 0x56: return ButtonCode::NumpadMinus;
	case 0x57: return ButtonCode::NumpadPlus;
	case 0x58: return ButtonCode::NumpadEnter;
	case 0x59: return ButtonCode::Numpad1;
	case 0x5a: return ButtonCode::Numpad2;
	case 0x5b: return ButtonCode::Numpad3;
	case 0x5c: return ButtonCode::Numpad4;
	case 0x5d: return ButtonCode::Numpad5;
	case 0x5e: return ButtonCode::Numpad6;
	case 0x5f: return ButtonCode::Numpad7;
	case 0x60: return ButtonCode::Numpad8;
	case 0x61: return ButtonCode::Numpad9;
	case 0x62: return ButtonCode::Numpad0;
	case 0x63: return ButtonCode::NumpadDecimal;

	case 0x64: return ButtonCode::OEM102;
	case 0x66: return ButtonCode::Power;
	case 0x67: return ButtonCode::NumadEquals;

	case 0x68: return ButtonCode::F13;
	case 0x69: return ButtonCode::F14;
	case 0x6a: return ButtonCode::F15;

	case 0x78: return ButtonCode::Stop;
	case 0x7f: return ButtonCode::Mute;
	case 0x80: return ButtonCode::VolumeUp;
	case 0x81: return ButtonCode::VolumeDown;
	case 0x85: return ButtonCode::NumpadComma;
	case 0x86: return ButtonCode::NumadEquals;
	case 0x89: return ButtonCode::Yen;

	case 0xe0: return ButtonCode::LeftControl;
	case 0xe1: return ButtonCode::LeftShift;
	case 0xe2: return ButtonCode::LeftAlt;
	case 0xe3: return ButtonCode::LeftWindows;
	case 0xe4: return ButtonCode::RightControl;
	case 0xe5: return ButtonCode::RightShift;
	case 0xe6: return ButtonCode::RightAlt;
	case 0xe7: return ButtonCode::RightWindows;

	case 0xe8: return ButtonCode::PlayPause;
	case 0xe9: return ButtonCode::MediaStop;
	case 0xea: return ButtonCode::PreviousTrack;
	case 0xeb: return ButtonCode::NextTrack;
	case 0xed: return ButtonCode::VolumeUp;
	case 0xee: return ButtonCode::VolumeDown;
	case 0xef: return ButtonCode::Mute;
	case 0xf0: return ButtonCode::WebSearch;
	case 0xf1: return ButtonCode::WebBack;
	case 0xf2: return ButtonCode::WebForward;
	case 0xf3: return ButtonCode::WebStop;
	case 0xf4: return ButtonCode::WebSearch;
	case 0xf8: return ButtonCode::Sleep;
	case 0xf9: return ButtonCode::Awake;
	case 0xfb: return ButtonCode::Calculator;

	default:
		return ButtonCode::Unassigned;
	}
}

/** Converts a gamepad button usage (as reported by the HID manager) into the engine's ButtonCode. */
static ButtonCode GamepadUsageToButtonCode(u32 usage)
{
	// These are based on the Xbox controller layout, which the ButtonCode names assume
	switch(usage)
	{
	case 1: return ButtonCode::GamepadA;
	case 2: return ButtonCode::GamepadB;
	case 3: return ButtonCode::GamepadX;
	case 4: return ButtonCode::GamepadY;
	case 5: return ButtonCode::GamepadLeftBumper;
	case 6: return ButtonCode::GamepadRightBumper;
	case 7: return ButtonCode::GamepadLeftStick;
	case 8: return ButtonCode::GamepadRightStick;
	case 9: return ButtonCode::GamepadStart;
	case 10: return ButtonCode::GamepadBack;
	case 11: return ButtonCode::GamepadButton1;
	case 12: return ButtonCode::GamepadDPadUp;
	case 13: return ButtonCode::GamepadDPadDown;
	case 14: return ButtonCode::GamepadDPadLeft;
	case 15: return ButtonCode::GamepadDPatRight;
	default:
		{
			if(usage < 16)
				return ButtonCode::Unassigned;

			// Map the remaining buttons to the unnamed generic entries
			const u32 buttonIdx = usage - 16;
			if(buttonIdx < 19)
				return (ButtonCode)((u32)ButtonCode::GamepadButton2 + buttonIdx);

			return ButtonCode::Unassigned;
		}
	}
}

/** Converts an 8-way hat switch direction index (0 = up, continuing clockwise) into the engine's ButtonCode. */
static ButtonCode HatDirectionToButtonCode(i32 direction)
{
	switch(direction)
	{
	case 0: return ButtonCode::GamepadDPadUp;
	case 1: return ButtonCode::GamepadDPadUpRight;
	case 2: return ButtonCode::GamepadDPatRight;
	case 3: return ButtonCode::GamepadDPadDownRight;
	case 4: return ButtonCode::GamepadDPadDown;
	case 5: return ButtonCode::GamepadDPadDownLeft;
	case 6: return ButtonCode::GamepadDPadLeft;
	case 7: return ButtonCode::GamepadDPadUpLeft;
	default:
		return ButtonCode::Unassigned; // Centered
	}
}

HIDManager::HIDManager(HIDType type, Input& input)
{
	mData.Type = type;
	mData.Owner = &input;
	B3DZeroOut(mData.MouseAxisValues);

	mHIDManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDManagerOptionNone);
	if(mHIDManager == nullptr)
		return;

	if(IOHIDManagerOpen(mHIDManager, kIOHIDOptionsTypeNone) != kIOReturnSuccess)
	{
		B3D_LOG(Error, LogPlatform, "Unable to open the IOKit HID manager, no input devices will be reported.");

		CFRelease(mHIDManager);
		mHIDManager = nullptr;
		return;
	}

	u32 numEntries = 0;
	const void* entries[3];

	switch(type)
	{
	case HIDType::Keyboard:
		entries[0] = CreateHIDDeviceMatchDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_Keyboard);
		numEntries = 1;
		break;
	case HIDType::Mouse:
		entries[0] = CreateHIDDeviceMatchDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_Mouse);
		numEntries = 1;
		break;
	case HIDType::Gamepad:
		entries[0] = CreateHIDDeviceMatchDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick);
		entries[1] = CreateHIDDeviceMatchDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad);
		entries[2] = CreateHIDDeviceMatchDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_MultiAxisController);
		numEntries = 3;
		break;
	}

	CFArrayRef entryArray = CFArrayCreate(kCFAllocatorDefault, entries, numEntries, &kCFTypeArrayCallBacks);

	IOHIDManagerSetDeviceMatchingMultiple(mHIDManager, entryArray);
	IOHIDManagerRegisterDeviceMatchingCallback(mHIDManager, HIDDeviceAddedCallback, &mData);
	IOHIDManagerRegisterDeviceRemovalCallback(mHIDManager, HIDDeviceRemovedCallback, &mData);

	// We only care about input callbacks for mice, so we can accumulate all axis movement
	if(type == HIDType::Mouse)
		IOHIDManagerRegisterInputValueCallback(mHIDManager, HIDValueChangedCallback, &mData);

	CFStringRef runLoopMode = GetRunLoopMode(type);
	IOHIDManagerScheduleWithRunLoop(mHIDManager, CFRunLoopGetCurrent(), runLoopMode);

	while(CFRunLoopRunInMode(runLoopMode, 0, TRUE) == kCFRunLoopRunHandledSource)
	{ /* Do nothing */
	}

	for(u32 entryIndex = 0; entryIndex < numEntries; entryIndex++)
	{
		if(entries[entryIndex])
			CFRelease((CFTypeRef)entries[entryIndex]);
	}

	CFRelease(entryArray);
}

HIDManager::~HIDManager()
{
	if(mHIDManager == nullptr)
		return;

	for(auto& device : mData.Devices)
	{
		IOHIDQueueStop(device.QueueRef);
		CFRelease(device.QueueRef);
	}

	CFStringRef runLoopMode = GetRunLoopMode(mData.Type);
	IOHIDManagerUnscheduleFromRunLoop(mHIDManager, CFRunLoopGetCurrent(), runLoopMode);

	IOHIDManagerClose(mHIDManager, kIOHIDOptionsTypeNone);
	CFRelease(mHIDManager);
}

void HIDManager::Capture(IOHIDDeviceRef device, bool ignoreEvents)
{
	if(mHIDManager == nullptr)
		return;

	if(mData.Type == HIDType::Mouse)
		B3DZeroOut(mData.MouseAxisValues);

	// First trigger any callbacks. This is also what pumps the device added/removed callbacks, handling hot-plug.
	CFStringRef runLoopMode = GetRunLoopMode(mData.Type);
	while(CFRunLoopRunInMode(runLoopMode, 0, TRUE) == kCFRunLoopRunHandledSource)
	{ /* Do nothing */
	}

	for(auto& entry : mData.Devices)
	{
		if(device != nullptr && entry.Ref != device)
			continue;

		// Poll non-queued elements. These are the gamepad axes, for which we only care about the latest absolute values.
		if(mData.Type == HIDType::Gamepad && !ignoreEvents)
		{
			struct AxisState
			{
				bool Moved;
				i32 Value;
			};

			AxisState axisState[kHIDGamepadAxisCount];
			B3DZeroOut(axisState);

			for(auto& axis : entry.Axes)
			{
				// Axes past RightTrigger have no matching InputAxis entries, and get reported on the generic slots
				// right after the named ones
				const i32 firstExtraAxis = (i32)InputAxis::RightTrigger + 1;

				i32 axisIndex = -1;
				switch(axis.Usage)
				{
				case kHIDUsage_GD_X:
					axisIndex = (i32)InputAxis::LeftStickX;
					break;
				case kHIDUsage_GD_Y:
					axisIndex = (i32)InputAxis::LeftStickY;
					break;
				case kHIDUsage_GD_Rx:
					axisIndex = (i32)InputAxis::RightStickX;
					break;
				case kHIDUsage_GD_Ry:
					axisIndex = (i32)InputAxis::RightStickY;
					break;
				case kHIDUsage_GD_Z:
					axisIndex = (i32)InputAxis::LeftTrigger;
					break;
				case kHIDUsage_GD_Rz:
					axisIndex = (i32)InputAxis::RightTrigger;
					break;
				case kHIDUsage_GD_Slider:
					axisIndex = firstExtraAxis + 1;
					break;
				case kHIDUsage_GD_Dial:
					axisIndex = firstExtraAxis + 2;
					break;
				case kHIDUsage_GD_Wheel:
					axisIndex = firstExtraAxis + 3;
					break;
				case kHIDUsage_Sim_Rudder:
					axisIndex = firstExtraAxis + 4;
					break;
				case kHIDUsage_Sim_Throttle:
					axisIndex = firstExtraAxis + 5;
					break;
				case kHIDUsage_Sim_Accelerator:
					axisIndex = firstExtraAxis + 6;
					break;
				case kHIDUsage_Sim_Brake:
					axisIndex = firstExtraAxis + 7;
					break;
				default:
					break;
				}

				if(axisIndex < 0 || axisIndex >= (i32)kHIDGamepadAxisCount)
					continue;

				IOHIDValueRef valueRef;
				if(IOHIDDeviceGetValue(entry.Ref, axis.Ref, &valueRef) != kIOReturnSuccess)
					continue;

				// Ignore if the axis value didn't change since the last query
				const u64 timestamp = IOHIDValueGetTimeStamp(valueRef);
				if(timestamp == entry.GamepadAxisTimestamps[axisIndex])
					continue;

				entry.GamepadAxisTimestamps[axisIndex] = timestamp;

				// Trigger axes are mapped to [0, kMaxAxis] so an idle trigger reports zero, matching the other
				// platform backends. All other axes get the full engine range.
				const bool isTrigger = axisIndex == (i32)InputAxis::LeftTrigger ||
					axisIndex == (i32)InputAxis::RightTrigger;

				const i32 rawValue = (i32)IOHIDValueGetIntegerValue(valueRef);
				axisState[axisIndex].Moved = true;
				axisState[axisIndex].Value = ScaleHIDAxisValue(axis, rawValue,
					isTrigger ? 0 : IInputBackend::kMinAxis);
			}

			for(u32 axisIndex = 0; axisIndex < kHIDGamepadAxisCount; axisIndex++)
			{
				if(axisState[axisIndex].Moved)
					mData.Owner->NotifyAxisMoved(entry.Id, axisIndex, axisState[axisIndex].Value);
			}
		}

		// Read queued elements (buttons and hats)
		while(true)
		{
			IOHIDValueRef valueRef = IOHIDQueueCopyNextValueWithTimeout(entry.QueueRef, 0);
			if(!valueRef)
				break;

			if(ignoreEvents)
			{
				CFRelease(valueRef);
				continue;
			}

			IOHIDElementRef elemRef = IOHIDValueGetElement(valueRef);
			const auto value = (i32)IOHIDValueGetIntegerValue(valueRef); // For buttons 1 when pressed, 0 when released
			const u64 timestamp = MachTimeToMs(IOHIDValueGetTimeStamp(valueRef));

			const u32 usage = IOHIDElementGetUsage(elemRef);
			const u32 usagePage = IOHIDElementGetUsagePage(elemRef);

			if(usagePage == kHIDPage_GenericDesktop && usage == kHIDUsage_GD_Hatswitch)
			{
				// The hat reports an absolute direction, converted here to press/release events on 8-way POV buttons
				const i32 direction = value - (i32)IOHIDElementGetLogicalMin(elemRef);
				const ButtonCode povButton = HatDirectionToButtonCode(direction);

				if(povButton != entry.PovState)
				{
					if(entry.PovState != ButtonCode::Unassigned)
						mData.Owner->NotifyButtonReleased(entry.Id, entry.PovState, timestamp);

					if(povButton != ButtonCode::Unassigned)
						mData.Owner->NotifyButtonPressed(entry.Id, povButton, timestamp);

					entry.PovState = povButton;
				}

				CFRelease(valueRef);
				continue;
			}

			ButtonCode button = ButtonCode::Unassigned;
			if(usagePage == kHIDPage_Button)
			{
				if(mData.Type == HIDType::Mouse)
				{
					if(usage > 0 && usage <= (u32)ButtonCode::MouseKeyCount)
						button = (ButtonCode)((u32)ButtonCode::MouseLeft + usage - 1);
				}
				else if(mData.Type == HIDType::Gamepad)
					button = GamepadUsageToButtonCode(usage);
			}
			else if(usagePage == kHIDPage_KeyboardOrKeypad)
			{
				// Usage -1 and 1 are special signals that happen along with every button press/release and should be
				// ignored
				if(usage != (u32)-1 && usage != 1)
					button = ScanCodeToButtonCode(usage);
			}

			if(button != ButtonCode::Unassigned)
			{
				if(value != 0)
					mData.Owner->NotifyButtonPressed(entry.Id, button, timestamp);
				else
					mData.Owner->NotifyButtonReleased(entry.Id, button, timestamp);
			}

			CFRelease(valueRef);
		}
	}

	// Report accumulated mouse movement
	if(mData.Type == HIDType::Mouse && !ignoreEvents)
	{
		if(mData.MouseAxisValues[0] != 0 || mData.MouseAxisValues[1] != 0 || mData.MouseAxisValues[2] != 0)
			mData.Owner->NotifyMouseMoved(mData.MouseAxisValues[0], mData.MouseAxisValues[1], mData.MouseAxisValues[2]);
	}
}

String HIDManager::GetDeviceName(u32 deviceId) const
{
	for(auto& entry : mData.Devices)
	{
		if(entry.Id == deviceId)
			return entry.Name;
	}

	return StringUtility::kBlank;
}

MacOSInputBackend::MacOSInputBackend(Input& owner)
{
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

	const bool isHeadless = gpuDevice == nullptr || gpuDevice->GetCapabilities().DeviceName == "Null" ||
		owner.GetWindowHandle() == 0;
	if(isHeadless)
		return;

	mKeyboard = B3DNew<HIDManager>(HIDType::Keyboard, owner);
	mMouse = B3DNew<HIDManager>(HIDType::Mouse, owner);
	mGamepad = B3DNew<HIDManager>(HIDType::Gamepad, owner);
}

MacOSInputBackend::~MacOSInputBackend()
{
	if(mMouse != nullptr)
		B3DDelete(mMouse);

	if(mKeyboard != nullptr)
		B3DDelete(mKeyboard);

	if(mGamepad != nullptr)
		B3DDelete(mGamepad);
}

void MacOSInputBackend::Update()
{
	// Note: Events are captured (or drained, when the window doesn't have focus) even with no focus, so stale input
	// doesn't get reported when focus returns
	if(mMouse != nullptr)
		mMouse->Capture(nullptr, !mHasInputFocus);

	if(mKeyboard != nullptr)
		mKeyboard->Capture(nullptr, !mHasInputFocus);

	if(mGamepad != nullptr)
		mGamepad->Capture(nullptr, !mHasInputFocus);
}

u32 MacOSInputBackend::GetDeviceCount(InputDevice device) const
{
	switch(device)
	{
	// Note: All keyboard/mouse HID devices get coalesced into a single logical device
	case InputDevice::Keyboard: return mKeyboard != nullptr ? 1 : 0;
	case InputDevice::Mouse: return mMouse != nullptr ? 1 : 0;
	case InputDevice::Gamepad: return mGamepad != nullptr ? mGamepad->GetDeviceCount() : 0;
	default:
	case InputDevice::Count: return 0;
	}
}

String MacOSInputBackend::GetDeviceName(InputDevice type, u32 deviceIndex) const
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
		if(mGamepad != nullptr)
			return mGamepad->GetDeviceName(deviceIndex);

		return StringUtility::kBlank;
	default:
		return StringUtility::kBlank;
	}
}

void MacOSInputBackend::ChangeCaptureContext(u64 windowHandle)
{
	// The HID manager reports input system-wide rather than per-window, so all that matters is whether any of the
	// application's windows have focus
	mHasInputFocus = windowHandle != 0;
}

namespace b3d
{
IInputBackend* CreateInputBackend(Input& owner)
{
	return B3DNew<MacOSInputBackend>(owner);
}
} // namespace b3d
