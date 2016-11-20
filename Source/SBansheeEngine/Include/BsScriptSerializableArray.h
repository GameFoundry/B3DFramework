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

	/**	Interop class between C++ & CLR for ManagedSerializableArray. */
	class BS_SCR_BE_EXPORT ScriptSerializableArray : public ScriptObject<ScriptSerializableArray>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "SerializableArray")

		/**
		 * Creates a new serializable array interop object from the data in the provided property. Caller must ensure the
		 * property references an array.
		 */
		static ScriptSerializableArray* create(const ScriptSerializableProperty* parentProperty);

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