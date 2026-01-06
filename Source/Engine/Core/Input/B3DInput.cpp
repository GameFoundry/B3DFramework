//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DInput.h"

#include "B3DApplication.h"
#include "Input/B3DMouse.h"
#include "Input/B3DKeyboard.h"
#include "Input/B3DGamepad.h"
#include "Utility/B3DTime.h"
#include "Math/B3DMath.h"
#include "Managers/B3DRenderWindowManager.h"

using namespace b3d;

// Note: Input polling methods for button/axis could be re-written so their query immediate state
// instead of returning cached state from event callbacks. This /might/ result in even less input lag?

const int Input::kHistoryBufferSize = 10; // Size of buffer used for input smoothing
const float Input::kWeightModifier = 0.5f;

Input::DeviceData::DeviceData()
{
	for(u32 keyIndex = 0; keyIndex < static_cast<unsigned>(ButtonCode::TotalKeyCount); keyIndex++)
		KeyStates[keyIndex] = ButtonState::Off;
}

Input::Input()
{
	SPtr<RenderWindow> primaryWindow = GetApplication().GetPrimaryWindow();
	mWindowHandle = primaryWindow->GetPlatformWindowHandle();

	// Subscribe to events
	mCharInputConn = Platform::onCharInput.Connect([this](u32 character) { CharInput(character); });
	mCursorMovedConn = Platform::onCursorMoved.Connect([this](const Vector2I& cursorPosition, const OSPointerButtonStates& buttonStates) { CursorMoved(cursorPosition, buttonStates); });
	mCursorPressedConn = Platform::onCursorButtonPressed.Connect([this](const Vector2I& cursorPosition, OSMouseButton button, const OSPointerButtonStates& buttonStates) { CursorPressed(cursorPosition, button, buttonStates); });
	mCursorReleasedConn = Platform::onCursorButtonReleased.Connect([this](const Vector2I& cursorPosition, OSMouseButton button, const OSPointerButtonStates& buttonStates) { CursorReleased(cursorPosition, button, buttonStates); });
	mCursorDoubleClickConn = Platform::onCursorDoubleClick.Connect([this](const Vector2I& cursorPosition, const OSPointerButtonStates& buttonStates) { CursorDoubleClick(cursorPosition, buttonStates); });
	mInputCommandConn = Platform::onInputCommand.Connect([this](InputCommandType commandType) { InputCommandEntered(commandType); });
	mMouseWheelScrolledConn = Platform::onMouseWheelScrolled.Connect([this](float scrollPosition) { MouseWheelScrolled(scrollPosition); });

	RenderWindowManager::Instance().OnFocusGained.Connect([this](RenderWindow& window) { InputWindowChanged(window); });
	RenderWindowManager::Instance().OnFocusLost.Connect([this](RenderWindow&) { InputFocusLost(); });

	for(int buttonIndex = 0; buttonIndex < 3; buttonIndex++)
		mPointerButtonStates[buttonIndex] = ButtonState::Off;

	// Mouse smoothing
	mMouseSampleAccumulator[0] = 0;
	mMouseSampleAccumulator[1] = 0;
	mTotalMouseSamplingTime[0] = 1.0f / 125.0f; // Use 125Hz as initial pooling rate for mice
	mTotalMouseSamplingTime[1] = 1.0f / 125.0f;
	mTotalMouseSampleCount[0] = 1;
	mTotalMouseSampleCount[1] = 1;
	mMouseSmoothedAxis[0] = 0.0f;
	mMouseSmoothedAxis[1] = 0.0f;
	mMouseZeroTime[0] = 0.0f;
	mMouseZeroTime[1] = 0.0f;

	// Raw input
	InitRawInput();
}

Input::~Input()
{
	CleanUpRawInput();

	mCharInputConn.Disconnect();
	mCursorMovedConn.Disconnect();
	mCursorPressedConn.Disconnect();
	mCursorReleasedConn.Disconnect();
	mCursorDoubleClickConn.Disconnect();
	mInputCommandConn.Disconnect();
	mMouseWheelScrolledConn.Disconnect();
}

