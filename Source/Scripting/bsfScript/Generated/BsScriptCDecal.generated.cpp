//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCDecal.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCDecal.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Material/BsMaterial.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptDecal::ScriptDecal(MonoObject* managedInstance, const GameObjectHandle<CDecal>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptDecal::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetMaterial", (void*)&ScriptDecal::InternalSetMaterial);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaterial", (void*)&ScriptDecal::InternalGetMaterial);
		metaData.ScriptClass->AddInternalCall("Internal_SetSize", (void*)&ScriptDecal::InternalSetSize);
		metaData.ScriptClass->AddInternalCall("Internal_GetSize", (void*)&ScriptDecal::InternalGetSize);
		metaData.ScriptClass->AddInternalCall("Internal_SetMaxDistance", (void*)&ScriptDecal::InternalSetMaxDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxDistance", (void*)&ScriptDecal::InternalGetMaxDistance);
		metaData.ScriptClass->AddInternalCall("Internal_SetLayer", (void*)&ScriptDecal::InternalSetLayer);
		metaData.ScriptClass->AddInternalCall("Internal_GetLayer", (void*)&ScriptDecal::InternalGetLayer);
		metaData.ScriptClass->AddInternalCall("Internal_SetLayerMask", (void*)&ScriptDecal::InternalSetLayerMask);
		metaData.ScriptClass->AddInternalCall("Internal_GetLayerMask", (void*)&ScriptDecal::InternalGetLayerMask);

	}

	void ScriptDecal::InternalSetMaterial(ScriptDecal* self, MonoObject* material)
	{
		TResourceHandle<Material> tmpmaterial;
		ScriptRRefBase* scriptObjectWrappermaterial;
		scriptObjectWrappermaterial = ScriptRRefBase::ToNative(material);
		if(scriptObjectWrappermaterial != nullptr)
			tmpmaterial = B3DStaticResourceCast<Material>(scriptObjectWrappermaterial->GetHandle());
		self->GetHandle()->SetMaterial(tmpmaterial);
	}

	MonoObject* ScriptDecal::InternalGetMaterial(ScriptDecal* self)
	{
		TResourceHandle<Material> tmp__output;
		tmp__output = self->GetHandle()->GetMaterial();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptDecal::InternalSetSize(ScriptDecal* self, TVector2<float>* size)
	{
		self->GetHandle()->SetSize(*size);
	}

	void ScriptDecal::InternalGetSize(ScriptDecal* self, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		tmp__output = self->GetHandle()->GetSize();

		*__output = tmp__output;
	}

	void ScriptDecal::InternalSetMaxDistance(ScriptDecal* self, float distance)
	{
		self->GetHandle()->SetMaxDistance(distance);
	}

	float ScriptDecal::InternalGetMaxDistance(ScriptDecal* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetMaxDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDecal::InternalSetLayer(ScriptDecal* self, uint64_t layer)
	{
		self->GetHandle()->SetLayer(layer);
	}

	uint64_t ScriptDecal::InternalGetLayer(ScriptDecal* self)
	{
		uint64_t tmp__output;
		tmp__output = self->GetHandle()->GetLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDecal::InternalSetLayerMask(ScriptDecal* self, uint32_t mask)
	{
		self->GetHandle()->SetLayerMask(mask);
	}

	uint32_t ScriptDecal::InternalGetLayerMask(ScriptDecal* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetHandle()->GetLayerMask();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}
}
