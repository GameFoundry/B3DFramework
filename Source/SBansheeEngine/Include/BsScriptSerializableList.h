//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for ManagedSerializableList. */
	class BS_SCR_BE_EXPORT ScriptSerializableList : public ScriptObject<ScriptSerializableList>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "SerializableList")

		/**
		 * Creates a new serializable list interop object from the data in the provided property. Caller must ensure the
		 * property references a System.Collections.Generic.List.
		 */
		static ScriptSerializableList* create(const ScriptSerializableProperty* parentProperty);

	private:
		ScriptSerializableList(MonoObject* instance, const SPtr<ManagedSerializableTypeInfoList>& typeInfo);

		SPtr<ManagedSerializableTypeInfoList> mTypeInfo;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* internal_createProperty(ScriptSerializableList* nativeInstance);
	};

	/** @} */
}