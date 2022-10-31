//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Platform/BsCursor.h"
#include "Platform/BsPlatform.h"
#include "Error/BsException.h"
#include "Resources/BsBuiltinResources.h"
#include "Debug/BsDebug.h"

using namespace bs;

Cursor::Cursor()
{
	for(u32 i = 0; i < (u32)CursorType::Count; i++)
		RestoreCursorIcon((CursorType)i);
}

void Cursor::SetScreenPosition(const Vector2I& screenPos)
{
	Platform::SetCursorPosition(screenPos);
}

Vector2I Cursor::GetScreenPosition()
{
	return Platform::GetCursorPosition();
}

void Cursor::Hide()
{
	Platform::HideCursor();
}

void Cursor::Show()
{
	Platform::ShowCursor();
}

void Cursor::ClipToWindow(const RenderWindow& window)
{
	Platform::ClipCursorToWindow(window);
}

void Cursor::ClipToRect(const Rect2I& screenRect)
{
	Platform::ClipCursorToRect(screenRect);
}

void Cursor::ClipDisable()
{
	Platform::ClipCursorDisable();
}

void Cursor::SetCursor(CursorType type)
{
	u32 id = (u32)type;
	if((u32)mActiveCursorId != id)
	{
		mActiveCursorId = id;
		UpdateCursorImage();
	}
}

void Cursor::SetCursor(const String& name)
{
	auto iterFind = mCustomIconNameToId.find(name);
	if(iterFind == mCustomIconNameToId.end())
	{
		BS_LOG(Warning, Platform, "Cannot find cursor icon with name: " + name);
		return;
	}

	u32 id = iterFind->second;
	if((u32)mActiveCursorId != id)
	{
		mActiveCursorId = id;
		UpdateCursorImage();
	}
}

void Cursor::SetCursorIcon(const String& name, const PixelData& pixelData, const Vector2I& hotSpot)
{
	auto iterFind = mCustomIconNameToId.find(name);
	if(iterFind != mCustomIconNameToId.end())
	{
		u32 id = iterFind->second;
		mCustomIcons[id] = CustomIcon(pixelData, hotSpot);

		if((u32)mActiveCursorId == id)
			UpdateCursorImage(); // Refresh active
	}
	else
	{
		u32 id = mNextUniqueId++;
		mCustomIconNameToId[name] = id;
		mCustomIcons[id] = CustomIcon(pixelData, hotSpot);
	}
}

void Cursor::SetCursorIcon(CursorType type, const PixelData& pixelData, const Vector2I& hotSpot)
{
	u32 id = (u32)type;

	mCustomIcons[id].PixelData = pixelData;
	mCustomIcons[id].HotSpot = hotSpot;

	if((u32)mActiveCursorId == id)
		UpdateCursorImage(); // Refresh active
}

void Cursor::ClearCursorIcon(const String& name)
{
	auto iterFind = mCustomIconNameToId.find(name);
	if(iterFind == mCustomIconNameToId.end())
		return;

	mCustomIcons.erase(iterFind->second);
	mCustomIconNameToId.erase(iterFind);
}

void Cursor::ClearCursorIcon(CursorType type)
{
	RestoreCursorIcon(type);

	if(mActiveCursorId == (i32)type)
		UpdateCursorImage(); // Refresh active
}

void Cursor::RestoreCursorIcon(CursorType type)
{
	u32 id = (u32)type;
	mCustomIcons[id] = CustomIcon();

	switch(type)
	{
	case CursorType::Arrow:
		mCustomIcons[id].PixelData = BuiltinResources::Instance().GetCursorArrow(mCustomIcons[id].HotSpot);
		return;
	case CursorType::Wait:
		mCustomIcons[id].PixelData = BuiltinResources::Instance().GetCursorWait(mCustomIcons[id].HotSpot);
		return;
	case CursorType::IBeam:
		mCustomIcons[id].PixelData = BuiltinResources::Instance().GetCursorIBeam(mCustomIcons[id].HotSpot);
		return;
	case CursorType::ArrowDrag:
		mCustomIcons[id].PixelData = BuiltinResources::Instance().GetCursorArrowDrag(mCustomIcons[id].HotSpot);
		return;
	case CursorType::SizeNESW:
		mCustomIcons[id].PixelData = BuiltinResources::Instance().GetCursorSizeNesw(mCustomIcons[id].HotSpot);
		return;
	case CursorType::SizeNS:
		mCustomIcons[id].PixelData = BuiltinResources::Instance().GetCursorSizeNs(mCustomIcons[id].HotSpot);
		return;
	case CursorType::SizeNWSE:
		mCustomIcons[id].PixelData = BuiltinResources::Instance().GetCursorSizeNwse(mCustomIcons[id].HotSpot);
		return;
	case CursorType::SizeWE:
		mCustomIcons[id].PixelData = BuiltinResources::Instance().GetCursorSizeWe(mCustomIcons[id].HotSpot);
		return;
	case CursorType::Deny:
		mCustomIcons[id].PixelData = BuiltinResources::Instance().GetCursorDeny(mCustomIcons[id].HotSpot);
		return;
	case CursorType::ArrowLeftRight:
		mCustomIcons[id].PixelData = BuiltinResources::Instance().GetCursorMoveLeftRight(mCustomIcons[id].HotSpot);
		return;
	default:
		break;
	}

	BS_EXCEPT(InvalidParametersException, "Invalid cursor type: " + toString((u32)type));
}

void Cursor::UpdateCursorImage()
{
	if(mActiveCursorId < 0)
		return;

	CustomIcon& customIcon = mCustomIcons[mActiveCursorId];
	Platform::SetCursor(customIcon.PixelData, customIcon.HotSpot);
}

Cursor& GetCursor()
{
	return static_cast<Cursor&>(Cursor::Instance());
}
