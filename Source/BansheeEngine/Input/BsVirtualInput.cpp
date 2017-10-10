//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Input/BsVirtualInput.h"
#include "Input/BsInput.h"
#include "Math/BsMath.h"
#include "Utility/BsTime.h"

using namespace std::placeholders;

namespace bs
{
	VirtualInput::VirtualInput()
		:mActiveModifiers((UINT32)ButtonModifier::None)
	{
		mInputConfiguration = createConfiguration();

		Input::instance().onButtonDown.connect(std::bind(&VirtualInput::buttonDown, this, _1));
		Input::instance().onButtonUp.connect(std::bind(&VirtualInput::buttonUp, this, _1));
	}

	SPtr<InputConfiguration> VirtualInput::createConfiguration()
	{
		return bs_shared_ptr_new<InputConfiguration>();
	}

	void VirtualInput::setConfiguration(const SPtr<InputConfiguration>& input)
	{
		mInputConfiguration = input;

		// Note: Technically this is slightly wrong as it will
		// "forget" any buttons currently held down, but shouldn't matter much in practice.
		for (auto& deviceData : mDevices)
			deviceData.cachedStates.clear(); 
	}

	bool VirtualInput::isButtonDown(const VirtualButton& button, UINT32 deviceIdx) const
	{
		if (deviceIdx >= (UINT32)mDevices.size())
			return false;

		const Map<UINT32, ButtonData>& cachedStates = mDevices[deviceIdx].cachedStates;
		auto iterFind = cachedStates.find(button.buttonIdentifier);

		if (iterFind != cachedStates.end())
			return iterFind->second.state == ButtonState::ToggledOn;
		
		return false;
	}

	bool VirtualInput::isButtonUp(const VirtualButton& button, UINT32 deviceIdx) const
	{
		if (deviceIdx >= (UINT32)mDevices.size())
			return false;

		const Map<UINT32, ButtonData>& cachedStates = mDevices[deviceIdx].cachedStates;
		auto iterFind = cachedStates.find(button.buttonIdentifier);

		if (iterFind != cachedStates.end())
			return iterFind->second.state == ButtonState::ToggledOff;

		return false;
	}

	bool VirtualInput::isButtonHeld(const VirtualButton& button, UINT32 deviceIdx) const
	{
		if (deviceIdx >= (UINT32)mDevices.size())
			return false;

		const Map<UINT32, ButtonData>& cachedStates = mDevices[deviceIdx].cachedStates;
		auto iterFind = cachedStates.find(button.buttonIdentifier);

		if (iterFind != cachedStates.end())
			return iterFind->second.state == ButtonState::On || iterFind->second.state == ButtonState::ToggledOn;

		return false;
	}

	float VirtualInput::getAxisValue(const VirtualAxis& axis, UINT32 deviceIdx) const
	{
		VIRTUAL_AXIS_DESC axisDesc;
		if (mInputConfiguration->_getAxis(axis, axisDesc))
		{
			float axisValue = gInput().getAxisValue((UINT32)axisDesc.type, deviceIdx);

			bool isMouseAxis = (UINT32)axisDesc.type <= (UINT32)InputAxis::MouseZ;
			bool isNormalized = axisDesc.normalize || !isMouseAxis;

			if (isNormalized && axisDesc.deadZone > 0.0f)
			{
				// Scale to [-1, 1] range after removing the dead zone
				if (axisValue > 0)
					axisValue = std::max(0.f, axisValue - axisDesc.deadZone) / (1.0f - axisDesc.deadZone);
				else
					axisValue = -std::max(0.f, -axisValue - axisDesc.deadZone) / (1.0f - axisDesc.deadZone);
			}

			if(axisDesc.normalize)
			{
				if(isMouseAxis)
				{
					// Currently normalizing using value of 1, which isn't doing anything, but keep the code in case that
					// changes
					axisValue /= 1.0f;
				}

				axisValue = Math::clamp(axisValue * axisDesc.sensitivity, -1.0f, 1.0f);
			}
			else
				axisValue *= axisDesc.sensitivity;

			if (axisDesc.invert)
				axisValue = -axisValue;

			return axisValue;
		}

		return 0.0f;
	}

