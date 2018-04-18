//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUITooltipManager.h"
#include "Scene/BsSceneObject.h"
#include "GUI/BsGUITooltip.h"

namespace bs
{
	GUITooltipManager::~GUITooltipManager()
	{
		hide();
	}

	void GUITooltipManager::show(const GUIWidget& widget, const Vector2I& position, const String& text)
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