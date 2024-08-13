//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCRenderable.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCRenderable.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Mesh/BsMesh.h"
#include "../../../Foundation/bsfCore/Material/BsMaterial.h"

namespace bs
{
	ScriptRenderable::ScriptRenderable(MonoObject* managedInstance, const GameObjectHandle<CRenderable>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptRenderable::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetMesh", (void*)&ScriptRenderable::InternalSetMesh);
		metaData.ScriptClass->AddInternalCall("Internal_GetMesh", (void*)&ScriptRenderable::InternalGetMesh);
		metaData.ScriptClass->AddInternalCall("Internal_SetMaterial", (void*)&ScriptRenderable::InternalSetMaterial);
		metaData.ScriptClass->AddInternalCall("Internal_SetMaterial0", (void*)&ScriptRenderable::InternalSetMaterial0);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaterial", (void*)&ScriptRenderable::InternalGetMaterial);
		metaData.ScriptClass->AddInternalCall("Internal_SetMaterials", (void*)&ScriptRenderable::InternalSetMaterials);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaterials", (void*)&ScriptRenderable::InternalGetMaterials);
		metaData.ScriptClass->AddInternalCall("Internal_SetCullDistanceFactor", (void*)&ScriptRenderable::InternalSetCullDistanceFactor);
		metaData.ScriptClass->AddInternalCall("Internal_GetCullDistanceFactor", (void*)&ScriptRenderable::InternalGetCullDistanceFactor);
		metaData.ScriptClass->AddInternalCall("Internal_SetWriteVelocity", (void*)&ScriptRenderable::InternalSetWriteVelocity);
		metaData.ScriptClass->AddInternalCall("Internal_GetWriteVelocity", (void*)&ScriptRenderable::InternalGetWriteVelocity);
		metaData.ScriptClass->AddInternalCall("Internal_SetLayer", (void*)&ScriptRenderable::InternalSetLayer);
		metaData.ScriptClass->AddInternalCall("Internal_GetLayer", (void*)&ScriptRenderable::InternalGetLayer);
		metaData.ScriptClass->AddInternalCall("Internal_GetBounds", (void*)&ScriptRenderable::InternalGetBounds);

	}

	void ScriptRenderable::InternalSetMesh(ScriptRenderable* self, MonoObject* mesh)
	{
		TResourceHandle<Mesh> tmpmesh;
		ScriptRRefBase* scriptObjectWrappermesh;
		scriptObjectWrappermesh = ScriptRRefBase::ToNative(mesh);
		if(scriptObjectWrappermesh != nullptr)
			tmpmesh = B3DStaticResourceCast<Mesh>(scriptObjectWrappermesh->GetHandle());
		self->GetHandle()->SetMesh(tmpmesh);
	}

	MonoObject* ScriptRenderable::InternalGetMesh(ScriptRenderable* self)
	{
		TResourceHandle<Mesh> tmp__output;
		tmp__output = self->GetHandle()->GetMesh();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptRenderable::InternalSetMaterial(ScriptRenderable* self, uint32_t idx, MonoObject* material)
	{
		TResourceHandle<Material> tmpmaterial;
		ScriptRRefBase* scriptObjectWrappermaterial;
		scriptObjectWrappermaterial = ScriptRRefBase::ToNative(material);
		if(scriptObjectWrappermaterial != nullptr)
			tmpmaterial = B3DStaticResourceCast<Material>(scriptObjectWrappermaterial->GetHandle());
		self->GetHandle()->SetMaterial(idx, tmpmaterial);
	}

	void ScriptRenderable::InternalSetMaterial0(ScriptRenderable* self, MonoObject* material)
	{
		TResourceHandle<Material> tmpmaterial;
		ScriptRRefBase* scriptObjectWrappermaterial;
		scriptObjectWrappermaterial = ScriptRRefBase::ToNative(material);
		if(scriptObjectWrappermaterial != nullptr)
			tmpmaterial = B3DStaticResourceCast<Material>(scriptObjectWrappermaterial->GetHandle());
		self->GetHandle()->SetMaterial(tmpmaterial);
	}

	MonoObject* ScriptRenderable::InternalGetMaterial(ScriptRenderable* self, uint32_t idx)
	{
		TResourceHandle<Material> tmp__output;
		tmp__output = self->GetHandle()->GetMaterial(idx);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptRenderable::InternalSetMaterials(ScriptRenderable* self, MonoArray* materials)
	{
		Vector<TResourceHandle<Material>> nativeArraymaterials;
		if(materials != nullptr)
		{
			ScriptArray scriptArraymaterials(materials);
			nativeArraymaterials.resize(scriptArraymaterials.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraymaterials.Size(); elementIndex++)
			{
				ScriptRRefBase* scriptObjectWrappermaterials;
				scriptObjectWrappermaterials = ScriptRRefBase::ToNative(scriptArraymaterials.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappermaterials != nullptr)
				{
					TResourceHandle<Material> arrayElementPointermaterials = B3DStaticResourceCast<Material>(scriptObjectWrappermaterials->GetHandle());
					nativeArraymaterials[elementIndex] = arrayElementPointermaterials;
				}
			}
		}
		self->GetHandle()->SetMaterials(nativeArraymaterials);
	}

	MonoArray* ScriptRenderable::InternalGetMaterials(ScriptRenderable* self)
	{
		Vector<TResourceHandle<Material>> nativeArray__output;
		nativeArray__output = self->GetHandle()->GetMaterials();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptRRefBase>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			ScriptRRefBase* scriptObjectWrapper__output;
			scriptObjectWrapper__output = ScriptResourceManager::Instance().GetScriptRRef(nativeArray__output[elementIndex]);
			if(scriptObjectWrapper__output != nullptr)
				scriptArray__output.Set(elementIndex, scriptObjectWrapper__output->GetManagedInstance());
			else
				scriptArray__output.Set(elementIndex, nullptr);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptRenderable::InternalSetCullDistanceFactor(ScriptRenderable* self, float factor)
	{
		self->GetHandle()->SetCullDistanceFactor(factor);
	}

	float ScriptRenderable::InternalGetCullDistanceFactor(ScriptRenderable* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetCullDistanceFactor();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderable::InternalSetWriteVelocity(ScriptRenderable* self, bool enable)
	{
		self->GetHandle()->SetWriteVelocity(enable);
	}

	bool ScriptRenderable::InternalGetWriteVelocity(ScriptRenderable* self)
	{
		bool tmp__output;
		tmp__output = self->GetHandle()->GetWriteVelocity();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderable::InternalSetLayer(ScriptRenderable* self, uint64_t layer)
	{
		self->GetHandle()->SetLayer(layer);
	}

	uint64_t ScriptRenderable::InternalGetLayer(ScriptRenderable* self)
	{
		uint64_t tmp__output;
		tmp__output = self->GetHandle()->GetLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderable::InternalGetBounds(ScriptRenderable* self, Bounds* __output)
	{
		Bounds tmp__output;
		tmp__output = self->GetHandle()->GetBounds();

		*__output = tmp__output;
	}
}
