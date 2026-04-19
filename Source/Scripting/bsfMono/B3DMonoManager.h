//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMonoPrerequisites.h"
#include "B3DScriptTypeMetaData.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup Mono
	 *  @{
	 */

	// TODO - Doc
	struct RegisteredScriptWrapperTypeInformation // TODO - This might better belong in ScriptAssemblyManager
	{
		ScriptTypeMetaData* MetaData; /**< Pointer to the meta-data on the associated TScriptObjectWrapper. */
		ScriptTypeMetaData LocalMetaData; /**< Local copy of the meta-data that will be used to initialize @p MetaData. */
	};

	/**	Available Mono versions. */
	enum class MonoVersion
	{
		v4_5
	};

	/**	Loads Mono script assemblies and manages script objects. */
	class B3D_MONO_EXPORT MonoManager : public Module<MonoManager>
	{
	public:
		MonoManager();
		~MonoManager();

#if B3D_USE_DOTNETCORE
		/** Loads the Mono library and sets up the relevant function pointers, if not loaded already. Must be called before performing any other actions in this class. */
		void LoadMonoLibrary();

		/** Unloads the Mono library if loaded. Caller must make sure to release all Mono objects before calling this method. */
		void UnloadMonoLibrary();
#endif

		/**
		 * Loads a new assembly from the provided path.
		 *
		 * @param[in]	path				Absolute path to the assembly .dll.
		 * @param[in]	name				Unique name for the assembly.
		 */
		MonoAssembly& LoadAssembly(const Path& path, const String& name);

		/** Unloads all assemblies and shuts down the runtime. Called automatically on module shut-down. */
		void UnloadAll();

		/**	Searches all loaded assemblies for the specified class. */
		MonoClass* FindClass(const MonoTypeIdentifier& typeIdentifier);

		/**	Searches all loaded assemblies for the specified class. */
		MonoClass* FindClass(const String& ns, const String& typeName);

		/**	Searches all loaded assemblies for the specified class. */
		MonoClass* FindClass(::MonoClass* rawMonoClass);

		/**	Returns the main (scripting) Mono domain. */
		MonoDomain* GetDomain() const { return mScriptDomain; }

		/**
		 * Attempts to find a previously loaded assembly with the specified name. Returns null if assembly cannot be found.
		 */
		MonoAssembly* GetAssembly(const String& name) const;

		/**
		 * Unloads the active domain (in which all script assemblies are loaded) and destroys any managed objects
		 * associated with it.
		 */
		void UnloadScriptDomain();

		/** Returns the absolute path of the folder where Mono framework assemblies are located. */
		Path GetFrameworkAssembliesFolder() const;

		/** Returns the absolute path to the Mono /etc folder that is required for initializing Mono. */
		Path GetMonoEtcFolder() const;

		/**	Returns the absolute path to the Mono compiler managed executable. */
		Path GetCompilerPath() const;

		/** Returns the absolute path to the executable capable of executing managed assemblies. */
		Path GetMonoExecPath() const;

		/**
		 * Registers a new script type. This should be done before any assembly loading is done. Upon assembly load these
		 * script types will be initialized with necessary information about their managed counterparts.
		 *
		 * @param metaData			Pointer to the meta-data object to initialize, stored statically on the associated TScriptObjectWrapper.
		 * @param localMetaData		Local copy of the meta-data that will be used to initialize @p metaData.
		 */
		static void RegisterScriptType(ScriptTypeMetaData* metaData, const ScriptTypeMetaData& localMetaData);

		/**	Returns a list of all registered script wrapper types, by assembly. */
		static UnorderedMap<String, Vector<RegisteredScriptWrapperTypeInformation>>& GetScriptWrapperTypeInformation()
		{
			static UnorderedMap<String, Vector<RegisteredScriptWrapperTypeInformation>> sRegisteredScriptWrapperTypes;
			return sRegisteredScriptWrapperTypes;
		}

	private:
		/**
		 * Initializes meta-data and sets up bindings for all script types registered with RegisterScriptType() for the provided assembly. This should be called
		 * after an assembly is loaded or reloaded.
		 */
		void RefreshScriptTypeMetaDataAndBindings(MonoAssembly& assembly);

		UnorderedMap<String, MonoAssembly*> mAssemblies;
		MonoDomain* mScriptDomain;
		MonoDomain* mRootDomain;
		MonoAssembly* mCorlibAssembly;
	};

	/** @} */
} // namespace b3d
