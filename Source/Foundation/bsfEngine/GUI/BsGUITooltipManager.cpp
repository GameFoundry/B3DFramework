//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUITooltipManager.h"
#include "Scene/BsSceneObject.h"
#include "GUI/BsGUITooltip.h"

using namespace bs;

GUITooltipManager::~GUITooltipManager()
{
	Hide();
}

void GUITooltipManager::Show(const GUIWidget& widget, const Vector2I& position, const String& text)
{
	Hide();

	mTooltipSO = SceneObject::Create("Tooltip", SOF_Internal | SOF_Persistent | SOF_DontSave);
	GameObjectHandle<GUITooltip> tooltip = mTooltipSO->AddComponent<GUITooltip>(widget, position, text);
}

void GUITooltipManager::Hide()
{
	if(mTooltipSO != nullptr)
	{
		mTooltipSO->Destroy();
		mTooltipSO = nullptr;
	}
}
