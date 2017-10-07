//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Input/BsInput.h"
#include "Input/BsMouse.h"
#include "Input/BsKeyboard.h"
#include "Input/BsGamepad.h"
#include "Utility/BsTime.h"
#include "Math/BsMath.h"
#include "Managers/BsRenderWindowManager.h"
#include "BsCoreApplication.h"

using namespace std::placeholders;

namespace bs
{
	// Note: Input polling methods for button/axis could be re-written so their query immediate state
	// instead of returning cached state from event callbacks. This /might/ result in even less input lag?

	const int Input::HISTORY_BUFFER_SIZE = 10; // Size of buffer used for input smoothing
	const float Input::WEIGHT_MODIFIER = 0.5f;

	Input::DeviceData::DeviceData()
	{
		for (UINT32 i = 0; i < BC_Count; i++)
			keyStates[i] = ButtonState::Off;
	}

	Input::Input()
		: mPointerDoubleClicked(false), mLastPositionSet(false), mMouseScroll(0.0f), mMouseSmoothingEnabled(false)
		, mMouse(nullptr), mKeyboard(nullptr)
	{
		SPtr<RenderWindow> primaryWindow = gCoreApplication().getPrimaryWindow();
		primaryWindow->getCustomAttribute("WINDOW", &mWindowHandle);

		// Subscribe to events
		mCharInputConn = Platform::onCharInput.connect(std::bind(&Input::charInput, this, _1));
		mCursorMovedConn = Platform::onCursorMoved.connect(std::bind(&Input::cursorMoved, this, _1, _2));
		mCursorPressedConn = Platform::onCursorButtonPressed.connect(std::bind(&Input::cursorPressed, this, _1, _2, _3));
		mCursorReleasedConn = Platform::onCursorButtonReleased.connect(std::bind(&Input::cursorReleased, this, _1, _2, _3));
		mCursorDoubleClickConn = Platform::onCursorDoubleClick.connect(std::bind(&Input::cursorDoubleClick, this, _1, _2));
		mInputCommandConn = Platform::onInputCommand.connect(std::bind(&Input::inputCommandEntered, this, _1));

		mMouseWheelScrolledConn  = Platform::onMouseWheelScrolled.connect(std::bind(&Input::mouseWheelScrolled, this, _1));

		RenderWindowManager::instance().onFocusGained.connect(std::bind(&Input::inputWindowChanged, this, _1));

		for (int i = 0; i < 3; i++)
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
		initRawInput();
	}

	Input::~Input()
	{
		cleanUpRawInput();

		mCharInputConn.disconnect();
		mCursorMovedConn.disconnect();
		mCursorPressedConn.disconnect();
		mCursorReleasedConn.disconnect();
		mCursorDoubleClickConn.disconnect();
		mInputCommandConn.disconnect();
		mMouseWheelScrolledConn.disconnect();
	}

