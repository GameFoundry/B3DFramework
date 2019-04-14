//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptMultiResource.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Importer/BsImporter.h"
#include "BsScriptSubResource.generated.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptMultiResource::ScriptMultiResource(MonoObject* managedInstance, const SPtr<MultiResource>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptMultiResource::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_MultiResource", (void*)&ScriptMultiResource::Internal_MultiResource);
		metaData.scriptClass->addInternalCall("Internal_MultiResource0", (void*)&ScriptMultiResource::Internal_MultiResource0);
		metaData.scriptClass->addInternalCall("Internal_getentries", (void*)&ScriptMultiResource::Internal_getentries);
		metaData.scriptClass->addInternalCall("Internal_setentries", (void*)&ScriptMultiResource::Internal_setentries);

	}

	MonoObject* ScriptMultiResource::create(const SPtr<MultiResource>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptMultiResource>()) ScriptMultiResource(managedInstance, value);
		return managedInstance;
	}
	void ScriptMultiResource::Internal_MultiResource(MonoObject* managedInstance)
	{
		SPtr<MultiResource> instance = bs_shared_ptr_new<MultiResource>();
		new (bs_alloc<ScriptMultiResource>())ScriptMultiResource(managedInstance, instance);
	}

	void ScriptMultiResource::Internal_MultiResource0(MonoObject* managedInstance, MonoArray* entries)
	{
		Vector<SubResource> vecentries;
		if(entries != nullptr)
		{
			ScriptArray arrayentries(entries);
			vecentries.resize(arrayentries.size());
			for(int i = 0; i < (int)arrayentries.size(); i++)
			{
				vecentries[i] = ScriptSubResource::fromInterop(arrayentries.get<__SubResourceInterop>(i));
			}
		}
		SPtr<MultiResource> instance = bs_shared_ptr_new<MultiResource>(vecentries);
		new (bs_alloc<ScriptMultiResource>())ScriptMultiResource(managedInstance, instance);
	}

	MonoArray* ScriptMultiResource::Internal_getentries(ScriptMultiResource* thisPtr)
	{
		Vector<SubResource> vec__output;
		vec__output = thisPtr->getInternal()->entries;

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptSubResource>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, ScriptSubResource::toInterop(vec__output[i]));
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptMultiResource::Internal_setentries(ScriptMultiResource* thisPtr, MonoArray* value)
	{
		Vector<SubResource> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.size(); i++)
			{
				vecvalue[i] = ScriptSubResource::fromInterop(arrayvalue.get<__SubResourceInterop>(i));
			}

		}
		thisPtr->getInternal()->entries = vecvalue;
	}
#endif
}
