//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptHandleSliderManager.h"
#include "BsScriptHandleSlider.h"

namespace bs
{
	ScriptHandleSliderManager::~ScriptHandleSliderManager()
	{
		for (auto& slider : mSliders)
			slider->destroyInternal();
	}

	void ScriptHandleSliderManager::registerSlider(ScriptHandleSliderBase* slider)
	{
		mSliders.insert(slider);
	}

	void ScriptHandleSliderManager::unregisterSlider(ScriptHandleSliderBase* slider)
	{
		mSliders.erase(slider);
	}
}