void Input::UpdateInternal()
{
	// Toggle states only remain active for a single frame before they are transitioned
	// into permanent state

	for(auto& deviceData : mDevices)
	{
		for(u32 keyIndex = 0; keyIndex < static_cast<unsigned>(ButtonCode::TotalKeyCount); keyIndex++)
		{
			if(deviceData.KeyStates[keyIndex] == ButtonState::ToggledOff || deviceData.KeyStates[keyIndex] == ButtonState::ToggledOnOff)
				deviceData.KeyStates[keyIndex] = ButtonState::Off;
			else if(deviceData.KeyStates[keyIndex] == ButtonState::ToggledOn)
				deviceData.KeyStates[keyIndex] = ButtonState::On;
		}

		u32 axisCount = (u32)deviceData.Axes.size();
		for(u32 axisIndex = 0; axisIndex < axisCount; axisIndex++)
			deviceData.Axes[axisIndex] = 0.0f;
	}

	for(u32 buttonIndex = 0; buttonIndex < 3; buttonIndex++)
	{
		if(mPointerButtonStates[buttonIndex] == ButtonState::ToggledOff || mPointerButtonStates[buttonIndex] == ButtonState::ToggledOnOff)
			mPointerButtonStates[buttonIndex] = ButtonState::Off;
		else if(mPointerButtonStates[buttonIndex] == ButtonState::ToggledOn)
			mPointerButtonStates[buttonIndex] = ButtonState::On;
	}

	mPointerDelta = Vector2I::kZero; // Reset delta in case we don't receive any mouse input this frame
	mPointerDoubleClicked = false;

	// Capture raw input
	if(mMouse != nullptr)
		mMouse->Capture();

	if(mKeyboard != nullptr)
		mKeyboard->Capture();

	for(auto& gamepad : mGamepads)
		gamepad->Capture();

	float rawXValue = 0.0f;
	float rawYValue = 0.0f;

	// Smooth mouse axes if needed
	if(mMouseSmoothingEnabled)
	{
		rawXValue = SmoothMouse((float)mMouseSampleAccumulator[0], 0);
		rawYValue = SmoothMouse((float)mMouseSampleAccumulator[1], 1);
	}
	else
	{
		rawXValue = (float)mMouseSampleAccumulator[0];
		rawYValue = (float)mMouseSampleAccumulator[1];
	}

	rawXValue *= 0.1f;
	rawYValue *= 0.1f;

	mMouseSampleAccumulator[0] = 0;
	mMouseSampleAccumulator[1] = 0;

	AxisMoved(0, -rawXValue, (u32)InputAxis::MouseX);
	AxisMoved(0, -rawYValue, (u32)InputAxis::MouseY);
}

