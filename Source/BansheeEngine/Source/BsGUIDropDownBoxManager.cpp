//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIDropDownBoxManager.h"
#include "BsSceneObject.h"

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

	void GUIDropDownBoxManager::closeDropDownBox()
	{
		if(mDropDownSO != nullptr)
		{
			mDropDownSO->destroy();
			mDropDownSO = nullptr;

			if(mOnClosedCallback != nullptr)
				mOnClosedCallback();

			mOnClosedCallback = nullptr;
		}
	}
}