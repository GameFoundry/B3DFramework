//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsDropDownWindowManager.h"
#include "BsDropDownWindow.h"

namespace bs
{
	DropDownWindowManager::DropDownWindowManager()
		:mOpenWindow(nullptr)
	{ }

	DropDownWindowManager::~DropDownWindowManager()
	{
		close();
	}

	void DropDownWindowManager::close()
	{
		if (mOpenWindow != nullptr)
		{
			bs_delete(mOpenWindow);
			mOpenWindow = nullptr;
		}
	}

	void DropDownWindowManager::update()
	{
		if (mOpenWindow != nullptr)
			mOpenWindow->update();
	}
}