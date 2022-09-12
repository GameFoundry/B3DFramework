//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/BsInputConfiguration.h"

namespace bs
{
	
	UINT32 VirtualButton::NextButtonId = 0;

	Map<String, UINT32> VirtualAxis::UniqueAxisIds;
	UINT32 VirtualAxis::NextAxisId = 0;

	VIRTUAL_BUTTON_DESC::VIRTUAL_BUTTON_DESC(ButtonCode buttonCode, ButtonModifier modifiers, bool repeatable)
		:buttonCode(buttonCode), modifiers(modifiers), repeatable(repeatable)
	{ }

	VIRTUAL_AXIS_DESC::VIRTUAL_AXIS_DESC(UINT32 type)
		:type(type)
	{ }

	VirtualButton::VirtualButton(const String& name)
	{
		Map<String, UINT32>& uniqueButtonIds = getUniqueButtonIds();

		auto findIter = uniqueButtonIds.find(name);

		if(findIter != uniqueButtonIds.end())
			buttonIdentifier = findIter->second;
		else
		{
			buttonIdentifier = NextButtonId;
			uniqueButtonIds[name] = NextButtonId++;
		}
	}

	Map<String, UINT32>& VirtualButton::GetUniqueButtonIds()
	{
		static Map<String, UINT32> uniqueButtonIds;
		return uniqueButtonIds;
	}

	VirtualAxis::VirtualAxis(const String& name)
	{
		auto findIter = UniqueAxisIds.find(name);

		if (findIter != UniqueAxisIds.end())
			axisIdentifier = findIter->second;
		else
		{
			axisIdentifier = NextAxisId;
			UniqueAxisIds[name] = NextAxisId++;
		}
	}

	void InputConfiguration::RegisterButton(const String& name, ButtonCode buttonCode, ButtonModifier modifiers, bool repeatable)
	{
		Vector<VirtualButtonData>& btnData = mButtons[buttonCode & 0x0000FFFF];

		INT32 idx = -1;
		for(UINT32 i = 0; i < (UINT32)btnData.size(); i++)
		{
			if (btnData[i].name == name)
			{
				idx = (INT32)i;
				break;
			}
		}

		if(idx == -1)
		{
			idx = (INT32)btnData.size();
			btnData.push_back(VirtualButtonData());
		}

		VirtualButtonData& btn = btnData[idx];
		btn.name = name;
		btn.desc = VIRTUAL_BUTTON_DESC(buttonCode, modifiers, repeatable);
		btn.button = VirtualButton(name);
	}

	void InputConfiguration::UnregisterButton(const String& name)
	{
		Vector<UINT32> toRemove;

		for(UINT32 i = 0; i < BC_Count; i++)
		{
			for(UINT32 j = 0; j < (UINT32)mButtons[i].size(); j++)
			{
				if(mButtons[i][j].name == name)
					toRemove.push_back(j);
			}

			UINT32 numRemoved = 0;
			for(auto& toRemoveIdx : toRemove)
			{
				mButtons[i].erase(mButtons[i].begin() + toRemoveIdx - numRemoved);

				numRemoved++;
			}

			toRemove.Clear();
		}
	}

	void InputConfiguration::RegisterAxis(const String& name, const VIRTUAL_AXIS_DESC& desc)
	{
		VirtualAxis Axis(name);

		if (axis.axisIdentifier >= (UINT32)mAxes.size())
			mAxes.Resize(axis.axisIdentifier + 1);

		mAxes[axis.axisIdentifier].name = name;
		mAxes[axis.axisIdentifier].desc = desc;
		mAxes[axis.axisIdentifier].axis = axis;
	}

	void InputConfiguration::UnregisterAxis(const String& name)
	{
		for (UINT32 i = 0; i < (UINT32)mAxes.size(); i++)
		{
			if (mAxes[i].name == name)
			{
				mAxes.erase(mAxes.begin() + i);
				i--;
			}
		}
	}

	bool InputConfiguration::_getButtons(ButtonCode code, UINT32 modifiers, Vector<VirtualButton>& btns, Vector<VIRTUAL_BUTTON_DESC>& btnDesc) const
	{
		const Vector<VirtualButtonData>& btnData = mButtons[code & 0x0000FFFF];

		bool foundAny = false;
		for(UINT32 i = 0; i < (UINT32)btnData.size(); i++)
		{
			if((((UINT32)btnData[i].desc.modifiers) & modifiers) == ((UINT32)btnData[i].desc.modifiers))
			{
				btns.push_back(btnData[i].button);
				btnDesc.push_back(btnData[i].desc);
				foundAny = true;
			}
		}

		return foundAny;
	}

	bool InputConfiguration::_getAxis(const VirtualAxis& axis, VIRTUAL_AXIS_DESC& axisDesc) const
	{
		if (axis.axisIdentifier >= (UINT32)mAxes.size())
			return false;

		axisDesc = mAxes[axis.axisIdentifier].desc;
		return true;
	}
}
