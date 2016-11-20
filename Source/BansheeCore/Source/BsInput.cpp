//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsInput.h"
#include "BsTime.h"
#include "BsMath.h"
#include "BsRect2I.h"
#include "BsDebug.h"
#include "BsRenderWindowManager.h"

using namespace std::placeholders;

namespace bs
{
	const int Input::HISTORY_BUFFER_SIZE = 10; // Size of buffer used for input smoothing
	const float Input::WEIGHT_MODIFIER = 0.5f;

	Input::DeviceData::DeviceData()
	{
		for (int i = 0; i < BC_Count; i++)
			keyStates[i] = ButtonState::Off;
	}

	Input::Input()
		:mPointerDoubleClicked(false), mLastPositionSet(false)
	{ 
		mOSInputHandler = bs_shared_ptr_new<OSInputHandler>();

		mOSInputHandler->onCharInput.connect(std::bind(&Input::charInput, this, _1));
		mOSInputHandler->onCursorMoved.connect(std::bind(&Input::cursorMoved, this, _1));
		mOSInputHandler->onCursorPressed.connect(std::bind(&Input::cursorPressed, this, _1));
		mOSInputHandler->onCursorReleased.connect(std::bind(&Input::cursorReleased, this, _1));
		mOSInputHandler->onDoubleClick.connect(std::bind(&Input::cursorDoubleClick, this, _1));
		mOSInputHandler->onInputCommand.connect(std::bind(&Input::inputCommandEntered, this, _1));

		RenderWindowManager::instance().onFocusGained.connect(std::bind(&Input::inputWindowChanged, this, _1));

		for (int i = 0; i < 3; i++)
			mPointerButtonStates[i] = ButtonState::Off;
	}

	Input::~Input()
	{ }

	void Input::_registerRawInputHandler(SPtr<RawInputHandler> inputHandler)
	{
		if(mRawInputHandler != inputHandler)
		{
			mRawInputHandler = inputHandler;

			if(mRawInputHandler != nullptr)
			{
				mRawInputHandler->onButtonDown.connect(std::bind(&Input::buttonDown, this, _1, _2, _3));
				mRawInputHandler->onButtonUp.connect(std::bind(&Input::buttonUp, this, _1, _2, _3));

				mRawInputHandler->onAxisMoved.connect(std::bind(&Input::axisMoved, this, _1, _2, _3));
			}
		}
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
			{
				deviceData.axes[i].rel = 0.0f;
			}
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

		if(mRawInputHandler == nullptr)
		{
			LOGERR("Raw input handler not initialized!");
			return;
		}
		else
			mRawInputHandler->_update();

		if(mOSInputHandler == nullptr)
		{
			LOGERR("OS input handler not initialized!");
			return;
		}
		else
			mOSInputHandler->_update();
	}

	void Input::_triggerCallbacks()
	{
		for (auto& event : mQueuedEvents)
		{
			switch (event.type)
			{
			case EventType::ButtonDown:
				onButtonDown(mButtonDownEvents[event.idx]);
				break;
			case EventType::ButtonUp:
				onButtonUp(mButtonUpEvents[event.idx]);
				break;
			case EventType::PointerDown:
				onPointerPressed(mPointerPressedEvents[event.idx]);
				break;
			case EventType::PointerUp:
				onPointerReleased(mPointerReleasedEvents[event.idx]);
				break;
			case EventType::PointerDoubleClick:
				onPointerDoubleClick(mPointerDoubleClickEvents[event.idx]);
				break;
			case EventType::PointerMoved:
				onPointerMoved(mPointerMovedEvents[event.idx]);
				break;
			case EventType::TextInput:
				onCharInput(mTextInputEvents[event.idx]);
				break;
			case EventType::Command:
				onInputCommand(mCommandEvents[event.idx]);
				break;
			}
		}

		mQueuedEvents.clear();
		mButtonDownEvents.clear();
		mButtonUpEvents.clear();
		mPointerPressedEvents.clear();
		mPointerReleasedEvents.clear();
		mPointerDoubleClickEvents.clear();
		mPointerMovedEvents.clear();
		mTextInputEvents.clear();
		mCommandEvents.clear();
	}

	void Input::inputWindowChanged(RenderWindow& win)
	{
		if(mRawInputHandler != nullptr)
			mRawInputHandler->_inputWindowChanged(win);

		if(mOSInputHandler != nullptr)
			mOSInputHandler->_inputWindowChanged(win);
	}

