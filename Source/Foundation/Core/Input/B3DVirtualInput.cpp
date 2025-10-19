//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DVirtualInput.h"
#include "Input/B3DInput.h"
#include "Math/B3DMath.h"
#include "Utility/B3DTime.h"

using namespace b3d;

u32 VirtualInput::sNextVirtualButtonId = 0;
u32 VirtualInput::sNextVirtualAxisId = 0;

VirtualInput::VirtualInput()
{
	mInputConfiguration = B3DMakeShared<InputConfiguration>();

	Input::Instance().OnButtonDown.Connect([this](const ButtonEvent& event) { ButtonDown(event); });
	Input::Instance().OnButtonUp.Connect([this](const ButtonEvent& event) { ButtonUp(event); });
}

void VirtualInput::SetConfiguration(const SPtr<InputConfiguration>& input)
{
	mInputConfiguration = input;

	// Note: Technically this is slightly wrong as it will
	// "forget" any buttons currently held down, but shouldn't matter much in practice.
	for(auto& deviceData : mDevices)
		deviceData.CachedStates.clear();
}

VirtualButton VirtualInput::GetOrCreateVirtualButton(const String& name)
{
	static UnorderedMap<String, u32> sUniqueVirtualButtonIds;
	if(auto found = sUniqueVirtualButtonIds.find(name); found != sUniqueVirtualButtonIds.end())
		return VirtualButton(found->second);

	const u32 newId = sNextVirtualButtonId++;
	sUniqueVirtualButtonIds[name] = newId;

	return VirtualButton(newId);
}

VirtualAxis VirtualInput::GetOrCreateVirtualAxis(const String& name)
{
	static UnorderedMap<String, u32> sUniqueVirtualAxisIds;
	if(auto found = sUniqueVirtualAxisIds.find(name); found != sUniqueVirtualAxisIds.end())
		return VirtualAxis(found->second);

	const u32 newId = sNextVirtualAxisId++;
	sUniqueVirtualAxisIds[name] = newId;

	return VirtualAxis(newId);
}

bool VirtualInput::IsButtonDown(const VirtualButton& button, u32 deviceIndex) const
{
	if(deviceIndex >= (u32)mDevices.size())
		return false;

	const Map<u32, ButtonData>& cachedStates = mDevices[deviceIndex].CachedStates;
	auto found = cachedStates.find(button.ButtonIdentifier);

	if(found != cachedStates.end())
		return found->second.State == ButtonState::ToggledOn;

	return false;
}

bool VirtualInput::IsButtonUp(const VirtualButton& button, u32 deviceIndex) const
{
	if(deviceIndex >= (u32)mDevices.size())
		return false;

	const Map<u32, ButtonData>& cachedStates = mDevices[deviceIndex].CachedStates;
	auto found = cachedStates.find(button.ButtonIdentifier);

	if(found != cachedStates.end())
		return found->second.State == ButtonState::ToggledOff;

	return false;
}

bool VirtualInput::IsButtonHeld(const VirtualButton& button, u32 deviceIndex) const
{
	if(deviceIndex >= (u32)mDevices.size())
		return false;

	const Map<u32, ButtonData>& cachedStates = mDevices[deviceIndex].CachedStates;
	auto found = cachedStates.find(button.ButtonIdentifier);

	if(found != cachedStates.end())
		return found->second.State == ButtonState::On || found->second.State == ButtonState::ToggledOn;

	return false;
}

float VirtualInput::GetAxisValue(const VirtualAxis& axis, u32 deviceIndex) const
{
	VirtualAxisInformation axisDescription;
	if(mInputConfiguration->GetAxisInternal(axis, axisDescription))
	{
		float axisValue = GetInput().GetAxisValue((u32)axisDescription.Type, deviceIndex);

		bool isMouseAxis = (u32)axisDescription.Type <= (u32)InputAxis::MouseZ;
		bool isNormalized = axisDescription.Normalize || !isMouseAxis;

		if(isNormalized && axisDescription.DeadZone > 0.0f)
		{
			// Scale to [-1, 1] range after removing the dead zone
			if(axisValue > 0)
				axisValue = std::max(0.f, axisValue - axisDescription.DeadZone) / (1.0f - axisDescription.DeadZone);
			else
				axisValue = -std::max(0.f, -axisValue - axisDescription.DeadZone) / (1.0f - axisDescription.DeadZone);
		}

		if(axisDescription.Normalize)
		{
			if(isMouseAxis)
			{
				// Currently normalizing using value of 1, which isn't doing anything, but keep the code in case that
				// changes
				axisValue /= 1.0f;
			}

			axisValue = Math::Clamp(axisValue * axisDescription.Sensitivity, -1.0f, 1.0f);
		}
		else
			axisValue *= axisDescription.Sensitivity;

		if(axisDescription.Invert)
			axisValue = -axisValue;

		return axisValue;
	}

	return 0.0f;
}

