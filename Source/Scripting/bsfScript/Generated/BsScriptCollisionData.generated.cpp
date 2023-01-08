//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCollisionData.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptGameObjectManager.h"
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
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
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
			ScriptArray arrayCollider(value.Collider);
			for(int i = 0; i < (int)arrayCollider.Size(); i++)
			{
				ScriptColliderBase* scriptCollider;
				scriptCollider = (ScriptColliderBase*)ScriptCollider::ToNative(arrayCollider.Get<MonoObject*>(i));
				if(scriptCollider != nullptr)
				{
					GameObjectHandle<CCollider> arrayElemPtrCollider = B3DStaticGameObjectCast<CCollider>(scriptCollider->GetComponent());
					vecCollider[i] = arrayElemPtrCollider;
				}
			}
		}
		auto tmpCollider = vecCollider;
		for(int i = 0; i < 2; ++i)
			output.Collider[i] = tmpCollider[i];
		Vector<ContactPoint> vecContactPoints;
		if(value.ContactPoints != nullptr)
		{
			ScriptArray arrayContactPoints(value.ContactPoints);
			vecContactPoints.resize(arrayContactPoints.Size());
			for(int i = 0; i < (int)arrayContactPoints.Size(); i++)
			{
				vecContactPoints[i] = ScriptContactPoint::FromInterop(arrayContactPoints.Get<__ContactPointInterop>(i));
			}
		}
		output.ContactPoints = vecContactPoints;

		return output;
	}

	__CollisionDataInterop ScriptCollisionData::ToInterop(const CollisionData& value)
	{
		__CollisionDataInterop output;
		int arraySizeCollider = 2;
		MonoArray* vecCollider;
		ScriptArray arrayCollider = ScriptArray::Create<ScriptCollider>(arraySizeCollider);
		for(int i = 0; i < arraySizeCollider; i++)
		{
			ScriptComponentBase* scriptCollider = nullptr;
			if(value.Collider[i])
				scriptCollider = ScriptGameObjectManager::Instance().GetBuiltinScriptComponent(B3DStaticGameObjectCast<Component>(value.Collider[i]));
			if(scriptCollider != nullptr)
				arrayCollider.Set(i, scriptCollider->GetManagedInstance());
			else
				arrayCollider.Set(i, nullptr);
		}
		vecCollider = arrayCollider.GetInternal();
		output.Collider = vecCollider;
		int arraySizeContactPoints = (int)value.ContactPoints.size();
		MonoArray* vecContactPoints;
		ScriptArray arrayContactPoints = ScriptArray::Create<ScriptContactPoint>(arraySizeContactPoints);
		for(int i = 0; i < arraySizeContactPoints; i++)
		{
			arrayContactPoints.Set(i, ScriptContactPoint::ToInterop(value.ContactPoints[i]));
		}
		vecContactPoints = arrayContactPoints.GetInternal();
		output.ContactPoints = vecContactPoints;

		return output;
	}

}
