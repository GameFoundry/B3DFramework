//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"

namespace BansheeEngine
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/**	Interop class between C++ & CLR for SerializedObject. */
	class BS_SCR_BED_EXPORT ScriptSerializedObject : public ScriptObject <ScriptSerializedObject>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "SerializedObject")

		/** Returns the serialized object wrapped by this object. */
		SPtr<ManagedSerializableObject> getInternal() const { return mSerializedObject; }

	private:
		ScriptSerializedObject(MonoObject* instance, const SPtr<ManagedSerializableObject>& obj);

		SPtr<ManagedSerializableObject> mSerializedObject;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* internal_CreateComponent(ScriptComponent* componentPtr);
		static MonoObject* internal_CreateResource(ScriptManagedResource* resourcePtr);
		static MonoObject* internal_CreateGeneric(MonoObject* obj);
		static MonoObject* internal_Deserialize(ScriptSerializedObject* thisPtr);
	};

	/** @} */
}