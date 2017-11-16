//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Base class for all GameObject interop classes. */
	class BS_SCR_BE_EXPORT ScriptGameObjectBase : public PersistentScriptObjectBase
	{
	public:
		ScriptGameObjectBase(MonoObject* instance);
		virtual ~ScriptGameObjectBase();

		/**	Returns the internal native GameObject handle. */
		virtual HGameObject getNativeHandle() const = 0;

		/**	Sets the internal native GameObject handle. */
		virtual void setNativeHandle(const HGameObject& gameObject) = 0;

		/** Returns the managed version of this game object. */
		MonoObject* getManagedInstance() const;

		/** @copydoc ScriptObjectBase::beginRefresh */
		ScriptObjectBackup beginRefresh() override;

		/** @copydoc ScriptObjectBase::endRefresh */
		void endRefresh(const ScriptObjectBackup& backupData) override;
	protected:
		/** 
		 * Makes the object reference the specific managed instance. Internally this allocates a GC handle that keeps a
		 * reference to the object and allows getManagedInstance to retrieve the managed instance when requested. Should
		 * be called on initial creation and whenever the managed instance changes (e.g. after assembly refresh). This
		 * creates a strong reference to the managed instance, and you need to make sure to release it with
		 * freeManagedInstance() when no longer required.
		 */
		void setManagedInstance(MonoObject* instance);

		/** 
		 * Frees a managed instace assigned with setManagedInstance(). Should be called before the object is destroyed or 
		 * when you changing the managed instance it points to (in order to release the previous instance). 
		 */
		void freeManagedInstance();

		bool mRefreshInProgress;
		UINT32 mGCHandle;
	};

	/**	Interop class between C++ & CLR for GameObject. */
	class BS_SCR_BE_EXPORT ScriptGameObject : public ScriptObject<ScriptGameObject, ScriptGameObjectBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GameObject")

	private:
		ScriptGameObject(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static UINT64 internal_getInstanceId(ScriptGameObject* nativeInstance);
	};

	/** @} */
}