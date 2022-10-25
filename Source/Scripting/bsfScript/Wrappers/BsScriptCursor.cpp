//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptCursor.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Platform/BsCursor.h"
#include "Generated/BsScriptPixelData.generated.h"

namespace bs
{
	ScriptCursor::ScriptCursor(MonoObject* instance)
		: ScriptObject(instance)
	{}

	void ScriptCursor::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetScreenPosition", (void*)&ScriptCursor::InternalGetScreenPosition);
		metaData.ScriptClass->AddInternalCall("Internal_SetScreenPosition", (void*)&ScriptCursor::InternalSetScreenPosition);
		metaData.ScriptClass->AddInternalCall("Internal_Hide", (void*)&ScriptCursor::InternalHide);
		metaData.ScriptClass->AddInternalCall("Internal_Show", (void*)&ScriptCursor::InternalShow);
		metaData.ScriptClass->AddInternalCall("Internal_ClipToRect", (void*)&ScriptCursor::InternalClipToRect);
		metaData.ScriptClass->AddInternalCall("Internal_ClipDisable", (void*)&ScriptCursor::InternalClipDisable);
		metaData.ScriptClass->AddInternalCall("Internal_SetCursor", (void*)&ScriptCursor::InternalSetCursor);
		metaData.ScriptClass->AddInternalCall("Internal_SetCursorStr", (void*)&ScriptCursor::InternalSetCursorStr);
		metaData.ScriptClass->AddInternalCall("Internal_SetCursorIcon", (void*)&ScriptCursor::InternalSetCursorIcon);
		metaData.ScriptClass->AddInternalCall("Internal_SetCursorIconStr", (void*)&ScriptCursor::InternalSetCursorIconStr);
		metaData.ScriptClass->AddInternalCall("Internal_ClearCursorIcon", (void*)&ScriptCursor::InternalClearCursorIcon);
		metaData.ScriptClass->AddInternalCall("Internal_ClearCursorIconStr", (void*)&ScriptCursor::InternalClearCursorIconStr);
	}

	void ScriptCursor::InternalGetScreenPosition(Vector2I* value)
	{
		Cursor::Instance().GetScreenPosition();
	}

	void ScriptCursor::InternalSetScreenPosition(Vector2I* value)
	{
		Cursor::Instance().SetScreenPosition(*value);
	}

	void ScriptCursor::InternalHide()
	{
		Cursor::Instance().Hide();
	}

	void ScriptCursor::InternalShow()
	{
		Cursor::Instance().Show();
	}

	void ScriptCursor::InternalClipToRect(Rect2I* value)
	{
		Cursor::Instance().ClipToRect(*value);
	}

	void ScriptCursor::InternalClipDisable()
	{
		Cursor::Instance().ClipDisable();
	}

	void ScriptCursor::InternalSetCursorStr(MonoString* name)
	{
		String nameStr = MonoUtil::MonoToString(name);
		Cursor::Instance().SetCursor(nameStr);
	}

	void ScriptCursor::InternalSetCursor(CursorType cursor)
	{
		Cursor::Instance().SetCursor(cursor);
	}

	void ScriptCursor::InternalSetCursorIconStr(MonoString* name, MonoObject* iconData, Vector2I* hotspot)
	{
		String nameStr = MonoUtil::MonoToString(name);

		ScriptPixelData* scriptPixelData = ScriptPixelData::ToNative(iconData);

		if(scriptPixelData != nullptr)
		{
			SPtr<PixelData> pixelData = scriptPixelData->GetInternal();
			Cursor::Instance().SetCursorIcon(nameStr, *pixelData, *hotspot);
		}
		else
			Cursor::Instance().ClearCursorIcon(nameStr);
	}

	void ScriptCursor::InternalSetCursorIcon(CursorType cursor, MonoObject* iconData, Vector2I* hotspot)
	{
		ScriptPixelData* scriptPixelData = ScriptPixelData::ToNative(iconData);

		if(scriptPixelData != nullptr)
		{
			SPtr<PixelData> pixelData = scriptPixelData->GetInternal();
			Cursor::Instance().SetCursorIcon(cursor, *pixelData, *hotspot);
		}
		else
			Cursor::Instance().ClearCursorIcon(cursor);
	}

	void ScriptCursor::InternalClearCursorIconStr(MonoString* name)
	{
		String nameStr = MonoUtil::MonoToString(name);
		Cursor::Instance().ClearCursorIcon(nameStr);
	}

	void ScriptCursor::InternalClearCursorIcon(CursorType cursor)
	{
		Cursor::Instance().ClearCursorIcon(cursor);
	}
} // namespace bs
