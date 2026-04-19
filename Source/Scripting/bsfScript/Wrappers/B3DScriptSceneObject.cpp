//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/B3DScriptSceneObject.h"
#include "B3DMonoField.h"
#include "B3DMonoClass.h"
#include "B3DMonoManager.h"
#include "Scene/B3DSceneObject.h"
#include "Scene/B3DSceneManager.h"
#include "Physics/B3DPhysics.h"
#include "B3DMonoUtil.h"

#include "Generated/B3DScriptSceneInstance.generated.h"

using namespace b3d;
ScriptSceneObject::ScriptSceneObject(const HSceneObject& nativeObject)
	: TScriptGameObjectWrapper(nativeObject)
{ }

void ScriptSceneObject::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptSceneObject::InternalCreateInstance);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetName", (void*)&ScriptSceneObject::InternalGetName);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetName", (void*)&ScriptSceneObject::InternalSetName);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetActive", (void*)&ScriptSceneObject::InternalGetActive);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetActive", (void*)&ScriptSceneObject::InternalSetActive);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_HasFlag", (void*)&ScriptSceneObject::InternalHasFlag);

	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMobility", (void*)&ScriptSceneObject::InternalGetMobility);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMobility", (void*)&ScriptSceneObject::InternalSetMobility);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetParent", (void*)&ScriptSceneObject::InternalGetParent);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetParent", (void*)&ScriptSceneObject::InternalGetParent);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetParent", (void*)&ScriptSceneObject::InternalSetParent);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetScene", (void*)&ScriptSceneObject::InternalGetScene);

	sInteropMetaData.ScriptClass->AddInternalCall("Internal_BreakPrefabLink", (void*)&ScriptSceneObject::InternalBreakPrefabLink);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsPrefabInstance", (void*)&ScriptSceneObject::InternalIsPrefabInstance);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPrefabInstanceRoot", (void*)&ScriptSceneObject::InternalGetPrefabInstanceRoot);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPrefabResourceId", (void*)&ScriptSceneObject::InternalGetPrefabResourceId);

	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetNumChildren", (void*)&ScriptSceneObject::InternalGetNumChildren);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetChild", (void*)&ScriptSceneObject::InternalGetChild);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_FindChild", (void*)&ScriptSceneObject::InternalFindChild);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_FindChildren", (void*)&ScriptSceneObject::InternalFindChildren);

	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPosition", (void*)&ScriptSceneObject::InternalGetPosition);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLocalPosition", (void*)&ScriptSceneObject::InternalGetLocalPosition);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRotation", (void*)&ScriptSceneObject::InternalGetRotation);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLocalRotation", (void*)&ScriptSceneObject::InternalGetLocalRotation);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetScale", (void*)&ScriptSceneObject::InternalGetScale);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLocalScale", (void*)&ScriptSceneObject::InternalGetLocalScale);

	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPosition", (void*)&ScriptSceneObject::InternalSetPosition);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLocalPosition", (void*)&ScriptSceneObject::InternalSetLocalPosition);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRotation", (void*)&ScriptSceneObject::InternalSetRotation);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLocalRotation", (void*)&ScriptSceneObject::InternalSetLocalRotation);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLocalScale", (void*)&ScriptSceneObject::InternalSetLocalScale);

	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLocalTransform", (void*)&ScriptSceneObject::InternalGetLocalTransform);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetWorldTransform", (void*)&ScriptSceneObject::InternalGetWorldTransform);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_LookAt", (void*)&ScriptSceneObject::InternalLookAt);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_Move", (void*)&ScriptSceneObject::InternalMove);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_MoveLocal", (void*)&ScriptSceneObject::InternalMoveLocal);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_Rotate", (void*)&ScriptSceneObject::InternalRotate);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_Roll", (void*)&ScriptSceneObject::InternalRoll);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_Yaw", (void*)&ScriptSceneObject::InternalYaw);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_Pitch", (void*)&ScriptSceneObject::InternalPitch);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetForward", (void*)&ScriptSceneObject::InternalSetForward);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetForward", (void*)&ScriptSceneObject::InternalGetForward);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUp", (void*)&ScriptSceneObject::InternalGetUp);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRight", (void*)&ScriptSceneObject::InternalGetRight);

	sInteropMetaData.ScriptClass->AddInternalCall("Internal_Destroy", (void*)&ScriptSceneObject::InternalDestroy);
}

