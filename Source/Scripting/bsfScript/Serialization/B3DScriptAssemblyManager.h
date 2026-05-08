//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Serialization/B3DManagedTypeInfo.h"
#include "Utility/B3DModule.h"

namespace b3d
{
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
		MonoClass* MissingResourceClass = nullptr;

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
		void LoadAssemblyInfo(const String& assemblyName);

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
		bool GetSerializableObjectInfo(const String& ns, const String& typeName, TShared<ManagedObjectInfo>& outInfo);

		/**
		 * Returns managed serializable object info for a specific managed type. Object must be serializable for this
		 * information to be present.
		 */
		TShared<ManagedObjectInfo> GetSerializableObjectInfo(MonoReflectionType* objectType);

		/**	Generates or retrieves a type info object for the specified managed class, if the class is serializable. */
		TShared<ManagedTypeInfo> GetTypeInfo(MonoClass* monoClass);

		/** Returns script wrapper object meta-data for the type as specified by the provided RTTI type ID. */
		const ScriptTypeMetaData* GetScriptWrapperMetaData(u32 typeId) const;

		/** Returns script wrapper object meta-data for the type as specified by the script type. */
		const ScriptTypeMetaData* GetScriptWrapperMetaData(::MonoReflectionType* type) const;

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
		TShared<IReflectable> GetReflectableFromManagedObject(MonoObject* value);

		/**
		 * Converts a reflectable object into a managed object. The system first checks if the IReflectable is just
		 * a serialized managed object, in which case the object is deserialized and returned. Otherwise the system
		 * assumes the reflectable type is script exportable and attempts to retrieve or the script object wrapper
		 * for the object.
		 */
		MonoObject* GetManagedObjectFromReflectable(const TShared<IReflectable>& object);

	private:
		/**	Deletes all stored managed serializable object infos for all assemblies. */
		void ClearScriptObjects();

		/**
		 * Initializes the base managed types. These are the types we expect must exist in loaded assemblies as they're used
		 * for various common operations.
		 */
		void InitializeBaseTypes();

		/**
		 * Creates a lookup that allows you to find script object wrapper type based on RTTI type ID or script type. Should be called after
		 * an assembly is loaded or reloaded.
		 */
		void InitializeScriptWrapperMetaDataLookup(MonoAssembly& assembly);

		UnorderedMap<String, TShared<ManagedAssemblyInfo>> mAssemblyInfos;

		UnorderedMap<u32, ScriptTypeMetaData*> mScriptWrapperMetaDataByTypeId;
		UnorderedMap<::MonoReflectionType*, ScriptTypeMetaData*> mScriptWrapperMetaDataByScriptClass;

		bool mBaseTypesInitialized = false;

		BuiltinScriptClasses mBuiltin;
	};

	/** @} */
} // namespace b3d
