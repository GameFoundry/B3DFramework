//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DShortcutKey.h"
#include "Platform/B3DPlatform.h"
#include "Localization/B3DHEString.h"
#include "Localization/B3DHString.h"

using namespace b3d;

const ShortcutKey ShortcutKey::kNone = ShortcutKey();

size_t ShortcutKey::Hash::operator()(const ShortcutKey& x) const
{
	size_t seed = 0;
	B3DCombineHash(seed, (u32)x.Button);
	B3DCombineHash(seed, (u32)x.Modifier);

	return seed;
}

bool ShortcutKey::Equals::operator()(const ShortcutKey& a, const ShortcutKey& b) const
{
	return a.Button == b.Button && a.Modifier == b.Modifier;
}

ShortcutKey::ShortcutKey(ButtonModifier modifier, ButtonCode code)
	: Modifier(modifier), Button(code)
{}

String ShortcutKey::GetName() const
{
	static const String kModifierToName[8] = {
		u8"", u8"Shift + ", u8"Ctrl + ", u8"Shift + Ctrl + ",
		u8"Alt + ", u8"Shift + Alt + ", u8"Ctrl + Alt + ",
		u8"Shift + Ctrl + Alt + "
	};

	static const UnorderedMap<ButtonCode, HEString> kFunctionKeyToName = {
		{ ButtonCode::Escape, HEString(u8"Escape") },
		{ ButtonCode::Backspace, HEString(u8"Backspace") },
		{ ButtonCode::Tab, HEString(u8"Tab") },
		{ ButtonCode::Enter, HEString(u8"Return") },
		{ ButtonCode::Space, HEString(u8"Space") },
		{ ButtonCode::CapsLock, HEString(u8"Caps Lock") },
		{ ButtonCode::F1, HEString(u8"F1") },
		{ ButtonCode::F2, HEString(u8"F2") },
		{ ButtonCode::F3, HEString(u8"F3") },
		{ ButtonCode::F4, HEString(u8"F4") },
		{ ButtonCode::F5, HEString(u8"F5") },
		{ ButtonCode::F6, HEString(u8"F6") },
		{ ButtonCode::F7, HEString(u8"F7") },
		{ ButtonCode::F8, HEString(u8"F8") },
		{ ButtonCode::F9, HEString(u8"F9") },
		{ ButtonCode::F10, HEString(u8"F10") },
		{ ButtonCode::F11, HEString(u8"F11") },
		{ ButtonCode::F12, HEString(u8"F12") },
		{ ButtonCode::Delete, HEString(u8"Delete") },
		{ ButtonCode::Insert, HEString(u8"Insert") },
		{ ButtonCode::ArrowUp, HEString(u8"Up") },
		{ ButtonCode::ArrowLeft, HEString(u8"Left") },
		{ ButtonCode::ArrowRight, HEString(u8"Right") },
		{ ButtonCode::ArrowDown, HEString(u8"Down") },
		{ ButtonCode::PageUp, HEString(u8"Page Up") },
		{ ButtonCode::PageDown, HEString(u8"Page Down") },
		{ ButtonCode::End, HEString(u8"End") },
		{ ButtonCode::Home, HEString(u8"Home") },
		{ ButtonCode::Pause, HEString(u8"Pause") },
	};

	if(Button == ButtonCode::Unassigned)
		return u8"";

	String charStr;

	auto iterFind = kFunctionKeyToName.find(Button);
	if(iterFind != kFunctionKeyToName.end())
	{
		charStr = ((String)(HString)iterFind->second);
	}
	else
	{
		charStr = Platform::KeyCodeToUnicode((u32)Button);
		StringUtility::ToUpperCase(charStr);
	}

	return kModifierToName[(u32)Modifier] + charStr;
}
