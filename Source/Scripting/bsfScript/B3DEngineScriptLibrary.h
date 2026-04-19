//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Script/B3DScriptManager.h"
#include "Serialization/B3DScriptAssemblyManager.h"

namespace b3d
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

		Path GetEngineAssemblyPath() const override;
		const char* GetEngineAssemblyName() const override { return kEngineAssembly; }

#if B3D_IS_ENGINE
		Path GetGameAssemblyPath() const override;
		const char* GetGameAssemblyName() const override { return kScriptGameAssembly; }
#endif

		Path GetBuiltinAssemblyFolder() const override;
		Path GetScriptAssemblyFolder() const override;

		Path GetBuiltinAssembliesPath() const override;
		Path GetScriptingRuntimePath() const override;

		/**	Returns the absolute path where the managed release assemblies are located. */
		static const Path& GetReleaseAssemblyPath();

		/**	Returns the absolute path where the managed debug assemblies are located. */
		static const Path& GetDebugAssemblyPath();

		/** Returns the singleton instance of this library. */
		static EngineScriptLibrary& Instance()
		{
			return static_cast<EngineScriptLibrary&>(*ScriptManager::Instance().GetScriptLibrary());
		}

	protected:
		/**	Loads all managed types and methods used by this module. */
		void LoadMonoTypes();

		/** Unloads all manages assemblies and the mono domain. */
		void UnloadAssemblies();

		/** Shuts down all script engine modules. */
		void ShutdownModules();

		/** Triggers the Application::OnInitialize() managed method. */
		void TriggerOnInitialize();

		/** Triggers the Application::OnDestroy() managed method. */
		void TriggerOnDestroy();

		/**	Triggered when an assembly refreshed finished. */
		void OnAssemblyRefreshComplete();

	private:
		bool mScriptAssembliesLoaded = false;

		MonoMethod* mUpdateMethod = nullptr;
		MonoAssembly* mEngineAssembly = nullptr;

		HEvent mOnAssemblyRefreshAssembliesLoadedConnection;
		HEvent mOnAssemblyRefreshDoneConnection;
	};

	/** @} */
} // namespace b3d
