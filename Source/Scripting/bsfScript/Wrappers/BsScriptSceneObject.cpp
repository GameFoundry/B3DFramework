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

using namespace bs;
ScriptSceneObject::ScriptSceneObject(MonoObject* instance, const HSceneObject& sceneObject)
	: ScriptObject(instance), mSceneObject(sceneObject)
{
	SetManagedInstance(instance);
}

void ScriptSceneObject::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptSceneObject::InternalCreateInstance);
	metaData.ScriptClass->AddInternalCall("Internal_GetName", (void*)&ScriptSceneObject::InternalGetName);
	metaData.ScriptClass->AddInternalCall("Internal_SetName", (void*)&ScriptSceneObject::InternalSetName);
	metaData.ScriptClass->AddInternalCall("Internal_GetActive", (void*)&ScriptSceneObject::InternalGetActive);
	metaData.ScriptClass->AddInternalCall("Internal_SetActive", (void*)&ScriptSceneObject::InternalSetActive);
	metaData.ScriptClass->AddInternalCall("Internal_HasFlag", (void*)&ScriptSceneObject::InternalHasFlag);

	metaData.ScriptClass->AddInternalCall("Internal_GetMobility", (void*)&ScriptSceneObject::InternalGetMobility);
	metaData.ScriptClass->AddInternalCall("Internal_SetMobility", (void*)&ScriptSceneObject::InternalSetMobility);
	metaData.ScriptClass->AddInternalCall("Internal_GetParent", (void*)&ScriptSceneObject::InternalGetParent);
	metaData.ScriptClass->AddInternalCall("Internal_GetParent", (void*)&ScriptSceneObject::InternalGetParent);
	metaData.ScriptClass->AddInternalCall("Internal_SetParent", (void*)&ScriptSceneObject::InternalSetParent);
	metaData.ScriptClass->AddInternalCall("Internal_GetScene", (void*)&ScriptSceneObject::InternalGetScene);
	metaData.ScriptClass->AddInternalCall("Internal_GetNumChildren", (void*)&ScriptSceneObject::InternalGetNumChildren);
	metaData.ScriptClass->AddInternalCall("Internal_GetChild", (void*)&ScriptSceneObject::InternalGetChild);
	metaData.ScriptClass->AddInternalCall("Internal_FindChild", (void*)&ScriptSceneObject::InternalFindChild);
	metaData.ScriptClass->AddInternalCall("Internal_FindChildren", (void*)&ScriptSceneObject::InternalFindChildren);

	metaData.ScriptClass->AddInternalCall("Internal_GetPosition", (void*)&ScriptSceneObject::InternalGetPosition);
	metaData.ScriptClass->AddInternalCall("Internal_GetLocalPosition", (void*)&ScriptSceneObject::InternalGetLocalPosition);
	metaData.ScriptClass->AddInternalCall("Internal_GetRotation", (void*)&ScriptSceneObject::InternalGetRotation);
	metaData.ScriptClass->AddInternalCall("Internal_GetLocalRotation", (void*)&ScriptSceneObject::InternalGetLocalRotation);
	metaData.ScriptClass->AddInternalCall("Internal_GetScale", (void*)&ScriptSceneObject::InternalGetScale);
	metaData.ScriptClass->AddInternalCall("Internal_GetLocalScale", (void*)&ScriptSceneObject::InternalGetLocalScale);

	metaData.ScriptClass->AddInternalCall("Internal_SetPosition", (void*)&ScriptSceneObject::InternalSetPosition);
	metaData.ScriptClass->AddInternalCall("Internal_SetLocalPosition", (void*)&ScriptSceneObject::InternalSetLocalPosition);
	metaData.ScriptClass->AddInternalCall("Internal_SetRotation", (void*)&ScriptSceneObject::InternalSetRotation);
	metaData.ScriptClass->AddInternalCall("Internal_SetLocalRotation", (void*)&ScriptSceneObject::InternalSetLocalRotation);
	metaData.ScriptClass->AddInternalCall("Internal_SetLocalScale", (void*)&ScriptSceneObject::InternalSetLocalScale);

	metaData.ScriptClass->AddInternalCall("Internal_GetLocalTransform", (void*)&ScriptSceneObject::InternalGetLocalTransform);
	metaData.ScriptClass->AddInternalCall("Internal_GetWorldTransform", (void*)&ScriptSceneObject::InternalGetWorldTransform);
	metaData.ScriptClass->AddInternalCall("Internal_LookAt", (void*)&ScriptSceneObject::InternalLookAt);
	metaData.ScriptClass->AddInternalCall("Internal_Move", (void*)&ScriptSceneObject::InternalMove);
	metaData.ScriptClass->AddInternalCall("Internal_MoveLocal", (void*)&ScriptSceneObject::InternalMoveLocal);
	metaData.ScriptClass->AddInternalCall("Internal_Rotate", (void*)&ScriptSceneObject::InternalRotate);
	metaData.ScriptClass->AddInternalCall("Internal_Roll", (void*)&ScriptSceneObject::InternalRoll);
	metaData.ScriptClass->AddInternalCall("Internal_Yaw", (void*)&ScriptSceneObject::InternalYaw);
	metaData.ScriptClass->AddInternalCall("Internal_Pitch", (void*)&ScriptSceneObject::InternalPitch);
	metaData.ScriptClass->AddInternalCall("Internal_SetForward", (void*)&ScriptSceneObject::InternalSetForward);
	metaData.ScriptClass->AddInternalCall("Internal_GetForward", (void*)&ScriptSceneObject::InternalGetForward);
	metaData.ScriptClass->AddInternalCall("Internal_GetUp", (void*)&ScriptSceneObject::InternalGetUp);
	metaData.ScriptClass->AddInternalCall("Internal_GetRight", (void*)&ScriptSceneObject::InternalGetRight);

	metaData.ScriptClass->AddInternalCall("Internal_Destroy", (void*)&ScriptSceneObject::InternalDestroy);
}

