//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

	void ScriptCollisionData::initRuntimeData()
	{ }

	MonoObject*ScriptCollisionData::box(const __CollisionDataInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__CollisionDataInterop ScriptCollisionData::unbox(MonoObject* value)
	{
		return *(__CollisionDataInterop*)MonoUtil::unbox(value);
	}

	CollisionData ScriptCollisionData::fromInterop(const __CollisionDataInterop& value)
	{
		CollisionData output;
		GameObjectHandle<CCollider> veccollider[2];
		if(value.collider != nullptr)
		{
			ScriptArray arraycollider(value.collider);
			for(int i = 0; i < (int)arraycollider.size(); i++)
			{
				ScriptCColliderBase* scriptcollider;
				scriptcollider = (ScriptCColliderBase*)ScriptCCollider::toNative(arraycollider.get<MonoObject*>(i));
				if(scriptcollider != nullptr)
				{
					GameObjectHandle<CCollider> arrayElemPtrcollider = static_object_cast<CCollider>(scriptcollider->getComponent());
					veccollider[i] = arrayElemPtrcollider;
				}
			}
		}
		auto tmpcollider = veccollider;
		for(int i = 0; i < 2; ++i)
			output.collider[i] = tmpcollider[i];
		Vector<ContactPoint> veccontactPoints;
		if(value.contactPoints != nullptr)
		{
			ScriptArray arraycontactPoints(value.contactPoints);
			veccontactPoints.resize(arraycontactPoints.size());
			for(int i = 0; i < (int)arraycontactPoints.size(); i++)
			{
				veccontactPoints[i] = ScriptContactPoint::fromInterop(arraycontactPoints.get<__ContactPointInterop>(i));
			}
		}
		output.contactPoints = veccontactPoints;

		return output;
	}

	__CollisionDataInterop ScriptCollisionData::toInterop(const CollisionData& value)
	{
		__CollisionDataInterop output;
		int arraySizecollider = 2;
		MonoArray* veccollider;
		ScriptArray arraycollider = ScriptArray::create<ScriptCCollider>(arraySizecollider);
		for(int i = 0; i < arraySizecollider; i++)
		{
			ScriptComponentBase* scriptcollider = nullptr;
			if(value.collider[i])
				scriptcollider = ScriptGameObjectManager::instance().getBuiltinScriptComponent(static_object_cast<Component>(value.collider[i]));
				if(scriptcollider != nullptr)
				arraycollider.set(i, scriptcollider->getManagedInstance());
			else
				arraycollider.set(i, nullptr);
		}
		veccollider = arraycollider.getInternal();
		output.collider = veccollider;
		int arraySizecontactPoints = (int)value.contactPoints.size();
		MonoArray* veccontactPoints;
		ScriptArray arraycontactPoints = ScriptArray::create<ScriptContactPoint>(arraySizecontactPoints);
		for(int i = 0; i < arraySizecontactPoints; i++)
		{
			arraycontactPoints.set(i, ScriptContactPoint::toInterop(value.contactPoints[i]));
		}
		veccontactPoints = arraycontactPoints.getInternal();
		output.contactPoints = veccontactPoints;

		return output;
	}

}
