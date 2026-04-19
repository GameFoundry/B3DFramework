//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DShortcutManager.h"
#include "Input/B3DInput.h"

using namespace b3d;

ShortcutManager::ShortcutManager()
{
	mOnButtonDownConn = Input::Instance().OnButtonDown.Connect([this](const ButtonEvent& event) { OnButtonDown(event); });
}

ShortcutManager::~ShortcutManager()
{
	mOnButtonDownConn.Disconnect();
}

void ShortcutManager::AddShortcut(const ShortcutKey& key, std::function<void()> callback)
{
	mShortcuts[key] = callback;
}

void ShortcutManager::RemoveShortcut(const ShortcutKey& key)
{
	mShortcuts.erase(key);
}

void ShortcutManager::OnButtonDown(const ButtonEvent& event)
{
	u32 modifiers = 0;
	if(Input::Instance().IsButtonHeld(ButtonCode::LeftShift) || Input::Instance().IsButtonHeld(ButtonCode::RightShift))
		modifiers |= (u32)ButtonModifier::Shift;

	if(Input::Instance().IsButtonHeld(ButtonCode::LeftControl) || Input::Instance().IsButtonHeld(ButtonCode::RightControl))
		modifiers |= (u32)ButtonModifier::Ctrl;

	if(Input::Instance().IsButtonHeld(ButtonCode::LeftAlt) || Input::Instance().IsButtonHeld(ButtonCode::RightAlt))
		modifiers |= (u32)ButtonModifier::Alt;

	ShortcutKey searchKey((ButtonModifier)modifiers, event.ButtonCode);

	auto iterFind = mShortcuts.find(searchKey);
	if(iterFind != mShortcuts.end())
	{
		if(iterFind->second != nullptr)
			iterFind->second();
	}
}
