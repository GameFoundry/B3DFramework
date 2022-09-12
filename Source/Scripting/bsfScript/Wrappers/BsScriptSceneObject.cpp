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
		setManagedInstance(instance);
	}

	void ScriptSceneObject::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptSceneObject::internal_createInstance);
		metaData.scriptClass->AddInternalCall("Internal_GetName", (void*)&ScriptSceneObject::internal_getName);
		metaData.scriptClass->AddInternalCall("Internal_SetName", (void*)&ScriptSceneObject::internal_setName);
		metaData.scriptClass->AddInternalCall("Internal_GetActive", (void*)&ScriptSceneObject::internal_getActive);
		metaData.scriptClass->AddInternalCall("Internal_SetActive", (void*)&ScriptSceneObject::internal_setActive);
		metaData.scriptClass->AddInternalCall("Internal_HasFlag", (void*)&ScriptSceneObject::internal_hasFlag);

		metaData.scriptClass->AddInternalCall("Internal_GetMobility", (void*)&ScriptSceneObject::internal_getMobility);
		metaData.scriptClass->AddInternalCall("Internal_SetMobility", (void*)&ScriptSceneObject::internal_setMobility);
		metaData.scriptClass->AddInternalCall("Internal_GetParent", (void*)&ScriptSceneObject::internal_getParent);
		metaData.scriptClass->AddInternalCall("Internal_GetParent", (void*)&ScriptSceneObject::internal_getParent);
		metaData.scriptClass->AddInternalCall("Internal_SetParent", (void*)&ScriptSceneObject::internal_setParent);
		metaData.scriptClass->AddInternalCall("Internal_GetScene", (void*)&ScriptSceneObject::internal_getScene);
		metaData.scriptClass->AddInternalCall("Internal_GetNumChildren", (void*)&ScriptSceneObject::internal_getNumChildren);
		metaData.scriptClass->AddInternalCall("Internal_GetChild", (void*)&ScriptSceneObject::internal_getChild);
		metaData.scriptClass->AddInternalCall("Internal_FindChild", (void*)&ScriptSceneObject::internal_findChild);
		metaData.scriptClass->AddInternalCall("Internal_FindChildren", (void*)&ScriptSceneObject::internal_findChildren);

		metaData.scriptClass->AddInternalCall("Internal_GetPosition", (void*)&ScriptSceneObject::internal_getPosition);
		metaData.scriptClass->AddInternalCall("Internal_GetLocalPosition", (void*)&ScriptSceneObject::internal_getLocalPosition);
		metaData.scriptClass->AddInternalCall("Internal_GetRotation", (void*)&ScriptSceneObject::internal_getRotation);
		metaData.scriptClass->AddInternalCall("Internal_GetLocalRotation", (void*)&ScriptSceneObject::internal_getLocalRotation);
		metaData.scriptClass->AddInternalCall("Internal_GetScale", (void*)&ScriptSceneObject::internal_getScale);
		metaData.scriptClass->AddInternalCall("Internal_GetLocalScale", (void*)&ScriptSceneObject::internal_getLocalScale);

		metaData.scriptClass->AddInternalCall("Internal_SetPosition", (void*)&ScriptSceneObject::internal_setPosition);
		metaData.scriptClass->AddInternalCall("Internal_SetLocalPosition", (void*)&ScriptSceneObject::internal_setLocalPosition);
		metaData.scriptClass->AddInternalCall("Internal_SetRotation", (void*)&ScriptSceneObject::internal_setRotation);
		metaData.scriptClass->AddInternalCall("Internal_SetLocalRotation", (void*)&ScriptSceneObject::internal_setLocalRotation);
		metaData.scriptClass->AddInternalCall("Internal_SetLocalScale", (void*)&ScriptSceneObject::internal_setLocalScale);

		metaData.scriptClass->AddInternalCall("Internal_GetLocalTransform", (void*)&ScriptSceneObject::internal_getLocalTransform);
		metaData.scriptClass->AddInternalCall("Internal_GetWorldTransform", (void*)&ScriptSceneObject::internal_getWorldTransform);
		metaData.scriptClass->AddInternalCall("Internal_LookAt", (void*)&ScriptSceneObject::internal_lookAt);
		metaData.scriptClass->AddInternalCall("Internal_Move", (void*)&ScriptSceneObject::internal_move);
		metaData.scriptClass->AddInternalCall("Internal_MoveLocal", (void*)&ScriptSceneObject::internal_moveLocal);
		metaData.scriptClass->AddInternalCall("Internal_Rotate", (void*)&ScriptSceneObject::internal_rotate);
		metaData.scriptClass->AddInternalCall("Internal_Roll", (void*)&ScriptSceneObject::internal_roll);
		metaData.scriptClass->AddInternalCall("Internal_Yaw", (void*)&ScriptSceneObject::internal_yaw);
		metaData.scriptClass->AddInternalCall("Internal_Pitch", (void*)&ScriptSceneObject::internal_pitch);
		metaData.scriptClass->AddInternalCall("Internal_SetForward", (void*)&ScriptSceneObject::internal_setForward);
		metaData.scriptClass->AddInternalCall("Internal_GetForward", (void*)&ScriptSceneObject::internal_getForward);
		metaData.scriptClass->AddInternalCall("Internal_GetUp", (void*)&ScriptSceneObject::internal_getUp);
		metaData.scriptClass->AddInternalCall("Internal_GetRight", (void*)&ScriptSceneObject::internal_getRight);

		metaData.scriptClass->AddInternalCall("Internal_Destroy", (void*)&ScriptSceneObject::internal_destroy);
	}

	void ScriptSceneObject::internal_createInstance(MonoObject* instance, MonoString* name, UINT32 flags)
	{
		HSceneObject sceneObject = SceneObject::create(MonoUtil::monoToString(name), flags);

		ScriptGameObjectManager::instance().CreateScriptSceneObject(instance, sceneObject);
	}

	void ScriptSceneObject::internal_setName(ScriptSceneObject* nativeInstance, MonoString* name)
	{
		if (checkIfDestroyed(nativeInstance))
			return;

		nativeInstance->mSceneObject->SetName(MonoUtil::monoToString(name));
	}

	MonoString* ScriptSceneObject::internal_getName(ScriptSceneObject* nativeInstance)
	{
		if (checkIfDestroyed(nativeInstance))
			return nullptr;

		String name = nativeInstance->mSceneObject->GetName();
		return MonoUtil::StringToMono(name);
	}

	void ScriptSceneObject::internal_setActive(ScriptSceneObject* nativeInstance, bool value)
	{
		if (checkIfDestroyed(nativeInstance))
			return;

		nativeInstance->mSceneObject->SetActive(value);
	}

	bool ScriptSceneObject::internal_getActive(ScriptSceneObject* nativeInstance)
	{
		if (checkIfDestroyed(nativeInstance))
			return false;

		return nativeInstance->mSceneObject->GetActive(true);
	}

	bool ScriptSceneObject::internal_hasFlag(ScriptSceneObject* nativeInstance, bs::UINT32 flag)
	{
		if (checkIfDestroyed(nativeInstance))
			return false;

		return nativeInstance->mSceneObject->HasFlag(flag);
	}

	void ScriptSceneObject::internal_setMobility(ScriptSceneObject* nativeInstance, int value)
	{
		if (checkIfDestroyed(nativeInstance))
			return;

		nativeInstance->mSceneObject->SetMobility((ObjectMobility)value);
	}

	int ScriptSceneObject::internal_getMobility(ScriptSceneObject* nativeInstance)
	{
		if (checkIfDestroyed(nativeInstance))
			return false;

		return (int)nativeInstance->mSceneObject->GetMobility();
	}
	void ScriptSceneObject::internal_setParent(ScriptSceneObject* nativeInstance, MonoObject* parent)
	{
		if (checkIfDestroyed(nativeInstance))
			return;

		ScriptSceneObject* parentScriptSO = ScriptSceneObject::toNative(parent);

		nativeInstance->mSceneObject->SetParent(parentScriptSO->mSceneObject);
	}

	MonoObject* ScriptSceneObject::internal_getParent(ScriptSceneObject* nativeInstance)
	{
		if (checkIfDestroyed(nativeInstance))
			return nullptr;

		HSceneObject parent = nativeInstance->mSceneObject->GetParent();
		if (parent != nullptr)
		{
			ScriptSceneObject* parentScriptSO = ScriptGameObjectManager::instance().GetOrCreateScriptSceneObject(parent);

			return parentScriptSO->GetManagedInstance();
		}

		return nullptr;
	}

	MonoObject* ScriptSceneObject::internal_getScene(ScriptSceneObject* nativeInstance)
	{
		if (checkIfDestroyed(nativeInstance))
			return nullptr;

		return ScriptSceneInstance::Create(nativeInstance->mSceneObject->GetScene());
	}

	void ScriptSceneObject::internal_getNumChildren(ScriptSceneObject* nativeInstance, UINT32* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetNumChildren();
		else
			*value = 0;
	}

	MonoObject* ScriptSceneObject::internal_getChild(ScriptSceneObject* nativeInstance, UINT32 idx)
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
		ScriptSceneObject* childScriptSO = ScriptGameObjectManager::instance().GetOrCreateScriptSceneObject(childSO);

		return childScriptSO->GetManagedInstance();
	}

	MonoObject* ScriptSceneObject::internal_findChild(ScriptSceneObject* nativeInstance, MonoString* name, bool recursive)
	{
		if (checkIfDestroyed(nativeInstance))
			return nullptr;

		String nativeName = MonoUtil::monoToString(name);
		HSceneObject child = nativeInstance->GetHandle()->findChild(nativeName, recursive);

		if (child == nullptr)
			return nullptr;

		ScriptSceneObject* scriptChild = ScriptGameObjectManager::instance().GetOrCreateScriptSceneObject(child);
		return scriptChild->GetManagedInstance();
	}

	MonoArray* ScriptSceneObject::internal_findChildren(ScriptSceneObject* nativeInstance, MonoString* name, bool recursive)
	{
		if (checkIfDestroyed(nativeInstance))
		{
			ScriptArray emptyArray = ScriptArray::create<ScriptSceneObject>(0);
			return emptyArray.GetInternal();
		}

		String nativeName = MonoUtil::monoToString(name);
		Vector<HSceneObject> children = nativeInstance->GetHandle()->findChildren(nativeName, recursive);

		UINT32 numChildren = (UINT32)children.Size();
		ScriptArray output = ScriptArray::create<ScriptSceneObject>(numChildren);

		for (UINT32 i = 0; i < numChildren; i++)
		{
			HSceneObject child = children[i];
			ScriptSceneObject* scriptChild = ScriptGameObjectManager::instance().GetOrCreateScriptSceneObject(child);

			output.Set(i, scriptChild->GetManagedInstance());
		}

		return output.GetInternal();
	}

	void ScriptSceneObject::internal_getPosition(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetTransform().GetPosition();
		else
			*value = Vector3(BsZero);
	}

	void ScriptSceneObject::internal_getLocalPosition(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetLocalTransform().GetPosition();
		else
			*value = Vector3(BsZero);
	}

	void ScriptSceneObject::internal_getRotation(ScriptSceneObject* nativeInstance, Quaternion* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetTransform().GetRotation();
		else
			*value = Quaternion(BsIdentity);
	}

	void ScriptSceneObject::internal_getLocalRotation(ScriptSceneObject* nativeInstance, Quaternion* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetLocalTransform().GetRotation();
		else
			*value = Quaternion(BsIdentity);
	}

	void ScriptSceneObject::internal_getScale(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetTransform().GetScale();
		else
			*value = Vector3(Vector3::ONE);
	}

	void ScriptSceneObject::internal_getLocalScale(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetLocalTransform().GetScale();
		else
			*value = Vector3(Vector3::ONE);
	}

	void ScriptSceneObject::internal_setPosition(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->SetWorldPosition(*value);
	}

	void ScriptSceneObject::internal_setLocalPosition(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->SetPosition(*value);
	}

	void ScriptSceneObject::internal_setRotation(ScriptSceneObject* nativeInstance, Quaternion* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->SetWorldRotation(*value);
	}

	void ScriptSceneObject::internal_setLocalRotation(ScriptSceneObject* nativeInstance, Quaternion* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->SetRotation(*value);
	}

	void ScriptSceneObject::internal_setLocalScale(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->SetScale(*value);
	}

	void ScriptSceneObject::internal_getLocalTransform(ScriptSceneObject* nativeInstance, Matrix4* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetLocalMatrix();
		else
			*value = Matrix4(BsIdentity);
	}

	void ScriptSceneObject::internal_getWorldTransform(ScriptSceneObject* nativeInstance, Matrix4* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetWorldMatrix();
		else
			*value = Matrix4(BsIdentity);
	}

	void ScriptSceneObject::internal_lookAt(ScriptSceneObject* nativeInstance, Vector3* direction, Vector3* up)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->LookAt(*direction, *up);
	}

	void ScriptSceneObject::internal_move(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->Move(*value);
	}

	void ScriptSceneObject::internal_moveLocal(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->MoveRelative(*value);
	}

	void ScriptSceneObject::internal_rotate(ScriptSceneObject* nativeInstance, Quaternion* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->Rotate(*value);
	}

	void ScriptSceneObject::internal_roll(ScriptSceneObject* nativeInstance, Radian* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->Roll(*value);
	}

	void ScriptSceneObject::internal_yaw(ScriptSceneObject* nativeInstance, Radian* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->Yaw(*value);
	}

	void ScriptSceneObject::internal_pitch(ScriptSceneObject* nativeInstance, Radian* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->Pitch(*value);
	}

	void ScriptSceneObject::internal_setForward(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->SetForward(*value);
	}

	void ScriptSceneObject::internal_getForward(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetTransform().GetForward();
		else
			*value = Vector3(-Vector3::UNIT_Z);
	}

	void ScriptSceneObject::internal_getUp(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetTransform().GetUp();
		else
			*value = Vector3(Vector3::UNIT_Y);
	}

	void ScriptSceneObject::internal_getRight(ScriptSceneObject* nativeInstance, Vector3* value)
	{
		if (!checkIfDestroyed(nativeInstance))
			*value = nativeInstance->mSceneObject->GetTransform().GetRight();
		else
			*value = Vector3(Vector3::UNIT_X);
	}

	void ScriptSceneObject::internal_destroy(ScriptSceneObject* nativeInstance, bool immediate)
	{
		if (!checkIfDestroyed(nativeInstance))
			nativeInstance->mSceneObject->Destroy(immediate);
	}

	bool ScriptSceneObject::CheckIfDestroyed(ScriptSceneObject* nativeInstance)
	{
		if (nativeInstance->mSceneObject.IsDestroyed())
		{
			BS_LOG(Warning, Scene, "Trying to access a destroyed SceneObject with instance ID: {0}", +
				nativeInstance->mSceneObject.GetInstanceId());
			return true;
		}

		return false;
	}

	void ScriptSceneObject::_onManagedInstanceDeleted(bool assemblyRefresh)
	{
		if (!assemblyRefresh || mSceneObject.IsDestroyed(true))
			ScriptGameObjectManager::instance().DestroyScriptSceneObject(this);
		else
			freeManagedInstance();
	}

	MonoObject* ScriptSceneObject::_createManagedInstance(bool construct)
	{
		MonoObject* managedInstance = metaData.scriptClass->CreateInstance(construct);
		setManagedInstance(managedInstance);

		return managedInstance;
	}

	void ScriptSceneObject::_clearManagedInstance()
	{
		freeManagedInstance();
	}

	void ScriptSceneObject::_notifyDestroyed()
	{
		freeManagedInstance();
	}

	void ScriptSceneObject::SetNativeHandle(const HGameObject& gameObject)
	{
		mSceneObject = static_object_cast<SceneObject>(gameObject);
	}
}
