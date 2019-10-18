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

	void ScriptCRenderable::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setMesh", (void*)&ScriptCRenderable::Internal_setMesh);
		metaData.scriptClass->addInternalCall("Internal_getMesh", (void*)&ScriptCRenderable::Internal_getMesh);
		metaData.scriptClass->addInternalCall("Internal_setMaterial", (void*)&ScriptCRenderable::Internal_setMaterial);
		metaData.scriptClass->addInternalCall("Internal_setMaterial0", (void*)&ScriptCRenderable::Internal_setMaterial0);
		metaData.scriptClass->addInternalCall("Internal_getMaterial", (void*)&ScriptCRenderable::Internal_getMaterial);
		metaData.scriptClass->addInternalCall("Internal_setMaterials", (void*)&ScriptCRenderable::Internal_setMaterials);
		metaData.scriptClass->addInternalCall("Internal_getMaterials", (void*)&ScriptCRenderable::Internal_getMaterials);
		metaData.scriptClass->addInternalCall("Internal_setCullDistanceFactor", (void*)&ScriptCRenderable::Internal_setCullDistanceFactor);
		metaData.scriptClass->addInternalCall("Internal_getCullDistanceFactor", (void*)&ScriptCRenderable::Internal_getCullDistanceFactor);
		metaData.scriptClass->addInternalCall("Internal_setWriteVelocity", (void*)&ScriptCRenderable::Internal_setWriteVelocity);
		metaData.scriptClass->addInternalCall("Internal_getWriteVelocity", (void*)&ScriptCRenderable::Internal_getWriteVelocity);
		metaData.scriptClass->addInternalCall("Internal_setLayer", (void*)&ScriptCRenderable::Internal_setLayer);
		metaData.scriptClass->addInternalCall("Internal_getLayer", (void*)&ScriptCRenderable::Internal_getLayer);
		metaData.scriptClass->addInternalCall("Internal_getBounds", (void*)&ScriptCRenderable::Internal_getBounds);

	}

	void ScriptCRenderable::Internal_setMesh(ScriptCRenderable* thisPtr, MonoObject* mesh)
	{
		ResourceHandle<Mesh> tmpmesh;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptRRefBase::toNative(mesh);
		if(scriptmesh != nullptr)
			tmpmesh = static_resource_cast<Mesh>(scriptmesh->getHandle());
		thisPtr->getHandle()->setMesh(tmpmesh);
	}

	MonoObject* ScriptCRenderable::Internal_getMesh(ScriptCRenderable* thisPtr)
	{
		ResourceHandle<Mesh> tmp__output;
		tmp__output = thisPtr->getHandle()->getMesh();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().getScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->getManagedInstance();
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
			tmpmaterial = static_resource_cast<Material>(scriptmaterial->getHandle());
		thisPtr->getHandle()->setMaterial(idx, tmpmaterial);
	}

	void ScriptCRenderable::Internal_setMaterial0(ScriptCRenderable* thisPtr, MonoObject* material)
	{
		ResourceHandle<Material> tmpmaterial;
		ScriptRRefBase* scriptmaterial;
		scriptmaterial = ScriptRRefBase::toNative(material);
		if(scriptmaterial != nullptr)
			tmpmaterial = static_resource_cast<Material>(scriptmaterial->getHandle());
		thisPtr->getHandle()->setMaterial(tmpmaterial);
	}

	MonoObject* ScriptCRenderable::Internal_getMaterial(ScriptCRenderable* thisPtr, uint32_t idx)
	{
		ResourceHandle<Material> tmp__output;
		tmp__output = thisPtr->getHandle()->getMaterial(idx);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().getScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->getManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptCRenderable::Internal_setMaterials(ScriptCRenderable* thisPtr, MonoArray* materials)
	{
		Vector<ResourceHandle<Material>> vecmaterials;
		if(materials != nullptr)
		{
			ScriptArray arraymaterials(materials);
			vecmaterials.resize(arraymaterials.size());
			for(int i = 0; i < (int)arraymaterials.size(); i++)
			{
				ScriptRRefBase* scriptmaterials;
				scriptmaterials = ScriptRRefBase::toNative(arraymaterials.get<MonoObject*>(i));
				if(scriptmaterials != nullptr)
				{
					ResourceHandle<Material> arrayElemPtrmaterials = static_resource_cast<Material>(scriptmaterials->getHandle());
					vecmaterials[i] = arrayElemPtrmaterials;
				}
			}
		}
		thisPtr->getHandle()->setMaterials(vecmaterials);
	}

	MonoArray* ScriptCRenderable::Internal_getMaterials(ScriptCRenderable* thisPtr)
	{
		Vector<ResourceHandle<Material>> vec__output;
		vec__output = thisPtr->getHandle()->getMaterials();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptRRefBase>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			ScriptRRefBase* script__output;
			script__output = ScriptResourceManager::instance().getScriptRRef(vec__output[i]);
			if(script__output != nullptr)
				array__output.set(i, script__output->getManagedInstance());
			else
				array__output.set(i, nullptr);
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptCRenderable::Internal_setCullDistanceFactor(ScriptCRenderable* thisPtr, float factor)
	{
		thisPtr->getHandle()->setCullDistanceFactor(factor);
	}

	float ScriptCRenderable::Internal_getCullDistanceFactor(ScriptCRenderable* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getCullDistanceFactor();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRenderable::Internal_setWriteVelocity(ScriptCRenderable* thisPtr, bool enable)
	{
		thisPtr->getHandle()->setWriteVelocity(enable);
	}

	bool ScriptCRenderable::Internal_getWriteVelocity(ScriptCRenderable* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->getWriteVelocity();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRenderable::Internal_setLayer(ScriptCRenderable* thisPtr, uint64_t layer)
	{
		thisPtr->getHandle()->setLayer(layer);
	}

	uint64_t ScriptCRenderable::Internal_getLayer(ScriptCRenderable* thisPtr)
	{
		uint64_t tmp__output;
		tmp__output = thisPtr->getHandle()->getLayer();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCRenderable::Internal_getBounds(ScriptCRenderable* thisPtr, Bounds* __output)
	{
		Bounds tmp__output;
		tmp__output = thisPtr->getHandle()->getBounds();

		*__output = tmp__output;
	}
}
