//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsShortcutManager.h"
#include "BsInput.h"

using namespace std::placeholders;

namespace bs
{
	ShortcutManager::ShortcutManager()
	{
		mOnButtonDownConn = Input::instance().onButtonDown.connect(std::bind(&ShortcutManager::onButtonDown, this, _1));
	}

	ShortcutManager::~ShortcutManager()
	{
		mOnButtonDownConn.disconnect();
	}

	void ShortcutManager::addShortcut(const ShortcutKey& key, std::function<void()> callback)
	{
		mShortcuts[key] = callback;
	}

	void ShortcutManager::removeShortcut(const ShortcutKey& key)
	{
		mShortcuts.erase(key);
	}

	void ShortcutManager::onButtonDown(const ButtonEvent& event)
	{
		UINT32 modifiers = 0;
		if (Input::instance().isButtonHeld(BC_LSHIFT) || Input::instance().isButtonHeld(BC_RSHIFT))
			modifiers |= (UINT32)ButtonModifier::Shift;

		if (Input::instance().isButtonHeld(BC_LCONTROL) || Input::instance().isButtonHeld(BC_RCONTROL))
			modifiers |= (UINT32)ButtonModifier::Ctrl;

		if (Input::instance().isButtonHeld(BC_LMENU) || Input::instance().isButtonHeld(BC_RMENU))
			modifiers |= (UINT32)ButtonModifier::Alt;

		ShortcutKey searchKey((ButtonModifier)modifiers, event.buttonCode);

		auto iterFind = mShortcuts.find(searchKey);
		if (iterFind != mShortcuts.end())
		{
			if (iterFind->second != nullptr)
				iterFind->second();
		}
	}
}