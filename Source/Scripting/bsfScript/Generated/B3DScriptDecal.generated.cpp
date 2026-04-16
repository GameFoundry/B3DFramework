//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptDecal.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Components/B3DDecal.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "B3DScriptTVector2.generated.h"
#include "../../../Engine/Core/Material/B3DMaterial.h"

namespace b3d
{
	ScriptDecal::ScriptDecal(const TGameObjectHandle<Decal>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptDecal::~ScriptDecal()
	{
		UnregisterEvents();
	}

	void ScriptDecal::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSize", (void*)&ScriptDecal::InternalSetSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMaterial", (void*)&ScriptDecal::InternalSetMaterial);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMaxDistance", (void*)&ScriptDecal::InternalSetMaxDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLayerMask", (void*)&ScriptDecal::InternalSetLayerMask);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLayer", (void*)&ScriptDecal::InternalSetLayer);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSize", (void*)&ScriptDecal::InternalGetSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaterial", (void*)&ScriptDecal::InternalGetMaterial);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxDistance", (void*)&ScriptDecal::InternalGetMaxDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLayerMask", (void*)&ScriptDecal::InternalGetLayerMask);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLayer", (void*)&ScriptDecal::InternalGetLayer);

	}

	MonoObject* ScriptDecal::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptDecal::InternalSetSize(ScriptDecal* self, TVector2<float>* size)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Decal*>(self->GetNativeObject())->SetSize(*size);
	}

	void ScriptDecal::InternalSetMaterial(ScriptDecal* self, MonoObject* material)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<Material> tmpmaterial;
		ScriptRRefBase* scriptObjectWrappermaterial;
		scriptObjectWrappermaterial = ScriptRRefBase::GetScriptObjectWrapper(material);
		if(scriptObjectWrappermaterial != nullptr)
			tmpmaterial = B3DStaticResourceCast<Material>(scriptObjectWrappermaterial->GetNativeObject());
		static_cast<Decal*>(self->GetNativeObject())->SetMaterial(tmpmaterial);
	}

	void ScriptDecal::InternalSetMaxDistance(ScriptDecal* self, float distance)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Decal*>(self->GetNativeObject())->SetMaxDistance(distance);
	}

	void ScriptDecal::InternalSetLayerMask(ScriptDecal* self, uint32_t mask)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Decal*>(self->GetNativeObject())->SetLayerMask(mask);
	}

	void ScriptDecal::InternalSetLayer(ScriptDecal* self, uint64_t layer)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Decal*>(self->GetNativeObject())->SetLayer(layer);
	}

	void ScriptDecal::InternalGetSize(ScriptDecal* self, TVector2<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector2<float> tmp__output;
		tmp__output = static_cast<Decal*>(self->GetNativeObject())->GetSize();

		*__output = tmp__output;
	}

	MonoObject* ScriptDecal::InternalGetMaterial(ScriptDecal* self)
	{
		TResourceHandle<Material> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Decal*>(self->GetNativeObject())->GetMaterial();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	float ScriptDecal::InternalGetMaxDistance(ScriptDecal* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Decal*>(self->GetNativeObject())->GetMaxDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptDecal::InternalGetLayerMask(ScriptDecal* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Decal*>(self->GetNativeObject())->GetLayerMask();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint64_t ScriptDecal::InternalGetLayer(ScriptDecal* self)
	{
		uint64_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Decal*>(self->GetNativeObject())->GetLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}
}
