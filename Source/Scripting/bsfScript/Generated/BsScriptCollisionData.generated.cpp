//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCollisionData.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCCollider.h"
#include "BsScriptCCollider.generated.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsCommon.h"
#include "BsScriptContactPoint.generated.h"

namespace bs
{
	ScriptCollisionData::ScriptCollisionData(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptCollisionData::InitRuntimeData()
	{ }

	MonoObject*ScriptCollisionData::Box(const __CollisionDataInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__CollisionDataInterop ScriptCollisionData::Unbox(MonoObject* value)
	{
		return *(__CollisionDataInterop*)MonoUtil::Unbox(value);
	}

	CollisionData ScriptCollisionData::FromInterop(const __CollisionDataInterop& value)
	{
		CollisionData output;
		GameObjectHandle<CCollider> vecCollider[2];
		if(value.Collider != nullptr)
		{
			ScriptArray scriptArrayCollider(value.Collider);
			for(int elementIndex = 0; elementIndex < (int)scriptArrayCollider.Size(); elementIndex++)
			{
				ScriptColliderWrapperBase* scriptWrapperObjectCollider;
				scriptWrapperObjectCollider = (ScriptColliderWrapperBase*)ScriptCollider::GetScriptObjectWrapper(scriptArrayCollider.Get<MonoObject*>(elementIndex));
				if(scriptWrapperObjectCollider != nullptr)
				{
					GameObjectHandle<CCollider> arrayElementPointerCollider = B3DStaticGameObjectCast<CCollider>(scriptWrapperObjectCollider->GetBaseNativeObjectAsHandle());
					vecCollider[elementIndex] = arrayElementPointerCollider;
				}
			}
		}
		auto tmpCollider = vecCollider;
		for(int i = 0; i < 2; ++i)
			output.Collider[i] = tmpCollider[i];
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
