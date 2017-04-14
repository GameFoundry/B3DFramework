//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsRTTIType.h"

namespace bs
{
	/** Begins the definition for the builtin resource lookup table. */
#define LOOKUP_BEGIN																										\
	class BuiltinResourceTypes																								\
	{																														\
	private:																												\
		struct META_FirstEntry {};																							\
		static void META_GetPrevEntries(Vector<BuiltinResourceInfo>& entries, META_FirstEntry id) { }						\
																															\
		typedef META_FirstEntry 

	/** Registers a new entry in the resource lookup table. */
#define ADD_ENTRY(ResourceType, ScriptType, ResourceTypeEnum)																\
		META_Entry_##ScriptType;																							\
																															\
	public:																													\
		static ScriptResourceBase* create##ScriptType(const HResource& resource, MonoObject* existingInstance)				\
		{																													\
			MonoObject* managedInstance;																					\
			if(existingInstance != nullptr)																					\
				managedInstance = existingInstance;																			\
			else																											\
				managedInstance = ScriptType::createInstance();																\
																															\
			ResourceHandle<ResourceType> castHandle = static_resource_cast<ResourceType>(resource);							\
			ScriptType* scriptResource = new (bs_alloc<ScriptType>()) ScriptType(managedInstance, castHandle);				\
																															\
			return scriptResource;																							\
		}																													\
																															\
		struct META_NextEntry_##ScriptType {};																				\
		static void META_GetPrevEntries(Vector<BuiltinResourceInfo>& entries, META_NextEntry_##ScriptType id)				\
		{																													\
			META_GetPrevEntries(entries, META_Entry_##ScriptType());														\
																															\
			BuiltinResourceInfo entry;																						\
			entry.metaData = ScriptType::getMetaData();																		\
			entry.typeId = ResourceType::getRTTIStatic()->getRTTIId();														\
			entry.monoClass = nullptr;																						\
			entry.resType = ResourceTypeEnum;																				\
			entry.createCallback = &create##ScriptType;																		\
																															\
			entries.push_back(entry);																						\
		}																													\
																															\
		typedef META_NextEntry_##ScriptType

	/** End the definition for the builtin resource lookup table. */
#define LOOKUP_END																											\
		META_LastEntry;																										\
																															\
	public:																													\
		static Vector<BuiltinResourceInfo> getEntries()																		\
		{																													\
			Vector<BuiltinResourceInfo> entries;																			\
			META_GetPrevEntries(entries, META_LastEntry());																	\
			return entries;																									\
		}																													\
	};
}