void Input::TriggerCallbacksInternal()
{
	Vector2I pointerPosition;
	float mouseScroll;
	OSPointerButtonStates pointerState;

	{
		Lock lock(mMutex);

		std::swap(mQueuedEvents[0], mQueuedEvents[1]);

		std::swap(mButtonDownEvents[0], mButtonDownEvents[1]);
		std::swap(mButtonUpEvents[0], mButtonUpEvents[1]);

		std::swap(mPointerPressedEvents[0], mPointerPressedEvents[1]);
		std::swap(mPointerReleasedEvents[0], mPointerReleasedEvents[1]);
		std::swap(mPointerDoubleClickEvents[0], mPointerDoubleClickEvents[1]);

		std::swap(mTextInputEvents[0], mTextInputEvents[1]);
		std::swap(mCommandEvents[0], mCommandEvents[1]);

		pointerPosition = mPointerPosition;
		mouseScroll = mMouseScroll;
		pointerState = mPointerState;

		mMouseScroll = 0.0f;
	}

	if(pointerPosition != mLastPointerPosition || mouseScroll != 0.0f)
	{
		PointerEvent event;
		event.Alt = false;
		event.Shift = pointerState.Shift;
		event.Control = pointerState.Ctrl;
		event.ButtonStates[0] = pointerState.MouseButtons[0];
		event.ButtonStates[1] = pointerState.MouseButtons[1];
		event.ButtonStates[2] = pointerState.MouseButtons[2];
		event.MouseWheelScrollAmount = mouseScroll;

		event.Type = PointerEventType::CursorMoved;
		event.ScreenPos = pointerPosition;

		if(mLastPositionSet)
			mPointerDelta = event.ScreenPos - mLastPointerPosition;

		event.Delta = mPointerDelta;

		OnPointerMoved(event);

		mLastPointerPosition = event.ScreenPos;
		mLastPositionSet = true;
	}

	for(auto& event : mQueuedEvents[1])
	{
		switch(event.Type)
		{
		case EventType::ButtonDown:
			{
				const ButtonEvent& eventData = mButtonDownEvents[1][event.Index];

				mDevices[eventData.DeviceIndex].KeyStates[(u32)eventData.ButtonCode & 0x0000FFFF] = ButtonState::ToggledOn;
				OnButtonDown(mButtonDownEvents[1][event.Index]);
			}
			break;
		case EventType::ButtonUp:
			{
				const ButtonEvent& eventData = mButtonUpEvents[1][event.Index];

				while(eventData.DeviceIndex >= (u32)mDevices.size())
					mDevices.push_back(DeviceData());

				if(mDevices[eventData.DeviceIndex].KeyStates[(u32)eventData.ButtonCode & 0x0000FFFF] == ButtonState::ToggledOn)
					mDevices[eventData.DeviceIndex].KeyStates[(u32)eventData.ButtonCode & 0x0000FFFF] = ButtonState::ToggledOnOff;
				else
					mDevices[eventData.DeviceIndex].KeyStates[(u32)eventData.ButtonCode & 0x0000FFFF] = ButtonState::ToggledOff;

				OnButtonUp(mButtonUpEvents[1][event.Index]);
			}
			break;
		case EventType::PointerDown:
			{
				const PointerEvent& eventData = mPointerPressedEvents[1][event.Index];
				mPointerButtonStates[(u32)eventData.Button] = ButtonState::ToggledOn;

				OnPointerPressed(eventData);
			}
			break;
		case EventType::PointerUp:
			{
				const PointerEvent& eventData = mPointerReleasedEvents[1][event.Index];

				if(mPointerButtonStates[(u32)eventData.Button] == ButtonState::ToggledOn)
					mPointerButtonStates[(u32)eventData.Button] = ButtonState::ToggledOnOff;
				else
					mPointerButtonStates[(u32)eventData.Button] = ButtonState::ToggledOff;

				OnPointerReleased(eventData);
			}
			break;
		case EventType::PointerDoubleClick:
			mPointerDoubleClicked = true;
			OnPointerDoubleClick(mPointerDoubleClickEvents[1][event.Index]);
			break;
		case EventType::TextInput:
			OnCharInput(mTextInputEvents[1][event.Index]);
			break;
		case EventType::Command:
			OnInputCommand(mCommandEvents[1][event.Index]);
			break;
		default:
			break;
		}
	}

	mQueuedEvents[1].clear();
	mButtonDownEvents[1].clear();
	mButtonUpEvents[1].clear();
	mPointerPressedEvents[1].clear();
	mPointerReleasedEvents[1].clear();
	mPointerDoubleClickEvents[1].clear();
	mTextInputEvents[1].clear();
	mCommandEvents[1].clear();
}

void Input::InputWindowChanged(RenderWindow& win)
{
	const u64 windowHandle = win.GetPlatformWindowHandle();

	if(mKeyboard != nullptr)
		mKeyboard->ChangeCaptureContext(windowHandle);

	if(mMouse != nullptr)
		mMouse->ChangeCaptureContext(windowHandle);

	for(auto& gamepad : mGamepads)
		gamepad->ChangeCaptureContext(windowHandle);
}

void Input::InputFocusLost()
{
	if(mKeyboard != nullptr)
		mKeyboard->ChangeCaptureContext(0);

	if(mMouse != nullptr)
		mMouse->ChangeCaptureContext(0);

	for(auto& gamepad : mGamepads)
		gamepad->ChangeCaptureContext(0);
}

