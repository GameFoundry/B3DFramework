//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptSceneObject.h"
#include "BsScriptGameObjectManager.h"
#include "BsScriptResourceManager.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "Scene/BsSceneObject.h"
#include "BsMonoUtil.h"

#include "Generated/BsScriptSceneInstance.generated.h"

namespace bs
{
	ScriptSceneObject::ScriptSceneObject(MonoObject* instance, const HSceneObject& sceneObject)
		:ScriptObject(instance), mSceneObject(sceneObject)
	{
		SetManagedInstance(instance);
	}

	void ScriptSceneObject::initRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptSceneObject::InternalCreateInstance);
		metaData.scriptClass->addInternalCall("Internal_GetName", (void*)&ScriptSceneObject::internal_getName);
		metaData.scriptClass->addInternalCall("Internal_SetName", (void*)&ScriptSceneObject::internal_setName);
		metaData.scriptClass->addInternalCall("Internal_GetActive", (void*)&ScriptSceneObject::internal_getActive);
		metaData.scriptClass->addInternalCall("Internal_SetActive", (void*)&ScriptSceneObject::internal_setActive);
		metaData.scriptClass->addInternalCall("Internal_HasFlag", (void*)&ScriptSceneObject::internal_hasFlag);

		metaData.scriptClass->addInternalCall("Internal_GetMobility", (void*)&ScriptSceneObject::internal_getMobility);
		metaData.scriptClass->addInternalCall("Internal_SetMobility", (void*)&ScriptSceneObject::internal_setMobility);
		metaData.scriptClass->addInternalCall("Internal_GetParent", (void*)&ScriptSceneObject::internal_getParent);
		metaData.scriptClass->addInternalCall("Internal_GetParent", (void*)&ScriptSceneObject::internal_getParent);
		metaData.scriptClass->addInternalCall("Internal_SetParent", (void*)&ScriptSceneObject::internal_setParent);
		metaData.scriptClass->addInternalCall("Internal_GetScene", (void*)&ScriptSceneObject::internal_getScene);
		metaData.scriptClass->addInternalCall("Internal_GetNumChildren", (void*)&ScriptSceneObject::internal_getNumChildren);
		metaData.scriptClass->addInternalCall("Internal_GetChild", (void*)&ScriptSceneObject::internal_getChild);
		metaData.scriptClass->addInternalCall("Internal_FindChild", (void*)&ScriptSceneObject::internal_findChild);
		metaData.scriptClass->addInternalCall("Internal_FindChildren", (void*)&ScriptSceneObject::internal_findChildren);

		metaData.scriptClass->addInternalCall("Internal_GetPosition", (void*)&ScriptSceneObject::internal_getPosition);
		metaData.scriptClass->addInternalCall("Internal_GetLocalPosition", (void*)&ScriptSceneObject::internal_getLocalPosition);
		metaData.scriptClass->addInternalCall("Internal_GetRotation", (void*)&ScriptSceneObject::internal_getRotation);
		metaData.scriptClass->addInternalCall("Internal_GetLocalRotation", (void*)&ScriptSceneObject::internal_getLocalRotation);
		metaData.scriptClass->addInternalCall("Internal_GetScale", (void*)&ScriptSceneObject::internal_getScale);
		metaData.scriptClass->addInternalCall("Internal_GetLocalScale", (void*)&ScriptSceneObject::internal_getLocalScale);

		metaData.scriptClass->addInternalCall("Internal_SetPosition", (void*)&ScriptSceneObject::internal_setPosition);
		metaData.scriptClass->addInternalCall("Internal_SetLocalPosition", (void*)&ScriptSceneObject::internal_setLocalPosition);
		metaData.scriptClass->addInternalCall("Internal_SetRotation", (void*)&ScriptSceneObject::internal_setRotation);
		metaData.scriptClass->addInternalCall("Internal_SetLocalRotation", (void*)&ScriptSceneObject::internal_setLocalRotation);
		metaData.scriptClass->addInternalCall("Internal_SetLocalScale", (void*)&ScriptSceneObject::internal_setLocalScale);

		metaData.scriptClass->addInternalCall("Internal_GetLocalTransform", (void*)&ScriptSceneObject::internal_getLocalTransform);
		metaData.scriptClass->addInternalCall("Internal_GetWorldTransform", (void*)&ScriptSceneObject::internal_getWorldTransform);
		metaData.scriptClass->addInternalCall("Internal_LookAt", (void*)&ScriptSceneObject::internal_lookAt);
		metaData.scriptClass->addInternalCall("Internal_Move", (void*)&ScriptSceneObject::internal_move);
		metaData.scriptClass->addInternalCall("Internal_MoveLocal", (void*)&ScriptSceneObject::internal_moveLocal);
		metaData.scriptClass->addInternalCall("Internal_Rotate", (void*)&ScriptSceneObject::internal_rotate);
		metaData.scriptClass->addInternalCall("Internal_Roll", (void*)&ScriptSceneObject::internal_roll);
		metaData.scriptClass->addInternalCall("Internal_Yaw", (void*)&ScriptSceneObject::internal_yaw);
		metaData.scriptClass->addInternalCall("Internal_Pitch", (void*)&ScriptSceneObject::internal_pitch);
		metaData.scriptClass->addInternalCall("Internal_SetForward", (void*)&ScriptSceneObject::internal_setForward);
		metaData.scriptClass->addInternalCall("Internal_GetForward", (void*)&ScriptSceneObject::internal_getForward);
		metaData.scriptClass->addInternalCall("Internal_GetUp", (void*)&ScriptSceneObject::internal_getUp);
		metaData.scriptClass->addInternalCall("Internal_GetRight", (void*)&ScriptSceneObject::internal_getRight);

		metaData.scriptClass->addInternalCall("Internal_Destroy", (void*)&ScriptSceneObject::internal_destroy);
	}

	void ScriptSceneObject::InternalCreateInstance(MonoObject* instance, MonoString* name, UINT32 flags)
	{
		HSceneObject sceneObject = SceneObject::Create(MonoUtil::monoToString(name), flags);

		ScriptGameObjectManager::Instance().createScriptSceneObject(instance, sceneObject);
	}

	void ScriptSceneObject::InternalSetName(ScriptSceneObject* nativeInstance, MonoString* name)
	{
		if (checkIfDestroyed(nativeInstance))
			return;

		nativeInstance->mSceneObject->SetName(MonoUtil::monoToString(name));
	}

	MonoString* ScriptSceneObject::InternalGetName(ScriptSceneObject* nativeInstance)
	{
		if (checkIfDestroyed(nativeInstance))
			return nullptr;

		String name = nativeInstance->mSceneObject->GetName();
		return MonoUtil::stringToMono(name);
	}

	void ScriptSceneObject::InternalSetActive(ScriptSceneObject* nativeInstance, bool value)
	{
		if (checkIfDestroyed(nativeInstance))
			return;

		nativeInstance->mSceneObject->SetActive(value);
	}

	bool ScriptSceneObject::InternalGetActive(ScriptSceneObject* nativeInstance)
	{
		if (checkIfDestroyed(nativeInstance))
			return false;

		return nativeInstance->mSceneObject->GetActive(true);
	}

	bool ScriptSceneObject::InternalHasFlag(ScriptSceneObject* nativeInstance, bs::UINT32 flag)
	{
		if (checkIfDestroyed(nativeInstance))
			return false;

		return nativeInstance->mSceneObject->HasFlag(flag);
	}

	void ScriptSceneObject::InternalSetMobility(ScriptSceneObject* nativeInstance, int value)
	{
		if (checkIfDestroyed(nativeInstance))
			return;

		nativeInstance->mSceneObject->SetMobility((ObjectMobility)value);
	}

	int ScriptSceneObject::InternalGetMobility(ScriptSceneObject* nativeInstance)
	{
		if (checkIfDestroyed(nativeInstance))
			return false;

		return (int)nativeInstance->mSceneObject->GetMobility();
	}
	void ScriptSceneObject::InternalSetParent(ScriptSceneObject* nativeInstance, MonoObject* parent)
	{
		if (checkIfDestroyed(nativeInstance))
			return;

		ScriptSceneObject* parentScriptSO = ScriptSceneObject::toNative(parent);

		nativeInstance->mSceneObject->SetParent(parentScriptSO->mSceneObject);
	}

	MonoObject* ScriptSceneObject::InternalGetParent(ScriptSceneObject* nativeInstance)
	{
		if (checkIfDestroyed(nativeInstance))
			return nullptr;

		HSceneObject parent = nativeInstance->mSceneObject->GetParent();
		if (parent != nullptr)
		{
			ScriptSceneObject* parentScriptSO = ScriptGameObjectManager::Instance().getOrCreateScriptSceneObject(parent);

			return parentScriptSO->GetManagedInstance();
		}

		return nullptr;
	}

	MonoObject* ScriptSceneObject::InternalGetScene(ScriptSceneObject* nativeInstance)
	{
		if (checkIfDestroyed(nativeInstance))
			return nullptr;

		return ScriptSceneInstance::Create(nativeInstance->mSceneObject->GetScene());
	}

	void ScriptSceneObject::InternalGetNumChildren(ScriptSceneObject* nativeInstance, UINT32* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetNumChildren();
		else
			*value = 0;
	}

	MonoObject* ScriptSceneObject::InternalGetChild(ScriptSceneObject* nativeInstance, UINT32 idx)
	{
		if (checkIfDestroyed(nativeInstance))
			return nullptr;

		UINT32 numChildren = nativeInstance->mSceneObject->GetNumChildren();
		if(idx >= numChildren)
		{
			BS_LOG(Warning, Scene, "Attempting to access an out of range SceneObject child. Provided index: \"{0}\". "
				"Valid range: [0, {1})", idx, numChildren);
			return nullptr;
		}

		HSceneObject childSO = nativeInstance->mSceneObject->GetChild(idx);
		ScriptSceneObject* childScriptSO = ScriptGameObjectManager::Instance().getOrCreateScriptSceneObject(childSO);

		return childScriptSO->GetManagedInstance();
	}

	MonoObject* ScriptSceneObject::InternalFindChild(ScriptSceneObject* nativeInstance, MonoString* name, bool recursive)
	{
		if (checkIfDestroyed(nativeInstance))
			return nullptr;

		String nativeName = MonoUtil::monoToString(name);
		HSceneObject child = nativeInstance->GetHandle()->findChild(nativeName, recursive);

		if (child == nullptr)
			return nullptr;

		ScriptSceneObject* scriptChild = ScriptGameObjectManager::Instance().getOrCreateScriptSceneObject(child);
		return scriptChild->GetManagedInstance();
	}

	MonoArray* ScriptSceneObject::InternalFindChildren(ScriptSceneObject* nativeInstance, MonoString* name, bool recursive)
	{
		if (checkIfDestroyed(nativeInstance))
		{
			ScriptArray emptyArray = ScriptArray::create<ScriptSceneObject>(0);
			return emptyArray.getInternal();
		}

		String nativeName = MonoUtil::monoToString(name);
		Vector<HSceneObject> children = nativeInstance->GetHandle()->findChildren(nativeName, recursive);

		UINT32 numChildren = (UINT32)children.size();
		ScriptArray output = ScriptArray::create<ScriptSceneObject>(numChildren);

		for (UINT32 i = 0; i < numChildren; i++)
		{
			HSceneObject child = children[i];
			ScriptSceneObject* scriptChild = ScriptGameObjectManager::Instance().getOrCreateScriptSceneObject(child);

			output.Set(i, scriptChild->GetManagedInstance());
		}

		return output.getInternal();
	}

	void ScriptSceneObject::InternalGetPosition(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetTransform().GetPosition();
		else
			*value = Vector3(BsZero);
	}

	void ScriptSceneObject::InternalGetLocalPosition(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetLocalTransform().GetPosition();
		else
			*value = Vector3(BsZero);
	}

	void ScriptSceneObject::InternalGetRotation(ScriptSceneObject* nativeInstance, Quaternion* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetTransform().GetRotation();
		else
			*value = Quaternion(BsIdentity);
	}

	void ScriptSceneObject::InternalGetLocalRotation(ScriptSceneObject* nativeInstance, Quaternion* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetLocalTransform().GetRotation();
		else
			*value = Quaternion(BsIdentity);
	}

	void ScriptSceneObject::InternalGetScale(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetTransform().GetScale();
		else
			*value = Vector3(Vector3::ONE);
	}

	void ScriptSceneObject::InternalGetLocalScale(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetLocalTransform().GetScale();
		else
			*value = Vector3(Vector3::ONE);
	}

	void ScriptSceneObject::InternalSetPosition(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->SetWorldPosition(*value);
	}

	void ScriptSceneObject::InternalSetLocalPosition(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->SetPosition(*value);
	}

	void ScriptSceneObject::InternalSetRotation(ScriptSceneObject* nativeInstance, Quaternion* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->SetWorldRotation(*value);
	}

	void ScriptSceneObject::InternalSetLocalRotation(ScriptSceneObject* nativeInstance, Quaternion* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->SetRotation(*value);
	}

	void ScriptSceneObject::InternalSetLocalScale(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->SetScale(*value);
	}

	void ScriptSceneObject::InternalGetLocalTransform(ScriptSceneObject* nativeInstance, Matrix4* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetLocalMatrix();
		else
			*value = Matrix4(BsIdentity);
	}

	void ScriptSceneObject::InternalGetWorldTransform(ScriptSceneObject* nativeInstance, Matrix4* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetWorldMatrix();
		else
			*value = Matrix4(BsIdentity);
	}

	void ScriptSceneObject::InternalLookAt(ScriptSceneObject* nativeInstance, Vector3* direction, Vector3* up)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->lookAt(*direction, *up);
	}

	void ScriptSceneObject::InternalMove(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->move(*value);
	}

	void ScriptSceneObject::InternalMoveLocal(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->moveRelative(*value);
	}

	void ScriptSceneObject::InternalRotate(ScriptSceneObject* nativeInstance, Quaternion* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->rotate(*value);
	}

	void ScriptSceneObject::InternalRoll(ScriptSceneObject* nativeInstance, Radian* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->roll(*value);
	}

	void ScriptSceneObject::InternalYaw(ScriptSceneObject* nativeInstance, Radian* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->yaw(*value);
	}

	void ScriptSceneObject::InternalPitch(ScriptSceneObject* nativeInstance, Radian* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->pitch(*value);
	}

	void ScriptSceneObject::InternalSetForward(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->SetForward(*value);
	}

	void ScriptSceneObject::InternalGetForward(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetTransform().getForward();
		else
			*value = Vector3(-Vector3::UNIT_Z);
	}

	void ScriptSceneObject::InternalGetUp(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetTransform().getUp();
		else
			*value = Vector3(Vector3::UNIT_Y);
	}

	void ScriptSceneObject::InternalGetRight(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetTransform().getRight();
		else
			*value = Vector3(Vector3::UNIT_X);
	}

	void ScriptSceneObject::InternalDestroy(ScriptSceneObject* nativeInstance, bool immediate)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->Destroy(immediate);
	}

	bool ScriptSceneObject::CheckIfDestroyed(ScriptSceneObject* nativeInstance)
	{
		if (nativeInstance->mSceneObject.isDestroyed())
		{
			BS_LOG(Warning, Scene, "Trying to access a destroyed SceneObject with instance ID: {0}", +
				nativeInstance->mSceneObject.getInstanceId());
			return true;
		}

		return false;
	}

	void ScriptSceneObject::OnManagedInstanceDeletedInternal(bool assemblyRefresh)
	{
		if (!assemblyRefresh || mSceneObject.isDestroyed(true))
			ScriptGameObjectManager::Instance().destroyScriptSceneObject(this);
		else
			freeManagedInstance();
	}

	MonoObject* ScriptSceneObject::CreateManagedInstanceInternal(bool construct)
	{
		MonoObject* managedInstance = metaData.scriptClass->createInstance(construct);
		setManagedInstance(managedInstance);

		return managedInstance;
	}

	void ScriptSceneObject::ClearManagedInstanceInternal()
	{
		freeManagedInstance();
	}

	void ScriptSceneObject::NotifyDestroyedInternal()
	{
		freeManagedInstance();
	}

	void ScriptSceneObject::SetNativeHandle(const HGameObject& gameObject)
	{
		mSceneObject = static_object_cast<SceneObject>(gameObject);
	}
}
