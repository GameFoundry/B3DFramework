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
		virtual ~ScriptGameObjectBase() { }

		/**	Returns the internal native GameObject handle. */
		virtual HGameObject getNativeHandle() const = 0;

		/**	Sets the internal native GameObject handle. */
		virtual void setNativeHandle(const HGameObject& gameObject) = 0;

		/** @copydoc ScriptObjectBase::beginRefresh */
		virtual ScriptObjectBackup beginRefresh() override;

		/** @copydoc ScriptObjectBase::endRefresh */
		virtual void endRefresh(const ScriptObjectBackup& backupData) override;
	protected:
		bool mRefreshInProgress;
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