void ScriptSceneObject::InternalCreateInstance(MonoObject* instance, MonoString* name, u32 flags)
{
	HSceneObject sceneObject = SceneObject::Create(MonoUtil::MonoToString(name), flags);

	ScriptGameObjectManager::Instance().CreateScriptSceneObject(instance, sceneObject);
}

void ScriptSceneObject::InternalSetName(ScriptSceneObject* nativeInstance, MonoString* name)
{
	if(CheckIfDestroyed(nativeInstance))
		return;

	nativeInstance->mSceneObject->SetName(MonoUtil::MonoToString(name));
}

MonoString* ScriptSceneObject::InternalGetName(ScriptSceneObject* nativeInstance)
{
	if(CheckIfDestroyed(nativeInstance))
		return nullptr;

	String name = nativeInstance->mSceneObject->GetName();
	return MonoUtil::StringToMono(name);
}

void ScriptSceneObject::InternalSetActive(ScriptSceneObject* nativeInstance, bool value)
{
	if(CheckIfDestroyed(nativeInstance))
		return;

	nativeInstance->mSceneObject->SetActive(value);
}

bool ScriptSceneObject::InternalGetActive(ScriptSceneObject* nativeInstance)
{
	if(CheckIfDestroyed(nativeInstance))
		return false;

	return nativeInstance->mSceneObject->GetActive(true);
}

bool ScriptSceneObject::InternalHasFlag(ScriptSceneObject* nativeInstance, bs::u32 flag)
{
	if(CheckIfDestroyed(nativeInstance))
		return false;

	return nativeInstance->mSceneObject->HasFlag(flag);
}

void ScriptSceneObject::InternalSetMobility(ScriptSceneObject* nativeInstance, int value)
{
	if(CheckIfDestroyed(nativeInstance))
		return;

	nativeInstance->mSceneObject->SetMobility((ObjectMobility)value);
}

int ScriptSceneObject::InternalGetMobility(ScriptSceneObject* nativeInstance)
{
	if(CheckIfDestroyed(nativeInstance))
		return false;

	return (int)nativeInstance->mSceneObject->GetMobility();
}

void ScriptSceneObject::InternalSetParent(ScriptSceneObject* nativeInstance, MonoObject* parent)
{
	if(CheckIfDestroyed(nativeInstance))
		return;

	ScriptSceneObject* parentScriptSO = ScriptSceneObject::ToNative(parent);

	nativeInstance->mSceneObject->SetParent(parentScriptSO->mSceneObject);
}

MonoObject* ScriptSceneObject::InternalGetParent(ScriptSceneObject* nativeInstance)
{
	if(CheckIfDestroyed(nativeInstance))
		return nullptr;

	HSceneObject parent = nativeInstance->mSceneObject->GetParent();
	if(parent != nullptr)
	{
		ScriptSceneObject* parentScriptSO = ScriptGameObjectManager::Instance().GetOrCreateScriptSceneObject(parent);

		return parentScriptSO->GetManagedInstance();
	}

	return nullptr;
}

MonoObject* ScriptSceneObject::InternalGetScene(ScriptSceneObject* nativeInstance)
{
	if(CheckIfDestroyed(nativeInstance))
		return nullptr;

	return ScriptSceneInstance::Create(nativeInstance->mSceneObject->GetScene());
}

