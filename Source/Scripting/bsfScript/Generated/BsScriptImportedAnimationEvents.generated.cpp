//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptImportedAnimationEvents.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptAnimationEvent.generated.h"

namespace bs
{
#if !B3D_IS_ENGINE
	ScriptImportedAnimationEvents::ScriptImportedAnimationEvents(MonoObject* managedInstance, const SPtr<ImportedAnimationEvents>& value)
		:TScriptReflectable(managedInstance, value)
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
		new (B3DAllocate<ScriptImportedAnimationEvents>()) ScriptImportedAnimationEvents(managedInstance, value);
		return managedInstance;
	}
	void ScriptImportedAnimationEvents::InternalImportedAnimationEvents(MonoObject* managedInstance)
	{
		SPtr<ImportedAnimationEvents> nativeObject = B3DMakeShared<ImportedAnimationEvents>();
		new (B3DAllocate<ScriptImportedAnimationEvents>())ScriptImportedAnimationEvents(managedInstance, nativeObject);
	}

	MonoString* ScriptImportedAnimationEvents::InternalGetName(ScriptImportedAnimationEvents* self)
	{
		String tmp__output;
		tmp__output = self->GetInternal()->Name;

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptImportedAnimationEvents::InternalSetName(ScriptImportedAnimationEvents* self, MonoString* value)
	{
		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		self->GetInternal()->Name = tmpvalue;
	}

	MonoArray* ScriptImportedAnimationEvents::InternalGetEvents(ScriptImportedAnimationEvents* self)
	{
		Vector<AnimationEvent> nativeArray__output;
		nativeArray__output = self->GetInternal()->Events;

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptAnimationEvent>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptAnimationEvent::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptImportedAnimationEvents::InternalSetEvents(ScriptImportedAnimationEvents* self, MonoArray* value)
	{
		Vector<AnimationEvent> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = ScriptAnimationEvent::FromInterop(scriptArrayvalue.Get<__AnimationEventInterop>(elementIndex));
			}

		}
		self->GetInternal()->Events = nativeArrayvalue;
	}
#endif
}
