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
		:ScriptObject(instance)
	{ }

	void ScriptCursor::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_GetScreenPosition", (void*)&ScriptCursor::internal_getScreenPosition);
		metaData.scriptClass->AddInternalCall("Internal_SetScreenPosition", (void*)&ScriptCursor::internal_setScreenPosition);
		metaData.scriptClass->AddInternalCall("Internal_Hide", (void*)&ScriptCursor::internal_hide);
		metaData.scriptClass->AddInternalCall("Internal_Show", (void*)&ScriptCursor::internal_show);
		metaData.scriptClass->AddInternalCall("Internal_ClipToRect", (void*)&ScriptCursor::internal_clipToRect);
		metaData.scriptClass->AddInternalCall("Internal_ClipDisable", (void*)&ScriptCursor::internal_clipDisable);
		metaData.scriptClass->AddInternalCall("Internal_SetCursor", (void*)&ScriptCursor::internal_setCursor);
		metaData.scriptClass->AddInternalCall("Internal_SetCursorStr", (void*)&ScriptCursor::internal_setCursorStr);
		metaData.scriptClass->AddInternalCall("Internal_SetCursorIcon", (void*)&ScriptCursor::internal_setCursorIcon);
		metaData.scriptClass->AddInternalCall("Internal_SetCursorIconStr", (void*)&ScriptCursor::internal_setCursorIconStr);
		metaData.scriptClass->AddInternalCall("Internal_ClearCursorIcon", (void*)&ScriptCursor::internal_clearCursorIcon);
		metaData.scriptClass->AddInternalCall("Internal_ClearCursorIconStr", (void*)&ScriptCursor::internal_clearCursorIconStr);
	}

	void ScriptCursor::internal_getScreenPosition(Vector2I* value)
	{
		Cursor::instance().GetScreenPosition();
	}

	void ScriptCursor::internal_setScreenPosition(Vector2I* value)
	{
		Cursor::instance().SetScreenPosition(*value);
	}

	void ScriptCursor::internal_hide()
	{
		Cursor::instance().Hide();
	}

	void ScriptCursor::internal_show()
	{
		Cursor::instance().Show();
	}

	void ScriptCursor::internal_clipToRect(Rect2I* value)
	{
		Cursor::instance().ClipToRect(*value);
	}

	void ScriptCursor::internal_clipDisable()
	{
		Cursor::instance().ClipDisable();
	}

	void ScriptCursor::internal_setCursorStr(MonoString* name)
	{
		String nameStr = MonoUtil::monoToString(name);
		Cursor::instance().SetCursor(nameStr);
	}

	void ScriptCursor::internal_setCursor(CursorType cursor)
	{
		Cursor::instance().SetCursor(cursor);
	}

	void ScriptCursor::internal_setCursorIconStr(MonoString* name, MonoObject* iconData, Vector2I* hotspot)
	{
		String nameStr = MonoUtil::monoToString(name);

		ScriptPixelData* scriptPixelData = ScriptPixelData::toNative(iconData);

		if (scriptPixelData != nullptr)
		{
			SPtr<PixelData> pixelData = scriptPixelData->GetInternal();
			Cursor::instance().SetCursorIcon(nameStr, *pixelData, *hotspot);
		}
		else
			Cursor::instance().ClearCursorIcon(nameStr);
	}

	void ScriptCursor::internal_setCursorIcon(CursorType cursor, MonoObject* iconData, Vector2I* hotspot)
	{
		ScriptPixelData* scriptPixelData = ScriptPixelData::toNative(iconData);

		if (scriptPixelData != nullptr)
		{
			SPtr<PixelData> pixelData = scriptPixelData->GetInternal();
			Cursor::instance().SetCursorIcon(cursor, *pixelData, *hotspot);
		}
		else
			Cursor::instance().ClearCursorIcon(cursor);
	}

	void ScriptCursor::internal_clearCursorIconStr(MonoString* name)
	{
		String nameStr = MonoUtil::monoToString(name);
		Cursor::instance().ClearCursorIcon(nameStr);
	}

	void ScriptCursor::internal_clearCursorIcon(CursorType cursor)
	{
		Cursor::instance().ClearCursorIcon(cursor);
	}
}
