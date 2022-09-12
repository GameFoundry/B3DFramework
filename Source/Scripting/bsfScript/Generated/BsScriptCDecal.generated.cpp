//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCDecal.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCDecal.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "Wrappers/BsScriptVector.h"
#include "../../../Foundation/bsfCore/Material/BsMaterial.h"

namespace bs
{
	ScriptCDecal::ScriptCDecal(MonoObject* managedInstance, const GameObjectHandle<CDecal>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCDecal::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_setMaterial", (void*)&ScriptCDecal::Internal_setMaterial);
		metaData.scriptClass->AddInternalCall("Internal_getMaterial", (void*)&ScriptCDecal::Internal_getMaterial);
		metaData.scriptClass->AddInternalCall("Internal_setSize", (void*)&ScriptCDecal::Internal_setSize);
		metaData.scriptClass->AddInternalCall("Internal_getSize", (void*)&ScriptCDecal::Internal_getSize);
		metaData.scriptClass->AddInternalCall("Internal_setMaxDistance", (void*)&ScriptCDecal::Internal_setMaxDistance);
		metaData.scriptClass->AddInternalCall("Internal_getMaxDistance", (void*)&ScriptCDecal::Internal_getMaxDistance);
		metaData.scriptClass->AddInternalCall("Internal_setLayer", (void*)&ScriptCDecal::Internal_setLayer);
		metaData.scriptClass->AddInternalCall("Internal_getLayer", (void*)&ScriptCDecal::Internal_getLayer);
		metaData.scriptClass->AddInternalCall("Internal_setLayerMask", (void*)&ScriptCDecal::Internal_setLayerMask);
		metaData.scriptClass->AddInternalCall("Internal_getLayerMask", (void*)&ScriptCDecal::Internal_getLayerMask);

	}

	void ScriptCDecal::Internal_setMaterial(ScriptCDecal* thisPtr, MonoObject* material)
	{
		ResourceHandle<Material> tmpmaterial;
		ScriptRRefBase* scriptmaterial;
		scriptmaterial = ScriptRRefBase::toNative(material);
		if(scriptmaterial != nullptr)
			tmpmaterial = static_resource_cast<Material>(scriptmaterial->GetHandle());
		thisPtr->GetHandle()->setMaterial(tmpmaterial);
	}

	MonoObject* ScriptCDecal::Internal_getMaterial(ScriptCDecal* thisPtr)
	{
		ResourceHandle<Material> tmp__output;
		tmp__output = thisPtr->GetHandle()->getMaterial();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptCDecal::Internal_setSize(ScriptCDecal* thisPtr, Vector2* size)
	{
		thisPtr->GetHandle()->setSize(*size);
	}

	void ScriptCDecal::Internal_getSize(ScriptCDecal* thisPtr, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetHandle()->getSize();

		*__output = tmp__output;
	}

	void ScriptCDecal::Internal_setMaxDistance(ScriptCDecal* thisPtr, float distance)
	{
		thisPtr->GetHandle()->setMaxDistance(distance);
	}

	float ScriptCDecal::Internal_getMaxDistance(ScriptCDecal* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getMaxDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCDecal::Internal_setLayer(ScriptCDecal* thisPtr, uint64_t layer)
	{
		thisPtr->GetHandle()->setLayer(layer);
	}

	uint64_t ScriptCDecal::Internal_getLayer(ScriptCDecal* thisPtr)
	{
		uint64_t tmp__output;
		tmp__output = thisPtr->GetHandle()->getLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCDecal::Internal_setLayerMask(ScriptCDecal* thisPtr, uint32_t mask)
	{
		thisPtr->GetHandle()->setLayerMask(mask);
	}

	uint32_t ScriptCDecal::Internal_getLayerMask(ScriptCDecal* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->getLayerMask();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}
}
