//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCAnimation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCAnimation.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationClip.h"
#include "BsScriptBlend1DInfo.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptBlend2DInfo.generated.h"
#include "BsScriptAnimationClipState.generated.h"

namespace bs
{
	ScriptAnimation::ScriptRebuildFloatPropertiesInternalThunkDef ScriptAnimation::ScriptRebuildFloatPropertiesInternalThunk; 
	ScriptAnimation::ScriptUpdateFloatPropertiesInternalThunkDef ScriptAnimation::ScriptUpdateFloatPropertiesInternalThunk; 
	ScriptAnimation::ScriptOnEventTriggeredInternalThunkDef ScriptAnimation::ScriptOnEventTriggeredInternalThunk; 

	ScriptAnimation::ScriptAnimation(MonoObject* managedInstance, const GameObjectHandle<CAnimation>& value)
		:TScriptComponent(managedInstance, value)
	{
		static_cast<GameObjectHandle<CAnimation>>(value)->ScriptRebuildFloatPropertiesInternal = std::bind(&ScriptAnimation::ScriptRebuildFloatPropertiesInternal, this, std::placeholders::_1);
		static_cast<GameObjectHandle<CAnimation>>(value)->ScriptUpdateFloatPropertiesInternal = std::bind(&ScriptAnimation::ScriptUpdateFloatPropertiesInternal, this);
		static_cast<GameObjectHandle<CAnimation>>(value)->ScriptOnEventTriggeredInternal = std::bind(&ScriptAnimation::ScriptOnEventTriggeredInternal, this, std::placeholders::_1, std::placeholders::_2);
	}

