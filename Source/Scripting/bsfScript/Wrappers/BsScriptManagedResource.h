//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "BsScriptObject.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for ManagedResource. */
	class BS_SCR_BE_EXPORT ScriptManagedResource : public ScriptObject<ScriptManagedResource, ScriptResourceBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ManagedResource")

		ScriptManagedResource(MonoObject* instance, const HManagedResource& resource);

		/** @copydoc ScriptResourceBase::getGenericHandle */
		HResource GetGenericHandle() const override { return mResource; }

		/** @copydoc ScriptResourceBase::setResource */
		void SetResource(const HResource& resource) override;

		/**	Returns a handle to the internal wrapped resource. */
		const HManagedResource& GetHandle() const { return mResource; }

	private:
		friend class ScriptResourceManager;
		friend class ManagedResource;

		/** @copydoc ScriptObjectBase::beginRefresh */
		ScriptObjectBackup BeginRefresh() override;

		/** @copydoc ScriptObjectBase::endRefresh */
		void EndRefresh(const ScriptObjectBackup& backupData) override;

		/** @copydoc ScriptObject::_createManagedInstance */
		MonoObject* CreateManagedInstanceInternal(bool construct) override;

		/** @copydoc ScriptObjectBase::_clearManagedInstance */
		void ClearManagedInstanceInternal() override;

		/** @copydoc ScriptObjectBase::_onManagedInstanceDeleted */
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
