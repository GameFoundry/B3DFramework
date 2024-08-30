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
	ScriptDecal::ScriptDecal(const GameObjectHandle<CDecal>& nativeObject, MonoObject* scriptObject)
		:TScriptGameObjectWrapper(nativeObject, scriptObject)
	{
		RegisterEvents();
	}

	void ScriptDecal::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMaterial", (void*)&ScriptDecal::InternalSetMaterial);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaterial", (void*)&ScriptDecal::InternalGetMaterial);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSize", (void*)&ScriptDecal::InternalSetSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSize", (void*)&ScriptDecal::InternalGetSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMaxDistance", (void*)&ScriptDecal::InternalSetMaxDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxDistance", (void*)&ScriptDecal::InternalGetMaxDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLayer", (void*)&ScriptDecal::InternalSetLayer);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLayer", (void*)&ScriptDecal::InternalGetLayer);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLayerMask", (void*)&ScriptDecal::InternalSetLayerMask);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLayerMask", (void*)&ScriptDecal::InternalGetLayerMask);

	}

	MonoObject* ScriptDecal::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptDecal::InternalSetMaterial(ScriptDecal* self, MonoObject* material)
	{
		TResourceHandle<Material> tmpmaterial;
		ScriptRRefBase* scriptObjectWrappermaterial;
		scriptObjectWrappermaterial = ScriptRRefBase::ToNative(material);
		if(scriptObjectWrappermaterial != nullptr)
			tmpmaterial = B3DStaticResourceCast<Material>(scriptObjectWrappermaterial->GetHandle());
		static_cast<CDecal*>(self->GetNativeObject())->SetMaterial(tmpmaterial);
	}

	MonoObject* ScriptDecal::InternalGetMaterial(ScriptDecal* self)
	{
		TResourceHandle<Material> tmp__output;
		tmp__output = static_cast<CDecal*>(self->GetNativeObject())->GetMaterial();

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
		static_cast<CDecal*>(self->GetNativeObject())->SetSize(*size);
	}

	void ScriptDecal::InternalGetSize(ScriptDecal* self, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		tmp__output = static_cast<CDecal*>(self->GetNativeObject())->GetSize();

		*__output = tmp__output;
	}

	void ScriptDecal::InternalSetMaxDistance(ScriptDecal* self, float distance)
	{
		static_cast<CDecal*>(self->GetNativeObject())->SetMaxDistance(distance);
	}

	float ScriptDecal::InternalGetMaxDistance(ScriptDecal* self)
	{
		float tmp__output;
		tmp__output = static_cast<CDecal*>(self->GetNativeObject())->GetMaxDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDecal::InternalSetLayer(ScriptDecal* self, uint64_t layer)
	{
		static_cast<CDecal*>(self->GetNativeObject())->SetLayer(layer);
	}

	uint64_t ScriptDecal::InternalGetLayer(ScriptDecal* self)
	{
		uint64_t tmp__output;
		tmp__output = static_cast<CDecal*>(self->GetNativeObject())->GetLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDecal::InternalSetLayerMask(ScriptDecal* self, uint32_t mask)
	{
		static_cast<CDecal*>(self->GetNativeObject())->SetLayerMask(mask);
	}

	uint32_t ScriptDecal::InternalGetLayerMask(ScriptDecal* self)
	{
		uint32_t tmp__output;
		tmp__output = static_cast<CDecal*>(self->GetNativeObject())->GetLayerMask();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}
}
