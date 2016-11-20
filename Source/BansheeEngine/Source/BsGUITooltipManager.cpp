//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUITooltipManager.h"
#include "BsSceneObject.h"
#include "BsGUITooltip.h"

namespace bs
{
	GUITooltipManager::~GUITooltipManager()
	{
		hide();
	}

	void GUITooltipManager::show(const GUIWidget& widget, const Vector2I& position, const WString& text)
	{
		hide();

		mTooltipSO = SceneObject::create("Tooltip", SOF_Internal | SOF_Persistent | SOF_DontSave);
		GameObjectHandle<GUITooltip> tooltip = mTooltipSO->addComponent<GUITooltip>(widget, position, text);
	}

	void GUITooltipManager::hide()
	{
		if (mTooltipSO != nullptr)
		{
			mTooltipSO->destroy();
			mTooltipSO = nullptr;
		}
	}
}