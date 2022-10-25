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
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "SerializableObject")

		/**	Creates a new serializable object interop object from the data in the provided property.  */
		static MonoObject* Create(const ScriptSerializableProperty* native, MonoObject* managed, MonoReflectionType* reflType);

		/**	Creates a new serializable object interop object from the data in the provided property.  */
		static MonoObject* Create(MonoObject* managed, MonoReflectionType* reflType);

	private:
		ScriptSerializableObject(MonoObject* instance, const SPtr<ManagedSerializableObjectInfo>& objInfo);

		/**
		 * Creates a new interop object for a serializable object from an existing managed instance and an object info
		 * structure describing the type of the managed instance.
		 *
		 * @param[in]	instance	Managed instance the interop object will reference.
		 * @param[in]	objInfo		Data about the type of the provided managed instance.
		 */
		static ScriptSerializableObject* CreateInternal(MonoObject* instance, const SPtr<ManagedSerializableObjectInfo>& objInfo);

		SPtr<ManagedSerializableObjectInfo> mObjInfo;
		static MonoField* FieldsField;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, MonoReflectionType* type);
		static MonoObject* InternalGetBaseClass(ScriptSerializableObject* thisPtr, MonoObject* owningObject);
	};

	/** @} */
} // namespace bs
