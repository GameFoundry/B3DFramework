//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace BansheeEngine
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for ManagedSerializableFieldInfo. */
	class BS_SCR_BE_EXPORT ScriptSerializableField : public ScriptObject<ScriptSerializableField>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "SerializableField")

		/**
		 * Creates a new serializable field interop object that references a specific field on a specific object.
		 *
		 * @param[in]	parentObject	Instance of the parent object the field belongs to.
		 * @param[in]	fieldInfo		Information about the field. Caller must ensure the type matches the type of the
		 *								provided parent object.
		 */
		static ScriptSerializableField* create(MonoObject* parentObject, const SPtr<ManagedSerializableFieldInfo>& fieldInfo);
	private:
		ScriptSerializableField(MonoObject* instance, const SPtr<ManagedSerializableFieldInfo>& fieldInfo);

		SPtr<ManagedSerializableFieldInfo> mFieldInfo;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* internal_createProperty(ScriptSerializableField* nativeInstance);
		static MonoObject* internal_getValue(ScriptSerializableField* nativeInstance, MonoObject* instance);
		static void internal_setValue(ScriptSerializableField* nativeInstance, MonoObject* instance, MonoObject* value);
		static float internal_getRangeMaximum(ScriptSerializableField* nativeInstance);
		static float internal_getRangeMinimum(ScriptSerializableField* nativeInstance);
		static float internal_getRangeStep(ScriptSerializableField* nativeInstance);
	};

	/** @} */
}