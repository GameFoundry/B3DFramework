//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptAnimationClip.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Animation/B3DAnimationClip.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "B3DScriptAnimationCurves.generated.h"
#include "B3DScriptRootMotion.generated.h"
#include "B3DScriptAnimationEvent.generated.h"
#include "../../../Engine/Core/Animation/B3DAnimationClip.h"

namespace b3d
{
	ScriptAnimationClip::ScriptAnimationClip(const TResourceHandle<AnimationClip>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptAnimationClip::~ScriptAnimationClip()
	{
		UnregisterEvents();
	}

	void ScriptAnimationClip::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptAnimationClip::InternalGetRef);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCurves", (void*)&ScriptAnimationClip::InternalGetCurves);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCurves", (void*)&ScriptAnimationClip::InternalSetCurves);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEvents", (void*)&ScriptAnimationClip::InternalGetEvents);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEvents", (void*)&ScriptAnimationClip::InternalSetEvents);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRootMotion", (void*)&ScriptAnimationClip::InternalGetRootMotion);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_HasRootMotion", (void*)&ScriptAnimationClip::InternalHasRootMotion);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsAdditive", (void*)&ScriptAnimationClip::InternalIsAdditive);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLength", (void*)&ScriptAnimationClip::InternalGetLength);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSampleRate", (void*)&ScriptAnimationClip::InternalGetSampleRate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSampleRate", (void*)&ScriptAnimationClip::InternalSetSampleRate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptAnimationClip::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptAnimationClip::InternalCreate0);

	}

	MonoObject* ScriptAnimationClip::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[2] = { &dummy, &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool,bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptAnimationClip::InternalGetRef(ScriptAnimationClip* self)
	{
		return self->GetOrCreateResourceReference();
	}

	MonoObject* ScriptAnimationClip::InternalGetCurves(ScriptAnimationClip* self)
	{
		TShared<AnimationCurves> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AnimationClip*>(self->GetNativeObject())->GetCurves();

		MonoObject* __output;
		__output = ScriptAnimationCurves::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptAnimationClip::InternalSetCurves(ScriptAnimationClip* self, MonoObject* curves)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<AnimationCurves> tmpcurves;
		ScriptAnimationCurves* scriptObjectWrappercurves;
		scriptObjectWrappercurves = ScriptAnimationCurves::GetScriptObjectWrapper(curves);
		if(scriptObjectWrappercurves != nullptr)
			tmpcurves = std::static_pointer_cast<AnimationCurves>(scriptObjectWrappercurves->GetBaseNativeObjectAsShared());
		static_cast<AnimationClip*>(self->GetNativeObject())->SetCurves(*tmpcurves);
	}

	MonoArray* ScriptAnimationClip::InternalGetEvents(ScriptAnimationClip* self)
	{
		Vector<AnimationEvent> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<AnimationClip*>(self->GetNativeObject())->GetEvents();

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

	void ScriptAnimationClip::InternalSetEvents(ScriptAnimationClip* self, MonoArray* events)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<AnimationEvent> nativeArrayevents;
		if(events != nullptr)
		{
			ScriptArray scriptArrayevents(events);
			nativeArrayevents.resize(scriptArrayevents.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayevents.Size(); elementIndex++)
			{
				nativeArrayevents[elementIndex] = ScriptAnimationEvent::FromInterop(scriptArrayevents.Get<__AnimationEventInterop>(elementIndex));
			}
		}
		static_cast<AnimationClip*>(self->GetNativeObject())->SetEvents(nativeArrayevents);
	}

	MonoObject* ScriptAnimationClip::InternalGetRootMotion(ScriptAnimationClip* self)
	{
		TShared<RootMotion> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AnimationClip*>(self->GetNativeObject())->GetRootMotion();

		MonoObject* __output;
		__output = ScriptRootMotion::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	bool ScriptAnimationClip::InternalHasRootMotion(ScriptAnimationClip* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AnimationClip*>(self->GetNativeObject())->HasRootMotion();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptAnimationClip::InternalIsAdditive(ScriptAnimationClip* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AnimationClip*>(self->GetNativeObject())->IsAdditive();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptAnimationClip::InternalGetLength(ScriptAnimationClip* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AnimationClip*>(self->GetNativeObject())->GetLength();

		float __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptAnimationClip::InternalGetSampleRate(ScriptAnimationClip* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AnimationClip*>(self->GetNativeObject())->GetSampleRate();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimationClip::InternalSetSampleRate(ScriptAnimationClip* self, uint32_t sampleRate)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AnimationClip*>(self->GetNativeObject())->SetSampleRate(sampleRate);
	}

	void ScriptAnimationClip::InternalCreate(MonoObject* scriptObject, bool isAdditive)
	{
		TResourceHandle<AnimationClip> nativeObject = AnimationClip::Create(isAdditive);
		ScriptObjectWrapper::Create<ScriptAnimationClip>(nativeObject, scriptObject);
	}

	void ScriptAnimationClip::InternalCreate0(MonoObject* scriptObject, MonoObject* curves, bool isAdditive, uint32_t sampleRate, MonoObject* rootMotion)
	{
		TShared<AnimationCurves> tmpcurves;
		ScriptAnimationCurves* scriptObjectWrappercurves;
		scriptObjectWrappercurves = ScriptAnimationCurves::GetScriptObjectWrapper(curves);
		if(scriptObjectWrappercurves != nullptr)
			tmpcurves = std::static_pointer_cast<AnimationCurves>(scriptObjectWrappercurves->GetBaseNativeObjectAsShared());
		TShared<RootMotion> tmprootMotion;
		ScriptRootMotion* scriptObjectWrapperrootMotion;
		scriptObjectWrapperrootMotion = ScriptRootMotion::GetScriptObjectWrapper(rootMotion);
		if(scriptObjectWrapperrootMotion != nullptr)
			tmprootMotion = std::static_pointer_cast<RootMotion>(scriptObjectWrapperrootMotion->GetBaseNativeObjectAsShared());
		TResourceHandle<AnimationClip> nativeObject = AnimationClip::Create(tmpcurves, isAdditive, sampleRate, tmprootMotion);
		ScriptObjectWrapper::Create<ScriptAnimationClip>(nativeObject, scriptObject);
	}
}