void VirtualInput::UpdateInternal()
{
	u64 frameIndex = GetTime().GetCurrentFrameIndex();
	for(auto& deviceData : mDevices)
	{
		for(auto& state : deviceData.CachedStates)
		{
			// We need to stay in toggled state for one frame.
			if(state.second.UpdateFrameIdx == frameIndex)
				continue;

			if(state.second.State == ButtonState::ToggledOff)
				state.second.State = ButtonState::Off;
			else if(state.second.State == ButtonState::ToggledOn)
				state.second.State = ButtonState::On;
		}
	}

	bool hasEvents = true;
	u64 repeatInterval = mInputConfiguration->GetRepeatInterval();
	u64 currentTime = GetTime().GetRealTimeInMilliseconds();

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

				u64 timeDifference = currentTime - state.second.Timestamp;
				if(timeDifference >= repeatInterval)
				{
					state.second.Timestamp += repeatInterval;

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
	u32 deviceIndex = 0;
	for(auto& deviceData : mDevices)
	{
		for(auto& buttonIdentifier : deviceData.HeldButtons)
		{
			Map<u32, ButtonData>& cachedStates = deviceData.CachedStates;
			ButtonData& data = cachedStates[buttonIdentifier];

			OnButtonHeld(data.Button, deviceIndex);
		}

		deviceIndex++;
	}
}

void VirtualInput::ButtonDown(const ButtonEvent& event)
{
	if(event.ButtonCode == ButtonCode::LeftShift || event.ButtonCode == ButtonCode::RightShift)
		mActiveModifiers |= (u32)ButtonModifier::Shift;
	else if(event.ButtonCode == ButtonCode::LeftControl || event.ButtonCode == ButtonCode::RightControl)
		mActiveModifiers |= (u32)ButtonModifier::Ctrl;
	else if(event.ButtonCode == ButtonCode::LeftAlt || event.ButtonCode == ButtonCode::RightAlt)
		mActiveModifiers |= (u32)ButtonModifier::Alt;

	tempButtons.clear();
	tempBtnDescs.clear();

	if(mInputConfiguration->GetButtonsInternal(event.ButtonCode, mActiveModifiers, tempButtons, tempBtnDescs))
	{
		while(event.DeviceIndex >= (u32)mDevices.size())
			mDevices.push_back(DeviceData());

		Map<u32, ButtonData>& cachedStates = mDevices[event.DeviceIndex].CachedStates;
		TArray<u32>& heldButtons = mDevices[event.DeviceIndex].HeldButtons;

		u32 buttonCount = (u32)tempButtons.size();
		for(u32 buttonIndex = 0; buttonIndex < buttonCount; buttonIndex++)
		{
			const VirtualButton& button = tempButtons[buttonIndex];
			const VirtualButtonInformation& buttonDescription = tempBtnDescs[buttonIndex];

			ButtonData& data = cachedStates[button.ButtonIdentifier];

			data.Button = button;
			data.State = ButtonState::ToggledOn;
			data.Timestamp = event.Timestamp;
			data.UpdateFrameIdx = GetTime().GetCurrentFrameIndex();
			data.AllowRepeat = buttonDescription.Repeatable;

			VirtualButtonEvent virtualEvent;
			virtualEvent.Button = button;
			virtualEvent.State = ButtonState::On;
			virtualEvent.DeviceIdx = event.DeviceIndex;

			mEvents.push(virtualEvent);
			heldButtons.Add(button.ButtonIdentifier);
		}
	}
}

void VirtualInput::ButtonUp(const ButtonEvent& event)
{
	if(event.ButtonCode == ButtonCode::LeftShift || event.ButtonCode == ButtonCode::RightShift)
		mActiveModifiers &= ~(u32)ButtonModifier::Shift;
	else if(event.ButtonCode == ButtonCode::LeftControl || event.ButtonCode == ButtonCode::RightControl)
		mActiveModifiers &= ~(u32)ButtonModifier::Ctrl;
	else if(event.ButtonCode == ButtonCode::LeftAlt || event.ButtonCode == ButtonCode::RightAlt)
		mActiveModifiers &= ~(u32)ButtonModifier::Alt;

	tempButtons.clear();
	tempBtnDescs.clear();

	if(mInputConfiguration->GetButtonsInternal(event.ButtonCode, mActiveModifiers, tempButtons, tempBtnDescs))
	{
		while(event.DeviceIndex >= (u32)mDevices.size())
			mDevices.push_back(DeviceData());

		Map<u32, ButtonData>& cachedStates = mDevices[event.DeviceIndex].CachedStates;
		TArray<u32>& heldButtons = mDevices[event.DeviceIndex].HeldButtons;

		u32 buttonCount = (u32)tempButtons.size();
		for(u32 buttonIndex = 0; buttonIndex < buttonCount; buttonIndex++)
		{
			const VirtualButton& button = tempButtons[buttonIndex];
			const VirtualButtonInformation& buttonDescription = tempBtnDescs[buttonIndex];

			ButtonData& data = cachedStates[button.ButtonIdentifier];

			data.Button = button;
			data.State = ButtonState::ToggledOff;
			data.Timestamp = event.Timestamp;
			data.UpdateFrameIdx = GetTime().GetCurrentFrameIndex();
			data.AllowRepeat = buttonDescription.Repeatable;

			VirtualButtonEvent virtualEvent;
			virtualEvent.Button = button;
			virtualEvent.State = ButtonState::Off;
			virtualEvent.DeviceIdx = event.DeviceIndex;

			mEvents.push(virtualEvent);

			auto found = std::find(heldButtons.begin(), heldButtons.end(), button.ButtonIdentifier);
			if(found != heldButtons.end())
				heldButtons.SwapAndErase(found);
		}
	}
}

namespace b3d
{
VirtualInput& GetVirtualInput()
{
	return VirtualInput::Instance();
}
} // namespace b3d