	void Input::buttonDown(UINT32 deviceIdx, ButtonCode code, UINT64 timestamp)
	{
		while (deviceIdx >= (UINT32)mDevices.size())
			mDevices.push_back(DeviceData());

		mDevices[deviceIdx].keyStates[code & 0x0000FFFF] = ButtonState::ToggledOn;

		ButtonEvent btnEvent;
		btnEvent.buttonCode = code;
		btnEvent.timestamp = timestamp;
		btnEvent.deviceIdx = deviceIdx;

		mQueuedEvents.push_back(QueuedEvent(EventType::ButtonDown, (UINT32)mButtonDownEvents.size()));
		mButtonDownEvents.push_back(btnEvent);
	}

	void Input::buttonUp(UINT32 deviceIdx, ButtonCode code, UINT64 timestamp)
	{
		while (deviceIdx >= (UINT32)mDevices.size())
			mDevices.push_back(DeviceData());

		if (mDevices[deviceIdx].keyStates[code & 0x0000FFFF] == ButtonState::ToggledOn)
			mDevices[deviceIdx].keyStates[code & 0x0000FFFF] = ButtonState::ToggledOnOff;
		else
			mDevices[deviceIdx].keyStates[code & 0x0000FFFF] = ButtonState::ToggledOff;

		ButtonEvent btnEvent;
		btnEvent.buttonCode = code;
		btnEvent.timestamp = timestamp;
		btnEvent.deviceIdx = deviceIdx;

		mQueuedEvents.push_back(QueuedEvent(EventType::ButtonUp, (UINT32)mButtonUpEvents.size()));
		mButtonUpEvents.push_back(btnEvent);
	}

	void Input::axisMoved(UINT32 deviceIdx, const RawAxisState& state, UINT32 axis)
	{
		while (deviceIdx >= (UINT32)mDevices.size())
			mDevices.push_back(DeviceData());

		Vector<RawAxisState>& axes = mDevices[deviceIdx].axes;
		while (axis >= (UINT32)axes.size())
			axes.push_back(RawAxisState());

		mDevices[deviceIdx].axes[axis] = state;
	}

	void Input::cursorMoved(const PointerEvent& event)
	{
		mQueuedEvents.push_back(QueuedEvent(EventType::PointerMoved, (UINT32)mPointerMovedEvents.size()));
		mPointerMovedEvents.push_back(event);

		if (mLastPositionSet)
			mPointerDelta = event.screenPos - mPointerPosition;

		mPointerPosition = event.screenPos;
		mLastPositionSet = true;
	}

	void Input::cursorPressed(const PointerEvent& event)
	{
		mPointerButtonStates[(UINT32)event.button] = ButtonState::ToggledOn;

		mQueuedEvents.push_back(QueuedEvent(EventType::PointerDown, (UINT32)mPointerPressedEvents.size()));
		mPointerPressedEvents.push_back(event);
	}

	void Input::cursorReleased(const PointerEvent& event)
	{
		if (mPointerButtonStates[(UINT32)event.button] == ButtonState::ToggledOn)
			mPointerButtonStates[(UINT32)event.button] = ButtonState::ToggledOnOff;
		else
			mPointerButtonStates[(UINT32)event.button] = ButtonState::ToggledOff;

		mQueuedEvents.push_back(QueuedEvent(EventType::PointerUp, (UINT32)mPointerReleasedEvents.size()));
		mPointerReleasedEvents.push_back(event);
	}

	void Input::cursorDoubleClick(const PointerEvent& event)
	{
		mPointerDoubleClicked = true;

		mQueuedEvents.push_back(QueuedEvent(EventType::PointerDoubleClick, (UINT32)mPointerDoubleClickEvents.size()));
		mPointerDoubleClickEvents.push_back(event);
	}

	void Input::inputCommandEntered(InputCommandType commandType)
	{
		mQueuedEvents.push_back(QueuedEvent(EventType::Command, (UINT32)mCommandEvents.size()));
		mCommandEvents.push_back(commandType);
	}

	void Input::charInput(UINT32 chr)
	{
		TextInputEvent textInputEvent;
		textInputEvent.textChar = chr;

		mQueuedEvents.push_back(QueuedEvent(EventType::TextInput, (UINT32)mTextInputEvents.size()));
		mTextInputEvents.push_back(textInputEvent);
	}

	float Input::getAxisValue(UINT32 type, UINT32 deviceIdx) const
	{
		if (deviceIdx >= (UINT32)mDevices.size())
			return 0.0f;

		const Vector<RawAxisState>& axes = mDevices[deviceIdx].axes;
		if (type >= (UINT32)axes.size())
			return 0.0f;

		return axes[type].rel;
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

	void Input::setMouseSmoothing(bool enable)
	{
		mRawInputHandler->setMouseSmoothing(enable);
	}

	Input& gInput()
	{
		return Input::instance();
	}
}