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
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ManagedComponent")

		/**	Returns a generic handle to the internal wrapped component. */
		HGameObject getNativeHandle() const override { return static_object_cast<GameObject>(mComponent); }

		/**	Sets the internal component this object wraps. */
		void setNativeHandle(const HGameObject& gameObject) override { mComponent = static_object_cast<ManagedComponent>(gameObject); }

		/**	Returns a handle to the internal wrapped component. */
		const HManagedComponent& getHandle() const { return mComponent; }

	private:
		friend class ScriptGameObjectManager;
		friend class ManagedComponent;

		ScriptManagedComponent(MonoObject* instance, const HManagedComponent& component);
		
		/** @copydoc ScriptObjectBase::beginRefresh */
		ScriptObjectBackup beginRefresh() override;

		/** @copydoc ScriptObjectBase::endRefresh */
		void endRefresh(const ScriptObjectBackup& backupData) override;

		/** @copydoc ScriptObject::_createManagedInstance */
		MonoObject* _createManagedInstance(bool construct) override;

		/** @copydoc ScriptObjectBase::_clearManagedInstance */
		void _clearManagedInstance() override;

		/** @copydoc ScriptObjectBase::_onManagedInstanceDeleted */
		void _onManagedInstanceDeleted(bool assemblyRefresh) override;

		/** @copydoc ScriptComponentBase::_notifyDestroyed */
		void _notifyDestroyed() override;

		HManagedComponent mComponent;
		String mNamespace;
		String mType;
		bool mTypeMissing;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_invoke(ScriptManagedComponent* nativeInstance, MonoString* name);
	};

	/** @} */
}
