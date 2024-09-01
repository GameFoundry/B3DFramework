//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "BsManagedResource.h"
#include "BsScriptObject.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for ManagedResource. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedResource : public ScriptObject<ScriptManagedResource, ScriptResourceBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ManagedResource")

		ScriptManagedResource(MonoObject* instance, const HManagedResource& resource);

		HResource GetGenericHandle() const override { return mResource; }

		/**	Returns a handle to the internal wrapped resource. */
		const HManagedResource& GetHandle() const { return mResource; }

	private:
		friend class ScriptResourceManager;
		friend class ManagedResource;

		ScriptObjectBackup BeginRefresh() override;
		void EndRefresh(const ScriptObjectBackup& backupData) override;
		MonoObject* CreateManagedInstanceInternal(bool construct) override;
		void ClearManagedInstanceInternal() override;
		void OnManagedInstanceDeletedInternal(bool assemblyRefresh) override;

		/** Called by the owned managed resource when it is destroyed. */
		void NotifyDestroyedInternal();

		HManagedResource mResource;
		String mNamespace;
		String mType;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance);
	};

	/** @} */
} // namespace bs
