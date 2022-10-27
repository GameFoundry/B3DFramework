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
	class BS_SCR_BE_EXPORT ScriptManagedComponent : public ScriptObject<ScriptManagedComponent, ScriptComponentBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ManagedComponent")

		/**	Returns a generic handle to the internal wrapped component. */
		HGameObject GetNativeHandle() const { return static_object_cast<GameObject>(mComponent); }

		/**	Sets the internal component this object wraps. */
		void SetNativeHandle(const HGameObject& gameObject) { mComponent = static_object_cast<ManagedComponent>(gameObject); }

		/**	Returns a handle to the internal wrapped component. */
		const HManagedComponent& GetHandle() const { return mComponent; }

	private:
		friend class ScriptGameObjectManager;
		friend class ManagedComponent;

		ScriptManagedComponent(MonoObject* instance, const HManagedComponent& component);

		/** @copydoc ScriptObjectBase::beginRefresh */
		ScriptObjectBackup BeginRefresh();

		/** @copydoc ScriptObjectBase::endRefresh */
		void EndRefresh(const ScriptObjectBackup& backupData);

		/** @copydoc ScriptObject::_createManagedInstance */
		MonoObject* CreateManagedInstanceInternal(bool construct) override;

		/** @copydoc ScriptObjectBase::_clearManagedInstance */
		void ClearManagedInstanceInternal() override;

		/** @copydoc ScriptObjectBase::_onManagedInstanceDeleted */
		void OnManagedInstanceDeletedInternal(bool assemblyRefresh) override;

		/** @copydoc ScriptComponentBase::_notifyDestroyed */
		void NotifyDestroyedInternal() override;

		HManagedComponent mComponent;
		String mNamespace;
		String mType;
		bool mTypeMissing;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalInvoke(ScriptManagedComponent* nativeInstance, MonoString* name);
	};

	/** @} */
} // namespace bs
