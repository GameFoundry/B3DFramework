//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIToggleGroup.h"
#include "BsGUIToggle.h"

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

	void GUIToggleGroup::initialize(const SPtr<GUIToggleGroup>& sharedPtr)
	{
		mThis = sharedPtr;
	}

	void GUIToggleGroup::_add(GUIToggle* toggle)
	{
		auto iterFind = std::find(begin(mButtons), end(mButtons), toggle);
		if(iterFind != end(mButtons))
			return;

		mButtons.push_back(toggle);
		toggle->_setToggleGroup(mThis.lock());
	}

	void GUIToggleGroup::_remove(GUIToggle* toggle)
	{
		auto sharedPtr = mThis.lock(); // Make sure we keep a reference because calling _setToggleGroup(nullptr) 
		                               // may otherwise clear the last reference and cause us to destruct

		auto iterFind = std::find(begin(mButtons), end(mButtons), toggle);
		if(iterFind == end(mButtons))
			return;

		(*iterFind)->_setToggleGroup(nullptr);
		mButtons.erase(iterFind);
	}
}