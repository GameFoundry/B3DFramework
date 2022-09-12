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
			button->_setToggleGroup(nullptr);
		}
	}

	void GUIToggleGroup::Initialize(const SPtr<GUIToggleGroup>& sharedPtr)
	{
		mThis = sharedPtr;
	}

	void GUIToggleGroup::_add(GUIToggle* toggle)
	{
		auto iterFind = std::find(begin(mButtons), end(mButtons), toggle);
		if(iterFind != end(mButtons))
			return;

		mButtons.push_back(toggle);
		toggle->_setToggleGroup(mThis.Lock());
	}

	void GUIToggleGroup::_remove(GUIToggle* toggle)
	{
		auto sharedPtr = mThis.Lock(); // Make sure we keep a reference because calling _setToggleGroup(nullptr)
		                               // may otherwise clear the last reference and cause us to destruct

		auto iterFind = std::find(begin(mButtons), end(mButtons), toggle);
		if(iterFind == end(mButtons))
			return;

		(*iterFind)->_setToggleGroup(nullptr);
		mButtons.Erase(iterFind);
	}
}
