//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Platform/B3DCursor.h"
#include "Platform/B3DPlatform.h"
#include "Error/B3DException.h"
#include "Resources/B3DBuiltinResources.h"
#include "Debug/B3DDebug.h"

using namespace b3d;

Cursor::Cursor()
{
	for(u32 typeIndex = 0; typeIndex < (u32)CursorType::Count; typeIndex++)
		RestoreCursorIcon((CursorType)typeIndex);
}

void Cursor::SetScreenPosition(const GUIPhysicalPoint& screenPos)
{
	Platform::SetCursorPosition(screenPos.To<i32>());
}

GUIPhysicalPoint Cursor::GetScreenPosition() const
{
	return Platform::GetCursorPosition().To<GUIPhysicalUnit>();
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

void Cursor::ClipToRect(const Area2I& screenRect)
{
	Platform::ClipCursorToRect(screenRect);
}

void Cursor::ClipDisable()
{
	Platform::ClipCursorDisable();
}

void Cursor::SetCursor(CursorType type)
{
	u32 iconId = (u32)type;
	if((u32)mActiveCursorId != iconId)
	{
		mActiveCursorId = iconId;
		UpdateCursorImage();
	}
}

void Cursor::SetCursor(const String& name)
{
	auto iteratorFind = mCustomIconNameToId.find(name);
	if(iteratorFind == mCustomIconNameToId.end())
	{
		B3D_LOG(Warning, Platform, "Cannot find cursor icon with name: {0}", name);
		return;
	}

	u32 iconId = iteratorFind->second;
	if((u32)mActiveCursorId != iconId)
	{
		mActiveCursorId = iconId;
		UpdateCursorImage();
	}
}

void Cursor::SetCursorIcon(const String& name, const PixelData& pixelData, const Vector2I& hotSpot)
{
	auto iteratorFind = mCustomIconNameToId.find(name);
	if(iteratorFind != mCustomIconNameToId.end())
	{
		u32 iconId = iteratorFind->second;
		mCustomIcons[iconId] = CustomIcon(pixelData, hotSpot);

		if((u32)mActiveCursorId == iconId)
			UpdateCursorImage(); // Refresh active
	}
	else
	{
		u32 iconId = mNextUniqueId++;
		mCustomIconNameToId[name] = iconId;
		mCustomIcons[iconId] = CustomIcon(pixelData, hotSpot);
	}
}

void Cursor::SetCursorIcon(CursorType type, const PixelData& pixelData, const Vector2I& hotSpot)
{
	u32 iconId = (u32)type;

	mCustomIcons[iconId].PixelData = pixelData;
	mCustomIcons[iconId].HotSpot = hotSpot;

	if((u32)mActiveCursorId == iconId)
		UpdateCursorImage(); // Refresh active
}

void Cursor::ClearCursorIcon(const String& name)
{
	auto iteratorFind = mCustomIconNameToId.find(name);
	if(iteratorFind == mCustomIconNameToId.end())
		return;

	mCustomIcons.erase(iteratorFind->second);
	mCustomIconNameToId.erase(iteratorFind);
}

void Cursor::ClearCursorIcon(CursorType type)
{
	RestoreCursorIcon(type);

	if(mActiveCursorId == (i32)type)
		UpdateCursorImage(); // Refresh active
}

void Cursor::RestoreCursorIcon(CursorType type)
{
	u32 iconId = (u32)type;
	mCustomIcons[iconId] = CustomIcon();

	switch(type)
	{
	case CursorType::Arrow:
		mCustomIcons[iconId].PixelData = BuiltinResources::Instance().GetCursorArrow(mCustomIcons[iconId].HotSpot);
		return;
	case CursorType::Wait:
		mCustomIcons[iconId].PixelData = BuiltinResources::Instance().GetCursorWait(mCustomIcons[iconId].HotSpot);
		return;
	case CursorType::IBeam:
		mCustomIcons[iconId].PixelData = BuiltinResources::Instance().GetCursorIBeam(mCustomIcons[iconId].HotSpot);
		return;
	case CursorType::ArrowDrag:
		mCustomIcons[iconId].PixelData = BuiltinResources::Instance().GetCursorArrowDrag(mCustomIcons[iconId].HotSpot);
		return;
	case CursorType::SizeNESW:
		mCustomIcons[iconId].PixelData = BuiltinResources::Instance().GetCursorSizeNesw(mCustomIcons[iconId].HotSpot);
		return;
	case CursorType::SizeNS:
		mCustomIcons[iconId].PixelData = BuiltinResources::Instance().GetCursorSizeNs(mCustomIcons[iconId].HotSpot);
		return;
	case CursorType::SizeNWSE:
		mCustomIcons[iconId].PixelData = BuiltinResources::Instance().GetCursorSizeNwse(mCustomIcons[iconId].HotSpot);
		return;
	case CursorType::SizeWE:
		mCustomIcons[iconId].PixelData = BuiltinResources::Instance().GetCursorSizeWe(mCustomIcons[iconId].HotSpot);
		return;
	case CursorType::Deny:
		mCustomIcons[iconId].PixelData = BuiltinResources::Instance().GetCursorDeny(mCustomIcons[iconId].HotSpot);
		return;
	case CursorType::ArrowLeftRight:
		mCustomIcons[iconId].PixelData = BuiltinResources::Instance().GetCursorMoveLeftRight(mCustomIcons[iconId].HotSpot);
		return;
	default:
		break;
	}

	B3D_EXCEPT(InvalidParametersException, "Invalid cursor type: " + ToString((u32)type));
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
