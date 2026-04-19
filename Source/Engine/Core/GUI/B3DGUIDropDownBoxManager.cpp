//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIDropDownBoxManager.h"
#include "Scene/B3DSceneObject.h"

using namespace b3d;

GUIDropDownBoxManager::~GUIDropDownBoxManager()
{
	CloseDropDownBox();
}

TGameObjectHandle<GUIDropDownMenu> GUIDropDownBoxManager::OpenDropDownBox(const DropDownBoxCreateInformation& createInformation, GUIDropDownType type, std::function<void()> onClosedCallback)
{
	CloseDropDownBox();

	mDropDownSO = SceneObject::Create("DropDownBox", SceneObjectFlag::Internal | SceneObjectFlag::RuntimePersistent);
	mDropDownBox = mDropDownSO->AddComponent<GUIDropDownMenu>(createInformation, type);
	mOnClosedCallback = onClosedCallback;

	return mDropDownBox;
}

void GUIDropDownBoxManager::CloseDropDownBox()
{
	if(mDropDownSO != nullptr)
	{
		mDropDownSO->Destroy();
		mDropDownSO = nullptr;

		if(mOnClosedCallback != nullptr)
			mOnClosedCallback();

		mOnClosedCallback = nullptr;
	}
}
