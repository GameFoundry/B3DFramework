//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptGameObject.h"
#include "BsScriptObject.h"
#include "Text/BsFont.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for SceneObject. */
	class BS_SCR_BE_EXPORT ScriptSceneObject : public ScriptObject<ScriptSceneObject, ScriptGameObjectBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "SceneObject")

		/** @copydoc ScriptGameObjectBase::getNativeHandle */
		HGameObject GetNativeHandle() const override { return static_object_cast<GameObject>(mSceneObject); }

		/** @copydoc ScriptGameObjectBase::setNativeHandle */
		void SetNativeHandle(const HGameObject& gameObject) override;

		/**	Returns the native internal scene object. */
		HSceneObject GetHandle() const { return mSceneObject; }

		/**	Checks is the scene object wrapped by the provided interop object destroyed. */
		static bool CheckIfDestroyed(ScriptSceneObject* nativeInstance);

	private:
		friend class ScriptGameObjectManager;

		ScriptSceneObject(MonoObject* instance, const HSceneObject& sceneObject);

		/** @copydoc ScriptObjectBase::_onManagedInstanceDeleted */
		void OnManagedInstanceDeletedInternal(bool assemblyRefresh) override;

		/** @copydoc ScriptObject::_createManagedInstance */
		MonoObject* CreateManagedInstanceInternal(bool construct) override;

		/** @copydoc ScriptObjectBase::_clearManagedInstance */
		void ClearManagedInstanceInternal() override;

		/**	Triggered by the script game object manager when the handle this object is referencing is destroyed. */
		void NotifyDestroyedInternal();

		HSceneObject mSceneObject;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, MonoString* name, u32 flags);

		static void InternalSetName(ScriptSceneObject* nativeInstance, MonoString* name);
		static MonoString* InternalGetName(ScriptSceneObject* nativeInstance);
		static void InternalSetActive(ScriptSceneObject* nativeInstance, bool value);
		static bool InternalGetActive(ScriptSceneObject* nativeInstance);
		static bool InternalHasFlag(ScriptSceneObject* nativeInstance, u32 flag);

		static void InternalSetMobility(ScriptSceneObject* nativeInstance, int value);
		static int InternalGetMobility(ScriptSceneObject* nativeInstance);

		static void InternalSetParent(ScriptSceneObject* nativeInstance, MonoObject* parent);
		static MonoObject* InternalGetParent(ScriptSceneObject* nativeInstance);
		static MonoObject* InternalGetScene(ScriptSceneObject* nativeInstance);

		static void InternalGetNumChildren(ScriptSceneObject* nativeInstance, u32* value);
		static MonoObject* InternalGetChild(ScriptSceneObject* nativeInstance, u32 idx);
		static MonoObject* InternalFindChild(ScriptSceneObject* nativeInstance, MonoString* name, bool recursive);
		static MonoArray* InternalFindChildren(ScriptSceneObject* nativeInstance, MonoString* name, bool recursive);

		static void InternalGetPosition(ScriptSceneObject* nativeInstance, Vector3* value);
		static void InternalGetLocalPosition(ScriptSceneObject* nativeInstance, Vector3* value);
		static void InternalGetRotation(ScriptSceneObject* nativeInstance, Quaternion* value);
		static void InternalGetLocalRotation(ScriptSceneObject* nativeInstance, Quaternion* value);
		static void InternalGetScale(ScriptSceneObject* nativeInstance, Vector3* value);
		static void InternalGetLocalScale(ScriptSceneObject* nativeInstance, Vector3* value);

		static void InternalSetPosition(ScriptSceneObject* nativeInstance, Vector3* value);
		static void InternalSetLocalPosition(ScriptSceneObject* nativeInstance, Vector3* value);
		static void InternalSetRotation(ScriptSceneObject* nativeInstance, Quaternion* value);
		static void InternalSetLocalRotation(ScriptSceneObject* nativeInstance, Quaternion* value);
		static void InternalSetLocalScale(ScriptSceneObject* nativeInstance, Vector3* value);

		static void InternalGetLocalTransform(ScriptSceneObject* nativeInstance, Matrix4* value);
		static void InternalGetWorldTransform(ScriptSceneObject* nativeInstance, Matrix4* value);
		static void InternalLookAt(ScriptSceneObject* nativeInstance, Vector3* direction, Vector3* up);
		static void InternalMove(ScriptSceneObject* nativeInstance, Vector3* value);
		static void InternalMoveLocal(ScriptSceneObject* nativeInstance, Vector3* value);
		static void InternalRotate(ScriptSceneObject* nativeInstance, Quaternion* value);
		static void InternalRoll(ScriptSceneObject* nativeInstance, Radian* value);
		static void InternalYaw(ScriptSceneObject* nativeInstance, Radian* value);
		static void InternalPitch(ScriptSceneObject* nativeInstance, Radian* value);
		static void InternalSetForward(ScriptSceneObject* nativeInstance, Vector3* value);
		static void InternalGetForward(ScriptSceneObject* nativeInstance, Vector3* value);
		static void InternalGetUp(ScriptSceneObject* nativeInstance, Vector3* value);
		static void InternalGetRight(ScriptSceneObject* nativeInstance, Vector3* value);

		static void InternalDestroy(ScriptSceneObject* nativeInstance, bool immediate);
	};

	/** @} */
} // namespace bs
