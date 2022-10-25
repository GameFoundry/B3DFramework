//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Script/BsScriptManager.h"
#include "Serialization/BsScriptAssemblyManager.h"

namespace bs
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	/** Handles initialization/shutdown of the script systems and loading/refresh of engine-specific assemblies. */
	class BS_SCR_BE_EXPORT EngineScriptLibrary : public ScriptLibrary
	{
	public:
		EngineScriptLibrary() = default;

		/** @copydoc ScriptLibrary::initialize */
		void Initialize() override;

		/** @copydoc ScriptLibrary::update */
		void Update() override;

		/** @copydoc ScriptLibrary::reload */
		void Reload() override;

		/** @copydoc ScriptLibrary::destroy */
		void Destroy() override;

		/**	Returns the absolute path to the builtin managed engine assembly file. */
		Path GetEngineAssemblyPath() const;

#if BS_IS_BANSHEE3D
		/**	Returns the absolute path to the game managed assembly file. */
		Path GetGameAssemblyPath() const;
#endif

		/**	Returns the absolute path to the folder where built-in assemblies are located in. */
		virtual Path GetBuiltinAssemblyFolder() const;

		/**	Returns the absolute path to the folder where script assemblies are located in. */
		virtual Path GetScriptAssemblyFolder() const;

		/**	Returns the absolute path where the managed release assemblies are located. */
		static const Path& GetReleaseAssemblyPath();

		/**	Returns the absolute path where the managed debug assemblies are located. */
		static const Path& GetDebugAssemblyPath();

		/** Returns the singleton instance of this library. */
		static EngineScriptLibrary& Instance()
		{
			return static_cast<EngineScriptLibrary&>(*ScriptManager::Instance().GetScriptLibraryInternal());
		}

	protected:
		/** Unloads all manages assemblies and the mono domain. */
		void UnloadAssemblies();

		/** Shuts down all script engine modules. */
		void ShutdownModules();

		BuiltinTypeMappings mEngineTypeMappings;

	private:
		bool mScriptAssembliesLoaded = false;
	};

	/** @} */
} // namespace bs
