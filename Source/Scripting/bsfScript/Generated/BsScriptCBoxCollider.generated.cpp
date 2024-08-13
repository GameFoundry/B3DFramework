//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCBoxCollider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCBoxCollider.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptBoxCollider::ScriptBoxCollider(MonoObject* managedInstance, const GameObjectHandle<CBoxCollider>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptBoxCollider::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetExtents", (void*)&ScriptBoxCollider::InternalSetExtents);
		metaData.ScriptClass->AddInternalCall("Internal_GetExtents", (void*)&ScriptBoxCollider::InternalGetExtents);
		metaData.ScriptClass->AddInternalCall("Internal_SetCenter", (void*)&ScriptBoxCollider::InternalSetCenter);
		metaData.ScriptClass->AddInternalCall("Internal_GetCenter", (void*)&ScriptBoxCollider::InternalGetCenter);

	}

	void ScriptBoxCollider::InternalSetExtents(ScriptBoxCollider* self, TVector3<float>* extents)
	{
		self->GetHandle()->SetExtents(*extents);
	}

	void ScriptBoxCollider::InternalGetExtents(ScriptBoxCollider* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->GetExtents();

		*__output = tmp__output;
	}

	void ScriptBoxCollider::InternalSetCenter(ScriptBoxCollider* self, TVector3<float>* center)
	{
		self->GetHandle()->SetCenter(*center);
	}

	void ScriptBoxCollider::InternalGetCenter(ScriptBoxCollider* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->GetCenter();

		*__output = tmp__output;
	}
}
