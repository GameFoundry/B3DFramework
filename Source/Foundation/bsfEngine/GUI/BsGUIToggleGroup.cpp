//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIToggleGroup.h"
#include "GUI/BsGUIToggle.h"

namespace bs
{
	GUIToggleGroup::GUIToggleGroup(bool allowAllOff)
		:mAllowAllOff(allowAllOff)
	{ }

	GUIToggleGroup::~GUIToggleGroup()
	{
		for(auto& button : mButtons)
		{
			button->SetToggleGroupInternal(nullptr);
		}
	}

	void GUIToggleGroup::initialize(const SPtr<GUIToggleGroup>& sharedPtr)
	{
		mThis = sharedPtr;
	}

	void GUIToggleGroup::AddInternal(GUIToggle* toggle)
	{
		auto iterFind = std::find(begin(mButtons), end(mButtons), toggle);
		if(iterFind != end(mButtons))
			return;

		mButtons.push_back(toggle);
		toggle->SetToggleGroupInternal(mThis.lock());
	}

	void GUIToggleGroup::RemoveInternal(GUIToggle* toggle)
	{
		auto sharedPtr = mThis.lock(); // Make sure we keep a reference because calling SetToggleGroupInternal(nullptr)
		                               // may otherwise clear the last reference and cause us to destruct

		auto iterFind = std::find(begin(mButtons), end(mButtons), toggle);
		if(iterFind == end(mButtons))
			return;

		(*iterFind)->SetToggleGroupInternal(nullptr);
		mButtons.erase(iterFind);
	}
}
