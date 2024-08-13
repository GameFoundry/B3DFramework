//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCPlaneCollider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCPlaneCollider.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptPlaneCollider::ScriptPlaneCollider(MonoObject* managedInstance, const GameObjectHandle<CPlaneCollider>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptPlaneCollider::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetNormal", (void*)&ScriptPlaneCollider::InternalSetNormal);
		metaData.ScriptClass->AddInternalCall("Internal_GetNormal", (void*)&ScriptPlaneCollider::InternalGetNormal);
		metaData.ScriptClass->AddInternalCall("Internal_SetDistance", (void*)&ScriptPlaneCollider::InternalSetDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetDistance", (void*)&ScriptPlaneCollider::InternalGetDistance);

	}

	void ScriptPlaneCollider::InternalSetNormal(ScriptPlaneCollider* self, TVector3<float>* normal)
	{
		self->GetHandle()->SetNormal(*normal);
	}

	void ScriptPlaneCollider::InternalGetNormal(ScriptPlaneCollider* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->GetNormal();

		*__output = tmp__output;
	}

	void ScriptPlaneCollider::InternalSetDistance(ScriptPlaneCollider* self, float distance)
	{
		self->GetHandle()->SetDistance(distance);
	}

	float ScriptPlaneCollider::InternalGetDistance(ScriptPlaneCollider* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}
}
