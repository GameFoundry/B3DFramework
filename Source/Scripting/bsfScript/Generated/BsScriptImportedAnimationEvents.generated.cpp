//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptImportedAnimationEvents.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptAnimationEvent.generated.h"

using namespace bs;
#if !BS_IS_BANSHEE3D
ScriptImportedAnimationEvents::ScriptImportedAnimationEvents(MonoObject* managedInstance, const SPtr<ImportedAnimationEvents>& value)
	: TScriptReflectable(managedInstance, value)
{
}

void ScriptImportedAnimationEvents::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_ImportedAnimationEvents", (void*)&ScriptImportedAnimationEvents::InternalImportedAnimationEvents);
	metaData.ScriptClass->AddInternalCall("Internal_GetName", (void*)&ScriptImportedAnimationEvents::InternalGetName);
	metaData.ScriptClass->AddInternalCall("Internal_SetName", (void*)&ScriptImportedAnimationEvents::InternalSetName);
	metaData.ScriptClass->AddInternalCall("Internal_GetEvents", (void*)&ScriptImportedAnimationEvents::InternalGetEvents);
	metaData.ScriptClass->AddInternalCall("Internal_SetEvents", (void*)&ScriptImportedAnimationEvents::InternalSetEvents);
}

MonoObject* ScriptImportedAnimationEvents::Create(const SPtr<ImportedAnimationEvents>& value)
{
	if(value == nullptr) return nullptr;

	bool dummy = false;
	void* ctorParams[1] = { &dummy };

	MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
	new(B3DAllocate<ScriptImportedAnimationEvents>()) ScriptImportedAnimationEvents(managedInstance, value);
	return managedInstance;
}

void ScriptImportedAnimationEvents::InternalImportedAnimationEvents(MonoObject* managedInstance)
{
	SPtr<ImportedAnimationEvents> instance = B3DMakeShared<ImportedAnimationEvents>();
	new(B3DAllocate<ScriptImportedAnimationEvents>()) ScriptImportedAnimationEvents(managedInstance, instance);
}

MonoString* ScriptImportedAnimationEvents::InternalGetName(ScriptImportedAnimationEvents* thisPtr)
{
	String tmp__output;
	tmp__output = thisPtr->GetInternal()->Name;

	MonoString* __output;
	__output = MonoUtil::StringToMono(tmp__output);

	return __output;
}

void ScriptImportedAnimationEvents::InternalSetName(ScriptImportedAnimationEvents* thisPtr, MonoString* value)
{
	String tmpvalue;
	tmpvalue = MonoUtil::MonoToString(value);
	thisPtr->GetInternal()->Name = tmpvalue;
}

MonoArray* ScriptImportedAnimationEvents::InternalGetEvents(ScriptImportedAnimationEvents* thisPtr)
{
	Vector<AnimationEvent> vec__output;
	vec__output = thisPtr->GetInternal()->Events;

	MonoArray* __output;
	int arraySize__output = (int)vec__output.size();
	ScriptArray array__output = ScriptArray::Create<ScriptAnimationEvent>(arraySize__output);
	for(int i = 0; i < arraySize__output; i++)
	{
		array__output.Set(i, ScriptAnimationEvent::ToInterop(vec__output[i]));
	}
	__output = array__output.GetInternal();

	return __output;
}

void ScriptImportedAnimationEvents::InternalSetEvents(ScriptImportedAnimationEvents* thisPtr, MonoArray* value)
{
	Vector<AnimationEvent> vecvalue;
	if(value != nullptr)
	{
		ScriptArray arrayvalue(value);
		vecvalue.resize(arrayvalue.Size());
		for(int i = 0; i < (int)arrayvalue.Size(); i++)
		{
			vecvalue[i] = ScriptAnimationEvent::FromInterop(arrayvalue.Get<__AnimationEventInterop>(i));
		}
	}
	thisPtr->GetInternal()->Events = vecvalue;
}
#endif
