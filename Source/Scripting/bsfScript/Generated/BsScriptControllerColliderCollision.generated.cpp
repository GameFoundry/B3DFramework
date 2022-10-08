//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptControllerColliderCollision.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptGameObjectManager.h"
#include "../../../Foundation/bsfCore/Components/BsCCollider.h"
#include "BsScriptCCollider.generated.h"
#include "Math/BsVector3.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptControllerColliderCollision::ScriptControllerColliderCollision(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptControllerColliderCollision::InitRuntimeData()
	{ }

	MonoObject*ScriptControllerColliderCollision::Box(const __ControllerColliderCollisionInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__ControllerColliderCollisionInterop ScriptControllerColliderCollision::Unbox(MonoObject* value)
	{
		return *(__ControllerColliderCollisionInterop*)MonoUtil::Unbox(value);
	}

	ControllerColliderCollision ScriptControllerColliderCollision::FromInterop(const __ControllerColliderCollisionInterop& value)
	{
		ControllerColliderCollision output;
		GameObjectHandle<CCollider> tmpCollider;
		ScriptCColliderBase* scriptCollider;
		scriptCollider = (ScriptCColliderBase*)ScriptCCollider::ToNative(value.Collider);
		if(scriptCollider != nullptr)
			tmpCollider = static_object_cast<CCollider>(scriptCollider->GetComponent());
		output.Collider = tmpCollider;
		output.TriangleIndex = value.TriangleIndex;
		output.Position = value.Position;
		output.Normal = value.Normal;
		output.MotionDir = value.MotionDir;
		output.MotionAmount = value.MotionAmount;

		return output;
	}

	__ControllerColliderCollisionInterop ScriptControllerColliderCollision::ToInterop(const ControllerColliderCollision& value)
	{
		__ControllerColliderCollisionInterop output;
		MonoObject* tmpCollider;
		ScriptComponentBase* scriptCollider = nullptr;
		if(value.Collider)
			scriptCollider = ScriptGameObjectManager::Instance().GetBuiltinScriptComponent(static_object_cast<Component>(value.Collider));
		if(scriptCollider != nullptr)
			tmpCollider = scriptCollider->GetManagedInstance();
		else
			tmpCollider = nullptr;
		output.Collider = tmpCollider;
		output.TriangleIndex = value.TriangleIndex;
		output.Position = value.Position;
		output.Normal = value.Normal;
		output.MotionDir = value.MotionDir;
		output.MotionAmount = value.MotionAmount;

		return output;
	}

}
