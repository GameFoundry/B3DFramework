//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsRect2I.h"
#include "BsVector2I.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Cursor. */
	class BS_SCR_BE_EXPORT ScriptCursor : public ScriptObject <ScriptCursor>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Cursor")

	private:
		ScriptCursor(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_getScreenPosition(Vector2I* value);
		static void internal_setScreenPosition(Vector2I* value);
		static void internal_hide();
		static void internal_show();
		static void internal_clipToRect(Rect2I* value);
		static void internal_clipDisable();
		static void internal_setCursorStr(MonoString* name);
		static void internal_setCursor(CursorType cursor);
		static void internal_setCursorIconStr(MonoString* name, MonoObject* iconData, Vector2I* hotspot);
		static void internal_setCursorIcon(CursorType cursor, MonoObject* iconData, Vector2I* hotspot);
		static void internal_clearCursorIconStr(MonoString* name);
		static void internal_clearCursorIcon(CursorType cursor);
	};

	/** @} */
}