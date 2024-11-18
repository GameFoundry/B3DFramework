//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCBoxCollider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCBoxCollider.h"
#include "BsScriptTVector3.generated.h"

namespace bs
{
	ScriptBoxCollider::ScriptBoxCollider(const GameObjectHandle<CBoxCollider>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptBoxCollider::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetExtents", (void*)&ScriptBoxCollider::InternalSetExtents);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetExtents", (void*)&ScriptBoxCollider::InternalGetExtents);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCenter", (void*)&ScriptBoxCollider::InternalSetCenter);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCenter", (void*)&ScriptBoxCollider::InternalGetCenter);

	}

	MonoObject* ScriptBoxCollider::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptBoxCollider::InternalSetExtents(ScriptBoxCollider* self, TVector3<float>* extents)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CBoxCollider*>(self->GetNativeObject())->SetExtents(*extents);
	}

	void ScriptBoxCollider::InternalGetExtents(ScriptBoxCollider* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		TVector3<float> tmp__output;
		tmp__output = static_cast<CBoxCollider*>(self->GetNativeObject())->GetExtents();

		*__output = tmp__output;
	}

	void ScriptBoxCollider::InternalSetCenter(ScriptBoxCollider* self, TVector3<float>* center)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CBoxCollider*>(self->GetNativeObject())->SetCenter(*center);
	}

	void ScriptBoxCollider::InternalGetCenter(ScriptBoxCollider* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		TVector3<float> tmp__output;
		tmp__output = static_cast<CBoxCollider*>(self->GetNativeObject())->GetCenter();

		*__output = tmp__output;
	}
}
