//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptSceneObject.h"
#include "BsScriptGameObjectManager.h"
#include "BsScriptResourceManager.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsSceneObject.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptSceneObject::ScriptSceneObject(MonoObject* instance, const HSceneObject& sceneObject)
		:ScriptObject(instance), mSceneObject(sceneObject)
	{
		mManagedHandle = MonoUtil::newGCHandle(instance);
	}

	void ScriptSceneObject::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptSceneObject::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetName", &ScriptSceneObject::internal_getName);
		metaData.scriptClass->addInternalCall("Internal_SetName", &ScriptSceneObject::internal_setName);
		metaData.scriptClass->addInternalCall("Internal_GetActive", &ScriptSceneObject::internal_getActive);
		metaData.scriptClass->addInternalCall("Internal_SetActive", &ScriptSceneObject::internal_setActive);
		metaData.scriptClass->addInternalCall("Internal_GetParent", &ScriptSceneObject::internal_getParent);
		metaData.scriptClass->addInternalCall("Internal_SetParent", &ScriptSceneObject::internal_setParent);
		metaData.scriptClass->addInternalCall("Internal_GetNumChildren", &ScriptSceneObject::internal_getNumChildren);
		metaData.scriptClass->addInternalCall("Internal_GetChild", &ScriptSceneObject::internal_getChild);
		metaData.scriptClass->addInternalCall("Internal_FindChild", &ScriptSceneObject::internal_findChild);
		metaData.scriptClass->addInternalCall("Internal_FindChildren", &ScriptSceneObject::internal_findChildren);

		metaData.scriptClass->addInternalCall("Internal_GetPosition", &ScriptSceneObject::internal_getPosition);
		metaData.scriptClass->addInternalCall("Internal_GetLocalPosition", &ScriptSceneObject::internal_getLocalPosition);
		metaData.scriptClass->addInternalCall("Internal_GetRotation", &ScriptSceneObject::internal_getRotation);
		metaData.scriptClass->addInternalCall("Internal_GetLocalRotation", &ScriptSceneObject::internal_getLocalRotation);
		metaData.scriptClass->addInternalCall("Internal_GetScale", &ScriptSceneObject::internal_getScale);
		metaData.scriptClass->addInternalCall("Internal_GetLocalScale", &ScriptSceneObject::internal_getLocalScale);

		metaData.scriptClass->addInternalCall("Internal_SetPosition", &ScriptSceneObject::internal_setPosition);
		metaData.scriptClass->addInternalCall("Internal_SetLocalPosition", &ScriptSceneObject::internal_setLocalPosition);
		metaData.scriptClass->addInternalCall("Internal_SetRotation", &ScriptSceneObject::internal_setRotation);
		metaData.scriptClass->addInternalCall("Internal_SetLocalRotation", &ScriptSceneObject::internal_setLocalRotation);
		metaData.scriptClass->addInternalCall("Internal_SetLocalScale", &ScriptSceneObject::internal_setLocalScale);

		metaData.scriptClass->addInternalCall("Internal_GetLocalTransform", &ScriptSceneObject::internal_getLocalTransform);
		metaData.scriptClass->addInternalCall("Internal_GetWorldTransform", &ScriptSceneObject::internal_getWorldTransform);
		metaData.scriptClass->addInternalCall("Internal_LookAt", &ScriptSceneObject::internal_lookAt);
		metaData.scriptClass->addInternalCall("Internal_Move", &ScriptSceneObject::internal_move);
		metaData.scriptClass->addInternalCall("Internal_MoveLocal", &ScriptSceneObject::internal_moveLocal);
		metaData.scriptClass->addInternalCall("Internal_Rotate", &ScriptSceneObject::internal_rotate);
		metaData.scriptClass->addInternalCall("Internal_Roll", &ScriptSceneObject::internal_roll);
		metaData.scriptClass->addInternalCall("Internal_Yaw", &ScriptSceneObject::internal_yaw);
		metaData.scriptClass->addInternalCall("Internal_Pitch", &ScriptSceneObject::internal_pitch);
		metaData.scriptClass->addInternalCall("Internal_SetForward", &ScriptSceneObject::internal_setForward);
		metaData.scriptClass->addInternalCall("Internal_GetForward", &ScriptSceneObject::internal_getForward);
		metaData.scriptClass->addInternalCall("Internal_GetUp", &ScriptSceneObject::internal_getUp);
		metaData.scriptClass->addInternalCall("Internal_GetRight", &ScriptSceneObject::internal_getRight);

		metaData.scriptClass->addInternalCall("Internal_Destroy", &ScriptSceneObject::internal_destroy);
	}

	void ScriptSceneObject::internal_createInstance(MonoObject* instance, MonoString* name, UINT32 flags)
	{
		HSceneObject sceneObject = SceneObject::create(toString(MonoUtil::monoToWString(name)), flags);

		ScriptGameObjectManager::instance().createScriptSceneObject(instance, sceneObject);
	}

	void ScriptSceneObject::internal_setName(ScriptSceneObject* nativeInstance, MonoString* name)
	{
		if (checkIfDestroyed(nativeInstance))
			return;

		nativeInstance->mSceneObject->setName(MonoUtil::monoToString(name));
	}

	MonoString* ScriptSceneObject::internal_getName(ScriptSceneObject* nativeInstance)
	{
		if (checkIfDestroyed(nativeInstance))
			return nullptr;

		String name = nativeInstance->mSceneObject->getName();
		return MonoUtil::stringToMono(name);
	}

	void ScriptSceneObject::internal_setActive(ScriptSceneObject* nativeInstance, bool value)
	{
		if (checkIfDestroyed(nativeInstance))
			return;

		nativeInstance->mSceneObject->setActive(value);
	}

	bool ScriptSceneObject::internal_getActive(ScriptSceneObject* nativeInstance)
	{
		if (checkIfDestroyed(nativeInstance))
			return false;

		return nativeInstance->mSceneObject->getActive(true);
	}

	void ScriptSceneObject::internal_setParent(ScriptSceneObject* nativeInstance, MonoObject* parent)
	{
		if (checkIfDestroyed(nativeInstance))
			return;

		ScriptSceneObject* parentScriptSO = ScriptSceneObject::toNative(parent);

		nativeInstance->mSceneObject->setParent(parentScriptSO->mSceneObject);
	}

	MonoObject* ScriptSceneObject::internal_getParent(ScriptSceneObject* nativeInstance)
	{
		if (checkIfDestroyed(nativeInstance))
			return nullptr;

		HSceneObject parent = nativeInstance->mSceneObject->getParent();
		if (parent != nullptr)
		{
			ScriptSceneObject* parentScriptSO = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(parent);

			return parentScriptSO->getManagedInstance();
		}

		return nullptr;
	}

	void ScriptSceneObject::internal_getNumChildren(ScriptSceneObject* nativeInstance, UINT32* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->getNumChildren();
		else
			*value = 0;
	}

	MonoObject* ScriptSceneObject::internal_getChild(ScriptSceneObject* nativeInstance, UINT32 idx)
	{
		if (checkIfDestroyed(nativeInstance))
			return nullptr;

		UINT32 numChildren = nativeInstance->mSceneObject->getNumChildren();
		if(idx >= numChildren)
		{
			LOGWRN("Attempting to access an out of range SceneObject child. Provided index: \"" + toString(idx)
				+ "\". Valid range: [0 .. " + toString(numChildren) + ")");
			return nullptr;
		}

		HSceneObject childSO = nativeInstance->mSceneObject->getChild(idx);
		ScriptSceneObject* childScriptSO = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(childSO);

		return childScriptSO->getManagedInstance();
	}

	MonoObject* ScriptSceneObject::internal_findChild(ScriptSceneObject* nativeInstance, MonoString* name, bool recursive)
	{
		if (checkIfDestroyed(nativeInstance))
			return nullptr;

		String nativeName = MonoUtil::monoToString(name);
		HSceneObject child = nativeInstance->getNativeSceneObject()->findChild(nativeName, recursive);

		if (child == nullptr)
			return nullptr;

		ScriptSceneObject* scriptChild = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(child);
		return scriptChild->getManagedInstance();
	}

	MonoArray* ScriptSceneObject::internal_findChildren(ScriptSceneObject* nativeInstance, MonoString* name, bool recursive)
	{
		if (checkIfDestroyed(nativeInstance))
		{
			ScriptArray emptyArray = ScriptArray::create<ScriptSceneObject>(0);
			return emptyArray.getInternal();
		}

		String nativeName = MonoUtil::monoToString(name);
		Vector<HSceneObject> children = nativeInstance->getNativeSceneObject()->findChildren(nativeName, recursive);

		UINT32 numChildren = (UINT32)children.size();
		ScriptArray output = ScriptArray::create<ScriptSceneObject>(numChildren);

		for (UINT32 i = 0; i < numChildren; i++)
		{
			HSceneObject child = children[i];
			ScriptSceneObject* scriptChild = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(child);

			output.set(i, scriptChild->getManagedInstance());
		}

		return output.getInternal();
	}

	void ScriptSceneObject::internal_getPosition(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->getWorldPosition();
		else
			*value = Vector3(BsZero);
	}

	void ScriptSceneObject::internal_getLocalPosition(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->getPosition();
		else
			*value = Vector3(BsZero);
	}

	void ScriptSceneObject::internal_getRotation(ScriptSceneObject* nativeInstance, Quaternion* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->getWorldRotation();
		else
			*value = Quaternion(BsIdentity);
	}

	void ScriptSceneObject::internal_getLocalRotation(ScriptSceneObject* nativeInstance, Quaternion* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->getRotation();
		else
			*value = Quaternion(BsIdentity);
	}

	void ScriptSceneObject::internal_getScale(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->getWorldScale();
		else
			*value = Vector3(Vector3::ONE);
	}

	void ScriptSceneObject::internal_getLocalScale(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->getScale();
		else
			*value = Vector3(Vector3::ONE);
	}

	void ScriptSceneObject::internal_setPosition(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->setWorldPosition(*value);
	}

	void ScriptSceneObject::internal_setLocalPosition(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->setPosition(*value);
	}

	void ScriptSceneObject::internal_setRotation(ScriptSceneObject* nativeInstance, Quaternion* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->setWorldRotation(*value);
	}

	void ScriptSceneObject::internal_setLocalRotation(ScriptSceneObject* nativeInstance, Quaternion* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->setRotation(*value);
	}

	void ScriptSceneObject::internal_setLocalScale(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->setScale(*value);
	}

	void ScriptSceneObject::internal_getLocalTransform(ScriptSceneObject* nativeInstance, Matrix4* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->getLocalTfrm();
		else
			*value = Matrix4();
	}

	void ScriptSceneObject::internal_getWorldTransform(ScriptSceneObject* nativeInstance, Matrix4* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->getWorldTfrm();
		else
			*value = Matrix4();
	}

	void ScriptSceneObject::internal_lookAt(ScriptSceneObject* nativeInstance, Vector3* direction, Vector3* up)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->lookAt(*direction, *up);
	}

	void ScriptSceneObject::internal_move(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->move(*value);
	}

	void ScriptSceneObject::internal_moveLocal(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->moveRelative(*value);
	}

	void ScriptSceneObject::internal_rotate(ScriptSceneObject* nativeInstance, Quaternion* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->rotate(*value);
	}

	void ScriptSceneObject::internal_roll(ScriptSceneObject* nativeInstance, Radian* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->roll(*value);
	}

	void ScriptSceneObject::internal_yaw(ScriptSceneObject* nativeInstance, Radian* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->yaw(*value);
	}

	void ScriptSceneObject::internal_pitch(ScriptSceneObject* nativeInstance, Radian* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->pitch(*value);
	}

	void ScriptSceneObject::internal_setForward(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->setForward(*value);
	}

	void ScriptSceneObject::internal_getForward(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->getForward();
		else
			*value = Vector3(-Vector3::UNIT_Z);
	}

	void ScriptSceneObject::internal_getUp(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->getUp();
		else
			*value = Vector3(Vector3::UNIT_Y);
	}

	void ScriptSceneObject::internal_getRight(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->getRight();
		else
			*value = Vector3(Vector3::UNIT_X);
	}

	void ScriptSceneObject::internal_destroy(ScriptSceneObject* nativeInstance, bool immediate)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->destroy(immediate);
	}

	bool ScriptSceneObject::checkIfDestroyed(ScriptSceneObject* nativeInstance)
	{
		if (nativeInstance->mSceneObject.isDestroyed())
		{
			LOGWRN("Trying to access a destroyed SceneObject with instance ID: " + toString(nativeInstance->mSceneObject.getInstanceId()));
			return true;
		}

		return false;
	}

	void ScriptSceneObject::_onManagedInstanceDeleted()
	{
		mManagedInstance = nullptr;

		if (!mRefreshInProgress)
			ScriptGameObjectManager::instance().destroyScriptSceneObject(this);
		else
		{
			MonoUtil::freeGCHandle(mManagedHandle);
			mManagedHandle = 0;
		}
	}

	MonoObject* ScriptSceneObject::_createManagedInstance(bool construct)
	{
		MonoObject* managedInstance = metaData.scriptClass->createInstance(construct);
		mManagedHandle = MonoUtil::newGCHandle(managedInstance);

		return managedInstance;
	}

	void ScriptSceneObject::_notifyDestroyed()
	{
		MonoUtil::freeGCHandle(mManagedHandle);
		mManagedHandle = 0;
	}

	void ScriptSceneObject::setNativeHandle(const HGameObject& gameObject)
	{
		mSceneObject = static_object_cast<SceneObject>(gameObject);
	}
}