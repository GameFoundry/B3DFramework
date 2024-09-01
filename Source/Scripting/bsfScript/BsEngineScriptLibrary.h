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
	class B3D_SCRIPT_INTEROP_EXPORT EngineScriptLibrary : public ScriptLibrary
	{
	public:
		EngineScriptLibrary() = default;

		void Initialize() override;
		void Update() override;
		void Reload() override;
		void Destroy() override;

		/**	Returns the absolute path to the builtin managed engine assembly file. */
		Path GetEngineAssemblyPath() const;

#if B3D_IS_ENGINE
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

	private:
		bool mScriptAssembliesLoaded = false;
	};

	/** @} */
} // namespace bs