	void Input::_update()
	{
		// Toggle states only remain active for a single frame before they are transitioned
		// into permanent state

		for (auto& deviceData : mDevices)
		{
			for (UINT32 i = 0; i < BC_Count; i++)
			{
				if (deviceData.keyStates[i] == ButtonState::ToggledOff || deviceData.keyStates[i] == ButtonState::ToggledOnOff)
					deviceData.keyStates[i] = ButtonState::Off;
				else if (deviceData.keyStates[i] == ButtonState::ToggledOn)
					deviceData.keyStates[i] = ButtonState::On;
			}

			UINT32 numAxes = (UINT32)deviceData.axes.size();
			for (UINT32 i = 0; i < numAxes; i++)
				deviceData.axes[i] = 0.0f;
		}

		for (UINT32 i = 0; i < 3; i++)
		{
			if (mPointerButtonStates[i] == ButtonState::ToggledOff || mPointerButtonStates[i] == ButtonState::ToggledOnOff)
				mPointerButtonStates[i] = ButtonState::Off;
			else if (mPointerButtonStates[i] == ButtonState::ToggledOn)
				mPointerButtonStates[i] = ButtonState::On;
		}

		mPointerDelta = Vector2I::ZERO; // Reset delta in case we don't receive any mouse input this frame
		mPointerDoubleClicked = false;

		// Capture raw input
		if (mMouse != nullptr)
			mMouse->capture();

		if (mKeyboard != nullptr)
			mKeyboard->capture();

		for (auto& gamepad : mGamepads)
			gamepad->capture();

		float rawXValue = 0.0f;
		float rawYValue = 0.0f;

		// Smooth mouse axes if needed
		if (mMouseSmoothingEnabled)
		{
			rawXValue = smoothMouse((float)mMouseSampleAccumulator[0], 0);
			rawYValue = smoothMouse((float)mMouseSampleAccumulator[1], 1);
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

		axisMoved(0, -rawXValue, (UINT32)InputAxis::MouseX);
		axisMoved(0, -rawYValue, (UINT32)InputAxis::MouseY);
	}

	void Input::_triggerCallbacks()
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

		if(pointerPos != mLastPointerPosition)
		{
			PointerEvent event;
			event.alt = false;
			event.shift = pointerState.shift;
			event.control = pointerState.ctrl;
			event.buttonStates[0] = pointerState.mouseButtons[0];
			event.buttonStates[1] = pointerState.mouseButtons[1];
			event.buttonStates[2] = pointerState.mouseButtons[2];
			event.mouseWheelScrollAmount = mouseScroll;

			event.type = PointerEventType::CursorMoved;
			event.screenPos = pointerPos;

			onPointerMoved(event);

			if (mLastPositionSet)
				mPointerDelta = event.screenPos - mLastPointerPosition;

			mLastPointerPosition = event.screenPos;
			mLastPositionSet = true;
		}

		for (auto& event : mQueuedEvents[1])
		{
			switch (event.type)
			{
			case EventType::ButtonDown:
			{
				const ButtonEvent& eventData = mButtonDownEvents[1][event.idx];

				mDevices[eventData.deviceIdx].keyStates[eventData.buttonCode & 0x0000FFFF] = ButtonState::ToggledOn;
				onButtonDown(mButtonDownEvents[1][event.idx]);
			}
				break;
			case EventType::ButtonUp:
			{
				const ButtonEvent& eventData = mButtonUpEvents[1][event.idx];

				while (eventData.deviceIdx >= (UINT32)mDevices.size())
					mDevices.push_back(DeviceData());

				if (mDevices[eventData.deviceIdx].keyStates[eventData.buttonCode & 0x0000FFFF] == ButtonState::ToggledOn)
					mDevices[eventData.deviceIdx].keyStates[eventData.buttonCode & 0x0000FFFF] = ButtonState::ToggledOnOff;
				else
					mDevices[eventData.deviceIdx].keyStates[eventData.buttonCode & 0x0000FFFF] = ButtonState::ToggledOff;

				onButtonUp(mButtonUpEvents[1][event.idx]);
			}
				break;
			case EventType::PointerDown:
			{
				const PointerEvent& eventData = mPointerPressedEvents[1][event.idx];
				mPointerButtonStates[(UINT32)eventData.button] = ButtonState::ToggledOn;

				onPointerPressed(eventData);
			}
				break;
			case EventType::PointerUp:
			{
				const PointerEvent& eventData = mPointerReleasedEvents[1][event.idx];

				if (mPointerButtonStates[(UINT32)eventData.button] == ButtonState::ToggledOn)
					mPointerButtonStates[(UINT32)eventData.button] = ButtonState::ToggledOnOff;
				else
					mPointerButtonStates[(UINT32)eventData.button] = ButtonState::ToggledOff;

				onPointerReleased(eventData);
			}
				break;
			case EventType::PointerDoubleClick:
				mPointerDoubleClicked = true;
				onPointerDoubleClick(mPointerDoubleClickEvents[1][event.idx]);
				break;
			case EventType::TextInput:
				onCharInput(mTextInputEvents[1][event.idx]);
				break;
			case EventType::Command:
				onInputCommand(mCommandEvents[1][event.idx]);
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

	void Input::inputWindowChanged(RenderWindow& win)
	{
		UINT64 hWnd = 0;
		win.getCustomAttribute("WINDOW", &hWnd);

		mKeyboard->changeCaptureContext(hWnd);
		mMouse->changeCaptureContext(hWnd);

		for (auto& gamepad : mGamepads)
			gamepad->changeCaptureContext(hWnd);
	}

	void Input::_notifyMouseMoved(INT32 relX, INT32 relY, INT32 relZ)
	{
		mMouseSampleAccumulator[0] += relX;
		mMouseSampleAccumulator[1] += relY;

		mTotalMouseNumSamples[0] += Math::roundToInt(Math::abs((float)relX));
		mTotalMouseNumSamples[1] += Math::roundToInt(Math::abs((float)relY));

		// Update sample times used for determining sampling rate. But only if something was
		// actually sampled, and only if this isn't the first non-zero sample.
		if (mLastMouseUpdateFrame != gTime().getFrameIdx())
		{
			if (relX != 0 && !Math::approxEquals(mMouseSmoothedAxis[0], 0.0f))
				mTotalMouseSamplingTime[0] += gTime().getFrameDelta();

			if (relY != 0 && !Math::approxEquals(mMouseSmoothedAxis[1], 0.0f))
				mTotalMouseSamplingTime[1] += gTime().getFrameDelta();

			mLastMouseUpdateFrame = gTime().getFrameIdx();
		}

		axisMoved(0, (float)relZ, (UINT32)InputAxis::MouseZ);
	}

	void Input::_notifyAxisMoved(UINT32 gamepadIdx, UINT32 axisIdx, INT32 value)
	{
		// Move axis values into [-1.0f, 1.0f] range
		float axisRange = Math::abs((float)Gamepad::MAX_AXIS) + Math::abs((float)Gamepad::MIN_AXIS);

		float axisValue = ((value + Math::abs((float)Gamepad::MIN_AXIS)) / axisRange) * 2.0f - 1.0f;
		axisMoved(gamepadIdx, axisValue, axisIdx);
	}

	void Input::_notifyButtonPressed(UINT32 deviceIdx, ButtonCode code, UINT64 timestamp)
	{
		buttonDown(deviceIdx, code, timestamp - mTimestampClockOffset);
	}

	void Input::_notifyButtonReleased(UINT32 deviceIdx, ButtonCode code, UINT64 timestamp)
	{
		buttonUp(deviceIdx, code, timestamp - mTimestampClockOffset);
	}

	void Input::buttonDown(UINT32 deviceIdx, ButtonCode code, UINT64 timestamp)
	{
		Lock lock(mMutex);

		while (deviceIdx >= (UINT32)mDevices.size())
			mDevices.push_back(DeviceData());

		ButtonEvent btnEvent;
		btnEvent.buttonCode = code;
		btnEvent.timestamp = timestamp;
		btnEvent.deviceIdx = deviceIdx;

		mQueuedEvents[0].push_back(QueuedEvent(EventType::ButtonDown, (UINT32)mButtonDownEvents[0].size()));
		mButtonDownEvents[0].push_back(btnEvent);
	}

	void Input::buttonUp(UINT32 deviceIdx, ButtonCode code, UINT64 timestamp)
	{
		Lock lock(mMutex);

		ButtonEvent btnEvent;
		btnEvent.buttonCode = code;
		btnEvent.timestamp = timestamp;
		btnEvent.deviceIdx = deviceIdx;

		mQueuedEvents[0].push_back(QueuedEvent(EventType::ButtonUp, (UINT32)mButtonUpEvents[0].size()));
		mButtonUpEvents[0].push_back(btnEvent);
	}

	void Input::axisMoved(UINT32 deviceIdx, float value, UINT32 axis)
	{
		// Note: This method must only ever be called from the main thread, as we don't lock access to axis data
		while (deviceIdx >= (UINT32)mDevices.size())
			mDevices.push_back(DeviceData());

		Vector<float>& axes = mDevices[deviceIdx].axes;
		while (axis >= (UINT32)axes.size())
			axes.push_back(0.0f);

		mDevices[deviceIdx].axes[axis] = value;
	}

	void Input::cursorMoved(const Vector2I& cursorPos, const OSPointerButtonStates& btnStates)
	{
		Lock lock(mMutex);

		mPointerPosition = cursorPos;
		mPointerState = btnStates;
	}

	void Input::cursorPressed(const Vector2I& cursorPos, OSMouseButton button, const OSPointerButtonStates& btnStates)
	{
		Lock lock(mMutex);

		PointerEvent event;
		event.alt = false;
		event.shift = btnStates.shift;
		event.control = btnStates.ctrl;
		event.buttonStates[0] = btnStates.mouseButtons[0];
		event.buttonStates[1] = btnStates.mouseButtons[1];
		event.buttonStates[2] = btnStates.mouseButtons[2];
		
		switch(button)
		{
		case OSMouseButton::Left:
			event.button = PointerEventButton::Left;
			break;
		case OSMouseButton::Middle:
			event.button = PointerEventButton::Middle;
			break;
		case OSMouseButton::Right:
			event.button = PointerEventButton::Right;
			break;
		default:
			break;
		}
		
		event.screenPos = cursorPos;
		event.type = PointerEventType::ButtonPressed;

		mQueuedEvents[0].push_back(QueuedEvent(EventType::PointerDown, (UINT32)mPointerPressedEvents[0].size()));
		mPointerPressedEvents[0].push_back(event);
	}

	void Input::cursorReleased(const Vector2I& cursorPos, OSMouseButton button, const OSPointerButtonStates& btnStates)
	{
		Lock lock(mMutex);

		PointerEvent event;
		event.alt = false;
		event.shift = btnStates.shift;
		event.control = btnStates.ctrl;
		event.buttonStates[0] = btnStates.mouseButtons[0];
		event.buttonStates[1] = btnStates.mouseButtons[1];
		event.buttonStates[2] = btnStates.mouseButtons[2];
		
		switch(button)
		{
		case OSMouseButton::Left:
			event.button = PointerEventButton::Left;
			break;
		case OSMouseButton::Middle:
			event.button = PointerEventButton::Middle;
			break;
		case OSMouseButton::Right:
			event.button = PointerEventButton::Right;
			break;
		default:
			break;
		}
		
		event.screenPos = cursorPos;
		event.type = PointerEventType::ButtonReleased;

		mQueuedEvents[0].push_back(QueuedEvent(EventType::PointerUp, (UINT32)mPointerReleasedEvents[0].size()));
		mPointerReleasedEvents[0].push_back(event);
	}

	void Input::cursorDoubleClick(const Vector2I& cursorPos, const OSPointerButtonStates& btnStates)
	{
		Lock lock(mMutex);

		PointerEvent event;
		event.alt = false;
		event.shift = btnStates.shift;
		event.control = btnStates.ctrl;
		event.buttonStates[0] = btnStates.mouseButtons[0];
		event.buttonStates[1] = btnStates.mouseButtons[1];
		event.buttonStates[2] = btnStates.mouseButtons[2];
		event.button = PointerEventButton::Left;
		event.screenPos = cursorPos;
		event.type = PointerEventType::DoubleClick;

		mQueuedEvents[0].push_back(QueuedEvent(EventType::PointerDoubleClick, (UINT32)mPointerDoubleClickEvents[0].size()));
		mPointerDoubleClickEvents[0].push_back(event);
	}

	void Input::inputCommandEntered(InputCommandType commandType)
	{
		Lock lock(mMutex);

		mQueuedEvents[0].push_back(QueuedEvent(EventType::Command, (UINT32)mCommandEvents[0].size()));
		mCommandEvents[0].push_back(commandType);
	}

	void Input::mouseWheelScrolled(float scrollPos)
	{
		Lock lock(mMutex);

		mMouseScroll = scrollPos;
	}

	void Input::charInput(UINT32 chr)
	{
		Lock lock(mMutex);

		TextInputEvent textInputEvent;
		textInputEvent.textChar = chr;

		mQueuedEvents[0].push_back(QueuedEvent(EventType::TextInput, (UINT32)mTextInputEvents[0].size()));
		mTextInputEvents[0].push_back(textInputEvent);
	}

	float Input::getAxisValue(UINT32 type, UINT32 deviceIdx) const
	{
		if (deviceIdx >= (UINT32)mDevices.size())
			return 0.0f;

		const Vector<float>& axes = mDevices[deviceIdx].axes;
		if (type >= (UINT32)axes.size())
			return 0.0f;

		return axes[type];
	}

	bool Input::isButtonHeld(ButtonCode button, UINT32 deviceIdx) const
	{
		if (deviceIdx >= (UINT32)mDevices.size())
			return false;

		return mDevices[deviceIdx].keyStates[button & 0x0000FFFF] == ButtonState::On || 
			mDevices[deviceIdx].keyStates[button & 0x0000FFFF] == ButtonState::ToggledOn ||
			mDevices[deviceIdx].keyStates[button & 0x0000FFFF] == ButtonState::ToggledOnOff;
	}

	bool Input::isButtonUp(ButtonCode button, UINT32 deviceIdx) const
	{
		if (deviceIdx >= (UINT32)mDevices.size())
			return false;

		return mDevices[deviceIdx].keyStates[button & 0x0000FFFF] == ButtonState::ToggledOff ||
			mDevices[deviceIdx].keyStates[button & 0x0000FFFF] == ButtonState::ToggledOnOff;
	}

	bool Input::isButtonDown(ButtonCode button, UINT32 deviceIdx) const
	{
		if (deviceIdx >= (UINT32)mDevices.size())
			return false;

		return mDevices[deviceIdx].keyStates[button & 0x0000FFFF] == ButtonState::ToggledOn ||
			mDevices[deviceIdx].keyStates[button & 0x0000FFFF] == ButtonState::ToggledOnOff;
	}

	bool Input::isPointerButtonHeld(PointerEventButton pointerButton) const
	{
		return mPointerButtonStates[(UINT32)pointerButton] == ButtonState::On ||
			mPointerButtonStates[(UINT32)pointerButton] == ButtonState::ToggledOn ||
			mPointerButtonStates[(UINT32)pointerButton] == ButtonState::ToggledOnOff;
	}

	bool Input::isPointerButtonUp(PointerEventButton pointerButton) const
	{
		return mPointerButtonStates[(UINT32)pointerButton] == ButtonState::ToggledOff ||
			mPointerButtonStates[(UINT32)pointerButton] == ButtonState::ToggledOnOff;
	}

	bool Input::isPointerButtonDown(PointerEventButton pointerButton) const
	{
		return mPointerButtonStates[(UINT32)pointerButton] == ButtonState::ToggledOn ||
			mPointerButtonStates[(UINT32)pointerButton] == ButtonState::ToggledOnOff;
	}

	bool Input::isPointerDoubleClicked() const
	{
		return mPointerDoubleClicked;
	}

	Vector2I Input::getPointerPosition() const
	{
		return mPointerPosition;
	}

	String Input::getDeviceName(InputDevice type, UINT32 idx)
	{
		switch(type)
		{
		case InputDevice::Keyboard: 
			if (mKeyboard != nullptr && idx == 0)
				return mKeyboard->getName();

			return StringUtil::BLANK;
		case InputDevice::Mouse:
			if (mMouse != nullptr && idx == 0)
				return mMouse->getName();

			return StringUtil::BLANK;
		case InputDevice::Gamepad: 
			if (idx < (UINT32)mGamepads.size())
				return mGamepads[idx]->getName();
			
			return StringUtil::BLANK;
		default:
			return StringUtil::BLANK;
		}
	}

	void Input::setMouseSmoothing(bool enable)
	{
		mMouseSmoothingEnabled = enable;
	}

	float Input::smoothMouse(float value, UINT32 idx)
	{
		UINT32 sampleCount = 1;

		float deltaTime = gTime().getFrameDelta();
		if (deltaTime < 0.25f)
		{
			float secondsPerSample = mTotalMouseSamplingTime[idx] / mTotalMouseNumSamples[idx];

			if (value == 0.0f)
			{
				mMouseZeroTime[idx] += deltaTime;
				if (mMouseZeroTime[idx] < secondsPerSample)
					value = mMouseSmoothedAxis[idx] * deltaTime / secondsPerSample;
				else
					mMouseSmoothedAxis[idx] = 0;
			}
			else
			{
				mMouseZeroTime[idx] = 0;
				if (mMouseSmoothedAxis[idx] != 0)
				{
					if (deltaTime < secondsPerSample * (sampleCount + 1))
						value = value * deltaTime / (secondsPerSample * sampleCount);
					else
						sampleCount = Math::roundToInt(deltaTime / secondsPerSample);
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

	Input& gInput()
	{
		return Input::instance();
	}
}