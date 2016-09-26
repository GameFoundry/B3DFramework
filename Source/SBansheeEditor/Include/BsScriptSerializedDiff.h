//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"

namespace BansheeEngine
{
	class ScriptSerializedObject;

	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/**	Interop class between C++ & CLR for SerializedDiff. */
	class BS_SCR_BED_EXPORT ScriptSerializedDiff : public ScriptObject <ScriptSerializedDiff>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "SerializedDiff")

	private:
		ScriptSerializedDiff(MonoObject* instance, const SPtr<ManagedSerializableDiff>& obj);

		SPtr<ManagedSerializableDiff> mSerializedDiff;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* internal_CreateDiff(ScriptSerializedObject* oldObj, MonoObject* newObj);
		static void internal_ApplyDiff(ScriptSerializedDiff* thisPtr, MonoObject* obj);
	};

	/** @} */
}