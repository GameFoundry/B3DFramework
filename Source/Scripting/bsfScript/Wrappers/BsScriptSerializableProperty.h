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

	/**
	 * Interop class between C++ & CLR for SerializableProperty (not implemented as its own class, but instead implemented
	 * directly in the inerop object). Serializable property wraps all types of serializable entries, whether they're
	 * object fields, array entries, dictionary entries, etc. and offers a simple interface to access them.
	 */
	class BS_SCR_BE_EXPORT ScriptSerializableProperty : public ScriptObject<ScriptSerializableProperty>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "SerializableProperty")

		/**
		 * Creates a new managed serializable property object for the specified type.
		 *
		 * @param[in]	typeInfo	Data about the type the property references.
		 */
		static MonoObject* Create(const SPtr<ManagedSerializableTypeInfo>& typeInfo);

		/**	Returns the data about the type the property is referencing. */
		SPtr<ManagedSerializableTypeInfo> GetTypeInfo() const { return mTypeInfo; }

		~ScriptSerializableProperty() = default;

	private:
		ScriptSerializableProperty(MonoObject* instance, const SPtr<ManagedSerializableTypeInfo>& typeInfo);

		SPtr<ManagedSerializableTypeInfo> mTypeInfo;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, MonoReflectionType* reflType);

		static MonoObject* InternalCreateObject(ScriptSerializableProperty* nativeInstance, MonoObject* managedInstance, MonoReflectionType* reflType);
		static MonoObject* InternalCreateArray(ScriptSerializableProperty* nativeInstance, MonoObject* managedInstance);
		static MonoObject* InternalCreateList(ScriptSerializableProperty* nativeInstance, MonoObject* managedInstance);
		static MonoObject* InternalCreateDictionary(ScriptSerializableProperty* nativeInstance, MonoObject* managedInstance);

		static MonoObject* InternalCreateManagedObjectInstance(ScriptSerializableProperty* nativeInstance);
		static MonoObject* InternalCreateManagedArrayInstance(ScriptSerializableProperty* nativeInstance, MonoArray* sizes);
		static MonoObject* InternalCreateManagedListInstance(ScriptSerializableProperty* nativeInstance, int size);
		static MonoObject* InternalCreateManagedDictionaryInstance(ScriptSerializableProperty* nativeInstance);
	};

	/** @} */
} // namespace bs
