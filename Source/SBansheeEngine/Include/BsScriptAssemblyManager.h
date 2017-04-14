//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsManagedSerializableObjectInfo.h"
#include "BsModule.h"

namespace bs
{
	struct BuiltinComponentInfo;

	/** @addtogroup SBansheeEngine
	 *  @{
	 */

	/**	Stores data about managed serializable objects in specified assemblies. */
	class BS_SCR_BE_EXPORT ScriptAssemblyManager : public Module<ScriptAssemblyManager>
	{
	public:
		ScriptAssemblyManager();
		~ScriptAssemblyManager();

		/**
		 * Loads all information about managed serializable objects in an assembly with the specified name. Assembly must be
		 * currently loaded. Once the data has been loaded you will be able to call getSerializableObjectInfo() and
		 * hasSerializableObjectInfo() to retrieve information about those objects. If an assembly already had data loaded
		 * it will be rebuilt.
		 */
		void loadAssemblyInfo(const String& assemblyName);

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
		 * Checks if the managed serializable object info for the specified type exists.
		 *
		 * @param[in]	ns			Namespace of the type.
		 * @param[in]	typeName	Name of the type.
		 * @return					True if the object info was found, false otherwise.
		 */
		bool hasSerializableObjectInfo(const String& ns, const String& typeName);

		/**	Returns names of all assemblies that currently have managed serializable object data loaded. */
		Vector<String> getScriptAssemblies() const;

		/**	Gets the managed class for System.Array type. */
		MonoClass* getSystemArrayClass() const { return mSystemArrayClass; }

		/**	Gets the managed class for System.Collections.Generic.List<T> type. */
		MonoClass* getSystemGenericListClass() const { return mSystemGenericListClass; }

		/**	Gets the managed class for System.Collections.Generic.Dictionary<T,U> type. */
		MonoClass* getSystemGenericDictionaryClass() const { return mSystemGenericDictionaryClass; }

		/**	Gets the managed class for System.Type type. */
		MonoClass* getSystemTypeClass() const { return mSystemTypeClass; }

		/**	Gets the managed class for BansheeEngine.Component type. */
		MonoClass* getComponentClass() const { return mComponentClass; }

		/**	Gets the managed class for BansheeEngine.ManagedComponent type. */
		MonoClass* getManagedComponentClass() const { return mManagedComponentClass; }

		/**	Gets the managed class for BansheeEngine.MissingComponent type. */
		MonoClass* getMissingComponentClass() const { return mMissingComponentClass; }

		/**	Gets the managed class for BansheeEngine.SceneObject type. */
		MonoClass* getSceneObjectClass() const { return mSceneObjectClass; }

		/** Gets the managed class for BansheeEngine.Range attribute */
		MonoClass* getRangeAttribute() const { return mRangeAttribute; }

		/** Gets the managed class for BansheeEngine.Step attribute */
		MonoClass* getStepAttribute() const { return mStepAttribute; }
	private:
		/**	Deletes all stored managed serializable object infos for all assemblies. */
		void clearScriptObjects();

		/**
		 * Initializes the base managed types. These are the types we expect must exist in loaded assemblies as they're used
		 * for various common operations.
		 */
		void initializeBaseTypes();

		/** Initializes information required for mapping builtin components to managed components. */
		void initializeBuiltinComponentInfos();

		/** Initializes information required for mapping builtin resources to managed resources. */
		void initializeBuiltinResourceInfos();

		UnorderedMap<String, SPtr<ManagedSerializableAssemblyInfo>> mAssemblyInfos;
		UnorderedMap<::MonoReflectionType*, BuiltinComponentInfo> mBuiltinComponentInfos;
		UnorderedMap<UINT32, BuiltinComponentInfo> mBuiltinComponentInfosByTID;
		UnorderedMap<::MonoReflectionType*, BuiltinResourceInfo> mBuiltinResourceInfos;
		UnorderedMap<UINT32, BuiltinResourceInfo> mBuiltinResourceInfosByTID;
		UnorderedMap<UINT32, BuiltinResourceInfo> mBuiltinResourceInfosByType;
		bool mBaseTypesInitialized;

		MonoClass* mSystemArrayClass;
		MonoClass* mSystemGenericListClass;
		MonoClass* mSystemGenericDictionaryClass;
		MonoClass* mSystemTypeClass;

		MonoClass* mComponentClass;
		MonoClass* mManagedComponentClass;
		MonoClass* mSceneObjectClass;
		MonoClass* mMissingComponentClass;

		MonoClass* mSerializeObjectAttribute;
		MonoClass* mDontSerializeFieldAttribute;
		MonoClass* mSerializeFieldAttribute;
		MonoClass* mHideInInspectorAttribute;
		MonoClass* mShowInInspectorAttribute;
		MonoClass* mRangeAttribute;
		MonoClass* mStepAttribute;
	};

	/** @} */
}