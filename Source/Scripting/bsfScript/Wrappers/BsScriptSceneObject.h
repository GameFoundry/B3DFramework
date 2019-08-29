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
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "SceneObject")

		/** @copydoc ScriptGameObjectBase::getNativeHandle */
		HGameObject getNativeHandle() const override { return static_object_cast<GameObject>(mSceneObject); }

		/** @copydoc ScriptGameObjectBase::setNativeHandle */
		void setNativeHandle(const HGameObject& gameObject) override;

		/**	Returns the native internal scene object. */
		HSceneObject getHandle() const { return mSceneObject; }

		/**	Checks is the scene object wrapped by the provided interop object destroyed. */
		static bool checkIfDestroyed(ScriptSceneObject* nativeInstance);

	private:
		friend class ScriptGameObjectManager;

		ScriptSceneObject(MonoObject* instance, const HSceneObject& sceneObject);

		/** @copydoc ScriptObjectBase::_onManagedInstanceDeleted */
		void _onManagedInstanceDeleted(bool assemblyRefresh) override;

		/** @copydoc ScriptObject::_createManagedInstance */
		MonoObject* _createManagedInstance(bool construct) override;

		/** @copydoc ScriptObjectBase::_clearManagedInstance */
		void _clearManagedInstance() override;

		/**	Triggered by the script game object manager when the handle this object is referencing is destroyed. */
		void _notifyDestroyed();

		HSceneObject mSceneObject;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, MonoString* name, UINT32 flags);

		static void internal_setName(ScriptSceneObject* nativeInstance, MonoString* name);
		static MonoString* internal_getName(ScriptSceneObject* nativeInstance);
		static void internal_setActive(ScriptSceneObject* nativeInstance, bool value);
		static bool internal_getActive(ScriptSceneObject* nativeInstance);
		static bool internal_hasFlag(ScriptSceneObject* nativeInstance, UINT32 flag);

		static void internal_setMobility(ScriptSceneObject* nativeInstance, int value);
		static int internal_getMobility(ScriptSceneObject* nativeInstance);

		static void internal_setParent(ScriptSceneObject* nativeInstance, MonoObject* parent);
		static MonoObject* internal_getParent(ScriptSceneObject* nativeInstance);
		static MonoObject* internal_getScene(ScriptSceneObject* nativeInstance);

		static void internal_getNumChildren(ScriptSceneObject* nativeInstance, UINT32* value);
		static MonoObject* internal_getChild(ScriptSceneObject* nativeInstance, UINT32 idx);
		static MonoObject* internal_findChild(ScriptSceneObject* nativeInstance, MonoString* name, bool recursive);
		static MonoArray* internal_findChildren(ScriptSceneObject* nativeInstance, MonoString* name, bool recursive);

		static void internal_getPosition(ScriptSceneObject* nativeInstance, Vector3* value);
		static void internal_getLocalPosition(ScriptSceneObject* nativeInstance, Vector3* value);
		static void internal_getRotation(ScriptSceneObject* nativeInstance, Quaternion* value);
		static void internal_getLocalRotation(ScriptSceneObject* nativeInstance, Quaternion* value);
		static void internal_getScale(ScriptSceneObject* nativeInstance, Vector3* value);
		static void internal_getLocalScale(ScriptSceneObject* nativeInstance, Vector3* value);

		static void internal_setPosition(ScriptSceneObject* nativeInstance, Vector3* value);
		static void internal_setLocalPosition(ScriptSceneObject* nativeInstance, Vector3* value);
		static void internal_setRotation(ScriptSceneObject* nativeInstance, Quaternion* value);
		static void internal_setLocalRotation(ScriptSceneObject* nativeInstance, Quaternion* value);
		static void internal_setLocalScale(ScriptSceneObject* nativeInstance, Vector3* value);

		static void internal_getLocalTransform(ScriptSceneObject* nativeInstance, Matrix4* value);
		static void internal_getWorldTransform(ScriptSceneObject* nativeInstance, Matrix4* value);
		static void internal_lookAt(ScriptSceneObject* nativeInstance, Vector3* direction, Vector3* up);
		static void internal_move(ScriptSceneObject* nativeInstance, Vector3* value);
		static void internal_moveLocal(ScriptSceneObject* nativeInstance, Vector3* value);
		static void internal_rotate(ScriptSceneObject* nativeInstance, Quaternion* value);
		static void internal_roll(ScriptSceneObject* nativeInstance, Radian* value);
		static void internal_yaw(ScriptSceneObject* nativeInstance, Radian* value);
		static void internal_pitch(ScriptSceneObject* nativeInstance, Radian* value);
		static void internal_setForward(ScriptSceneObject* nativeInstance, Vector3* value);
		static void internal_getForward(ScriptSceneObject* nativeInstance, Vector3* value);
		static void internal_getUp(ScriptSceneObject* nativeInstance, Vector3* value);
		static void internal_getRight(ScriptSceneObject* nativeInstance, Vector3* value);

		static void internal_destroy(ScriptSceneObject* nativeInstance, bool immediate);
	};

	/** @} */
}
