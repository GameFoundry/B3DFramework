//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsShortcutKey.h"
#include "BsPlatform.h"
#include "BsHEString.h"
#include "BsHString.h"

namespace bs
{
	const ShortcutKey ShortcutKey::NONE = ShortcutKey();

	inline size_t ShortcutKey::Hash::operator()(const ShortcutKey& x) const
	{
		size_t seed = 0;
		hash_combine(seed, (UINT32)x.button);
		hash_combine(seed, (UINT32)x.modifier);

		return seed;
	}

	inline bool ShortcutKey::Equals::operator()(const ShortcutKey& a, const ShortcutKey& b) const
	{
		return a.button == b.button && a.modifier == b.modifier;
	}

	ShortcutKey::ShortcutKey()
		:modifier(ButtonModifier::None), button(BC_UNASSIGNED)
	{ }

	ShortcutKey::ShortcutKey(ButtonModifier modifier, ButtonCode code)
		: modifier(modifier), button(code)
	{ }

	WString ShortcutKey::getName() const
	{
		static const WString MODIFIER_TO_NAME[8] =
		{
			L"", L"Shift + ", L"Ctrl + ", L"Shift + Ctrl + ",
			L"Alt + ", L"Shift + Alt + ", L"Ctrl + Alt + ",
			L"Shift + Ctrl + Alt + "
		};

		static const UnorderedMap<ButtonCode, HEString> FUNCTION_KEY_TO_NAME =
		{
			{BC_ESCAPE, HEString(L"Escape")},
			{BC_BACK, HEString(L"Backspace")},
			{BC_TAB, HEString(L"Tab")},
			{BC_RETURN, HEString(L"Return")},
			{BC_SPACE, HEString(L"Space")},
			{BC_CAPITAL, HEString(L"Caps Lock")},
			{BC_F1, HEString(L"F1")},
			{BC_F2, HEString(L"F2")},
			{BC_F3, HEString(L"F3")},
			{BC_F4, HEString(L"F4")},
			{BC_F5, HEString(L"F5")},
			{BC_F6, HEString(L"F6")},
			{BC_F7, HEString(L"F7")},
			{BC_F8, HEString(L"F8")},
			{BC_F9, HEString(L"F9")},
			{BC_F10, HEString(L"F10")},
			{BC_F11, HEString(L"F11")},
			{BC_F12, HEString(L"F12")},
			{BC_DELETE, HEString(L"Delete")},
			{BC_INSERT, HEString(L"Insert")},
			{BC_UP, HEString(L"Up")},
			{BC_LEFT, HEString(L"Left")},
			{BC_RIGHT, HEString(L"Right")},
			{BC_DOWN, HEString(L"Down")},
			{BC_PGUP, HEString(L"Page Up")},
			{BC_PGDOWN, HEString(L"Page Down")},
			{BC_END, HEString(L"End")},
			{BC_HOME, HEString(L"Home")},
			{BC_PAUSE, HEString(L"Pause")},
		};

		if (button == BC_UNASSIGNED)
			return L"";

		WString charStr;

		auto iterFind = FUNCTION_KEY_TO_NAME.find(button);
		if (iterFind != FUNCTION_KEY_TO_NAME.end())
		{
			charStr = ((HString)iterFind->second);
		}
		else
		{
			charStr = PlatformUtility::keyCodeToUnicode((UINT32)button);
			StringUtil::toUpperCase(charStr);
		}

		return MODIFIER_TO_NAME[(UINT32)modifier] + charStr;
	}
}