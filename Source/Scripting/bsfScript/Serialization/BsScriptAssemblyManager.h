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
		MonoClass* SystemArrayClass = nullptr;
		MonoClass* SystemGenericListClass = nullptr;
		MonoClass* SystemGenericDictionaryClass = nullptr;
		MonoClass* SystemTypeClass = nullptr;

		MonoClass* ComponentClass = nullptr;
		MonoClass* ManagedComponentClass = nullptr;
		MonoClass* SceneObjectClass = nullptr;
		MonoClass* MissingComponentClass = nullptr;

		MonoClass* RrefBaseClass = nullptr;
		MonoClass* GenericRRefClass = nullptr;
		MonoClass* GenericAsyncOpClass = nullptr;

		MonoClass* SerializeObjectAttribute = nullptr;
		MonoClass* DontSerializeFieldAttribute = nullptr;
		MonoClass* SerializeFieldAttribute = nullptr;
		MonoClass* HideInInspectorAttribute = nullptr;
		MonoClass* ShowInInspectorAttribute = nullptr;
		MonoClass* RangeAttribute = nullptr;
		MonoClass* StepAttribute = nullptr;
		MonoClass* LayerMaskAttribute = nullptr;
		MonoClass* NativeWrapperAttribute = nullptr;
		MonoClass* NotNullAttribute = nullptr;
		MonoClass* PassByCopyAttribute = nullptr;
		MonoClass* ApplyOnDirtyAttribute = nullptr;
		MonoClass* AsQuaternionAttribute = nullptr;
		MonoClass* CategoryAttribute = nullptr;
		MonoClass* OrderAttribute = nullptr;
		MonoClass* InlineAttribute = nullptr;
		MonoClass* LoadOnAssignAttribute = nullptr;
		MonoClass* HdrAttribute = nullptr;
	};

	/** Contains mapping between managed objects and their native wrappers for various types. */
	struct BuiltinTypeMappings
	{
		Vector<BuiltinComponentInfo> Components;
		Vector<BuiltinResourceInfo> Resources;
		Vector<ReflectableTypeInfo> ReflectableObjects;

		static const BuiltinTypeMappings kEmpty;
	};

	/**	Stores data about managed serializable objects in specified assemblies. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAssemblyManager : public Module<ScriptAssemblyManager>
	{
	public:
		/**
		 * Loads all information about managed serializable objects in an assembly with the specified name. Assembly must be
		 * currently loaded. Once the data has been loaded you will be able to call GetSerializableObjectInfo() and
		 * HasSerializableObjectInfo() to retrieve information about those objects. If an assembly already had data loaded
		 * it will be rebuilt.
		 *
		 * @param[in]	assemblyName		Name of the assembly to load the information about.
		 * @param[in]	typeMappings		Contains information about managed objects that wrap native objects.
		 */
		void LoadAssemblyInfo(const String& assemblyName, const BuiltinTypeMappings& typeMappings);

		/**	Clears any assembly data previously loaded with loadAssemblyInfo(). */
		void ClearAssemblyInfo();

		/**
		 * Returns managed serializable object info for a specific managed type.
		 *
		 * @param[in]	ns			Namespace of the type.
		 * @param[in]	typeName	Name of the type.
		 * @param[out]	outInfo		Output object containing information about the type if the type was found, unmodified
		 *							otherwise.
		 * @return					True if the type was found, false otherwise.
		 */
		bool GetSerializableObjectInfo(const String& ns, const String& typeName, SPtr<ManagedSerializableObjectInfo>& outInfo);

		/**	Generates or retrieves a type info object for the specified managed class, if the class is serializable. */
		SPtr<ManagedSerializableTypeInfo> GetTypeInfo(MonoClass* monoClass);

		/**
		 * Maps a mono type to information about a wrapped built-in component. Returns null if type doesn't correspond to
		 * a builtin component.
		 */
		BuiltinComponentInfo* GetBuiltinComponentInfo(::MonoReflectionType* type);

		/**
		 * Maps a type id to information about a wrapped built-in component. Returns null if type id doesn't correspond to
		 * a builtin component.
		 */
		BuiltinComponentInfo* GetBuiltinComponentInfo(u32 rttiTypeId);

		/**
		 * Maps a mono type to information about a wrapped built-in resource. Returns null if type doesn't correspond to
		 * a builtin resource.
		 */
		BuiltinResourceInfo* GetBuiltinResourceInfo(::MonoReflectionType* type);

		/**
		 * Maps a type id to information about a wrapped built-in resource. Returns null if type id doesn't correspond to
		 * a builtin resource.
		 */
		BuiltinResourceInfo* GetBuiltinResourceInfo(u32 rttiTypeId);

		/**
		 * Maps a resource type to information about a wrapped built-in resource. Returns null if type id doesn't correspond to
		 * a builtin resource.
		 */
		BuiltinResourceInfo* GetBuiltinResourceInfo(ScriptResourceType type);

		/**
		 * Maps a mono type to information about a wrapped reflectable object. Returns null if type doesn't correspond to
		 * a reflectable object.
		 */
		ReflectableTypeInfo* GetReflectableTypeInfo(::MonoReflectionType* type);

		/**
		 * Maps a type id to information about a wrapped reflectable object. Returns null if type id doesn't correspond to
		 * a reflectable object.
		 */
		ReflectableTypeInfo* GetReflectableTypeInfo(u32 rttiTypeId);

		/**
		 * Checks if the managed serializable object info for the specified type exists.
		 *
		 * @param[in]	ns			Namespace of the type.
		 * @param[in]	typeName	Name of the type.
		 * @return					True if the object info was found, false otherwise.
		 */
		bool HasSerializableObjectInfo(const String& ns, const String& typeName);

		/**	Returns names of all assemblies that currently have managed serializable object data loaded. */
		Vector<String> GetScriptAssemblies() const;

		/** Returns type information for various built-in classes. */
		const BuiltinScriptClasses& GetBuiltinClasses() const { return mBuiltin; }

		/**
		 * Converts a managed object into an IReflectable object. The system first checks if the managed object is just a
		 * wrapper for a reflectable object already, and if so returns the wrapped reflectable object. Otherwise the managed
		 * object is serialized and the serialized version of the object is returned. The provided object cannot be an array,
		 * list, dictionary, component or a resource.
		 */
		SPtr<IReflectable> GetReflectableFromManagedObject(MonoObject* value);

	private:
		/**	Deletes all stored managed serializable object infos for all assemblies. */
		void ClearScriptObjects();

		/**
		 * Initializes the base managed types. These are the types we expect must exist in loaded assemblies as they're used
		 * for various common operations.
		 */
		void InitializeBaseTypes();

		/**
		 * Adds mappings between managed objects and their native wrappers to their respective lookup tables and
		 * initializes any assembly specific data.
		 */
		void LoadTypeMappings(MonoAssembly& assembly, const BuiltinTypeMappings& mapping);

		UnorderedMap<String, SPtr<ManagedSerializableAssemblyInfo>> mAssemblyInfos;
		UnorderedMap<::MonoReflectionType*, BuiltinComponentInfo> mBuiltinComponentInfos;
		UnorderedMap<u32, BuiltinComponentInfo> mBuiltinComponentInfosByTID;
		UnorderedMap<::MonoReflectionType*, BuiltinResourceInfo> mBuiltinResourceInfos;
		UnorderedMap<u32, BuiltinResourceInfo> mBuiltinResourceInfosByTID;
		UnorderedMap<u32, BuiltinResourceInfo> mBuiltinResourceInfosByType;
		UnorderedMap<u32, ReflectableTypeInfo> mReflectableTypeInfosByTID;
		UnorderedMap<::MonoReflectionType*, ReflectableTypeInfo> mReflectableTypeInfos;
		bool mBaseTypesInitialized = false;

		BuiltinScriptClasses mBuiltin;
	};

	/** @} */
} // namespace bs