SceneObject* ScriptSceneObject::GetNativeObject() const
{
	return static_cast<SceneObject*>(TScriptGameObjectWrapper::GetNativeObject());
}

MonoObject* ScriptSceneObject::CreateScriptObject(bool construct)
{
	return sInteropMetaData.ScriptClass->CreateInstance(construct);
}

void ScriptSceneObject::InternalCreateInstance(MonoObject* scriptObject, MonoString* name, u32 flags)
{
	HSceneObject sceneObject = SceneObject::Create(MonoUtil::MonoToString(name), flags);
	ScriptObjectWrapper::Create<ScriptSceneObject>(sceneObject, scriptObject);
}

void ScriptSceneObject::InternalSetName(ScriptSceneObject* self, MonoString* name)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetName(MonoUtil::MonoToString(name));
}

MonoString* ScriptSceneObject::InternalGetName(ScriptSceneObject* self)
{
	if(!self->IsNativeObjectValid())
		return nullptr;

	const String& name = self->GetNativeObject()->GetName();
	return MonoUtil::StringToMono(name);
}

void ScriptSceneObject::InternalSetActive(ScriptSceneObject* self, bool value)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetActive(value);
}

bool ScriptSceneObject::InternalGetActive(ScriptSceneObject* self)
{
	if(!self->IsNativeObjectValid())
		return false;

	return self->GetNativeObject()->GetActive(true);
}

bool ScriptSceneObject::InternalHasFlag(ScriptSceneObject* self, b3d::u32 flag)
{
	if(!self->IsNativeObjectValid())
		return false;

	return self->GetNativeObject()->HasFlag((SceneObjectFlag)flag);
}

void ScriptSceneObject::InternalSetMobility(ScriptSceneObject* self, int value)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->SetMobility((ObjectMobility)value);
}

int ScriptSceneObject::InternalGetMobility(ScriptSceneObject* self)
{
	if(!self->IsNativeObjectValid())
		return false;

	return (int)self->GetNativeObject()->GetMobility();
}

void ScriptSceneObject::InternalSetParent(ScriptSceneObject* self, MonoObject* parent)
{
	if(!self->IsNativeObjectValid())
		return;

	ScriptSceneObject* const parentScriptSceneObject = GetScriptObjectWrapper(parent);
	self->GetNativeObject()->SetParent(parentScriptSceneObject->GetNativeObjectAsHandle());
}

MonoObject* ScriptSceneObject::InternalGetParent(ScriptSceneObject* self)
{
	if(!self->IsNativeObjectValid())
		return nullptr;

	HSceneObject parent = self->GetNativeObject()->GetParent();
	if(parent != nullptr)
		return GetOrCreateScriptObject(parent);

	return nullptr;
}

MonoObject* ScriptSceneObject::InternalGetScene(ScriptSceneObject* self)
{
	if(!self->IsNativeObjectValid())
		return nullptr;

	return ScriptSceneInstance::GetOrCreateScriptObject(self->GetNativeObject()->GetScene());
}

void ScriptSceneObject::InternalBreakPrefabLink(ScriptSceneObject* self)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->BreakPrefabLink();
}

bool ScriptSceneObject::InternalIsPrefabInstance(ScriptSceneObject* self)
{
	if(!self->IsNativeObjectValid())
		return false;

	return self->GetNativeObject()->IsPrefabInstance();
	
}

MonoObject* ScriptSceneObject::InternalGetPrefabInstanceRoot(ScriptSceneObject* self)
{
	if(!self->IsNativeObjectValid())
		return nullptr;

	HSceneObject parent = self->GetNativeObject()->GetPrefabInstanceRoot();

	if(parent != nullptr)
		return GetOrCreateScriptObject(parent);

	return nullptr;
}

void ScriptSceneObject::InternalGetPrefabResourceId(ScriptSceneObject* self, UUID* uuid)
{
	*uuid = UUID::kEmpty;

	if(!self->IsNativeObjectValid())
		return;

	*uuid = self->GetNativeObject()->GetPrefabResourceId();
}

