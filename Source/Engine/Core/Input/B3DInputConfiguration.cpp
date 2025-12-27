//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DInputConfiguration.h"

#include "B3DVirtualInput.h"

using namespace b3d;

VirtualButtonInformation::VirtualButtonInformation(b3d::ButtonCode buttonCode, ButtonModifier modifiers, bool repeatable)
	: ButtonCode(buttonCode), Modifiers(modifiers), Repeatable(repeatable)
{}

VirtualAxisInformation::VirtualAxisInformation(u32 type)
	: Type(type)
{}

void InputConfiguration::RegisterButton(const String& name, ButtonCode buttonCode, ButtonModifier modifiers, bool repeatable)
{
	Vector<VirtualButtonData>& buttonData = mButtons[(u32)buttonCode & 0x0000FFFF];

	i32 existingIndex = -1;
	for(u32 buttonIndex = 0; buttonIndex < (u32)buttonData.size(); buttonIndex++)
	{
		if(buttonData[buttonIndex].Name == name)
		{
			existingIndex = (i32)buttonIndex;
			break;
		}
	}

	if(existingIndex == -1)
	{
		existingIndex = (i32)buttonData.size();
		buttonData.push_back(VirtualButtonData());
	}

	VirtualButtonData& button = buttonData[existingIndex];
	button.Name = name;
	button.Desc = VirtualButtonInformation(buttonCode, modifiers, repeatable);
	button.Button = VirtualInput::GetOrCreateVirtualButton(name);
}

void InputConfiguration::UnregisterButton(const String& name)
{
	Vector<u32> indicesToRemove;

	for(u32 keyIndex = 0; keyIndex < static_cast<unsigned>(ButtonCode::TotalKeyCount); keyIndex++)
	{
		for(u32 buttonIndex = 0; buttonIndex < (u32)mButtons[keyIndex].size(); buttonIndex++)
		{
			if(mButtons[keyIndex][buttonIndex].Name == name)
				indicesToRemove.push_back(buttonIndex);
		}

		u32 removedCount = 0;
		for(auto& indexToRemove : indicesToRemove)
		{
			mButtons[keyIndex].erase(mButtons[keyIndex].begin() + indexToRemove - removedCount);

			removedCount++;
		}

		indicesToRemove.clear();
	}
}

void InputConfiguration::RegisterAxis(const String& name, const VirtualAxisCreateInformation& createInformation)
{
	VirtualAxis axis = VirtualInput::GetOrCreateVirtualAxis(name);

	if(axis.AxisIdentifier >= (u32)mAxes.size())
		mAxes.resize(axis.AxisIdentifier + 1);

	mAxes[axis.AxisIdentifier].Name = name;
	mAxes[axis.AxisIdentifier].Desc = createInformation;
	mAxes[axis.AxisIdentifier].Axis = axis;
}

void InputConfiguration::UnregisterAxis(const String& name)
{
	for(u32 axisIndex = 0; axisIndex < (u32)mAxes.size(); axisIndex++)
	{
		if(mAxes[axisIndex].Name == name)
		{
			mAxes.erase(mAxes.begin() + axisIndex);
			axisIndex--;
		}
	}
}

bool InputConfiguration::GetButtonsInternal(ButtonCode code, u32 modifiers, Vector<VirtualButton>& buttons, Vector<VirtualButtonInformation>& buttonDescriptions) const
{
	const Vector<VirtualButtonData>& buttonData = mButtons[(u32)code & 0x0000FFFF];

	bool foundAny = false;
	for(u32 buttonIndex = 0; buttonIndex < (u32)buttonData.size(); buttonIndex++)
	{
		if((((u32)buttonData[buttonIndex].Desc.Modifiers) & modifiers) == ((u32)buttonData[buttonIndex].Desc.Modifiers))
		{
			buttons.push_back(buttonData[buttonIndex].Button);
			buttonDescriptions.push_back(buttonData[buttonIndex].Desc);
			foundAny = true;
		}
	}

	return foundAny;
}

bool InputConfiguration::GetAxisInternal(const VirtualAxis& axis, VirtualAxisInformation& axisDescription) const
{
	if(axis.AxisIdentifier >= (u32)mAxes.size())
		return false;

	axisDescription = mAxes[axis.AxisIdentifier].Desc;
	return true;
}