void Input::NotifyMouseMovedInternal(i32 relativeX, i32 relativeY, i32 relativeZ)
{
	mMouseSampleAccumulator[0] += relativeX;
	mMouseSampleAccumulator[1] += relativeY;

	mTotalMouseSampleCount[0] += Math::RoundToI32(Math::Abs((float)relativeX));
	mTotalMouseSampleCount[1] += Math::RoundToI32(Math::Abs((float)relativeY));

	// Update sample times used for determining sampling rate. But only if something was
	// actually sampled, and only if this isn't the first non-zero sample.
	if(mLastMouseUpdateFrame != GetTime().GetCurrentFrameIndex())
	{
		if(relativeX != 0 && !Math::ApproxEquals(mMouseSmoothedAxis[0], 0.0f))
			mTotalMouseSamplingTime[0] += GetTime().GetFrameDelta();

		if(relativeY != 0 && !Math::ApproxEquals(mMouseSmoothedAxis[1], 0.0f))
			mTotalMouseSamplingTime[1] += GetTime().GetFrameDelta();

		mLastMouseUpdateFrame = GetTime().GetCurrentFrameIndex();
	}

	AxisMoved(0, (float)relativeZ, (u32)InputAxis::MouseZ);
}

void Input::NotifyAxisMovedInternal(u32 gamepadIndex, u32 axisIndex, i32 value)
{
	// Move axis values into [-1.0f, 1.0f] range
	float axisRange = Math::Abs((float)Gamepad::kMaxAxis) + Math::Abs((float)Gamepad::kMinAxis);

	float axisValue = ((value + Math::Abs((float)Gamepad::kMinAxis)) / axisRange) * 2.0f - 1.0f;
	AxisMoved(gamepadIndex, axisValue, axisIndex);
}

void Input::NotifyButtonPressedInternal(u32 deviceIndex, ButtonCode code, u64 timestamp)
{
	ButtonDown(deviceIndex, code, timestamp - mTimestampClockOffset);
}

void Input::NotifyButtonReleasedInternal(u32 deviceIndex, ButtonCode code, u64 timestamp)
{
	ButtonUp(deviceIndex, code, timestamp - mTimestampClockOffset);
}

void Input::ButtonDown(u32 deviceIndex, ButtonCode code, u64 timestamp)
{
	Lock lock(mMutex);

	while(deviceIndex >= (u32)mDevices.size())
		mDevices.push_back(DeviceData());

	ButtonEvent buttonEvent;
	buttonEvent.ButtonCode = code;
	buttonEvent.Timestamp = timestamp;
	buttonEvent.DeviceIndex = deviceIndex;

	mQueuedEvents[0].push_back(QueuedEvent(EventType::ButtonDown, (u32)mButtonDownEvents[0].size()));
	mButtonDownEvents[0].push_back(buttonEvent);
}

void Input::ButtonUp(u32 deviceIndex, ButtonCode code, u64 timestamp)
{
	Lock lock(mMutex);

	ButtonEvent buttonEvent;
	buttonEvent.ButtonCode = code;
	buttonEvent.Timestamp = timestamp;
	buttonEvent.DeviceIndex = deviceIndex;

	mQueuedEvents[0].push_back(QueuedEvent(EventType::ButtonUp, (u32)mButtonUpEvents[0].size()));
	mButtonUpEvents[0].push_back(buttonEvent);
}

void Input::AxisMoved(u32 deviceIndex, float value, u32 axis)
{
	// Note: This method must only ever be called from the main thread, as we don't lock access to axis data
	while(deviceIndex >= (u32)mDevices.size())
		mDevices.push_back(DeviceData());

	Vector<float>& axes = mDevices[deviceIndex].Axes;
	while(axis >= (u32)axes.size())
		axes.push_back(0.0f);

	mDevices[deviceIndex].Axes[axis] = value;
}

void Input::CursorMoved(const Vector2I& cursorPosition, const OSPointerButtonStates& buttonStates)
{
	Lock lock(mMutex);

	mPointerPosition = cursorPosition;
	mPointerState = buttonStates;
}

void Input::CursorPressed(const Vector2I& cursorPosition, OSMouseButton button, const OSPointerButtonStates& buttonStates)
{
	Lock lock(mMutex);

	PointerEvent event;
	event.Alt = false;
	event.Shift = buttonStates.Shift;
	event.Control = buttonStates.Ctrl;
	event.ButtonStates[0] = buttonStates.MouseButtons[0];
	event.ButtonStates[1] = buttonStates.MouseButtons[1];
	event.ButtonStates[2] = buttonStates.MouseButtons[2];

	switch(button)
	{
	case OSMouseButton::Left:
		event.Button = PointerEventButton::Left;
		break;
	case OSMouseButton::Middle:
		event.Button = PointerEventButton::Middle;
		break;
	case OSMouseButton::Right:
		event.Button = PointerEventButton::Right;
		break;
	default:
		break;
	}

	event.ScreenPos = cursorPosition;
	event.Type = PointerEventType::ButtonPressed;

	mQueuedEvents[0].push_back(QueuedEvent(EventType::PointerDown, (u32)mPointerPressedEvents[0].size()));
	mPointerPressedEvents[0].push_back(event);
}

