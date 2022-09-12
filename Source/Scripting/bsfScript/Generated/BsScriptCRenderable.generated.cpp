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
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCRenderable::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_setMesh", (void*)&ScriptCRenderable::Internal_setMesh);
		metaData.scriptClass->AddInternalCall("Internal_getMesh", (void*)&ScriptCRenderable::Internal_getMesh);
		metaData.scriptClass->AddInternalCall("Internal_setMaterial", (void*)&ScriptCRenderable::Internal_setMaterial);
		metaData.scriptClass->AddInternalCall("Internal_setMaterial0", (void*)&ScriptCRenderable::Internal_setMaterial0);
		metaData.scriptClass->AddInternalCall("Internal_getMaterial", (void*)&ScriptCRenderable::Internal_getMaterial);
		metaData.scriptClass->AddInternalCall("Internal_setMaterials", (void*)&ScriptCRenderable::Internal_setMaterials);
		metaData.scriptClass->AddInternalCall("Internal_getMaterials", (void*)&ScriptCRenderable::Internal_getMaterials);
		metaData.scriptClass->AddInternalCall("Internal_setCullDistanceFactor", (void*)&ScriptCRenderable::Internal_setCullDistanceFactor);
		metaData.scriptClass->AddInternalCall("Internal_getCullDistanceFactor", (void*)&ScriptCRenderable::Internal_getCullDistanceFactor);
		metaData.scriptClass->AddInternalCall("Internal_setWriteVelocity", (void*)&ScriptCRenderable::Internal_setWriteVelocity);
		metaData.scriptClass->AddInternalCall("Internal_getWriteVelocity", (void*)&ScriptCRenderable::Internal_getWriteVelocity);
		metaData.scriptClass->AddInternalCall("Internal_setLayer", (void*)&ScriptCRenderable::Internal_setLayer);
		metaData.scriptClass->AddInternalCall("Internal_getLayer", (void*)&ScriptCRenderable::Internal_getLayer);
		metaData.scriptClass->AddInternalCall("Internal_getBounds", (void*)&ScriptCRenderable::Internal_getBounds);

	}

	void ScriptCRenderable::Internal_setMesh(ScriptCRenderable* thisPtr, MonoObject* mesh)
	{
		ResourceHandle<Mesh> tmpmesh;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptRRefBase::toNative(mesh);
		if(scriptmesh != nullptr)
			tmpmesh = static_resource_cast<Mesh>(scriptmesh->GetHandle());
		thisPtr->GetHandle()->setMesh(tmpmesh);
	}

	MonoObject* ScriptCRenderable::Internal_getMesh(ScriptCRenderable* thisPtr)
	{
		ResourceHandle<Mesh> tmp__output;
		tmp__output = thisPtr->GetHandle()->getMesh();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptCRenderable::Internal_setMaterial(ScriptCRenderable* thisPtr, uint32_t idx, MonoObject* material)
	{
		ResourceHandle<Material> tmpmaterial;
		ScriptRRefBase* scriptmaterial;
		scriptmaterial = ScriptRRefBase::toNative(material);
		if(scriptmaterial != nullptr)
			tmpmaterial = static_resource_cast<Material>(scriptmaterial->GetHandle());
		thisPtr->GetHandle()->setMaterial(idx, tmpmaterial);
	}

	void ScriptCRenderable::Internal_setMaterial0(ScriptCRenderable* thisPtr, MonoObject* material)
	{
		ResourceHandle<Material> tmpmaterial;
		ScriptRRefBase* scriptmaterial;
		scriptmaterial = ScriptRRefBase::toNative(material);
		if(scriptmaterial != nullptr)
			tmpmaterial = static_resource_cast<Material>(scriptmaterial->GetHandle());
		thisPtr->GetHandle()->setMaterial(tmpmaterial);
	}

	MonoObject* ScriptCRenderable::Internal_getMaterial(ScriptCRenderable* thisPtr, uint32_t idx)
	{
		ResourceHandle<Material> tmp__output;
		tmp__output = thisPtr->GetHandle()->getMaterial(idx);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptCRenderable::Internal_setMaterials(ScriptCRenderable* thisPtr, MonoArray* materials)
	{
		Vector<ResourceHandle<Material>> vecmaterials;
		if(materials != nullptr)
		{
			ScriptArray Arraymaterials(materials);
			vecmaterials.Resize(arraymaterials.size());
			for(int i = 0; i < (int)arraymaterials.Size(); i++)
			{
				ScriptRRefBase* scriptmaterials;
				scriptmaterials = ScriptRRefBase::toNative(arraymaterials.get<MonoObject*>(i));
				if(scriptmaterials != nullptr)
				{
					ResourceHandle<Material> arrayElemPtrmaterials = static_resource_cast<Material>(scriptmaterials->GetHandle());
					vecmaterials[i] = arrayElemPtrmaterials;
				}
			}
		}
		thisPtr->GetHandle()->setMaterials(vecmaterials);
	}

	MonoArray* ScriptCRenderable::Internal_getMaterials(ScriptCRenderable* thisPtr)
	{
		Vector<ResourceHandle<Material>> vec__output;
		vec__output = thisPtr->GetHandle()->getMaterials();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptRRefBase>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			ScriptRRefBase* script__output;
			script__output = ScriptResourceManager::instance().GetScriptRRef(vec__output[i]);
			if(script__output != nullptr)
				array__output.Set(i, script__output->GetManagedInstance());
			else
				array__output.Set(i, nullptr);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptCRenderable::Internal_setCullDistanceFactor(ScriptCRenderable* thisPtr, float factor)
	{
		thisPtr->GetHandle()->setCullDistanceFactor(factor);
	}

	float ScriptCRenderable::Internal_getCullDistanceFactor(ScriptCRenderable* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getCullDistanceFactor();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRenderable::Internal_setWriteVelocity(ScriptCRenderable* thisPtr, bool enable)
	{
		thisPtr->GetHandle()->setWriteVelocity(enable);
	}

	bool ScriptCRenderable::Internal_getWriteVelocity(ScriptCRenderable* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->getWriteVelocity();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRenderable::Internal_setLayer(ScriptCRenderable* thisPtr, uint64_t layer)
	{
		thisPtr->GetHandle()->setLayer(layer);
	}

	uint64_t ScriptCRenderable::Internal_getLayer(ScriptCRenderable* thisPtr)
	{
		uint64_t tmp__output;
		tmp__output = thisPtr->GetHandle()->getLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRenderable::Internal_getBounds(ScriptCRenderable* thisPtr, Bounds* __output)
	{
		Bounds tmp__output;
		tmp__output = thisPtr->GetHandle()->getBounds();

		*__output = tmp__output;
	}
}
