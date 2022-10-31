//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/BsVirtualInput.h"
#include "Input/BsInput.h"
#include "Math/BsMath.h"
#include "Utility/BsTime.h"

using namespace std::placeholders;

using namespace bs;

VirtualInput::VirtualInput()
{
	mInputConfiguration = CreateConfiguration();

	Input::Instance().OnButtonDown.Connect(std::bind(&VirtualInput::ButtonDown, this, _1));
	Input::Instance().OnButtonUp.Connect(std::bind(&VirtualInput::ButtonUp, this, _1));
}

SPtr<InputConfiguration> VirtualInput::CreateConfiguration()
{
	return bs_shared_ptr_new<InputConfiguration>();
}

void VirtualInput::SetConfiguration(const SPtr<InputConfiguration>& input)
{
	mInputConfiguration = input;

	// Note: Technically this is slightly wrong as it will
	// "forget" any buttons currently held down, but shouldn't matter much in practice.
	for(auto& deviceData : mDevices)
		deviceData.CachedStates.clear();
}

bool VirtualInput::IsButtonDown(const VirtualButton& button, u32 deviceIdx) const
{
	if(deviceIdx >= (u32)mDevices.size())
		return false;

	const Map<u32, ButtonData>& cachedStates = mDevices[deviceIdx].CachedStates;
	auto iterFind = cachedStates.find(button.ButtonIdentifier);

	if(iterFind != cachedStates.end())
		return iterFind->second.State == ButtonState::ToggledOn;

	return false;
}

bool VirtualInput::IsButtonUp(const VirtualButton& button, u32 deviceIdx) const
{
	if(deviceIdx >= (u32)mDevices.size())
		return false;

	const Map<u32, ButtonData>& cachedStates = mDevices[deviceIdx].CachedStates;
	auto iterFind = cachedStates.find(button.ButtonIdentifier);

	if(iterFind != cachedStates.end())
		return iterFind->second.State == ButtonState::ToggledOff;

	return false;
}

bool VirtualInput::IsButtonHeld(const VirtualButton& button, u32 deviceIdx) const
{
	if(deviceIdx >= (u32)mDevices.size())
		return false;

	const Map<u32, ButtonData>& cachedStates = mDevices[deviceIdx].CachedStates;
	auto iterFind = cachedStates.find(button.ButtonIdentifier);

	if(iterFind != cachedStates.end())
		return iterFind->second.State == ButtonState::On || iterFind->second.State == ButtonState::ToggledOn;

	return false;
}

float VirtualInput::GetAxisValue(const VirtualAxis& axis, u32 deviceIdx) const
{
	VIRTUAL_AXIS_DESC axisDesc;
	if(mInputConfiguration->GetAxisInternal(axis, axisDesc))
	{
		float axisValue = GetInput().GetAxisValue((u32)axisDesc.Type, deviceIdx);

		bool isMouseAxis = (u32)axisDesc.Type <= (u32)InputAxis::MouseZ;
		bool isNormalized = axisDesc.Normalize || !isMouseAxis;

		if(isNormalized && axisDesc.DeadZone > 0.0f)
		{
			// Scale to [-1, 1] range after removing the dead zone
			if(axisValue > 0)
				axisValue = std::max(0.f, axisValue - axisDesc.DeadZone) / (1.0f - axisDesc.DeadZone);
			else
				axisValue = -std::max(0.f, -axisValue - axisDesc.DeadZone) / (1.0f - axisDesc.DeadZone);
		}

		if(axisDesc.Normalize)
		{
			if(isMouseAxis)
			{
				// Currently normalizing using value of 1, which isn't doing anything, but keep the code in case that
				// changes
				axisValue /= 1.0f;
			}

			axisValue = Math::Clamp(axisValue * axisDesc.Sensitivity, -1.0f, 1.0f);
		}
		else
			axisValue *= axisDesc.Sensitivity;

		if(axisDesc.Invert)
			axisValue = -axisValue;

		return axisValue;
	}

	return 0.0f;
}

void VirtualInput::UpdateInternal()
{
	u64 frameIdx = GetTime().GetFrameIdx();
	for(auto& deviceData : mDevices)
	{
		for(auto& state : deviceData.CachedStates)
		{
			// We need to stay in toggled state for one frame.
			if(state.second.UpdateFrameIdx == frameIdx)
				continue;

			if(state.second.State == ButtonState::ToggledOff)
				state.second.State = ButtonState::Off;
			else if(state.second.State == ButtonState::ToggledOn)
				state.second.State = ButtonState::On;
		}
	}

	bool hasEvents = true;
	u64 repeatInternal = mInputConfiguration->GetRepeatInterval();
	u64 currentTime = GetTime().GetTimeMs();

	// Trigger all events
	while(hasEvents)
	{
		while(!mEvents.empty())
		{
			VirtualButtonEvent& event = mEvents.front();

			if(event.State == ButtonState::On)
			{
				if(!OnButtonDown.Empty())
					OnButtonDown(event.Button, event.DeviceIdx);
			}
			else if(event.State == ButtonState::Off)
			{
				if(!OnButtonUp.Empty())
					OnButtonUp(event.Button, event.DeviceIdx);
			}

			mEvents.pop();
		}

		// Queue up any repeatable events
		hasEvents = false;

		for(auto& deviceData : mDevices)
		{
			for(auto& state : deviceData.CachedStates)
			{
				if(state.second.State != ButtonState::On)
					continue;

				if(!state.second.AllowRepeat)
					continue;

				u64 diff = currentTime - state.second.Timestamp;
				if(diff >= repeatInternal)
				{
					state.second.Timestamp += repeatInternal;

					VirtualButtonEvent event;
					event.Button = state.second.Button;
					event.State = ButtonState::On;
					event.DeviceIdx = 0;

					mEvents.push(event);
					hasEvents = true;
				}
			}

			break; // Only repeat the first device. Repeat only makes sense for keyboard which there is only one of.
		}
	}

	// Send button held events
	u32 deviceIdx = 0;
	for(auto& deviceData : mDevices)
	{
		for(auto& btnIdentifier : deviceData.HeldButtons)
		{
			Map<u32, ButtonData>& cachedStates = deviceData.CachedStates;
			ButtonData& data = cachedStates[btnIdentifier];

			OnButtonHeld(data.Button, deviceIdx);
		}

		deviceIdx++;
	}
}