	void ScriptAnimation::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetDefaultClip", (void*)&ScriptAnimation::InternalSetDefaultClip);
		metaData.ScriptClass->AddInternalCall("Internal_GetDefaultClip", (void*)&ScriptAnimation::InternalGetDefaultClip);
		metaData.ScriptClass->AddInternalCall("Internal_SetWrapMode", (void*)&ScriptAnimation::InternalSetWrapMode);
		metaData.ScriptClass->AddInternalCall("Internal_GetWrapMode", (void*)&ScriptAnimation::InternalGetWrapMode);
		metaData.ScriptClass->AddInternalCall("Internal_SetSpeed", (void*)&ScriptAnimation::InternalSetSpeed);
		metaData.ScriptClass->AddInternalCall("Internal_GetSpeed", (void*)&ScriptAnimation::InternalGetSpeed);
		metaData.ScriptClass->AddInternalCall("Internal_Play", (void*)&ScriptAnimation::InternalPlay);
		metaData.ScriptClass->AddInternalCall("Internal_BlendAdditive", (void*)&ScriptAnimation::InternalBlendAdditive);
		metaData.ScriptClass->AddInternalCall("Internal_Blend1D", (void*)&ScriptAnimation::InternalBlend1D);
		metaData.ScriptClass->AddInternalCall("Internal_Blend2D", (void*)&ScriptAnimation::InternalBlend2D);
		metaData.ScriptClass->AddInternalCall("Internal_CrossFade", (void*)&ScriptAnimation::InternalCrossFade);
		metaData.ScriptClass->AddInternalCall("Internal_Sample", (void*)&ScriptAnimation::InternalSample);
		metaData.ScriptClass->AddInternalCall("Internal_Stop", (void*)&ScriptAnimation::InternalStop);
		metaData.ScriptClass->AddInternalCall("Internal_StopAll", (void*)&ScriptAnimation::InternalStopAll);
		metaData.ScriptClass->AddInternalCall("Internal_IsPlaying", (void*)&ScriptAnimation::InternalIsPlaying);
		metaData.ScriptClass->AddInternalCall("Internal_GetState", (void*)&ScriptAnimation::InternalGetState);
		metaData.ScriptClass->AddInternalCall("Internal_SetState", (void*)&ScriptAnimation::InternalSetState);
		metaData.ScriptClass->AddInternalCall("Internal_SetMorphChannelWeight", (void*)&ScriptAnimation::InternalSetMorphChannelWeight);
		metaData.ScriptClass->AddInternalCall("Internal_SetBounds", (void*)&ScriptAnimation::InternalSetBounds);
		metaData.ScriptClass->AddInternalCall("Internal_GetBounds", (void*)&ScriptAnimation::InternalGetBounds);
		metaData.ScriptClass->AddInternalCall("Internal_SetUseBounds", (void*)&ScriptAnimation::InternalSetUseBounds);
		metaData.ScriptClass->AddInternalCall("Internal_GetUseBounds", (void*)&ScriptAnimation::InternalGetUseBounds);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnableCull", (void*)&ScriptAnimation::InternalSetEnableCull);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnableCull", (void*)&ScriptAnimation::InternalGetEnableCull);
		metaData.ScriptClass->AddInternalCall("Internal_GetNumClips", (void*)&ScriptAnimation::InternalGetNumClips);
		metaData.ScriptClass->AddInternalCall("Internal_GetClip", (void*)&ScriptAnimation::InternalGetClip);
		metaData.ScriptClass->AddInternalCall("Internal_RefreshClipMappingsInternal", (void*)&ScriptAnimation::InternalRefreshClipMappingsInternal);
		metaData.ScriptClass->AddInternalCall("Internal_GetGenericCurveValueInternal", (void*)&ScriptAnimation::InternalGetGenericCurveValueInternal);
		metaData.ScriptClass->AddInternalCall("Internal_TogglePreviewModeInternal", (void*)&ScriptAnimation::InternalTogglePreviewModeInternal);

		ScriptRebuildFloatPropertiesInternalThunk = (ScriptRebuildFloatPropertiesInternalThunkDef)metaData.ScriptClass->GetMethodExact("Internal_ScriptRebuildFloatPropertiesInternal", "RRef`1<AnimationClip>")->GetThunk();
		ScriptUpdateFloatPropertiesInternalThunk = (ScriptUpdateFloatPropertiesInternalThunkDef)metaData.ScriptClass->GetMethodExact("Internal_ScriptUpdateFloatPropertiesInternal", "")->GetThunk();
		ScriptOnEventTriggeredInternalThunk = (ScriptOnEventTriggeredInternalThunkDef)metaData.ScriptClass->GetMethodExact("Internal_ScriptOnEventTriggeredInternal", "RRef`1<AnimationClip>,string")->GetThunk();
	}

	void ScriptAnimation::ScriptRebuildFloatPropertiesInternal(const TResourceHandle<AnimationClip>& p0)
	{
		MonoObject* tmpp0;
		ScriptRRefBase* scriptp0;
		scriptp0 = ScriptResourceManager::Instance().GetScriptRRef(p0);
		if(scriptp0 != nullptr)
			tmpp0 = scriptp0->GetManagedInstance();
		else
			tmpp0 = nullptr;
		MonoUtil::InvokeThunk(ScriptRebuildFloatPropertiesInternalThunk, GetManagedInstance(), tmpp0);
	}

	void ScriptAnimation::ScriptUpdateFloatPropertiesInternal()
	{
		MonoUtil::InvokeThunk(ScriptUpdateFloatPropertiesInternalThunk, GetManagedInstance());
	}

	void ScriptAnimation::ScriptOnEventTriggeredInternal(const TResourceHandle<AnimationClip>& p0, const String& p1)
	{
		MonoObject* tmpp0;
		ScriptRRefBase* scriptp0;
		scriptp0 = ScriptResourceManager::Instance().GetScriptRRef(p0);
		if(scriptp0 != nullptr)
			tmpp0 = scriptp0->GetManagedInstance();
		else
			tmpp0 = nullptr;
		MonoString* tmpp1;
		tmpp1 = MonoUtil::StringToMono(p1);
		MonoUtil::InvokeThunk(ScriptOnEventTriggeredInternalThunk, GetManagedInstance(), tmpp0, tmpp1);
	}

	void ScriptAnimation::InternalSetDefaultClip(ScriptAnimation* self, MonoObject* clip)
	{
		TResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::ToNative(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperclip->GetHandle());
		self->GetHandle()->SetDefaultClip(tmpclip);
	}

	MonoObject* ScriptAnimation::InternalGetDefaultClip(ScriptAnimation* self)
	{
		TResourceHandle<AnimationClip> tmp__output;
		tmp__output = self->GetHandle()->GetDefaultClip();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptAnimation::InternalSetWrapMode(ScriptAnimation* self, AnimWrapMode wrapMode)
	{
		self->GetHandle()->SetWrapMode(wrapMode);
	}

	AnimWrapMode ScriptAnimation::InternalGetWrapMode(ScriptAnimation* self)
	{
		AnimWrapMode tmp__output;
		tmp__output = self->GetHandle()->GetWrapMode();

		AnimWrapMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimation::InternalSetSpeed(ScriptAnimation* self, float speed)
	{
		self->GetHandle()->SetSpeed(speed);
	}

	float ScriptAnimation::InternalGetSpeed(ScriptAnimation* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetSpeed();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimation::InternalPlay(ScriptAnimation* self, MonoObject* clip)
	{
		TResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::ToNative(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperclip->GetHandle());
		self->GetHandle()->Play(tmpclip);
	}

	void ScriptAnimation::InternalBlendAdditive(ScriptAnimation* self, MonoObject* clip, float weight, float fadeLength, uint32_t layer)
	{
		TResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::ToNative(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperclip->GetHandle());
		self->GetHandle()->BlendAdditive(tmpclip, weight, fadeLength, layer);
	}

	void ScriptAnimation::InternalBlend1D(ScriptAnimation* self, __Blend1DInfoInterop* info, float t)
	{
		Blend1DInfo tmpinfo;
		tmpinfo = ScriptBlend1DInfo::FromInterop(*info);
		self->GetHandle()->Blend1D(tmpinfo, t);
	}

	void ScriptAnimation::InternalBlend2D(ScriptAnimation* self, __Blend2DInfoInterop* info, TVector2<float>* t)
	{
		Blend2DInfo tmpinfo;
		tmpinfo = ScriptBlend2DInfo::FromInterop(*info);
		self->GetHandle()->Blend2D(tmpinfo, *t);
	}

	void ScriptAnimation::InternalCrossFade(ScriptAnimation* self, MonoObject* clip, float fadeLength)
	{
		TResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::ToNative(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperclip->GetHandle());
		self->GetHandle()->CrossFade(tmpclip, fadeLength);
	}

	void ScriptAnimation::InternalSample(ScriptAnimation* self, MonoObject* clip, float time)
	{
		TResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::ToNative(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperclip->GetHandle());
		self->GetHandle()->Sample(tmpclip, time);
	}

	void ScriptAnimation::InternalStop(ScriptAnimation* self, uint32_t layer)
	{
		self->GetHandle()->Stop(layer);
	}

	void ScriptAnimation::InternalStopAll(ScriptAnimation* self)
	{
		self->GetHandle()->StopAll();
	}

	bool ScriptAnimation::InternalIsPlaying(ScriptAnimation* self)
	{
		bool tmp__output;
		tmp__output = self->GetHandle()->IsPlaying();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptAnimation::InternalGetState(ScriptAnimation* self, MonoObject* clip, AnimationClipState* state)
	{
		bool tmp__output;
		TResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::ToNative(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperclip->GetHandle());
		tmp__output = self->GetHandle()->GetState(tmpclip, *state);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimation::InternalSetState(ScriptAnimation* self, MonoObject* clip, AnimationClipState* state)
	{
		TResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::ToNative(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperclip->GetHandle());
		self->GetHandle()->SetState(tmpclip, *state);
	}

	void ScriptAnimation::InternalSetMorphChannelWeight(ScriptAnimation* self, MonoString* name, float weight)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		self->GetHandle()->SetMorphChannelWeight(tmpname, weight);
	}

	void ScriptAnimation::InternalSetBounds(ScriptAnimation* self, AABox* bounds)
	{
		self->GetHandle()->SetBounds(*bounds);
	}

	void ScriptAnimation::InternalGetBounds(ScriptAnimation* self, AABox* __output)
	{
		AABox tmp__output;
		tmp__output = self->GetHandle()->GetBounds();

		*__output = tmp__output;
	}

	void ScriptAnimation::InternalSetUseBounds(ScriptAnimation* self, bool enable)
	{
		self->GetHandle()->SetUseBounds(enable);
	}

	bool ScriptAnimation::InternalGetUseBounds(ScriptAnimation* self)
	{
		bool tmp__output;
		tmp__output = self->GetHandle()->GetUseBounds();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimation::InternalSetEnableCull(ScriptAnimation* self, bool enable)
	{
		self->GetHandle()->SetEnableCull(enable);
	}

	bool ScriptAnimation::InternalGetEnableCull(ScriptAnimation* self)
	{
		bool tmp__output;
		tmp__output = self->GetHandle()->GetEnableCull();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptAnimation::InternalGetNumClips(ScriptAnimation* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetHandle()->GetNumClips();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptAnimation::InternalGetClip(ScriptAnimation* self, uint32_t idx)
	{
		TResourceHandle<AnimationClip> tmp__output;
		tmp__output = self->GetHandle()->GetClip(idx);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptAnimation::InternalRefreshClipMappingsInternal(ScriptAnimation* self)
	{
		self->GetHandle()->RefreshClipMappingsInternal();
	}

	bool ScriptAnimation::InternalGetGenericCurveValueInternal(ScriptAnimation* self, uint32_t curveIdx, float* value)
	{
		bool tmp__output;
		tmp__output = self->GetHandle()->GetGenericCurveValueInternal(curveIdx, *value);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptAnimation::InternalTogglePreviewModeInternal(ScriptAnimation* self, bool enabled)
	{
		bool tmp__output;
		tmp__output = self->GetHandle()->TogglePreviewModeInternal(enabled);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
