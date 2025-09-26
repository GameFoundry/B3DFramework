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

using namespace std::placeholders;

using namespace b3d;

// Note: Input polling methods for button/axis could be re-written so their query immediate state
// instead of returning cached state from event callbacks. This /might/ result in even less input lag?

const int Input::kHistoryBufferSize = 10; // Size of buffer used for input smoothing
const float Input::kWeightModifier = 0.5f;

Input::DeviceData::DeviceData()
{
	for(u32 i = 0; i < static_cast<unsigned>(ButtonCode::TotalKeyCount); i++)
		KeyStates[i] = ButtonState::Off;
}

Input::Input()
{
	SPtr<RenderWindow> primaryWindow = GetApplication().GetPrimaryWindow();
	mWindowHandle = primaryWindow->GetPlatformWindowHandle();

	// Subscribe to events
	mCharInputConn = Platform::onCharInput.Connect(std::bind(&Input::CharInput, this, _1));
	mCursorMovedConn = Platform::onCursorMoved.Connect(std::bind(&Input::CursorMoved, this, _1, _2));
	mCursorPressedConn = Platform::onCursorButtonPressed.Connect(std::bind(&Input::CursorPressed, this, _1, _2, _3));
	mCursorReleasedConn = Platform::onCursorButtonReleased.Connect(std::bind(&Input::CursorReleased, this, _1, _2, _3));
	mCursorDoubleClickConn = Platform::onCursorDoubleClick.Connect(std::bind(&Input::CursorDoubleClick, this, _1, _2));
	mInputCommandConn = Platform::onInputCommand.Connect(std::bind(&Input::InputCommandEntered, this, _1));

	mMouseWheelScrolledConn = Platform::onMouseWheelScrolled.Connect(std::bind(&Input::MouseWheelScrolled, this, _1));

	RenderWindowManager::Instance().OnFocusGained.Connect(std::bind(&Input::InputWindowChanged, this, _1));
	RenderWindowManager::Instance().OnFocusLost.Connect(std::bind(&Input::InputFocusLost, this));

	for(int i = 0; i < 3; i++)
		mPointerButtonStates[i] = ButtonState::Off;

	// Mouse smoothing
	mMouseSampleAccumulator[0] = 0;
	mMouseSampleAccumulator[1] = 0;
	mTotalMouseSamplingTime[0] = 1.0f / 125.0f; // Use 125Hz as initial pooling rate for mice
	mTotalMouseSamplingTime[1] = 1.0f / 125.0f;
	mTotalMouseNumSamples[0] = 1;
	mTotalMouseNumSamples[1] = 1;
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
		for(u32 i = 0; i < static_cast<unsigned>(ButtonCode::TotalKeyCount); i++)
		{
			if(deviceData.KeyStates[i] == ButtonState::ToggledOff || deviceData.KeyStates[i] == ButtonState::ToggledOnOff)
				deviceData.KeyStates[i] = ButtonState::Off;
			else if(deviceData.KeyStates[i] == ButtonState::ToggledOn)
				deviceData.KeyStates[i] = ButtonState::On;
		}

		u32 numAxes = (u32)deviceData.Axes.size();
		for(u32 i = 0; i < numAxes; i++)
			deviceData.Axes[i] = 0.0f;
	}

	for(u32 i = 0; i < 3; i++)
	{
		if(mPointerButtonStates[i] == ButtonState::ToggledOff || mPointerButtonStates[i] == ButtonState::ToggledOnOff)
			mPointerButtonStates[i] = ButtonState::Off;
		else if(mPointerButtonStates[i] == ButtonState::ToggledOn)
			mPointerButtonStates[i] = ButtonState::On;
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
	Vector2I pointerPos;
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

		pointerPos = mPointerPosition;
		mouseScroll = mMouseScroll;
		pointerState = mPointerState;

		mMouseScroll = 0.0f;
	}

	if(pointerPos != mLastPointerPosition || mouseScroll != 0.0f)
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
		event.ScreenPos = pointerPos;

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
				const ButtonEvent& eventData = mButtonDownEvents[1][event.Idx];

				mDevices[eventData.DeviceIndex].KeyStates[(u32)eventData.ButtonCode & 0x0000FFFF] = ButtonState::ToggledOn;
				OnButtonDown(mButtonDownEvents[1][event.Idx]);
			}
			break;
		case EventType::ButtonUp:
			{
				const ButtonEvent& eventData = mButtonUpEvents[1][event.Idx];

				while(eventData.DeviceIndex >= (u32)mDevices.size())
					mDevices.push_back(DeviceData());

				if(mDevices[eventData.DeviceIndex].KeyStates[(u32)eventData.ButtonCode & 0x0000FFFF] == ButtonState::ToggledOn)
					mDevices[eventData.DeviceIndex].KeyStates[(u32)eventData.ButtonCode & 0x0000FFFF] = ButtonState::ToggledOnOff;
				else
					mDevices[eventData.DeviceIndex].KeyStates[(u32)eventData.ButtonCode & 0x0000FFFF] = ButtonState::ToggledOff;

				OnButtonUp(mButtonUpEvents[1][event.Idx]);
			}
			break;
		case EventType::PointerDown:
			{
				const PointerEvent& eventData = mPointerPressedEvents[1][event.Idx];
				mPointerButtonStates[(u32)eventData.Button] = ButtonState::ToggledOn;

				OnPointerPressed(eventData);
			}
			break;
		case EventType::PointerUp:
			{
				const PointerEvent& eventData = mPointerReleasedEvents[1][event.Idx];

				if(mPointerButtonStates[(u32)eventData.Button] == ButtonState::ToggledOn)
					mPointerButtonStates[(u32)eventData.Button] = ButtonState::ToggledOnOff;
				else
					mPointerButtonStates[(u32)eventData.Button] = ButtonState::ToggledOff;

				OnPointerReleased(eventData);
			}
			break;
		case EventType::PointerDoubleClick:
			mPointerDoubleClicked = true;
			OnPointerDoubleClick(mPointerDoubleClickEvents[1][event.Idx]);
			break;
		case EventType::TextInput:
			OnCharInput(mTextInputEvents[1][event.Idx]);
			break;
		case EventType::Command:
			OnInputCommand(mCommandEvents[1][event.Idx]);
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
	const u64 hWnd = win.GetPlatformWindowHandle();

	if(mKeyboard != nullptr)
		mKeyboard->ChangeCaptureContext(hWnd);

	if(mMouse != nullptr)
		mMouse->ChangeCaptureContext(hWnd);

	for(auto& gamepad : mGamepads)
		gamepad->ChangeCaptureContext(hWnd);
}