void Input::CursorReleased(const Vector2I& cursorPosition, OSMouseButton button, const OSPointerButtonStates& buttonStates)
{
	Lock lock(mMutex);

	PointerEvent event;
	event.Alt = false;
	event.Shift = buttonStates.Shift;
	event.Control = buttonStates.Ctrl;
	event.ButtonStates[0] = buttonStates.MouseButtons[0];
	event.ButtonStates[1] = buttonStates.MouseButtons[1];
	event.ButtonStates[2] = buttonStates.MouseButtons[2];

	switch(button)
	{
	case OSMouseButton::Left:
		event.Button = PointerEventButton::Left;
		break;
	case OSMouseButton::Middle:
		event.Button = PointerEventButton::Middle;
		break;
	case OSMouseButton::Right:
		event.Button = PointerEventButton::Right;
		break;
	default:
		break;
	}

	event.ScreenPos = cursorPosition;
	event.Type = PointerEventType::ButtonReleased;

	mQueuedEvents[0].push_back(QueuedEvent(EventType::PointerUp, (u32)mPointerReleasedEvents[0].size()));
	mPointerReleasedEvents[0].push_back(event);
}

void Input::CursorDoubleClick(const Vector2I& cursorPosition, const OSPointerButtonStates& buttonStates)
{
	Lock lock(mMutex);

	PointerEvent event;
	event.Alt = false;
	event.Shift = buttonStates.Shift;
	event.Control = buttonStates.Ctrl;
	event.ButtonStates[0] = buttonStates.MouseButtons[0];
	event.ButtonStates[1] = buttonStates.MouseButtons[1];
	event.ButtonStates[2] = buttonStates.MouseButtons[2];
	event.Button = PointerEventButton::Left;
	event.ScreenPos = cursorPosition;
	event.Type = PointerEventType::DoubleClick;

	mQueuedEvents[0].push_back(QueuedEvent(EventType::PointerDoubleClick, (u32)mPointerDoubleClickEvents[0].size()));
	mPointerDoubleClickEvents[0].push_back(event);
}

void Input::InputCommandEntered(InputCommandType commandType)
{
	Lock lock(mMutex);

	mQueuedEvents[0].push_back(QueuedEvent(EventType::Command, (u32)mCommandEvents[0].size()));
	mCommandEvents[0].push_back(commandType);
}

void Input::MouseWheelScrolled(float scrollPosition)
{
	Lock lock(mMutex);

	mMouseScroll = scrollPosition;
}

void Input::CharInput(u32 character)
{
	Lock lock(mMutex);

	TextInputEvent textInputEvent;
	textInputEvent.TextChar = character;

	mQueuedEvents[0].push_back(QueuedEvent(EventType::TextInput, (u32)mTextInputEvents[0].size()));
	mTextInputEvents[0].push_back(textInputEvent);
}

float Input::GetAxisValue(u32 type, u32 deviceIndex) const
{
	if(deviceIndex >= (u32)mDevices.size())
		return 0.0f;

	const Vector<float>& axes = mDevices[deviceIndex].Axes;
	if(type >= (u32)axes.size())
		return 0.0f;

	return axes[type];
}

bool Input::IsButtonHeld(ButtonCode button, u32 deviceIndex) const
{
	if(deviceIndex >= (u32)mDevices.size())
		return false;

	return mDevices[deviceIndex].KeyStates[(u32)button & 0x0000FFFF] == ButtonState::On ||
		mDevices[deviceIndex].KeyStates[(u32)button & 0x0000FFFF] == ButtonState::ToggledOn ||
		mDevices[deviceIndex].KeyStates[(u32)button & 0x0000FFFF] == ButtonState::ToggledOnOff;
}

