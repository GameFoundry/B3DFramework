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

	/**	Interop class between C++ & CLR for ManagedSerializableArray. */
	class BS_SCR_BE_EXPORT ScriptSerializableArray : public ScriptObject<ScriptSerializableArray>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "SerializableArray")

		/**
		 * Creates a new serializable array interop object from the data in the provided property. Caller must ensure the
		 * property references an array.
		 */
		static MonoObject* create(const ScriptSerializableProperty* native, MonoObject* managed);

	private:
		ScriptSerializableArray(MonoObject* instance, const SPtr<ManagedSerializableTypeInfoArray>& typeInfo);

		SPtr<ManagedSerializableTypeInfoArray> mTypeInfo;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* internal_createProperty(ScriptSerializableArray* nativeInstance);
	};

	/** @} */
}