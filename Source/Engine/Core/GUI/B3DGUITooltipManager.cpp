//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUITooltipManager.h"
#include "Scene/B3DSceneObject.h"
#include "GUI/B3DGUITooltip.h"

using namespace b3d;

GUITooltipManager::~GUITooltipManager()
{
	Hide();
}

void GUITooltipManager::Show(const GUIWidget& widget, const GUIPhysicalPoint& position, const String& text)
{
	Hide();

	mTooltipSO = SceneObject::Create("Tooltip", SceneObjectFlag::Internal | SceneObjectFlag::RuntimePersistent);
	TGameObjectHandle<GUITooltip> tooltip = mTooltipSO->AddComponent<GUITooltip>(widget, position, text);
}

void GUITooltipManager::Hide()
{
	if(mTooltipSO != nullptr)
	{
		mTooltipSO->Destroy();
		mTooltipSO = nullptr;
	}
}
