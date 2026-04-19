//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for ManagedComponent. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedComponent : public TScriptGameObjectWrapper<ManagedComponent, ScriptManagedComponent>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedComponent")

		ScriptManagedComponent(const HManagedComponent& nativeObject);

		static void SetupScriptBindings();

		/**
		 * Returns null as managed components cannot be created statically. Their script object type is mutable depending on the script type they are referencing. Use non-static CreateAndBindScriptObject()
		 * member instead.
		 */
		static MonoObject* CreateScriptObject(bool construct)
		{
			return nullptr;
		}

		/** Creates a new script object of the correct component type and binds it to the script object wrapper. Script object wrapper must not have a script object assigned. */
		void CreateAndBindScriptObject();

	private:
		friend class ManagedComponent;

		void RecreateScriptObjectAfterScriptReload() override;
		TOptional<ScriptObjectReloadPersistentData> BackupDataBeforeScriptReload() override;
		void RestoreDataAfterScriptReload(const ScriptObjectReloadPersistentData& data) override;
		void NotifyScriptReloadFinished() override;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalInvoke(ScriptManagedComponent* self, MonoString* name);
	};

	/** @} */
} // namespace b3d