void VirtualInput::ButtonDown(const ButtonEvent& event)
{
	if(event.ButtonCode == BC_LSHIFT || event.ButtonCode == BC_RSHIFT)
		mActiveModifiers |= (u32)ButtonModifier::Shift;
	else if(event.ButtonCode == BC_LCONTROL || event.ButtonCode == BC_RCONTROL)
		mActiveModifiers |= (u32)ButtonModifier::Ctrl;
	else if(event.ButtonCode == BC_LMENU || event.ButtonCode == BC_RMENU)
		mActiveModifiers |= (u32)ButtonModifier::Alt;

	tempButtons.clear();
	tempBtnDescs.clear();

	if(mInputConfiguration->GetButtonsInternal(event.ButtonCode, mActiveModifiers, tempButtons, tempBtnDescs))
	{
		while(event.DeviceIdx >= (u32)mDevices.size())
			mDevices.push_back(DeviceData());

		Map<u32, ButtonData>& cachedStates = mDevices[event.DeviceIdx].CachedStates;
		DynArray<u32>& heldButtons = mDevices[event.DeviceIdx].HeldButtons;

		u32 numButtons = (u32)tempButtons.size();
		for(u32 i = 0; i < numButtons; i++)
		{
			const VirtualButton& btn = tempButtons[i];
			const VIRTUAL_BUTTON_DESC& btnDesc = tempBtnDescs[i];

			ButtonData& data = cachedStates[btn.ButtonIdentifier];

			data.Button = btn;
			data.State = ButtonState::ToggledOn;
			data.Timestamp = event.Timestamp;
			data.UpdateFrameIdx = GetTime().GetFrameIdx();
			data.AllowRepeat = btnDesc.Repeatable;

			VirtualButtonEvent virtualEvent;
			virtualEvent.Button = btn;
			virtualEvent.State = ButtonState::On;
			virtualEvent.DeviceIdx = event.DeviceIdx;

			mEvents.push(virtualEvent);
			heldButtons.Add(btn.ButtonIdentifier);
		}
	}
}

void VirtualInput::ButtonUp(const ButtonEvent& event)
{
	if(event.ButtonCode == BC_LSHIFT || event.ButtonCode == BC_RSHIFT)
		mActiveModifiers &= ~(u32)ButtonModifier::Shift;
	else if(event.ButtonCode == BC_LCONTROL || event.ButtonCode == BC_RCONTROL)
		mActiveModifiers &= ~(u32)ButtonModifier::Ctrl;
	else if(event.ButtonCode == BC_LMENU || event.ButtonCode == BC_RMENU)
		mActiveModifiers &= ~(u32)ButtonModifier::Alt;

	tempButtons.clear();
	tempBtnDescs.clear();

	if(mInputConfiguration->GetButtonsInternal(event.ButtonCode, mActiveModifiers, tempButtons, tempBtnDescs))
	{
		while(event.DeviceIdx >= (u32)mDevices.size())
			mDevices.push_back(DeviceData());

		Map<u32, ButtonData>& cachedStates = mDevices[event.DeviceIdx].CachedStates;
		DynArray<u32>& heldButtons = mDevices[event.DeviceIdx].HeldButtons;

		u32 numButtons = (u32)tempButtons.size();
		for(u32 i = 0; i < numButtons; i++)
		{
			const VirtualButton& btn = tempButtons[i];
			const VIRTUAL_BUTTON_DESC& btnDesc = tempBtnDescs[i];

			ButtonData& data = cachedStates[btn.ButtonIdentifier];

			data.Button = btn;
			data.State = ButtonState::ToggledOff;
			data.Timestamp = event.Timestamp;
			data.UpdateFrameIdx = GetTime().GetFrameIdx();
			data.AllowRepeat = btnDesc.Repeatable;

			VirtualButtonEvent virtualEvent;
			virtualEvent.Button = btn;
			virtualEvent.State = ButtonState::Off;
			virtualEvent.DeviceIdx = event.DeviceIdx;

			mEvents.push(virtualEvent);

			auto iterFind = std::find(heldButtons.begin(), heldButtons.end(), btn.ButtonIdentifier);
			if(iterFind != heldButtons.end())
				heldButtons.SwapAndErase(iterFind);
		}
	}
}

namespace bs
{
VirtualInput& GetVirtualInput()
{
	return VirtualInput::Instance();
}
} // namespace bs
