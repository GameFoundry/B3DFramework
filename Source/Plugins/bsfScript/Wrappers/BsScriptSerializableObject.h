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

	/**	Interop class between C++ & CLR for ManagedSerializableObject. */
	class BS_SCR_BE_EXPORT ScriptSerializableObject : public ScriptObject<ScriptSerializableObject>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "SerializableObject")

		/**	Creates a new serializable object interop object from the data in the provided property.  */
		static MonoObject* create(const ScriptSerializableProperty* native, MonoObject* managed, 
			MonoReflectionType* reflType);

	private:
		ScriptSerializableObject(MonoObject* instance, const SPtr<ManagedSerializableTypeInfo>& typeInfo);

		/**
		 * Creates a new interop object for a serializable object from an existing managed instance and an object info
		 * structure describing the type of the managed instance.
		 *
		 * @param[in]	instance	Managed instance the interop object will reference.
		 * @param[in]	objInfo		Data about the type of the provided managed instance.
		 */
		static ScriptSerializableObject* createInternal(MonoObject* instance, const SPtr<ManagedSerializableObjectInfo>& objInfo);

		SPtr<ManagedSerializableTypeInfo> mTypeInfo;
		static MonoField* FieldsField;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoReflectionType* type);
	};

	/** @} */
}