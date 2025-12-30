//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptControllerColliderCollision.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DCollider.h"
#include "B3DScriptCollider.generated.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"
#include "B3DScriptTVector3.generated.h"

namespace b3d
{
	ScriptControllerColliderCollision::ScriptControllerColliderCollision()
	{ }

	MonoObject* ScriptControllerColliderCollision::Box(const __ControllerColliderCollisionInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ControllerColliderCollisionInterop ScriptControllerColliderCollision::Unbox(MonoObject* value)
	{
		return *(__ControllerColliderCollisionInterop*)MonoUtil::Unbox(value);
	}

	ControllerColliderCollision ScriptControllerColliderCollision::FromInterop(const __ControllerColliderCollisionInterop& value)
	{
		ControllerColliderCollision output;
		TGameObjectHandle<Collider> tmpCollider;
		ScriptColliderWrapperBase* scriptObjectWrapperCollider;
		scriptObjectWrapperCollider = (ScriptColliderWrapperBase*)ScriptCollider::GetScriptObjectWrapper(value.Collider);
		if(scriptObjectWrapperCollider != nullptr)
			tmpCollider = B3DStaticGameObjectCast<Collider>(scriptObjectWrapperCollider->GetBaseNativeObjectAsHandle());
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
		MonoObject* temptmpCollider = nullptr;
		if(value.Collider)
			temptmpCollider = ScriptComponent::GetOrCreateScriptObject(value.Collider);
		tmpCollider = temptmpCollider;
		output.Collider = tmpCollider;
		output.TriangleIndex = value.TriangleIndex;
		output.Position = value.Position;
		output.Normal = value.Normal;
		output.MotionDir = value.MotionDir;
		output.MotionAmount = value.MotionAmount;

		return output;
	}

}