void ScriptSceneObject::InternalGetNumChildren(ScriptSceneObject* self, u32* value)
{
	if(self->IsNativeObjectValid())
		*value = self->GetNativeObject()->GetChildCount();
	else
		*value = 0;
}

MonoObject* ScriptSceneObject::InternalGetChild(ScriptSceneObject* self, u32 childIndex)
{
	if(!self->IsNativeObjectValid())
		return nullptr;

	const u32 childCount = self->GetNativeObject()->GetChildCount();
	if(childIndex >= childCount)
	{
		B3D_LOG(Warning, LogScene, "Attempting to access an out of range SceneObject child. Provided index: \"{0}\". Valid range: [0, {1})", childIndex, childCount);
		return nullptr;
	}

	HSceneObject childSceneObject = self->GetNativeObject()->GetChild(childIndex);
	return GetOrCreateScriptObject(childSceneObject);
}

MonoObject* ScriptSceneObject::InternalFindChild(ScriptSceneObject* self, MonoString* name, bool recursive)
{
	if(!self->IsNativeObjectValid())
		return nullptr;

	String nativeName = MonoUtil::MonoToString(name);
	HSceneObject child = self->GetNativeObject()->FindChild(nativeName, recursive);

	if(child == nullptr)
		return nullptr;

	return GetOrCreateScriptObject(child);
}

MonoArray* ScriptSceneObject::InternalFindChildren(ScriptSceneObject* self, MonoString* name, bool recursive)
{
	if(!self->IsNativeObjectValid())
	{
		ScriptArray emptyArray = ScriptArray::Create<ScriptSceneObject>(0);
		return emptyArray.GetInternal();
	}

	String nativeName = MonoUtil::MonoToString(name);
	Vector<HSceneObject> children = self->GetNativeObject()->FindChildren(nativeName, recursive);

	const u32 childCount = (u32)children.size();
	ScriptArray output = ScriptArray::Create<ScriptSceneObject>(childCount);

	for(u32 childIndex = 0; childIndex < childCount; childIndex++)
	{
		HSceneObject child = children[childIndex];
		output.Set(childIndex, GetOrCreateScriptObject(child));
	}

	return output.GetInternal();
}

void ScriptSceneObject::InternalGetPosition(ScriptSceneObject* self, Vector3* value)
{
	if(self->IsNativeObjectValid())
		*value = self->GetNativeObject()->GetTransform().GetPosition();
	else
		*value = Vector3(kZeroTag);
}

void ScriptSceneObject::InternalGetLocalPosition(ScriptSceneObject* self, Vector3* value)
{
	if(self->IsNativeObjectValid())
		*value = self->GetNativeObject()->GetLocalTransform().GetPosition();
	else
		*value = Vector3(kZeroTag);
}

void ScriptSceneObject::InternalGetRotation(ScriptSceneObject* self, Quaternion* value)
{
	if(self->IsNativeObjectValid())
		*value = self->GetNativeObject()->GetTransform().GetRotation();
	else
		*value = Quaternion(kIdentityTag);
}

void ScriptSceneObject::InternalGetLocalRotation(ScriptSceneObject* self, Quaternion* value)
{
	if(self->IsNativeObjectValid())
		*value = self->GetNativeObject()->GetLocalTransform().GetRotation();
	else
		*value = Quaternion(kIdentityTag);
}

void ScriptSceneObject::InternalGetScale(ScriptSceneObject* self, Vector3* value)
{
	if(self->IsNativeObjectValid())
		*value = self->GetNativeObject()->GetTransform().GetScale();
	else
		*value = Vector3(Vector3::kOne);
}

void ScriptSceneObject::InternalGetLocalScale(ScriptSceneObject* self, Vector3* value)
{
	if(self->IsNativeObjectValid())
		*value = self->GetNativeObject()->GetLocalTransform().GetScale();
	else
		*value = Vector3(Vector3::kOne);
}

void ScriptSceneObject::InternalSetPosition(ScriptSceneObject* self, Vector3* value)
{
	if(self->IsNativeObjectValid())
		self->GetNativeObject()->SetWorldPosition(*value);
}

