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

	/**
	 * Interop class between C++ & CLR for SerializableProperty (not implemented as its own class, but instead implemented
	 * directly in the inerop object). Serializable property wraps all types of serializable entries, whether they're
	 * object fields, array entries, dictionary entries, etc. and offers a simple interface to access them.
	 */
	class BS_SCR_BE_EXPORT ScriptSerializableProperty : public ScriptObject<ScriptSerializableProperty>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "SerializableProperty")

		/**
		 * Creates a new managed serializable property object for the specified type.
		 *
		 * @param[in]	typeInfo	Data about the type the property references.
		 */
		static MonoObject* create(const SPtr<ManagedSerializableTypeInfo>& typeInfo);

		/**	Returns the data about the type the property is referencing. */
		SPtr<ManagedSerializableTypeInfo> getTypeInfo() const { return mTypeInfo; }

		~ScriptSerializableProperty() {}

	private:
		ScriptSerializableProperty(MonoObject* instance, const SPtr<ManagedSerializableTypeInfo>& typeInfo);

		SPtr<ManagedSerializableTypeInfo> mTypeInfo;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, MonoReflectionType* reflType);

		static MonoObject* internal_createObject(ScriptSerializableProperty* nativeInstance, MonoObject* managedInstance);
		static MonoObject* internal_createArray(ScriptSerializableProperty* nativeInstance, MonoObject* managedInstance);
		static MonoObject* internal_createList(ScriptSerializableProperty* nativeInstance, MonoObject* managedInstance);
		static MonoObject* internal_createDictionary(ScriptSerializableProperty* nativeInstance, MonoObject* managedInstance);

		static MonoObject* internal_createManagedObjectInstance(ScriptSerializableProperty* nativeInstance);
		static MonoObject* internal_createManagedArrayInstance(ScriptSerializableProperty* nativeInstance, MonoArray* sizes);
		static MonoObject* internal_createManagedListInstance(ScriptSerializableProperty* nativeInstance, int size);
		static MonoObject* internal_createManagedDictionaryInstance(ScriptSerializableProperty* nativeInstance);
	};

	/** @} */
}