void Input::InputFocusLost()
{
	if(mKeyboard != nullptr)
		mKeyboard->ChangeCaptureContext((u64)-1);

	if(mMouse != nullptr)
		mMouse->ChangeCaptureContext((u64)-1);

	for(auto& gamepad : mGamepads)
		gamepad->ChangeCaptureContext((u64)-1);
}

void Input::NotifyMouseMovedInternal(i32 relX, i32 relY, i32 relZ)
{
	mMouseSampleAccumulator[0] += relX;
	mMouseSampleAccumulator[1] += relY;

	mTotalMouseNumSamples[0] += Math::RoundToI32(Math::Abs((float)relX));
	mTotalMouseNumSamples[1] += Math::RoundToI32(Math::Abs((float)relY));

	// Update sample times used for determining sampling rate. But only if something was
	// actually sampled, and only if this isn't the first non-zero sample.
	if(mLastMouseUpdateFrame != GetTime().GetCurrentFrameIndex())
	{
		if(relX != 0 && !Math::ApproxEquals(mMouseSmoothedAxis[0], 0.0f))
			mTotalMouseSamplingTime[0] += GetTime().GetFrameDelta();

		if(relY != 0 && !Math::ApproxEquals(mMouseSmoothedAxis[1], 0.0f))
			mTotalMouseSamplingTime[1] += GetTime().GetFrameDelta();

		mLastMouseUpdateFrame = GetTime().GetCurrentFrameIndex();
	}

	AxisMoved(0, (float)relZ, (u32)InputAxis::MouseZ);
}

void Input::NotifyAxisMovedInternal(u32 gamepadIdx, u32 axisIdx, i32 value)
{
	// Move axis values into [-1.0f, 1.0f] range
	float axisRange = Math::Abs((float)Gamepad::kMaxAxis) + Math::Abs((float)Gamepad::kMinAxis);

	float axisValue = ((value + Math::Abs((float)Gamepad::kMinAxis)) / axisRange) * 2.0f - 1.0f;
	AxisMoved(gamepadIdx, axisValue, axisIdx);
}