bool Input::IsButtonUp(ButtonCode button, u32 deviceIndex) const
{
	if(deviceIndex >= (u32)mDevices.size())
		return false;

	return mDevices[deviceIndex].KeyStates[(u32)button & 0x0000FFFF] == ButtonState::ToggledOff ||
		mDevices[deviceIndex].KeyStates[(u32)button & 0x0000FFFF] == ButtonState::ToggledOnOff;
}

bool Input::IsButtonDown(ButtonCode button, u32 deviceIndex) const
{
	if(deviceIndex >= (u32)mDevices.size())
		return false;

	return mDevices[deviceIndex].KeyStates[(u32)button & 0x0000FFFF] == ButtonState::ToggledOn ||
		mDevices[deviceIndex].KeyStates[(u32)button & 0x0000FFFF] == ButtonState::ToggledOnOff;
}

bool Input::IsPointerButtonHeld(PointerEventButton pointerButton) const
{
	return mPointerButtonStates[(u32)pointerButton] == ButtonState::On ||
		mPointerButtonStates[(u32)pointerButton] == ButtonState::ToggledOn ||
		mPointerButtonStates[(u32)pointerButton] == ButtonState::ToggledOnOff;
}

bool Input::IsPointerButtonUp(PointerEventButton pointerButton) const
{
	return mPointerButtonStates[(u32)pointerButton] == ButtonState::ToggledOff ||
		mPointerButtonStates[(u32)pointerButton] == ButtonState::ToggledOnOff;
}

bool Input::IsPointerButtonDown(PointerEventButton pointerButton) const
{
	return mPointerButtonStates[(u32)pointerButton] == ButtonState::ToggledOn ||
		mPointerButtonStates[(u32)pointerButton] == ButtonState::ToggledOnOff;
}

bool Input::IsPointerDoubleClicked() const
{
	return mPointerDoubleClicked;
}

Vector2I Input::GetPointerPosition() const
{
	return mPointerPosition;
}

String Input::GetDeviceName(InputDevice type, u32 deviceIndex)
{
	switch(type)
	{
	case InputDevice::Keyboard:
		if(mKeyboard != nullptr && deviceIndex == 0)
			return mKeyboard->GetName();

		return StringUtility::kBlank;
	case InputDevice::Mouse:
		if(mMouse != nullptr && deviceIndex == 0)
			return mMouse->GetName();

		return StringUtility::kBlank;
	case InputDevice::Gamepad:
		if(deviceIndex < (u32)mGamepads.size())
			return mGamepads[deviceIndex]->GetName();

		return StringUtility::kBlank;
	default:
		return StringUtility::kBlank;
	}
}

void Input::SetMouseSmoothing(bool enable)
{
	mMouseSmoothingEnabled = enable;
}

float Input::SmoothMouse(float value, u32 axisIndex)
{
	u32 sampleCount = 1;

	float deltaTime = GetTime().GetFrameDelta();
	if(deltaTime < 0.25f)
	{
		float secondsPerSample = mTotalMouseSamplingTime[axisIndex] / mTotalMouseSampleCount[axisIndex];

		if(value == 0.0f)
		{
			mMouseZeroTime[axisIndex] += deltaTime;
			if(mMouseZeroTime[axisIndex] < secondsPerSample)
				value = mMouseSmoothedAxis[axisIndex] * deltaTime / secondsPerSample;
			else
				mMouseSmoothedAxis[axisIndex] = 0;
		}
		else
		{
			mMouseZeroTime[axisIndex] = 0;
			if(mMouseSmoothedAxis[axisIndex] != 0)
			{
				if(deltaTime < secondsPerSample * (sampleCount + 1))
					value = value * deltaTime / (secondsPerSample * sampleCount);
				else
					sampleCount = Math::RoundToI32(deltaTime / secondsPerSample);
			}

			mMouseSmoothedAxis[axisIndex] = value / sampleCount;
		}
	}
	else
	{
		mMouseSmoothedAxis[axisIndex] = 0.0f;
		mMouseZeroTime[axisIndex] = 0.0f;
	}

	return value;
}

namespace b3d
{
Input& GetInput()
{
	return Input::Instance();
}
} // namespace b3d