void ScriptSceneObject::InternalGetNumChildren(ScriptSceneObject* nativeInstance, u32* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		*value = nativeInstance->mSceneObject->GetNumChildren();
	else
		*value = 0;
}

MonoObject* ScriptSceneObject::InternalGetChild(ScriptSceneObject* nativeInstance, u32 idx)
{
	if(CheckIfDestroyed(nativeInstance))
		return nullptr;

	u32 numChildren = nativeInstance->mSceneObject->GetNumChildren();
	if(idx >= numChildren)
	{
		BS_LOG(Warning, Scene, "Attempting to access an out of range SceneObject child. Provided index: \"{0}\". "
							   "Valid range: [0, {1})",
			   idx, numChildren);
		return nullptr;
	}

	HSceneObject childSO = nativeInstance->mSceneObject->GetChild(idx);
	ScriptSceneObject* childScriptSO = ScriptGameObjectManager::Instance().GetOrCreateScriptSceneObject(childSO);

	return childScriptSO->GetManagedInstance();
}

MonoObject* ScriptSceneObject::InternalFindChild(ScriptSceneObject* nativeInstance, MonoString* name, bool recursive)
{
	if(CheckIfDestroyed(nativeInstance))
		return nullptr;

	String nativeName = MonoUtil::MonoToString(name);
	HSceneObject child = nativeInstance->GetHandle()->FindChild(nativeName, recursive);

	if(child == nullptr)
		return nullptr;

	ScriptSceneObject* scriptChild = ScriptGameObjectManager::Instance().GetOrCreateScriptSceneObject(child);
	return scriptChild->GetManagedInstance();
}

MonoArray* ScriptSceneObject::InternalFindChildren(ScriptSceneObject* nativeInstance, MonoString* name, bool recursive)
{
	if(CheckIfDestroyed(nativeInstance))
	{
		ScriptArray emptyArray = ScriptArray::Create<ScriptSceneObject>(0);
		return emptyArray.GetInternal();
	}

	String nativeName = MonoUtil::MonoToString(name);
	Vector<HSceneObject> children = nativeInstance->GetHandle()->FindChildren(nativeName, recursive);

	u32 numChildren = (u32)children.size();
	ScriptArray output = ScriptArray::Create<ScriptSceneObject>(numChildren);

	for(u32 i = 0; i < numChildren; i++)
	{
		HSceneObject child = children[i];
		ScriptSceneObject* scriptChild = ScriptGameObjectManager::Instance().GetOrCreateScriptSceneObject(child);

		output.Set(i, scriptChild->GetManagedInstance());
	}

	return output.GetInternal();
}

void ScriptSceneObject::InternalGetPosition(ScriptSceneObject* nativeInstance, Vector3* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		*value = nativeInstance->mSceneObject->GetTransform().GetPosition();
	else
		*value = Vector3(BsZero);
}

void ScriptSceneObject::InternalGetLocalPosition(ScriptSceneObject* nativeInstance, Vector3* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		*value = nativeInstance->mSceneObject->GetLocalTransform().GetPosition();
	else
		*value = Vector3(BsZero);
}

void ScriptSceneObject::InternalGetRotation(ScriptSceneObject* nativeInstance, Quaternion* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		*value = nativeInstance->mSceneObject->GetTransform().GetRotation();
	else
		*value = Quaternion(BsIdentity);
}

void ScriptSceneObject::InternalGetLocalRotation(ScriptSceneObject* nativeInstance, Quaternion* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		*value = nativeInstance->mSceneObject->GetLocalTransform().GetRotation();
	else
		*value = Quaternion(BsIdentity);
}

void ScriptSceneObject::InternalGetScale(ScriptSceneObject* nativeInstance, Vector3* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		*value = nativeInstance->mSceneObject->GetTransform().GetScale();
	else
		*value = Vector3(Vector3::kOne);
}

void ScriptSceneObject::InternalGetLocalScale(ScriptSceneObject* nativeInstance, Vector3* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		*value = nativeInstance->mSceneObject->GetLocalTransform().GetScale();
	else
		*value = Vector3(Vector3::kOne);
}

void ScriptSceneObject::InternalSetPosition(ScriptSceneObject* nativeInstance, Vector3* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		nativeInstance->mSceneObject->SetWorldPosition(*value);
}

void ScriptSceneObject::InternalSetLocalPosition(ScriptSceneObject* nativeInstance, Vector3* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		nativeInstance->mSceneObject->SetPosition(*value);
}

void ScriptSceneObject::InternalSetRotation(ScriptSceneObject* nativeInstance, Quaternion* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		nativeInstance->mSceneObject->SetWorldRotation(*value);
}

