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

	void ScriptRenderable::InternalSetMesh(ScriptRenderable* thisPtr, MonoObject* mesh)
	{
		ResourceHandle<Mesh> tmpmesh;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptRRefBase::ToNative(mesh);
		if(scriptmesh != nullptr)
			tmpmesh = B3DStaticResourceCast<Mesh>(scriptmesh->GetHandle());
		thisPtr->GetHandle()->SetMesh(tmpmesh);
	}

	MonoObject* ScriptRenderable::InternalGetMesh(ScriptRenderable* thisPtr)
	{
		ResourceHandle<Mesh> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMesh();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptRenderable::InternalSetMaterial(ScriptRenderable* thisPtr, uint32_t idx, MonoObject* material)
	{
		ResourceHandle<Material> tmpmaterial;
		ScriptRRefBase* scriptmaterial;
		scriptmaterial = ScriptRRefBase::ToNative(material);
		if(scriptmaterial != nullptr)
			tmpmaterial = B3DStaticResourceCast<Material>(scriptmaterial->GetHandle());
		thisPtr->GetHandle()->SetMaterial(idx, tmpmaterial);
	}

	void ScriptRenderable::InternalSetMaterial0(ScriptRenderable* thisPtr, MonoObject* material)
	{
		ResourceHandle<Material> tmpmaterial;
		ScriptRRefBase* scriptmaterial;
		scriptmaterial = ScriptRRefBase::ToNative(material);
		if(scriptmaterial != nullptr)
			tmpmaterial = B3DStaticResourceCast<Material>(scriptmaterial->GetHandle());
		thisPtr->GetHandle()->SetMaterial(tmpmaterial);
	}

	MonoObject* ScriptRenderable::InternalGetMaterial(ScriptRenderable* thisPtr, uint32_t idx)
	{
		ResourceHandle<Material> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMaterial(idx);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptRenderable::InternalSetMaterials(ScriptRenderable* thisPtr, MonoArray* materials)
	{
		Vector<ResourceHandle<Material>> vecmaterials;
		if(materials != nullptr)
		{
			ScriptArray arraymaterials(materials);
			vecmaterials.resize(arraymaterials.Size());
			for(int i = 0; i < (int)arraymaterials.Size(); i++)
			{
				ScriptRRefBase* scriptmaterials;
				scriptmaterials = ScriptRRefBase::ToNative(arraymaterials.Get<MonoObject*>(i));
				if(scriptmaterials != nullptr)
				{
					ResourceHandle<Material> arrayElemPtrmaterials = B3DStaticResourceCast<Material>(scriptmaterials->GetHandle());
					vecmaterials[i] = arrayElemPtrmaterials;
				}
			}
		}
		thisPtr->GetHandle()->SetMaterials(vecmaterials);
	}

	MonoArray* ScriptRenderable::InternalGetMaterials(ScriptRenderable* thisPtr)
	{
		Vector<ResourceHandle<Material>> vec__output;
		vec__output = thisPtr->GetHandle()->GetMaterials();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptRRefBase>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			ScriptRRefBase* script__output;
			script__output = ScriptResourceManager::Instance().GetScriptRRef(vec__output[i]);
			if(script__output != nullptr)
				array__output.Set(i, script__output->GetManagedInstance());
			else
				array__output.Set(i, nullptr);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptRenderable::InternalSetCullDistanceFactor(ScriptRenderable* thisPtr, float factor)
	{
		thisPtr->GetHandle()->SetCullDistanceFactor(factor);
	}

	float ScriptRenderable::InternalGetCullDistanceFactor(ScriptRenderable* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetCullDistanceFactor();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderable::InternalSetWriteVelocity(ScriptRenderable* thisPtr, bool enable)
	{
		thisPtr->GetHandle()->SetWriteVelocity(enable);
	}

	bool ScriptRenderable::InternalGetWriteVelocity(ScriptRenderable* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->GetWriteVelocity();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderable::InternalSetLayer(ScriptRenderable* thisPtr, uint64_t layer)
	{
		thisPtr->GetHandle()->SetLayer(layer);
	}

	uint64_t ScriptRenderable::InternalGetLayer(ScriptRenderable* thisPtr)
	{
		uint64_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderable::InternalGetBounds(ScriptRenderable* thisPtr, Bounds* __output)
	{
		Bounds tmp__output;
		tmp__output = thisPtr->GetHandle()->GetBounds();

		*__output = tmp__output;
	}
}
