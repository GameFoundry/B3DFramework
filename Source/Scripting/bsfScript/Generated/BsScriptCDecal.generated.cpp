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

	void ScriptDecal::InternalSetMaterial(ScriptDecal* thisPtr, MonoObject* material)
	{
		ResourceHandle<Material> tmpmaterial;
		ScriptRRefBase* scriptmaterial;
		scriptmaterial = ScriptRRefBase::ToNative(material);
		if(scriptmaterial != nullptr)
			tmpmaterial = B3DStaticResourceCast<Material>(scriptmaterial->GetHandle());
		thisPtr->GetHandle()->SetMaterial(tmpmaterial);
	}

	MonoObject* ScriptDecal::InternalGetMaterial(ScriptDecal* thisPtr)
	{
		ResourceHandle<Material> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMaterial();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptDecal::InternalSetSize(ScriptDecal* thisPtr, TVector2<float>* size)
	{
		thisPtr->GetHandle()->SetSize(*size);
	}

	void ScriptDecal::InternalGetSize(ScriptDecal* thisPtr, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetSize();

		*__output = tmp__output;
	}

	void ScriptDecal::InternalSetMaxDistance(ScriptDecal* thisPtr, float distance)
	{
		thisPtr->GetHandle()->SetMaxDistance(distance);
	}

	float ScriptDecal::InternalGetMaxDistance(ScriptDecal* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMaxDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDecal::InternalSetLayer(ScriptDecal* thisPtr, uint64_t layer)
	{
		thisPtr->GetHandle()->SetLayer(layer);
	}

	uint64_t ScriptDecal::InternalGetLayer(ScriptDecal* thisPtr)
	{
		uint64_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDecal::InternalSetLayerMask(ScriptDecal* thisPtr, uint32_t mask)
	{
		thisPtr->GetHandle()->SetLayerMask(mask);
	}

	uint32_t ScriptDecal::InternalGetLayerMask(ScriptDecal* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetLayerMask();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}
}
