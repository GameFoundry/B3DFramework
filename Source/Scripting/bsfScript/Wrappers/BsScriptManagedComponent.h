//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptObject.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for ManagedComponent. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedComponent : public TScriptGameObjectWrapper<ManagedComponent, ScriptManagedComponent>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "ManagedComponent")

		ScriptManagedComponent(const HManagedComponent& nativeObject, MonoObject* scriptObject);

		static MonoObject* CreateScriptObject(bool construct)
		{
			return nullptr;
		}

	private:
		friend class ManagedComponent;

		void RecreateScriptObjectAfterScriptReload() override;
		Optional<ScriptObjectReloadPersistentData> BackupDataBeforeScriptReload() override;
		void RestoreDataAfterScriptReload(const ScriptObjectReloadPersistentData& data) override;

		String mNamespace;
		String mType;
		bool mTypeMissing = false;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalInvoke(ScriptManagedComponent* self, MonoString* name);
	};

	/** @} */
} // namespace bs