void ScriptSceneObject::InternalSetLocalPosition(ScriptSceneObject* self, Vector3* value)
{
	if(self->IsNativeObjectValid())
		self->GetNativeObject()->SetPosition(*value);
}

void ScriptSceneObject::InternalSetRotation(ScriptSceneObject* self, Quaternion* value)
{
	if(self->IsNativeObjectValid())
		self->GetNativeObject()->SetWorldRotation(*value);
}

void ScriptSceneObject::InternalSetLocalRotation(ScriptSceneObject* self, Quaternion* value)
{
	if(self->IsNativeObjectValid())
		self->GetNativeObject()->SetRotation(*value);
}

void ScriptSceneObject::InternalSetLocalScale(ScriptSceneObject* self, Vector3* value)
{
	if(self->IsNativeObjectValid())
		self->GetNativeObject()->SetScale(*value);
}

void ScriptSceneObject::InternalGetLocalTransform(ScriptSceneObject* self, Matrix4* value)
{
	if(self->IsNativeObjectValid())
		*value = self->GetNativeObject()->GetLocalMatrix();
	else
		*value = Matrix4(kIdentityTag);
}

void ScriptSceneObject::InternalGetWorldTransform(ScriptSceneObject* self, Matrix4* value)
{
	if(self->IsNativeObjectValid())
		*value = self->GetNativeObject()->GetWorldMatrix();
	else
		*value = Matrix4(kIdentityTag);
}

void ScriptSceneObject::InternalLookAt(ScriptSceneObject* self, Vector3* direction, Vector3* up)
{
	if(self->IsNativeObjectValid())
		self->GetNativeObject()->LookAt(*direction, *up);
}

void ScriptSceneObject::InternalMove(ScriptSceneObject* self, Vector3* value)
{
	if(self->IsNativeObjectValid())
		self->GetNativeObject()->Move(*value);
}

void ScriptSceneObject::InternalMoveLocal(ScriptSceneObject* self, Vector3* value)
{
	if(self->IsNativeObjectValid())
		self->GetNativeObject()->MoveRelative(*value);
}

void ScriptSceneObject::InternalRotate(ScriptSceneObject* self, Quaternion* value)
{
	if(self->IsNativeObjectValid())
		self->GetNativeObject()->Rotate(*value);
}

void ScriptSceneObject::InternalRoll(ScriptSceneObject* self, Radian* value)
{
	if(self->IsNativeObjectValid())
		self->GetNativeObject()->Roll(*value);
}

void ScriptSceneObject::InternalYaw(ScriptSceneObject* self, Radian* value)
{
	if(self->IsNativeObjectValid())
		self->GetNativeObject()->Yaw(*value);
}

void ScriptSceneObject::InternalPitch(ScriptSceneObject* self, Radian* value)
{
	if(self->IsNativeObjectValid())
		self->GetNativeObject()->Pitch(*value);
}

void ScriptSceneObject::InternalSetForward(ScriptSceneObject* self, Vector3* value)
{
	if(self->IsNativeObjectValid())
		self->GetNativeObject()->SetForward(*value);
}

void ScriptSceneObject::InternalGetForward(ScriptSceneObject* self, Vector3* value)
{
	if(self->IsNativeObjectValid())
		*value = self->GetNativeObject()->GetTransform().GetForward();
	else
		*value = Vector3(-Vector3::kUnitZ);
}

void ScriptSceneObject::InternalGetUp(ScriptSceneObject* self, Vector3* value)
{
	if(self->IsNativeObjectValid())
		*value = self->GetNativeObject()->GetTransform().GetUp();
	else
		*value = Vector3(Vector3::kUnitY);
}

void ScriptSceneObject::InternalGetRight(ScriptSceneObject* self, Vector3* value)
{
	if(self->IsNativeObjectValid())
		*value = self->GetNativeObject()->GetTransform().GetRight();
	else
		*value = Vector3(Vector3::kUnitX);
}

void ScriptSceneObject::InternalDestroy(ScriptSceneObject* self, bool immediate)
{
	if(self->IsNativeObjectValid())
		self->GetNativeObject()->Destroy(immediate);
}
