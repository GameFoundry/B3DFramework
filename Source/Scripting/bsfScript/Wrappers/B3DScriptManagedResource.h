//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DManagedResource.h"
#include "B3DScriptResourceWrapper.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for ManagedResource. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedResource : public TScriptResourceWrapper<ManagedResource, ScriptManagedResource>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedResource")

		ScriptManagedResource(const HManagedResource& nativeObject);

		ScriptObjectLifetimeTrackingMode GetLifetimeTrackingMode() const override { return ScriptObjectLifetimeTrackingMode::StrongHandleWithExplicitDestroy; }

		static void SetupScriptBindings();

		/**
		 * Returns null as managed resources cannot be created statically. Their script object type is mutable depending on the script type they are referencing. Use non-static CreateAndBindScriptObject()
		 * member instead.
		 */
		static MonoObject* CreateScriptObject(bool construct) { return nullptr; }

		/** Creates a new script object of the correct resource type and binds it to the script object wrapper. Script object wrapper must not have a script object assigned. */
		void CreateAndBindScriptObject();

	private:
		friend class ManagedResource;

		void RecreateScriptObjectAfterScriptReload() override;
		TOptional<ScriptObjectReloadPersistentData> BackupDataBeforeScriptReload() override;
		void RestoreDataAfterScriptReload(const ScriptObjectReloadPersistentData& data) override;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* scriptObject);
	};

	/** @} */
} // namespace b3d
