//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup ScriptExport-Internal
	 *  @{
	 */

	/**	Abstraction that handles a specific set of script libraries. */
	class B3D_EXPORT ScriptLibrary
	{
	public:
		virtual ~ScriptLibrary() = default;

		/**	Called when the script system is being activated. */
		virtual void Initialize() = 0;

		/** Called once per frame. */
		virtual void Update() {}

		/** Called when the script libraries should be reloaded (for example when they are recompiled). */
		virtual void Reload() = 0;

		/**	Called when the script system is being destroyed. */
		virtual void Destroy() = 0;

		/**	Returns the absolute path to the builtin managed engine assembly file. */
		virtual Path GetEngineAssemblyPath() const { return Path::kBlank; }

		/** Returns the name of the assembly that wraps built-in engine functionality, without extension. */
		virtual const char* GetEngineAssemblyName() const { return nullptr; }

#if B3D_IS_ENGINE
		/**	Returns the absolute path to the game managed assembly file. */
		virtual Path GetGameAssemblyPath() const { return Path::kBlank; }

		/** Returns the name of the assembly that will store user's runtime application code, without extension. */
		virtual const char* GetGameAssemblyName() const { return nullptr; }
#endif

		// TODO #if B3D_IS_EDITOR
		/**	Returns the absolute path to the built-in managed editor assembly file. */
		virtual Path GetEditorAssemblyPath() const { return Path::kBlank; }

		/**	Returns the absolute path of the managed editor script assembly file. */
		virtual Path GetEditorScriptAssemblyPath() const { return Path::kBlank; }

		/** Returns the name of the assembly that wraps built-in editor functionality, without extension. */
		virtual const char* GetEditorAssemblyName() const { return nullptr; }

		/** Returns the name of the assembly that wraps user implemented editor functionality, without extension. */
		virtual const char* GetScriptEditorAssemblyName() const { return nullptr; }
		// TODO #endif

		/**	Returns the absolute path to the folder where built-in assemblies are located in. */
		virtual Path GetBuiltinAssemblyFolder() const { return Path::kBlank; }

		/**	Returns the absolute path to the folder where script assemblies are located in. */
		virtual Path GetScriptAssemblyFolder() const { return Path::kBlank; }

		/** Returns the absolute path at which builtin assemblies requires by the scripting runtime are located. */
		virtual Path GetBuiltinAssembliesPath() const = 0;
		
		/** Returns the absolute path at which miscellaneous files required by the scripting runtime are located. */
		virtual Path GetScriptingRuntimePath() const = 0;
	};

	/**	Handles initialization of a scripting system. */
	class B3D_EXPORT ScriptManager : public Module<ScriptManager>
	{
	public:
		ScriptManager();
		~ScriptManager();

		/** Called once per frame. */
		void Update();

		/**
		 * Reloads any scripts in the currently active library. Should be called after some change to the scripts was made
		 * (for example project was changed, or scripts were recompiled).
		 */
		void Reload();

		/**
		 * Sets the active script library that controls what kind and which scripts are loaded. Must be called before
		 * the module is started up.
		 */
		static void SetScriptLibrary(const SPtr<ScriptLibrary>& library) { sScriptLibrary = library; }

		/** Returns the currently assigned script library. */
		static const SPtr<ScriptLibrary>& GetScriptLibrary() { return sScriptLibrary; }

	private:
		static SPtr<ScriptLibrary> sScriptLibrary;
	};

	/** @} */
} // namespace b3d
