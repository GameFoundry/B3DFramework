//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsMonoPrerequisites.h"
#include "BsScriptMeta.h"
#include "Utility/BsModule.h"

namespace bs
{
	/** @addtogroup Mono
	 *  @{
	 */

	/**	Available Mono versions. */
	enum class MonoVersion
	{
		v4_5
	};

	/**	Loads Mono script assemblies and manages script objects. */
	class BS_MONO_EXPORT MonoManager : public Module<MonoManager>
	{
	public:
		MonoManager();
		~MonoManager();

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
		 */
		static void RegisterScriptType(ScriptMeta* metaData, const ScriptMeta& localMetaData);

		/** Triggered when the assembly domain and all relevant assemblies are about to be unloaded. */
		Event<void()> OnDomainUnload;

	private:
		struct ScriptMetaInfo
		{
			ScriptMeta* MetaData;
			ScriptMeta LocalMetaData;
		};

		/**
		 * Initializes any script types registered with registerScriptType() for this assembly. This sets up any
		 * native <-> managed internal calls and other similar code for such types.
		 */
		void InitializeScriptTypes(MonoAssembly& assembly);

		/**	Returns a list of all types that will be initializes with their assembly gets loaded. */
		static UnorderedMap<String, Vector<ScriptMetaInfo>>& GetScriptMetaData()
		{
			static UnorderedMap<String, Vector<ScriptMetaInfo>> mTypesToInitialize;
			return mTypesToInitialize;
		}

		UnorderedMap<String, MonoAssembly*> mAssemblies;
		MonoDomain* mScriptDomain;
		MonoDomain* mRootDomain;
		MonoAssembly* mCorlibAssembly;
	};

	/** @} */
} // namespace bs
