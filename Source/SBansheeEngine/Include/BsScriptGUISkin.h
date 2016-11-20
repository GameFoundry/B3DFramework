//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResource.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUISkin. */
	class BS_SCR_BE_EXPORT ScriptGUISkin : public TScriptResource<ScriptGUISkin, GUISkin>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUISkin")

		/**	Creates an empty, uninitialized managed instance of the resource interop object. */
		static MonoObject* createInstance();

	private:
		friend class ScriptResourceManager;

		ScriptGUISkin(MonoObject* instance, const HGUISkin& skin);

		/** @copydoc ScriptObjectBase::_createManagedInstance */
		MonoObject* _createManagedInstance(bool construct) override;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
		static bool internal_HasStyle(ScriptGUISkin* thisPtr, MonoString* name);
		static MonoObject* internal_GetStyle(ScriptGUISkin* thisPtr, MonoString* name);
		static void internal_SetStyle(ScriptGUISkin* thisPtr, MonoString* name, ScriptGUIElementStyle* style);
		static void internal_RemoveStyle(ScriptGUISkin* thisPtr, MonoString* name);
		static MonoArray* internal_GetStyleNames(ScriptGUISkin* thisPtr);
	};

	/** @} */
}