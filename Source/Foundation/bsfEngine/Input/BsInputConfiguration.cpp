//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/BsInputConfiguration.h"

#include "BsVirtualInput.h"

using namespace bs;

VirtualButtonInformation::VirtualButtonInformation(bs::ButtonCode buttonCode, ButtonModifier modifiers, bool repeatable)
	: ButtonCode(buttonCode), Modifiers(modifiers), Repeatable(repeatable)
{}

VirtualAxisInformation::VirtualAxisInformation(u32 type)
	: Type(type)
{}

void InputConfiguration::RegisterButton(const String& name, ButtonCode buttonCode, ButtonModifier modifiers, bool repeatable)
{
	Vector<VirtualButtonData>& btnData = mButtons[(u32)buttonCode & 0x0000FFFF];

	i32 idx = -1;
	for(u32 i = 0; i < (u32)btnData.size(); i++)
	{
		if(btnData[i].Name == name)
		{
			idx = (i32)i;
			break;
		}
	}

	if(idx == -1)
	{
		idx = (i32)btnData.size();
		btnData.push_back(VirtualButtonData());
	}

	VirtualButtonData& btn = btnData[idx];
	btn.Name = name;
	btn.Desc = VirtualButtonInformation(buttonCode, modifiers, repeatable);
	btn.Button = VirtualInput::GetOrCreateVirtualButton(name);
}

void InputConfiguration::UnregisterButton(const String& name)
{
	Vector<u32> toRemove;

	for(u32 i = 0; i < static_cast<unsigned>(ButtonCode::TotalKeyCount); i++)
	{
		for(u32 j = 0; j < (u32)mButtons[i].size(); j++)
		{
			if(mButtons[i][j].Name == name)
				toRemove.push_back(j);
		}

		u32 numRemoved = 0;
		for(auto& toRemoveIdx : toRemove)
		{
			mButtons[i].erase(mButtons[i].begin() + toRemoveIdx - numRemoved);

			numRemoved++;
		}

		toRemove.clear();
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
	for(u32 i = 0; i < (u32)mAxes.size(); i++)
	{
		if(mAxes[i].Name == name)
		{
			mAxes.erase(mAxes.begin() + i);
			i--;
		}
	}
}

bool InputConfiguration::GetButtonsInternal(ButtonCode code, u32 modifiers, Vector<VirtualButton>& btns, Vector<VirtualButtonInformation>& btnDesc) const
{
	const Vector<VirtualButtonData>& btnData = mButtons[(u32)code & 0x0000FFFF];

	bool foundAny = false;
	for(u32 i = 0; i < (u32)btnData.size(); i++)
	{
		if((((u32)btnData[i].Desc.Modifiers) & modifiers) == ((u32)btnData[i].Desc.Modifiers))
		{
			btns.push_back(btnData[i].Button);
			btnDesc.push_back(btnData[i].Desc);
			foundAny = true;
		}
	}

	return foundAny;
}

bool InputConfiguration::GetAxisInternal(const VirtualAxis& axis, VirtualAxisInformation& axisDesc) const
{
	if(axis.AxisIdentifier >= (u32)mAxes.size())
		return false;

	axisDesc = mAxes[axis.AxisIdentifier].Desc;
	return true;
}
