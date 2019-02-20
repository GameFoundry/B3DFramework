//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptImportedAnimationEvents.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptAnimationEvent.generated.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptImportedAnimationEvents::ScriptImportedAnimationEvents(MonoObject* managedInstance, const SPtr<ImportedAnimationEvents>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptImportedAnimationEvents::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_ImportedAnimationEvents", (void*)&ScriptImportedAnimationEvents::Internal_ImportedAnimationEvents);
		metaData.scriptClass->addInternalCall("Internal_getname", (void*)&ScriptImportedAnimationEvents::Internal_getname);
		metaData.scriptClass->addInternalCall("Internal_setname", (void*)&ScriptImportedAnimationEvents::Internal_setname);
		metaData.scriptClass->addInternalCall("Internal_getevents", (void*)&ScriptImportedAnimationEvents::Internal_getevents);
		metaData.scriptClass->addInternalCall("Internal_setevents", (void*)&ScriptImportedAnimationEvents::Internal_setevents);

	}

	MonoObject* ScriptImportedAnimationEvents::create(const SPtr<ImportedAnimationEvents>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptImportedAnimationEvents>()) ScriptImportedAnimationEvents(managedInstance, value);
		return managedInstance;
	}
	void ScriptImportedAnimationEvents::Internal_ImportedAnimationEvents(MonoObject* managedInstance)
	{
		SPtr<ImportedAnimationEvents> instance = bs_shared_ptr_new<ImportedAnimationEvents>();
		new (bs_alloc<ScriptImportedAnimationEvents>())ScriptImportedAnimationEvents(managedInstance, instance);
	}

	MonoString* ScriptImportedAnimationEvents::Internal_getname(ScriptImportedAnimationEvents* thisPtr)
	{
		String tmp__output;
		tmp__output = thisPtr->getInternal()->name;

		MonoString* __output;
		__output = MonoUtil::stringToMono(tmp__output);

		return __output;
	}

	void ScriptImportedAnimationEvents::Internal_setname(ScriptImportedAnimationEvents* thisPtr, MonoString* value)
	{
		String tmpvalue;
		tmpvalue = MonoUtil::monoToString(value);
		thisPtr->getInternal()->name = tmpvalue;
	}

	MonoArray* ScriptImportedAnimationEvents::Internal_getevents(ScriptImportedAnimationEvents* thisPtr)
	{
		Vector<AnimationEvent> vec__output;
		vec__output = thisPtr->getInternal()->events;

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptAnimationEvent>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, ScriptAnimationEvent::toInterop(vec__output[i]));
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptImportedAnimationEvents::Internal_setevents(ScriptImportedAnimationEvents* thisPtr, MonoArray* value)
	{
		Vector<AnimationEvent> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.size(); i++)
			{
				vecvalue[i] = ScriptAnimationEvent::fromInterop(arrayvalue.get<__AnimationEventInterop>(i));
			}

		}
		thisPtr->getInternal()->events = vecvalue;
	}
#endif
}