void Input::NotifyButtonPressedInternal(u32 deviceIdx, ButtonCode code, u64 timestamp)
{
	ButtonDown(deviceIdx, code, timestamp - mTimestampClockOffset);
}

void Input::NotifyButtonReleasedInternal(u32 deviceIdx, ButtonCode code, u64 timestamp)
{
	ButtonUp(deviceIdx, code, timestamp - mTimestampClockOffset);
}

void Input::ButtonDown(u32 deviceIdx, ButtonCode code, u64 timestamp)
{
	Lock lock(mMutex);

	while(deviceIdx >= (u32)mDevices.size())
		mDevices.push_back(DeviceData());

	ButtonEvent btnEvent;
	btnEvent.ButtonCode = code;
	btnEvent.Timestamp = timestamp;
	btnEvent.DeviceIndex = deviceIdx;

	mQueuedEvents[0].push_back(QueuedEvent(EventType::ButtonDown, (u32)mButtonDownEvents[0].size()));
	mButtonDownEvents[0].push_back(btnEvent);
}

void Input::ButtonUp(u32 deviceIdx, ButtonCode code, u64 timestamp)
{
	Lock lock(mMutex);

	ButtonEvent btnEvent;
	btnEvent.ButtonCode = code;
	btnEvent.Timestamp = timestamp;
	btnEvent.DeviceIndex = deviceIdx;

	mQueuedEvents[0].push_back(QueuedEvent(EventType::ButtonUp, (u32)mButtonUpEvents[0].size()));
	mButtonUpEvents[0].push_back(btnEvent);
}

void Input::AxisMoved(u32 deviceIdx, float value, u32 axis)
{
	// Note: This method must only ever be called from the main thread, as we don't lock access to axis data
	while(deviceIdx >= (u32)mDevices.size())
		mDevices.push_back(DeviceData());

	Vector<float>& axes = mDevices[deviceIdx].Axes;
	while(axis >= (u32)axes.size())
		axes.push_back(0.0f);

	mDevices[deviceIdx].Axes[axis] = value;
}

void Input::CursorMoved(const Vector2I& cursorPos, const OSPointerButtonStates& btnStates)
{
	Lock lock(mMutex);

	mPointerPosition = cursorPos;
	mPointerState = btnStates;
}

void Input::CursorPressed(const Vector2I& cursorPos, OSMouseButton button, const OSPointerButtonStates& btnStates)
{
	Lock lock(mMutex);

	PointerEvent event;
	event.Alt = false;
	event.Shift = btnStates.Shift;
	event.Control = btnStates.Ctrl;
	event.ButtonStates[0] = btnStates.MouseButtons[0];
	event.ButtonStates[1] = btnStates.MouseButtons[1];
	event.ButtonStates[2] = btnStates.MouseButtons[2];

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

	event.ScreenPos = cursorPos;
	event.Type = PointerEventType::ButtonPressed;

	mQueuedEvents[0].push_back(QueuedEvent(EventType::PointerDown, (u32)mPointerPressedEvents[0].size()));
	mPointerPressedEvents[0].push_back(event);
}

void Input::CursorReleased(const Vector2I& cursorPos, OSMouseButton button, const OSPointerButtonStates& btnStates)
{
	Lock lock(mMutex);

	PointerEvent event;
	event.Alt = false;
	event.Shift = btnStates.Shift;
	event.Control = btnStates.Ctrl;
	event.ButtonStates[0] = btnStates.MouseButtons[0];
	event.ButtonStates[1] = btnStates.MouseButtons[1];
	event.ButtonStates[2] = btnStates.MouseButtons[2];

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

	event.ScreenPos = cursorPos;
	event.Type = PointerEventType::ButtonReleased;

	mQueuedEvents[0].push_back(QueuedEvent(EventType::PointerUp, (u32)mPointerReleasedEvents[0].size()));
	mPointerReleasedEvents[0].push_back(event);
}

