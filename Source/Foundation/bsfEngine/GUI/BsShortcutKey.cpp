//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsShortcutKey.h"
#include "Platform/BsPlatform.h"
#include "Localization/BsHEString.h"
#include "Localization/BsHString.h"

namespace bs
{
	const ShortcutKey ShortcutKey::NONE = ShortcutKey();

	size_t ShortcutKey::Hash::operator()(const ShortcutKey& x) const
	{
		size_t seed = 0;
		hash_combine(seed, (UINT32)x.button);
		hash_combine(seed, (UINT32)x.modifier);

		return seed;
	}

	bool ShortcutKey::Equals::operator()(const ShortcutKey& a, const ShortcutKey& b) const
	{
		return a.button == b.button && a.modifier == b.modifier;
	}

	ShortcutKey::ShortcutKey()
		:modifier(ButtonModifier::None), button(BC_UNASSIGNED)
	{ }

	ShortcutKey::ShortcutKey(ButtonModifier modifier, ButtonCode code)
		: modifier(modifier), button(code)
	{ }

	String ShortcutKey::getName() const
	{
		static const String MODIFIER_TO_NAME[8] =
		{
			u8"", u8"Shift + ", u8"Ctrl + ", u8"Shift + Ctrl + ",
			u8"Alt + ", u8"Shift + Alt + ", u8"Ctrl + Alt + ",
			u8"Shift + Ctrl + Alt + "
		};

		static const UnorderedMap<ButtonCode, HEString> FUNCTION_KEY_TO_NAME =
		{
			{BC_ESCAPE, HEString(u8"Escape")},
			{BC_BACK, HEString(u8"Backspace")},
			{BC_TAB, HEString(u8"Tab")},
			{BC_RETURN, HEString(u8"Return")},
			{BC_SPACE, HEString(u8"Space")},
			{BC_CAPITAL, HEString(u8"Caps Lock")},
			{BC_F1, HEString(u8"F1")},
			{BC_F2, HEString(u8"F2")},
			{BC_F3, HEString(u8"F3")},
			{BC_F4, HEString(u8"F4")},
			{BC_F5, HEString(u8"F5")},
			{BC_F6, HEString(u8"F6")},
			{BC_F7, HEString(u8"F7")},
			{BC_F8, HEString(u8"F8")},
			{BC_F9, HEString(u8"F9")},
			{BC_F10, HEString(u8"F10")},
			{BC_F11, HEString(u8"F11")},
			{BC_F12, HEString(u8"F12")},
			{BC_DELETE, HEString(u8"Delete")},
			{BC_INSERT, HEString(u8"Insert")},
			{BC_UP, HEString(u8"Up")},
			{BC_LEFT, HEString(u8"Left")},
			{BC_RIGHT, HEString(u8"Right")},
			{BC_DOWN, HEString(u8"Down")},
			{BC_PGUP, HEString(u8"Page Up")},
			{BC_PGDOWN, HEString(u8"Page Down")},
			{BC_END, HEString(u8"End")},
			{BC_HOME, HEString(u8"Home")},
			{BC_PAUSE, HEString(u8"Pause")},
		};

		if (button == BC_UNASSIGNED)
			return u8"";

		String charStr;

		auto iterFind = FUNCTION_KEY_TO_NAME.find(button);
		if (iterFind != FUNCTION_KEY_TO_NAME.end())
		{
			charStr = ((HString)iterFind->second);
		}
		else
		{
			charStr = Platform::keyCodeToUnicode((UINT32)button);
			StringUtil::toUpperCase(charStr);
		}

		return MODIFIER_TO_NAME[(UINT32)modifier] + charStr;
	}
}