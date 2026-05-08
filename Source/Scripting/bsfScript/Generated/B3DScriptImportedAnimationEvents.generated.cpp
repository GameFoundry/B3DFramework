//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptImportedAnimationEvents.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptAnimationEvent.generated.h"

namespace b3d
{
#if !B3D_IS_ENGINE
	ScriptImportedAnimationEvents::ScriptImportedAnimationEvents(const TShared<ImportedAnimationEvents>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptImportedAnimationEvents::~ScriptImportedAnimationEvents()
	{
		UnregisterEvents();
	}

	void ScriptImportedAnimationEvents::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ImportedAnimationEvents", (void*)&ScriptImportedAnimationEvents::InternalImportedAnimationEvents);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetName", (void*)&ScriptImportedAnimationEvents::InternalGetName);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetName", (void*)&ScriptImportedAnimationEvents::InternalSetName);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEvents", (void*)&ScriptImportedAnimationEvents::InternalGetEvents);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEvents", (void*)&ScriptImportedAnimationEvents::InternalSetEvents);

	}

	MonoObject* ScriptImportedAnimationEvents::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptImportedAnimationEvents::InternalImportedAnimationEvents(MonoObject* scriptObject)
	{
		TShared<ImportedAnimationEvents> nativeObject = B3DMakeShared<ImportedAnimationEvents>();
		ScriptObjectWrapper::Create<ScriptImportedAnimationEvents>(nativeObject, scriptObject);
	}

	MonoString* ScriptImportedAnimationEvents::InternalGetName(ScriptImportedAnimationEvents* self)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ImportedAnimationEvents*>(self->GetNativeObject())->Name;

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptImportedAnimationEvents::InternalSetName(ScriptImportedAnimationEvents* self, MonoString* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		static_cast<ImportedAnimationEvents*>(self->GetNativeObject())->Name = tmpvalue;
	}

	MonoArray* ScriptImportedAnimationEvents::InternalGetEvents(ScriptImportedAnimationEvents* self)
	{
		Vector<AnimationEvent> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<ImportedAnimationEvents*>(self->GetNativeObject())->Events;

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
		if(!self->IsNativeObjectValid())
			return;

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
		static_cast<ImportedAnimationEvents*>(self->GetNativeObject())->Events = nativeArrayvalue;
	}
#endif
}
