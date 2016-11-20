//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsMonoPrerequisites.h"
#include "BsScriptMeta.h"
#include "BsModule.h"

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
		 * @param[in]	path	Absolute path to the assembly .dll.
		 * @param[in]	name	Unique name for the assembly.
		 */
		MonoAssembly& loadAssembly(const WString& path, const String& name);

		/**	Searches all loaded assemblies for the specified class. */
		MonoClass* findClass(const String& ns, const String& typeName);

		/**	Searches all loaded assemblies for the specified class. */
		MonoClass* findClass(::MonoClass* rawMonoClass);

		/**	Returns the current Mono domain. */
		MonoDomain* getDomain() const { return mScriptDomain; }

		/**
		 * Attempts to find a previously loaded assembly with the specified name. Returns null if assembly cannot be found.
		 */
		MonoAssembly* getAssembly(const String& name) const;

		/**
		 * Unloads the active domain (in which all script assemblies are loaded) and destroys any managed objects
		 * associated with it.
		 */
		void unloadScriptDomain();

		/**
		 * Loads a new script domain. If another domain already exists it will be unloaded. This will also restore any
		 * previously loaded assemblies.
		 */
		void loadScriptDomain();

		/** Returns the absolute path of the folder where Mono framework assemblies are located. */
		Path getFrameworkAssembliesFolder() const;

		/** Returns the absolute path to the Mono /etc folder that is required for initializing Mono. */
		Path getMonoEtcFolder() const;

		/**	Returns the absolute path to the Mono compiler. */
		Path getCompilerPath() const;

		/**
		 * Registers a new script type. This should be done before any assembly loading is done. Upon assembly load these
		 * script types will be initialized with necessary information about their managed counterparts.
		 */
		static void registerScriptType(ScriptMeta* metaData);

		/** Triggered when the assembly domain and all relevant assemblies are about to be unloaded. */
		Event<void()> onDomainUnload;
	private:
		/**	Initializes a previous loaded assembly. */
		void initializeAssembly(MonoAssembly& assembly);

		/**	Returns a list of all types that will be initializes with their assembly gets loaded. */
		static UnorderedMap<String, Vector<ScriptMeta*>>& getScriptMetaData()
		{
			static UnorderedMap<String, Vector<ScriptMeta*>> mTypesToInitialize;
			return mTypesToInitialize;
		}

		UnorderedMap<String, MonoAssembly*> mAssemblies;
		MonoDomain* mScriptDomain;
		MonoDomain* mRootDomain;
		bool mIsCoreLoaded;
	};

	/** @} */
}