void Input::CursorDoubleClick(const Vector2I& cursorPos, const OSPointerButtonStates& btnStates)
{
	Lock lock(mMutex);

	PointerEvent event;
	event.Alt = false;
	event.Shift = btnStates.Shift;
	event.Control = btnStates.Ctrl;
	event.ButtonStates[0] = btnStates.MouseButtons[0];
	event.ButtonStates[1] = btnStates.MouseButtons[1];
	event.ButtonStates[2] = btnStates.MouseButtons[2];
	event.Button = PointerEventButton::Left;
	event.ScreenPos = cursorPos;
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

void Input::MouseWheelScrolled(float scrollPos)
{
	Lock lock(mMutex);

	mMouseScroll = scrollPos;
}

void Input::CharInput(u32 chr)
{
	Lock lock(mMutex);

	TextInputEvent textInputEvent;
	textInputEvent.TextChar = chr;

	mQueuedEvents[0].push_back(QueuedEvent(EventType::TextInput, (u32)mTextInputEvents[0].size()));
	mTextInputEvents[0].push_back(textInputEvent);
}

float Input::GetAxisValue(u32 type, u32 deviceIdx) const
{
	if(deviceIdx >= (u32)mDevices.size())
		return 0.0f;

	const Vector<float>& axes = mDevices[deviceIdx].Axes;
	if(type >= (u32)axes.size())
		return 0.0f;

	return axes[type];
}

bool Input::IsButtonHeld(ButtonCode button, u32 deviceIdx) const
{
	if(deviceIdx >= (u32)mDevices.size())
		return false;

	return mDevices[deviceIdx].KeyStates[(u32)button & 0x0000FFFF] == ButtonState::On ||
		mDevices[deviceIdx].KeyStates[(u32)button & 0x0000FFFF] == ButtonState::ToggledOn ||
		mDevices[deviceIdx].KeyStates[(u32)button & 0x0000FFFF] == ButtonState::ToggledOnOff;
}

bool Input::IsButtonUp(ButtonCode button, u32 deviceIdx) const
{
	if(deviceIdx >= (u32)mDevices.size())
		return false;

	return mDevices[deviceIdx].KeyStates[(u32)button & 0x0000FFFF] == ButtonState::ToggledOff ||
		mDevices[deviceIdx].KeyStates[(u32)button & 0x0000FFFF] == ButtonState::ToggledOnOff;
}

bool Input::IsButtonDown(ButtonCode button, u32 deviceIdx) const
{
	if(deviceIdx >= (u32)mDevices.size())
		return false;

	return mDevices[deviceIdx].KeyStates[(u32)button & 0x0000FFFF] == ButtonState::ToggledOn ||
		mDevices[deviceIdx].KeyStates[(u32)button & 0x0000FFFF] == ButtonState::ToggledOnOff;
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

String Input::GetDeviceName(InputDevice type, u32 idx)
{
	switch(type)
	{
	case InputDevice::Keyboard:
		if(mKeyboard != nullptr && idx == 0)
			return mKeyboard->GetName();

		return StringUtil::kBlank;
	case InputDevice::Mouse:
		if(mMouse != nullptr && idx == 0)
			return mMouse->GetName();

		return StringUtil::kBlank;
	case InputDevice::Gamepad:
		if(idx < (u32)mGamepads.size())
			return mGamepads[idx]->GetName();

		return StringUtil::kBlank;
	default:
		return StringUtil::kBlank;
	}
}

void Input::SetMouseSmoothing(bool enable)
{
	mMouseSmoothingEnabled = enable;
}

float Input::SmoothMouse(float value, u32 idx)
{
	u32 sampleCount = 1;

	float deltaTime = GetTime().GetFrameDelta();
	if(deltaTime < 0.25f)
	{
		float secondsPerSample = mTotalMouseSamplingTime[idx] / mTotalMouseNumSamples[idx];

		if(value == 0.0f)
		{
			mMouseZeroTime[idx] += deltaTime;
			if(mMouseZeroTime[idx] < secondsPerSample)
				value = mMouseSmoothedAxis[idx] * deltaTime / secondsPerSample;
			else
				mMouseSmoothedAxis[idx] = 0;
		}
		else
		{
			mMouseZeroTime[idx] = 0;
			if(mMouseSmoothedAxis[idx] != 0)
			{
				if(deltaTime < secondsPerSample * (sampleCount + 1))
					value = value * deltaTime / (secondsPerSample * sampleCount);
				else
					sampleCount = Math::RoundToI32(deltaTime / secondsPerSample);
			}

			mMouseSmoothedAxis[idx] = value / sampleCount;
		}
	}
	else
	{
		mMouseSmoothedAxis[idx] = 0.0f;
		mMouseZeroTime[idx] = 0.0f;
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
