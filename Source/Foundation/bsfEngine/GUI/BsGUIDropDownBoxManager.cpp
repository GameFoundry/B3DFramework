//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIDropDownBoxManager.h"
#include "Scene/BsSceneObject.h"

namespace bs
{
	GUIDropDownBoxManager::~GUIDropDownBoxManager()
	{
		closeDropDownBox();
	}

	GameObjectHandle<GUIDropDownMenu> GUIDropDownBoxManager::openDropDownBox(const DROP_DOWN_BOX_DESC& desc,
		GUIDropDownType type, std::function<void()> onClosedCallback)
	{
		closeDropDownBox();

		mDropDownSO = SceneObject::create("DropDownBox", SOF_Internal | SOF_Persistent | SOF_DontSave);
		mDropDownBox = mDropDownSO->addComponent<GUIDropDownMenu>(desc, type);
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
}
