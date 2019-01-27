//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		static MonoObject* create(const ScriptSerializableProperty* native, MonoObject* managed);

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