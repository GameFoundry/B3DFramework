//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/BsInputConfiguration.h"

namespace bs
{

	u32 VirtualButton::NextButtonId = 0;

	Map<String, u32> VirtualAxis::UniqueAxisIds;
	u32 VirtualAxis::NextAxisId = 0;

	VIRTUAL_BUTTON_DESC::VIRTUAL_BUTTON_DESC(bs::ButtonCode buttonCode, ButtonModifier modifiers, bool repeatable)
		: ButtonCode(buttonCode), Modifiers(modifiers), Repeatable(repeatable)
	{}

	VIRTUAL_AXIS_DESC::VIRTUAL_AXIS_DESC(u32 type)
		: Type(type)
	{}

	VirtualButton::VirtualButton(const String& name)
	{
		Map<String, u32>& uniqueButtonIds = GetUniqueButtonIds();

		auto findIter = uniqueButtonIds.find(name);

		if(findIter != uniqueButtonIds.end())
			ButtonIdentifier = findIter->second;
		else
		{
			ButtonIdentifier = NextButtonId;
			uniqueButtonIds[name] = NextButtonId++;
		}
	}

	Map<String, u32>& VirtualButton::GetUniqueButtonIds()
	{
		static Map<String, u32> uniqueButtonIds;
		return uniqueButtonIds;
	}

	VirtualAxis::VirtualAxis(const String& name)
	{
		auto findIter = UniqueAxisIds.find(name);

		if(findIter != UniqueAxisIds.end())
			AxisIdentifier = findIter->second;
		else
		{
			AxisIdentifier = NextAxisId;
			UniqueAxisIds[name] = NextAxisId++;
		}
	}

	void InputConfiguration::RegisterButton(const String& name, ButtonCode buttonCode, ButtonModifier modifiers, bool repeatable)
	{
		Vector<VirtualButtonData>& btnData = mButtons[buttonCode & 0x0000FFFF];

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
		btn.Desc = VIRTUAL_BUTTON_DESC(buttonCode, modifiers, repeatable);
		btn.Button = VirtualButton(name);
	}

	void InputConfiguration::UnregisterButton(const String& name)
	{
		Vector<u32> toRemove;

		for(u32 i = 0; i < BC_Count; i++)
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

	void InputConfiguration::RegisterAxis(const String& name, const VIRTUAL_AXIS_DESC& desc)
	{
		VirtualAxis axis(name);

		if(axis.AxisIdentifier >= (u32)mAxes.size())
			mAxes.resize(axis.AxisIdentifier + 1);

		mAxes[axis.AxisIdentifier].Name = name;
		mAxes[axis.AxisIdentifier].Desc = desc;
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

	bool InputConfiguration::GetButtonsInternal(ButtonCode code, u32 modifiers, Vector<VirtualButton>& btns, Vector<VIRTUAL_BUTTON_DESC>& btnDesc) const
	{
		const Vector<VirtualButtonData>& btnData = mButtons[code & 0x0000FFFF];

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

	bool InputConfiguration::GetAxisInternal(const VirtualAxis& axis, VIRTUAL_AXIS_DESC& axisDesc) const
	{
		if(axis.AxisIdentifier >= (u32)mAxes.size())
			return false;

		axisDesc = mAxes[axis.AxisIdentifier].Desc;
		return true;
	}
} // namespace bs
