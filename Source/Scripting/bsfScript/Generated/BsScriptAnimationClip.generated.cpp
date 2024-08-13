//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAnimationClip.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationClip.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "BsScriptAnimationCurves.generated.h"
#include "BsScriptRootMotion.generated.h"
#include "BsScriptAnimationEvent.generated.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationClip.h"

namespace bs
{
	ScriptAnimationClip::ScriptAnimationClip(MonoObject* managedInstance, const TResourceHandle<AnimationClip>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptAnimationClip::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptAnimationClip::InternalGetRef);
		metaData.ScriptClass->AddInternalCall("Internal_GetCurves", (void*)&ScriptAnimationClip::InternalGetCurves);
		metaData.ScriptClass->AddInternalCall("Internal_SetCurves", (void*)&ScriptAnimationClip::InternalSetCurves);
		metaData.ScriptClass->AddInternalCall("Internal_GetEvents", (void*)&ScriptAnimationClip::InternalGetEvents);
		metaData.ScriptClass->AddInternalCall("Internal_SetEvents", (void*)&ScriptAnimationClip::InternalSetEvents);
		metaData.ScriptClass->AddInternalCall("Internal_GetRootMotion", (void*)&ScriptAnimationClip::InternalGetRootMotion);
		metaData.ScriptClass->AddInternalCall("Internal_HasRootMotion", (void*)&ScriptAnimationClip::InternalHasRootMotion);
		metaData.ScriptClass->AddInternalCall("Internal_IsAdditive", (void*)&ScriptAnimationClip::InternalIsAdditive);
		metaData.ScriptClass->AddInternalCall("Internal_GetLength", (void*)&ScriptAnimationClip::InternalGetLength);
		metaData.ScriptClass->AddInternalCall("Internal_GetSampleRate", (void*)&ScriptAnimationClip::InternalGetSampleRate);
		metaData.ScriptClass->AddInternalCall("Internal_SetSampleRate", (void*)&ScriptAnimationClip::InternalSetSampleRate);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptAnimationClip::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptAnimationClip::InternalCreate0);

	}

	 MonoObject*ScriptAnimationClip::CreateInstance()
	{
		bool dummy = false;
		void* ctorParams[2] = { &dummy, &dummy };

		return metaData.ScriptClass->CreateInstance("bool,bool", ctorParams);
	}
	MonoObject* ScriptAnimationClip::InternalGetRef(ScriptAnimationClip* self)
	{
		return self->GetRRef();
	}

	MonoObject* ScriptAnimationClip::InternalGetCurves(ScriptAnimationClip* self)
	{
		SPtr<AnimationCurves> tmp__output;
		tmp__output = self->GetHandle()->GetCurves();

		MonoObject* __output;
		__output = ScriptAnimationCurves::Create(tmp__output);

		return __output;
	}

	void ScriptAnimationClip::InternalSetCurves(ScriptAnimationClip* self, MonoObject* curves)
	{
		SPtr<AnimationCurves> tmpcurves;
		ScriptAnimationCurves* scriptObjectWrappercurves;
		scriptObjectWrappercurves = ScriptAnimationCurves::ToNative(curves);
		if(scriptObjectWrappercurves != nullptr)
			tmpcurves = scriptObjectWrappercurves->GetInternal();
		self->GetHandle()->SetCurves(*tmpcurves);
	}

	MonoArray* ScriptAnimationClip::InternalGetEvents(ScriptAnimationClip* self)
	{
		Vector<AnimationEvent> nativeArray__output;
		nativeArray__output = self->GetHandle()->GetEvents();

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
		self->GetHandle()->SetEvents(nativeArrayevents);
	}

	MonoObject* ScriptAnimationClip::InternalGetRootMotion(ScriptAnimationClip* self)
	{
		SPtr<RootMotion> tmp__output;
		tmp__output = self->GetHandle()->GetRootMotion();

		MonoObject* __output;
		__output = ScriptRootMotion::Create(tmp__output);

		return __output;
	}

	bool ScriptAnimationClip::InternalHasRootMotion(ScriptAnimationClip* self)
	{
		bool tmp__output;
		tmp__output = self->GetHandle()->HasRootMotion();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptAnimationClip::InternalIsAdditive(ScriptAnimationClip* self)
	{
		bool tmp__output;
		tmp__output = self->GetHandle()->IsAdditive();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptAnimationClip::InternalGetLength(ScriptAnimationClip* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetLength();

		float __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptAnimationClip::InternalGetSampleRate(ScriptAnimationClip* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetHandle()->GetSampleRate();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimationClip::InternalSetSampleRate(ScriptAnimationClip* self, uint32_t sampleRate)
	{
		self->GetHandle()->SetSampleRate(sampleRate);
	}

	void ScriptAnimationClip::InternalCreate(MonoObject* managedInstance, bool isAdditive)
	{
		TResourceHandle<AnimationClip> nativeObject = AnimationClip::Create(isAdditive);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(nativeObject, managedInstance);
	}

	void ScriptAnimationClip::InternalCreate0(MonoObject* managedInstance, MonoObject* curves, bool isAdditive, uint32_t sampleRate, MonoObject* rootMotion)
	{
		SPtr<AnimationCurves> tmpcurves;
		ScriptAnimationCurves* scriptObjectWrappercurves;
		scriptObjectWrappercurves = ScriptAnimationCurves::ToNative(curves);
		if(scriptObjectWrappercurves != nullptr)
			tmpcurves = scriptObjectWrappercurves->GetInternal();
		SPtr<RootMotion> tmprootMotion;
		ScriptRootMotion* scriptObjectWrapperrootMotion;
		scriptObjectWrapperrootMotion = ScriptRootMotion::ToNative(rootMotion);
		if(scriptObjectWrapperrootMotion != nullptr)
			tmprootMotion = scriptObjectWrapperrootMotion->GetInternal();
		TResourceHandle<AnimationClip> nativeObject = AnimationClip::Create(tmpcurves, isAdditive, sampleRate, tmprootMotion);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(nativeObject, managedInstance);
	}
}