	void VirtualInput::_update()
	{
		UINT64 frameIdx = gTime().getFrameIdx();
		for (auto& deviceData : mDevices)
		{
			for (auto& state : deviceData.cachedStates)
			{
				// We need to stay in toggled state for one frame.
				if (state.second.updateFrameIdx == frameIdx)
					continue;

				if (state.second.state == ButtonState::ToggledOff)
					state.second.state = ButtonState::Off;
				else if (state.second.state == ButtonState::ToggledOn)
					state.second.state = ButtonState::On;
			}
		}

		bool hasEvents = true;
		UINT64 repeatInternal = mInputConfiguration->getRepeatInterval();
		UINT64 currentTime = gTime().getTimeMs();

		// Trigger all events
		while(hasEvents)
		{
			while(!mEvents.empty())
			{
				VirtualButtonEvent& event = mEvents.front();

				if(event.state == ButtonState::On)
				{
					if(!onButtonDown.empty())
						onButtonDown(event.button, event.deviceIdx);
				}
				else if(event.state == ButtonState::Off)
				{
					if(!onButtonUp.empty())
						onButtonUp(event.button, event.deviceIdx);
				}

				mEvents.pop();
			}

			// Queue up any repeatable events
			hasEvents = false;

			for (auto& deviceData : mDevices)
			{
				for (auto& state : deviceData.cachedStates)
				{
					if (state.second.state != ButtonState::On)
						continue;

					if (!state.second.allowRepeat)
						continue;

					UINT64 diff = currentTime - state.second.timestamp;
					if (diff >= repeatInternal)
					{
						state.second.timestamp += repeatInternal;

						VirtualButtonEvent event;
						event.button = state.second.button;
						event.state = ButtonState::On;
						event.deviceIdx = 0;

						mEvents.push(event);
						hasEvents = true;
					}
				}

				break; // Only repeat the first device. Repeat only makes sense for keyboard which there is only one of.
			}
		}
	}

	void VirtualInput::buttonDown(const ButtonEvent& event)
	{
		if(event.buttonCode == BC_LSHIFT || event.buttonCode == BC_RSHIFT)
			mActiveModifiers |= (UINT32)ButtonModifier::Shift;
		else if(event.buttonCode == BC_LCONTROL || event.buttonCode == BC_RCONTROL)
			mActiveModifiers |= (UINT32)ButtonModifier::Ctrl;
		else if(event.buttonCode == BC_LMENU || event.buttonCode == BC_RMENU)
			mActiveModifiers |= (UINT32)ButtonModifier::Alt;

		tempButtons.clear();
		tempBtnDescs.clear();

		if (mInputConfiguration->_getButtons(event.buttonCode, mActiveModifiers, tempButtons, tempBtnDescs))
		{
			while (event.deviceIdx >= (UINT32)mDevices.size())
				mDevices.push_back(DeviceData());

			Map<UINT32, ButtonData>& cachedStates = mDevices[event.deviceIdx].cachedStates;

			UINT32 numButtons = (UINT32)tempButtons.size();
			for (UINT32 i = 0; i < numButtons; i++)
			{
				const VirtualButton& btn = tempButtons[i];
				const VIRTUAL_BUTTON_DESC& btnDesc = tempBtnDescs[i];

				ButtonData& data = cachedStates[btn.buttonIdentifier];

				data.button = btn;
				data.state = ButtonState::ToggledOn;
				data.timestamp = event.timestamp;
				data.updateFrameIdx = gTime().getFrameIdx();
				data.allowRepeat = btnDesc.repeatable;

				VirtualButtonEvent virtualEvent;
				virtualEvent.button = btn;
				virtualEvent.state = ButtonState::On;
				virtualEvent.deviceIdx = event.deviceIdx;

				mEvents.push(virtualEvent);
			}
		}
	}

	void VirtualInput::buttonUp(const ButtonEvent& event)
	{
		if(event.buttonCode == BC_LSHIFT || event.buttonCode == BC_RSHIFT)
			mActiveModifiers &= ~(UINT32)ButtonModifier::Shift;
		else if(event.buttonCode == BC_LCONTROL || event.buttonCode == BC_RCONTROL)
			mActiveModifiers &= ~(UINT32)ButtonModifier::Ctrl;
		else if(event.buttonCode == BC_LMENU || event.buttonCode == BC_RMENU)
			mActiveModifiers &= ~(UINT32)ButtonModifier::Alt;

		tempButtons.clear();
		tempBtnDescs.clear();

		if (mInputConfiguration->_getButtons(event.buttonCode, mActiveModifiers, tempButtons, tempBtnDescs))
		{
			while (event.deviceIdx >= (UINT32)mDevices.size())
				mDevices.push_back(DeviceData());

			Map<UINT32, ButtonData>& cachedStates = mDevices[event.deviceIdx].cachedStates;

			UINT32 numButtons = (UINT32)tempButtons.size();
			for (UINT32 i = 0; i < numButtons; i++)
			{
				const VirtualButton& btn = tempButtons[i];
				const VIRTUAL_BUTTON_DESC& btnDesc = tempBtnDescs[i];

				ButtonData& data = cachedStates[btn.buttonIdentifier];

				data.button = btn;
				data.state = ButtonState::ToggledOff;
				data.timestamp = event.timestamp;
				data.updateFrameIdx = gTime().getFrameIdx();
				data.allowRepeat = btnDesc.repeatable;

				VirtualButtonEvent virtualEvent;
				virtualEvent.button = btn;
				virtualEvent.state = ButtonState::Off;
				virtualEvent.deviceIdx = event.deviceIdx;

				mEvents.push(virtualEvent);
			}
		}
	}

	VirtualInput& gVirtualInput()
	{
		return VirtualInput::instance();
	}
}