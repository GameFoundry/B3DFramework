//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Serialization/BsManagedSerializableObjectInfo.h"
#include "Utility/BsModule.h"

namespace bs
{
	struct BuiltinComponentInfo;

	/** @addtogroup bsfScript
	 *  @{
	 */

	/** Contains all the built-in script classes that are always available. */
	struct BuiltinScriptClasses
	{
		MonoClass* systemArrayClass = nullptr;
		MonoClass* systemGenericListClass = nullptr;
		MonoClass* systemGenericDictionaryClass = nullptr;
		MonoClass* systemTypeClass = nullptr;

		MonoClass* componentClass = nullptr;
		MonoClass* managedComponentClass = nullptr;
		MonoClass* sceneObjectClass = nullptr;
		MonoClass* missingComponentClass = nullptr;

		MonoClass* rrefBaseClass = nullptr;
		MonoClass* genericRRefClass = nullptr;

		MonoClass* serializeObjectAttribute = nullptr;
		MonoClass* dontSerializeFieldAttribute = nullptr;
		MonoClass* serializeFieldAttribute = nullptr;
		MonoClass* hideInInspectorAttribute = nullptr;
		MonoClass* showInInspectorAttribute = nullptr;
		MonoClass* rangeAttribute = nullptr;
		MonoClass* stepAttribute = nullptr;
		MonoClass* layerMaskAttribute = nullptr;
		MonoClass* nativeWrapperAttribute = nullptr;
		MonoClass* notNullAttribute = nullptr;
		MonoClass* passByCopyAttribute = nullptr;
		MonoClass* applyOnDirtyAttribute = nullptr;
		MonoClass* asQuaternionAttribute = nullptr;
		MonoClass* categoryAttribute = nullptr;
		MonoClass* orderAttribute = nullptr;
		MonoClass* inlineAttribute = nullptr;
	};

	/** Contains mapping between managed objects and their native wrappers for various types. */
	struct BuiltinTypeMappings
	{
		Vector<BuiltinComponentInfo> components;
		Vector<BuiltinResourceInfo> resources;
		Vector<ReflectableTypeInfo> reflectableObjects;

		static BuiltinTypeMappings EMPTY;
	};

	/**	Stores data about managed serializable objects in specified assemblies. */
	class BS_SCR_BE_EXPORT ScriptAssemblyManager : public Module<ScriptAssemblyManager>
	{
	public:
		/**
		 * Loads all information about managed serializable objects in an assembly with the specified name. Assembly must be
		 * currently loaded. Once the data has been loaded you will be able to call getSerializableObjectInfo() and
		 * hasSerializableObjectInfo() to retrieve information about those objects. If an assembly already had data loaded
		 * it will be rebuilt.
		 * 
		 * @param[in]	assemblyName		Name of the assembly to load the information about.
		 * @param[in]	typeMappings		Contains information about managed objects that wrap native objects.
		 */
		void loadAssemblyInfo(const String& assemblyName, const BuiltinTypeMappings& typeMappings);

		/**	Clears any assembly data previously loaded with loadAssemblyInfo(). */
		void clearAssemblyInfo();

		/**
		 * Returns managed serializable object info for a specific managed type.
		 *
		 * @param[in]	ns			Namespace of the type.
		 * @param[in]	typeName	Name of the type.
		 * @param[out]	outInfo		Output object containing information about the type if the type was found, unmodified
		 *							otherwise.
		 * @return					True if the type was found, false otherwise.
		 */
		bool getSerializableObjectInfo(const String& ns, const String& typeName, 
			SPtr<ManagedSerializableObjectInfo>& outInfo);

		/**	Generates or retrieves a type info object for the specified managed class, if the class is serializable. */
		SPtr<ManagedSerializableTypeInfo> getTypeInfo(MonoClass* monoClass);

		/** 
		 * Maps a mono type to information about a wrapped built-in component. Returns null if type doesn't correspond to
		 * a builtin component. 
		 */
		BuiltinComponentInfo* getBuiltinComponentInfo(::MonoReflectionType* type);

		/** 
		 * Maps a type id to information about a wrapped built-in component. Returns null if type id doesn't correspond to
		 * a builtin component. 
		 */
		BuiltinComponentInfo* getBuiltinComponentInfo(UINT32 rttiTypeId);

		/** 
		 * Maps a mono type to information about a wrapped built-in resource. Returns null if type doesn't correspond to
		 * a builtin resource.
		 */
		BuiltinResourceInfo* getBuiltinResourceInfo(::MonoReflectionType* type);

		/** 
		 * Maps a type id to information about a wrapped built-in resource. Returns null if type id doesn't correspond to
		 * a builtin resource. 
		 */
		BuiltinResourceInfo* getBuiltinResourceInfo(UINT32 rttiTypeId);

		/** 
		 * Maps a resource type to information about a wrapped built-in resource. Returns null if type id doesn't correspond to
		 * a builtin resource. 
		 */
		BuiltinResourceInfo* getBuiltinResourceInfo(ScriptResourceType type);

		/** 
		 * Maps a mono type to information about a wrapped reflectable object. Returns null if type doesn't correspond to
		 * a reflectable object. 
		 */
		ReflectableTypeInfo* getReflectableTypeInfo(::MonoReflectionType* type);

		/** 
		 * Maps a type id to information about a wrapped reflectable object. Returns null if type id doesn't correspond to
		 * a reflectable object. 
		 */
		ReflectableTypeInfo* getReflectableTypeInfo(UINT32 rttiTypeId);

		/**
		 * Checks if the managed serializable object info for the specified type exists.
		 *
		 * @param[in]	ns			Namespace of the type.
		 * @param[in]	typeName	Name of the type.
		 * @return					True if the object info was found, false otherwise.
		 */
		bool hasSerializableObjectInfo(const String& ns, const String& typeName);

		/**	Returns names of all assemblies that currently have managed serializable object data loaded. */
		Vector<String> getScriptAssemblies() const;

		/** Returns type information for various built-in classes. */
		const BuiltinScriptClasses& getBuiltinClasses() const { return mBuiltin; }

	private:
		/**	Deletes all stored managed serializable object infos for all assemblies. */
		void clearScriptObjects();

		/**
		 * Initializes the base managed types. These are the types we expect must exist in loaded assemblies as they're used
		 * for various common operations.
		 */
		void initializeBaseTypes();

		/** 
		 * Adds mappings between managed objects and their native wrappers to their respective lookup tables and 
		 * initializes any assembly specific data. 
		 */
		void loadTypeMappings(MonoAssembly& assembly, const BuiltinTypeMappings& mapping);

		UnorderedMap<String, SPtr<ManagedSerializableAssemblyInfo>> mAssemblyInfos;
		UnorderedMap<::MonoReflectionType*, BuiltinComponentInfo> mBuiltinComponentInfos;
		UnorderedMap<UINT32, BuiltinComponentInfo> mBuiltinComponentInfosByTID;
		UnorderedMap<::MonoReflectionType*, BuiltinResourceInfo> mBuiltinResourceInfos;
		UnorderedMap<UINT32, BuiltinResourceInfo> mBuiltinResourceInfosByTID;
		UnorderedMap<UINT32, BuiltinResourceInfo> mBuiltinResourceInfosByType;
		UnorderedMap<UINT32, ReflectableTypeInfo> mReflectableTypeInfosByTID;
		UnorderedMap<::MonoReflectionType*, ReflectableTypeInfo> mReflectableTypeInfos;
		bool mBaseTypesInitialized = false;

		BuiltinScriptClasses mBuiltin;
	};

	/** @} */
}