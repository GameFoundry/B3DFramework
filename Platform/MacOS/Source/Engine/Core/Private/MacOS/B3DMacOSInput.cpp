//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DInput.h"
#include "Private/MacOS/B3DMacOSInput.h"
#include "Private/MacOS/B3DMacOSPlatform.h"

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

	// Create a queue. IOKit returns null when the device cannot be opened, so the device is still
	// registered (its axes are polled directly), just without queued button/hat events.
	newDevice.QueueRef = IOHIDQueueCreate(kCFAllocatorDefault, device, 128, kIOHIDOptionsTypeNone);
	if(newDevice.QueueRef != nullptr)
	{
		for(auto& button : newDevice.Buttons)
			IOHIDQueueAddElement(newDevice.QueueRef, button.Ref);

		for(auto& hat : newDevice.Hats)
			IOHIDQueueAddElement(newDevice.QueueRef, hat.Ref);

		IOHIDQueueStart(newDevice.QueueRef);
	}

	// Assign a device ID
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

	data->Devices.push_back(newDevice);

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
		if(iterFind->QueueRef != nullptr)
		{
			IOHIDQueueStop(iterFind->QueueRef);
			CFRelease(iterFind->QueueRef);
		}

		// Release any input the device was holding at the moment of removal
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

HIDGamepadManager::HIDGamepadManager(Input& input)
{
	mData.Owner = &input;
	mHIDManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDManagerOptionNone);
	if(mHIDManager == nullptr)
		return;

	const void* entries[] = {
		CreateHIDDeviceMatchDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick),
		CreateHIDDeviceMatchDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad),
		CreateHIDDeviceMatchDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_MultiAxisController)
	};

	CFArrayRef entryArray = CFArrayCreate(kCFAllocatorDefault, entries, 3, &kCFTypeArrayCallBacks);
	IOHIDManagerSetDeviceMatchingMultiple(mHIDManager, entryArray);
	CFRelease(entryArray);

	for(const void* entry : entries)
		CFRelease((CFTypeRef)entry);

	// Restrict the device set before opening it so keyboards and mice are never opened.
	if(IOHIDManagerOpen(mHIDManager, kIOHIDOptionsTypeNone) != kIOReturnSuccess)
	{
		B3D_LOG(Warning, LogPlatform, "Unable to open the IOKit game controller manager.");
		CFRelease(mHIDManager);
		mHIDManager = nullptr;
		return;
	}

	IOHIDManagerRegisterDeviceMatchingCallback(mHIDManager, HIDDeviceAddedCallback, &mData);
	IOHIDManagerRegisterDeviceRemovalCallback(mHIDManager, HIDDeviceRemovedCallback, &mData);
	IOHIDManagerScheduleWithRunLoop(mHIDManager, CFRunLoopGetCurrent(), CFSTR("B3DGamepad"));

	while(CFRunLoopRunInMode(CFSTR("B3DGamepad"), 0, TRUE) == kCFRunLoopRunHandledSource)
	{ /* Do nothing */
	}
}

HIDGamepadManager::~HIDGamepadManager()
{
	if(mHIDManager == nullptr)
		return;

	for(auto& device : mData.Devices)
	{
		if(device.QueueRef == nullptr)
			continue;

		IOHIDQueueStop(device.QueueRef);
		CFRelease(device.QueueRef);
	}

	CFStringRef runLoopMode = CFSTR("B3DGamepad");
	IOHIDManagerUnscheduleFromRunLoop(mHIDManager, CFRunLoopGetCurrent(), runLoopMode);

	IOHIDManagerClose(mHIDManager, kIOHIDOptionsTypeNone);
	CFRelease(mHIDManager);
}