void ScriptSceneObject::InternalSetLocalRotation(ScriptSceneObject* nativeInstance, Quaternion* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		nativeInstance->mSceneObject->SetRotation(*value);
}

void ScriptSceneObject::InternalSetLocalScale(ScriptSceneObject* nativeInstance, Vector3* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		nativeInstance->mSceneObject->SetScale(*value);
}

void ScriptSceneObject::InternalGetLocalTransform(ScriptSceneObject* nativeInstance, Matrix4* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		*value = nativeInstance->mSceneObject->GetLocalMatrix();
	else
		*value = Matrix4(BsIdentity);
}

void ScriptSceneObject::InternalGetWorldTransform(ScriptSceneObject* nativeInstance, Matrix4* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		*value = nativeInstance->mSceneObject->GetWorldMatrix();
	else
		*value = Matrix4(BsIdentity);
}

void ScriptSceneObject::InternalLookAt(ScriptSceneObject* nativeInstance, Vector3* direction, Vector3* up)
{
	if(!CheckIfDestroyed(nativeInstance))
		nativeInstance->mSceneObject->LookAt(*direction, *up);
}

void ScriptSceneObject::InternalMove(ScriptSceneObject* nativeInstance, Vector3* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		nativeInstance->mSceneObject->Move(*value);
}

void ScriptSceneObject::InternalMoveLocal(ScriptSceneObject* nativeInstance, Vector3* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		nativeInstance->mSceneObject->MoveRelative(*value);
}

void ScriptSceneObject::InternalRotate(ScriptSceneObject* nativeInstance, Quaternion* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		nativeInstance->mSceneObject->Rotate(*value);
}

void ScriptSceneObject::InternalRoll(ScriptSceneObject* nativeInstance, Radian* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		nativeInstance->mSceneObject->Roll(*value);
}

void ScriptSceneObject::InternalYaw(ScriptSceneObject* nativeInstance, Radian* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		nativeInstance->mSceneObject->Yaw(*value);
}

void ScriptSceneObject::InternalPitch(ScriptSceneObject* nativeInstance, Radian* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		nativeInstance->mSceneObject->Pitch(*value);
}

void ScriptSceneObject::InternalSetForward(ScriptSceneObject* nativeInstance, Vector3* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		nativeInstance->mSceneObject->SetForward(*value);
}

void ScriptSceneObject::InternalGetForward(ScriptSceneObject* nativeInstance, Vector3* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		*value = nativeInstance->mSceneObject->GetTransform().GetForward();
	else
		*value = Vector3(-Vector3::kUnitZ);
}

void ScriptSceneObject::InternalGetUp(ScriptSceneObject* nativeInstance, Vector3* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		*value = nativeInstance->mSceneObject->GetTransform().GetUp();
	else
		*value = Vector3(Vector3::kUnitY);
}

void ScriptSceneObject::InternalGetRight(ScriptSceneObject* nativeInstance, Vector3* value)
{
	if(!CheckIfDestroyed(nativeInstance))
		*value = nativeInstance->mSceneObject->GetTransform().GetRight();
	else
		*value = Vector3(Vector3::kUnitX);
}

void ScriptSceneObject::InternalDestroy(ScriptSceneObject* nativeInstance, bool immediate)
{
	if(!CheckIfDestroyed(nativeInstance))
		nativeInstance->mSceneObject->Destroy(immediate);
}

bool ScriptSceneObject::CheckIfDestroyed(ScriptSceneObject* nativeInstance)
{
	if(nativeInstance->mSceneObject.IsDestroyed())
	{
		BS_LOG(Warning, Scene, "Trying to access a destroyed SceneObject with instance ID: {0}", +nativeInstance->mSceneObject.GetInstanceId());
		return true;
	}

	return false;
}

void ScriptSceneObject::OnManagedInstanceDeletedInternal(bool assemblyRefresh)
{
	if(!assemblyRefresh || mSceneObject.IsDestroyed(true))
		ScriptGameObjectManager::Instance().DestroyScriptSceneObject(this);
	else
		FreeManagedInstance();
}

MonoObject* ScriptSceneObject::CreateManagedInstanceInternal(bool construct)
{
	MonoObject* managedInstance = metaData.ScriptClass->CreateInstance(construct);
	SetManagedInstance(managedInstance);

	return managedInstance;
}

void ScriptSceneObject::ClearManagedInstanceInternal()
{
	FreeManagedInstance();
}

void ScriptSceneObject::NotifyDestroyedInternal()
{
	FreeManagedInstance();
}

void ScriptSceneObject::SetNativeHandle(const HGameObject& gameObject)
{
	mSceneObject = static_object_cast<SceneObject>(gameObject);
}
