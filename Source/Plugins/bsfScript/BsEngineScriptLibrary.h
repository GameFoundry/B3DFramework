//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Script/BsScriptManager.h"

namespace bs
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	/** Handles initialization/shutdown of the script systems and loading/refresh of engine-specific assemblies. */
	class BS_SCR_BE_EXPORT EngineScriptLibrary : public ScriptLibrary
	{
	public:
		EngineScriptLibrary();

		/** @copydoc ScriptLibrary::initialize */
		void initialize() override;

		/** @copydoc ScriptLibrary::reload */
		void reload() override;

		/** @copydoc ScriptLibrary::destroy */
		void destroy() override;

		/**	Returns the absolute path to the builtin managed engine assembly file. */
		Path getEngineAssemblyPath() const;

#if BS_IS_BANSHEE3D
		/**	Returns the absolute path to the game managed assembly file. */
		Path getGameAssemblyPath() const;
#endif

		/**	Returns the absolute path to the folder where built-in assemblies are located in. */
		virtual Path getBuiltinAssemblyFolder() const;

		/**	Returns the absolute path to the folder where script assemblies are located in. */
		virtual Path getScriptAssemblyFolder() const;

		/**	Returns the absolute path where the managed release assemblies are located. */
		static const Path& getReleaseAssemblyPath();

		/**	Returns the absolute path where the managed debug assemblies are located. */
		static const Path& getDebugAssemblyPath();

		/** Returns the singleton instance of this library. */
		static EngineScriptLibrary& instance()
		{
			return static_cast<EngineScriptLibrary&>(*ScriptManager::instance()._getScriptLibrary());
		}

	protected:
		/** Unloads all manages assemblies and the mono domain. */
		void unloadAssemblies();

		/** Shuts down all script engine modules. */
		void shutdownModules();

	private:
		bool mScriptAssembliesLoaded;
	};

	/** @} */
}