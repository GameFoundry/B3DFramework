//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"

namespace bs
{
	/** Begins the definition for the builtin component lookup table. */
#define LOOKUP_BEGIN																										\
	class BuiltinComponents																									\
	{																														\
	private:																												\
		struct META_FirstEntry {};																							\
		static void META_GetPrevEntries(Vector<BuiltinComponentInfo>& entries, META_FirstEntry id) { }						\
																															\
		typedef META_FirstEntry 

	/** Registers a new entry in the component lookup table. */
#define ADD_ENTRY(ComponentType, ScriptType)																				\
		META_Entry_##ScriptType;																							\
																															\
	public:																													\
		static ScriptComponentBase* create##ScriptType(const HComponent& component)											\
		{																													\
			MonoObject* managedInstance = ScriptType::getMetaData()->scriptClass->createInstance();							\
			ScriptType* scriptComponent = new (bs_alloc<ScriptType>())														\
				ScriptType(managedInstance, static_object_cast<ComponentType>(component));									\
																															\
			return scriptComponent;																							\
		}																													\
																															\
		struct META_NextEntry_##ScriptType {};																				\
		static void META_GetPrevEntries(Vector<BuiltinComponentInfo>& entries, META_NextEntry_##ScriptType id)				\
		{																													\
			META_GetPrevEntries(entries, META_Entry_##ScriptType());														\
																															\
			BuiltinComponentInfo entry;																						\
			entry.metaData = ScriptType::getMetaData();																		\
			entry.typeId = ComponentType::getRTTIStatic()->getRTTIId();														\
			entry.monoClass = nullptr;																						\
			entry.createCallback = &create##ScriptType;																		\
																															\
			entries.push_back(entry);																						\
		}																													\
																															\
		typedef META_NextEntry_##ScriptType

	/** End the definition for the builtin component lookup table. */
#define LOOKUP_END																											\
		META_LastEntry;																										\
																															\
	public:																													\
		static Vector<BuiltinComponentInfo> getEntries()																	\
		{																													\
			Vector<BuiltinComponentInfo> entries;																			\
			META_GetPrevEntries(entries, META_LastEntry());																	\
			return entries;																									\
		}																													\
	};
}