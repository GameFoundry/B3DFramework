//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		virtual HGameObject GetNativeHandle() const = 0;

		/**	Sets the internal native GameObject handle. */
		virtual void SetNativeHandle(const HGameObject& gameObject) = 0;

		/** Returns the managed version of this game object. */
		MonoObject* GetManagedInstance() const;

	protected:
		/**
		 * Makes the object reference the specific managed instance. Internally this allocates a GC handle that keeps a
		 * reference to the object and allows GetManagedInstance to retrieve the managed instance when requested. Should
		 * be called on initial creation and whenever the managed instance changes (e.g. after assembly refresh). This
		 * creates a strong reference to the managed instance, and you need to make sure to release it with
		 * freeManagedInstance() when no longer required.
		 */
		void SetManagedInstance(MonoObject* instance);

		/**
		 * Frees a managed instace assigned with setManagedInstance(). Should be called before the object is destroyed or
		 * when you changing the managed instance it points to (in order to release the previous instance).
		 */
		void FreeManagedInstance();

		u32 mGCHandle = 0;
	};

	/**	Interop class between C++ & CLR for GameObject. */
	class BS_SCR_BE_EXPORT ScriptGameObject : public ScriptObject<ScriptGameObject, ScriptGameObjectBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GameObject")

	private:
		ScriptGameObject(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static u64 InternalGetInstanceId(ScriptGameObject* nativeInstance);
		static void InternalGetUuid(ScriptGameObject* nativeInstance, UUID* uuid);
		static bool InternalGetIsDestroyed(ScriptGameObject* nativeInstance);
	};

	/** @} */
} // namespace bs
