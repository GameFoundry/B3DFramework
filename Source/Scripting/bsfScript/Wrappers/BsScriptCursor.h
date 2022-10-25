//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Math/BsRect2I.h"
#include "Math/BsVector2I.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Cursor. */
	class BS_SCR_BE_EXPORT ScriptCursor : public ScriptObject<ScriptCursor>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Cursor")

	private:
		ScriptCursor(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalGetScreenPosition(Vector2I* value);
		static void InternalSetScreenPosition(Vector2I* value);
		static void InternalHide();
		static void InternalShow();
		static void InternalClipToRect(Rect2I* value);
		static void InternalClipDisable();
		static void InternalSetCursorStr(MonoString* name);
		static void InternalSetCursor(CursorType cursor);
		static void InternalSetCursorIconStr(MonoString* name, MonoObject* iconData, Vector2I* hotspot);
		static void InternalSetCursorIcon(CursorType cursor, MonoObject* iconData, Vector2I* hotspot);
		static void InternalClearCursorIconStr(MonoString* name);
		static void InternalClearCursorIcon(CursorType cursor);
	};

	/** @} */
} // namespace bs
