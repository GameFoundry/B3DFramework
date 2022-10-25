//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
	ScriptCRenderable::ScriptCRenderable(MonoObject* managedInstance, const GameObjectHandle<CRenderable>& value)
		: TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCRenderable::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetMesh", (void*)&ScriptCRenderable::InternalSetMesh);
		metaData.ScriptClass->AddInternalCall("Internal_GetMesh", (void*)&ScriptCRenderable::InternalGetMesh);
		metaData.ScriptClass->AddInternalCall("Internal_SetMaterial", (void*)&ScriptCRenderable::InternalSetMaterial);
		metaData.ScriptClass->AddInternalCall("Internal_SetMaterial0", (void*)&ScriptCRenderable::InternalSetMaterial0);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaterial", (void*)&ScriptCRenderable::InternalGetMaterial);
		metaData.ScriptClass->AddInternalCall("Internal_SetMaterials", (void*)&ScriptCRenderable::InternalSetMaterials);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaterials", (void*)&ScriptCRenderable::InternalGetMaterials);
		metaData.ScriptClass->AddInternalCall("Internal_SetCullDistanceFactor", (void*)&ScriptCRenderable::InternalSetCullDistanceFactor);
		metaData.ScriptClass->AddInternalCall("Internal_GetCullDistanceFactor", (void*)&ScriptCRenderable::InternalGetCullDistanceFactor);
		metaData.ScriptClass->AddInternalCall("Internal_SetWriteVelocity", (void*)&ScriptCRenderable::InternalSetWriteVelocity);
		metaData.ScriptClass->AddInternalCall("Internal_GetWriteVelocity", (void*)&ScriptCRenderable::InternalGetWriteVelocity);
		metaData.ScriptClass->AddInternalCall("Internal_SetLayer", (void*)&ScriptCRenderable::InternalSetLayer);
		metaData.ScriptClass->AddInternalCall("Internal_GetLayer", (void*)&ScriptCRenderable::InternalGetLayer);
		metaData.ScriptClass->AddInternalCall("Internal_GetBounds", (void*)&ScriptCRenderable::InternalGetBounds);
	}

	void ScriptCRenderable::InternalSetMesh(ScriptCRenderable* thisPtr, MonoObject* mesh)
	{
		ResourceHandle<Mesh> tmpmesh;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptRRefBase::ToNative(mesh);
		if(scriptmesh != nullptr)
			tmpmesh = static_resource_cast<Mesh>(scriptmesh->GetHandle());
		thisPtr->GetHandle()->SetMesh(tmpmesh);
	}

	MonoObject* ScriptCRenderable::InternalGetMesh(ScriptCRenderable* thisPtr)
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

	void ScriptCRenderable::InternalSetMaterial(ScriptCRenderable* thisPtr, uint32_t idx, MonoObject* material)
	{
		ResourceHandle<Material> tmpmaterial;
		ScriptRRefBase* scriptmaterial;
		scriptmaterial = ScriptRRefBase::ToNative(material);
		if(scriptmaterial != nullptr)
			tmpmaterial = static_resource_cast<Material>(scriptmaterial->GetHandle());
		thisPtr->GetHandle()->SetMaterial(idx, tmpmaterial);
	}

	void ScriptCRenderable::InternalSetMaterial0(ScriptCRenderable* thisPtr, MonoObject* material)
	{
		ResourceHandle<Material> tmpmaterial;
		ScriptRRefBase* scriptmaterial;
		scriptmaterial = ScriptRRefBase::ToNative(material);
		if(scriptmaterial != nullptr)
			tmpmaterial = static_resource_cast<Material>(scriptmaterial->GetHandle());
		thisPtr->GetHandle()->SetMaterial(tmpmaterial);
	}

	MonoObject* ScriptCRenderable::InternalGetMaterial(ScriptCRenderable* thisPtr, uint32_t idx)
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

	void ScriptCRenderable::InternalSetMaterials(ScriptCRenderable* thisPtr, MonoArray* materials)
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
					ResourceHandle<Material> arrayElemPtrmaterials = static_resource_cast<Material>(scriptmaterials->GetHandle());
					vecmaterials[i] = arrayElemPtrmaterials;
				}
			}
		}
		thisPtr->GetHandle()->SetMaterials(vecmaterials);
	}

	MonoArray* ScriptCRenderable::InternalGetMaterials(ScriptCRenderable* thisPtr)
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

	void ScriptCRenderable::InternalSetCullDistanceFactor(ScriptCRenderable* thisPtr, float factor)
	{
		thisPtr->GetHandle()->SetCullDistanceFactor(factor);
	}

	float ScriptCRenderable::InternalGetCullDistanceFactor(ScriptCRenderable* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetCullDistanceFactor();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRenderable::InternalSetWriteVelocity(ScriptCRenderable* thisPtr, bool enable)
	{
		thisPtr->GetHandle()->SetWriteVelocity(enable);
	}

	bool ScriptCRenderable::InternalGetWriteVelocity(ScriptCRenderable* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->GetWriteVelocity();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRenderable::InternalSetLayer(ScriptCRenderable* thisPtr, uint64_t layer)
	{
		thisPtr->GetHandle()->SetLayer(layer);
	}

	uint64_t ScriptCRenderable::InternalGetLayer(ScriptCRenderable* thisPtr)
	{
		uint64_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRenderable::InternalGetBounds(ScriptCRenderable* thisPtr, Bounds* __output)
	{
		Bounds tmp__output;
		tmp__output = thisPtr->GetHandle()->GetBounds();

		*__output = tmp__output;
	}
} // namespace bs
