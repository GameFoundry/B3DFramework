//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptCollisionData.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DCollider.h"
#include "B3DScriptCollider.generated.h"
#include "../../../Foundation/Core/Physics/B3DColliderShape.h"
#include "B3DScriptColliderShape.generated.h"
#include "../../../Foundation/Core/Physics/B3DPhysicsCommon.h"
#include "B3DScriptContactPoint.generated.h"

namespace b3d
{
	ScriptCollisionData::ScriptCollisionData()
	{ }

	MonoObject* ScriptCollisionData::Box(const __CollisionDataInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__CollisionDataInterop ScriptCollisionData::Unbox(MonoObject* value)
	{
		return *(__CollisionDataInterop*)MonoUtil::Unbox(value);
	}

	CollisionData ScriptCollisionData::FromInterop(const __CollisionDataInterop& value)
	{
		CollisionData output;
		TGameObjectHandle<Collider> vecCollider[2];
		if(value.Collider != nullptr)
		{
			ScriptArray scriptArrayCollider(value.Collider);
			for(int elementIndex = 0; elementIndex < (int)scriptArrayCollider.Size(); elementIndex++)
			{
				TGameObjectHandle<Collider> arrayElementPointerCollider;
				ScriptColliderWrapperBase* scriptObjectWrapperCollider;
				scriptObjectWrapperCollider = (ScriptColliderWrapperBase*)ScriptCollider::GetScriptObjectWrapper(scriptArrayCollider.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrapperCollider != nullptr)
				{
					arrayElementPointerCollider = B3DStaticGameObjectCast<Collider>(scriptObjectWrapperCollider->GetBaseNativeObjectAsHandle());
					vecCollider[elementIndex] = arrayElementPointerCollider;
				}
			}
		}
		auto tmpCollider = vecCollider;
		for(int i = 0; i < 2; ++i)
			output.Collider[i] = tmpCollider[i];
		SPtr<ColliderShape> vecColliderShapes[2];
		if(value.ColliderShapes != nullptr)
		{
			ScriptArray scriptArrayColliderShapes(value.ColliderShapes);
			for(int elementIndex = 0; elementIndex < (int)scriptArrayColliderShapes.Size(); elementIndex++)
			{
				SPtr<ColliderShape> arrayElementPointerColliderShapes;
				ScriptColliderShape* scriptObjectWrapperColliderShapes;
				scriptObjectWrapperColliderShapes = ScriptColliderShape::GetScriptObjectWrapper(scriptArrayColliderShapes.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrapperColliderShapes != nullptr)
				{
					arrayElementPointerColliderShapes = std::static_pointer_cast<ColliderShape>(scriptObjectWrapperColliderShapes->GetBaseNativeObjectAsShared());
					vecColliderShapes[elementIndex] = arrayElementPointerColliderShapes;
				}
			}
		}
		auto tmpColliderShapes = vecColliderShapes;
		for(int i = 0; i < 2; ++i)
			output.ColliderShapes[i] = tmpColliderShapes[i];
		Vector<ContactPoint> vecContactPoints;
		if(value.ContactPoints != nullptr)
		{
			ScriptArray scriptArrayContactPoints(value.ContactPoints);
			vecContactPoints.resize(scriptArrayContactPoints.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayContactPoints.Size(); elementIndex++)
			{
				vecContactPoints[elementIndex] = ScriptContactPoint::FromInterop(scriptArrayContactPoints.Get<__ContactPointInterop>(elementIndex));
			}
		}
		output.ContactPoints = vecContactPoints;

		return output;
	}

	__CollisionDataInterop ScriptCollisionData::ToInterop(const CollisionData& value)
	{
		__CollisionDataInterop output;
		int elementCountCollider = 2;
		MonoArray* vecCollider;
		ScriptArray scriptArrayCollider = ScriptArray::Create<ScriptCollider>(elementCountCollider);
		for(int elementIndex = 0; elementIndex < elementCountCollider; elementIndex++)
		{
			MonoObject* tempscriptArrayCollider = nullptr;
			if(value.Collider[elementIndex])
				tempscriptArrayCollider = ScriptComponent::GetOrCreateScriptObject(value.Collider[elementIndex]);
			scriptArrayCollider.Set(elementIndex, tempscriptArrayCollider);
		}
		vecCollider = scriptArrayCollider.GetInternal();
		output.Collider = vecCollider;
		int elementCountColliderShapes = 2;
		MonoArray* vecColliderShapes;
		ScriptArray scriptArrayColliderShapes = ScriptArray::Create<ScriptColliderShape>(elementCountColliderShapes);
		for(int elementIndex = 0; elementIndex < elementCountColliderShapes; elementIndex++)
		{
			SPtr<ColliderShape> arrayElementPointerColliderShapes = value.ColliderShapes[elementIndex];
			MonoObject* arrayElementColliderShapes;
			arrayElementColliderShapes = ScriptColliderShape::GetOrCreateScriptObject(arrayElementPointerColliderShapes);
			scriptArrayColliderShapes.Set(elementIndex, arrayElementColliderShapes);
		}
		vecColliderShapes = scriptArrayColliderShapes.GetInternal();
		output.ColliderShapes = vecColliderShapes;
		int elementCountContactPoints = (int)value.ContactPoints.size();
		MonoArray* vecContactPoints;
		ScriptArray scriptArrayContactPoints = ScriptArray::Create<ScriptContactPoint>(elementCountContactPoints);
		for(int elementIndex = 0; elementIndex < elementCountContactPoints; elementIndex++)
		{
			scriptArrayContactPoints.Set(elementIndex, ScriptContactPoint::ToInterop(value.ContactPoints[elementIndex]));
		}
		vecContactPoints = scriptArrayContactPoints.GetInternal();
		output.ContactPoints = vecContactPoints;

		return output;
	}

}