void HIDGamepadManager::Capture(IOHIDDeviceRef device, bool ignoreEvents)
{
	if(mHIDManager == nullptr)
		return;

	// First trigger any callbacks. This is also what pumps the device added/removed callbacks, handling hot-plug.
	CFStringRef runLoopMode = CFSTR("B3DGamepad");
	while(CFRunLoopRunInMode(runLoopMode, 0, TRUE) == kCFRunLoopRunHandledSource)
	{ /* Do nothing */
	}

	for(auto& entry : mData.Devices)
	{
		if(device != nullptr && entry.Ref != device)
			continue;

		// Poll non-queued elements. These are the gamepad axes, for which we only care about the latest absolute values.
		if(!ignoreEvents)
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

		// Read queued elements (buttons and hats). Devices whose queue could not be created report no
		// button or hat events; see HIDDeviceAddedCallback.
		while(entry.QueueRef != nullptr)
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
				button = GamepadUsageToButtonCode(usage);

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

}

String HIDGamepadManager::GetDeviceName(u32 deviceId) const
{
	for(auto& entry : mData.Devices)
	{
		if(entry.Id == deviceId)
			return entry.Name;
	}

	return StringUtility::kBlank;
}

MacOSInputBackend::MacOSInputBackend(Input& owner)
	: mOwner(owner)
{
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	mHasDesktopInput = gpuDevice != nullptr && gpuDevice->GetCapabilities().DeviceName != "Null" && owner.GetWindowHandle() != 0;
	if(!mHasDesktopInput)
		return;

	mButtonChangedConnection = MacOSPlatform::OnButtonChanged.Connect([this](ButtonCode button, bool pressed, u64 timestamp)
	{
		Lock lock(mMutex);
		const u32 index = (u32)button & 0xFFFF;
		if(!mHasInputFocus || button == ButtonCode::Unassigned || index >= std::size(mPressedButtons) || mPressedButtons[index] == pressed)
			return;

		mPressedButtons[index] = pressed;
		if(pressed)
			mOwner.NotifyButtonPressed(0, button, timestamp);
		else
			mOwner.NotifyButtonReleased(0, button, timestamp);
	});

	mMouseMovedConnection = MacOSPlatform::OnMouseMoved.Connect([this](float x, float y, float z)
	{
		Lock lock(mMutex);
		if(!mHasInputFocus)
			return;

		mMouseDelta[0] += x;
		mMouseDelta[1] += y;
		mMouseDelta[2] += z;
	});

	mGamepad = B3DNew<HIDGamepadManager>(owner);
}

MacOSInputBackend::~MacOSInputBackend()
{
	mButtonChangedConnection.Disconnect();
	mMouseMovedConnection.Disconnect();

	if(mGamepad != nullptr)
		B3DDelete(mGamepad);
}

void MacOSInputBackend::Update()
{
	bool hasInputFocus;
	{
		Lock lock(mMutex);
		hasInputFocus = mHasInputFocus;
		i32 motion[3];
		for(u32 axis = 0; axis < 3; axis++)
		{
			motion[axis] = (i32)mMouseDelta[axis];
			mMouseDelta[axis] -= (float)motion[axis];
		}

		if(motion[0] != 0 || motion[1] != 0 || motion[2] != 0)
			mOwner.NotifyMouseMoved(motion[0], motion[1], motion[2]);
	}

	// Drain controller events while unfocused so stale input is not delivered on reactivation.
	if(mGamepad != nullptr)
		mGamepad->Capture(nullptr, !hasInputFocus);
}

u32 MacOSInputBackend::GetDeviceCount(InputDevice device) const
{
	switch(device)
	{
	case InputDevice::Keyboard:
	case InputDevice::Mouse: return mHasDesktopInput ? 1 : 0;
	case InputDevice::Gamepad: return mGamepad != nullptr ? mGamepad->GetDeviceCount() : 0;
	default: return 0;
	}
}

String MacOSInputBackend::GetDeviceName(InputDevice type, u32 deviceIndex) const
{
	switch(type)
	{
	case InputDevice::Keyboard: return mHasDesktopInput && deviceIndex == 0 ? "Keyboard" : StringUtility::kBlank;
	case InputDevice::Mouse: return mHasDesktopInput && deviceIndex == 0 ? "Mouse" : StringUtility::kBlank;
	case InputDevice::Gamepad: return mGamepad != nullptr ? mGamepad->GetDeviceName(deviceIndex) : StringUtility::kBlank;
	default: return StringUtility::kBlank;
	}
}

void MacOSInputBackend::ChangeCaptureContext(u64 windowHandle)
{
	Lock lock(mMutex);
	mHasInputFocus = windowHandle != 0;
	if(mHasInputFocus)
		return;

	const u64 timestamp = MachTimeToMs(mach_absolute_time());
	for(u32 index = 0; index < std::size(mPressedButtons); index++)
	{
		if(!mPressedButtons[index])
			continue;

		mPressedButtons[index] = false;
		const u32 flags = index >= (u32)ButtonCode::KeyboardKeyCount ? 0x80000000 : 0;
		mOwner.NotifyButtonReleased(0, (ButtonCode)(flags | index), timestamp);
	}

	B3DZeroOut(mMouseDelta);
}

namespace b3d
{
IInputBackend* CreateInputBackend(Input& owner)
{
	return B3DNew<MacOSInputBackend>(owner);
}
} // namespace b3d
