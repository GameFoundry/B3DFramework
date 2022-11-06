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
#include "BsScriptBlend2DInfo.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptAnimationClipState.generated.h"

namespace bs
{
	ScriptCAnimation::ScriptRebuildFloatPropertiesInternalThunkDef ScriptCAnimation::ScriptRebuildFloatPropertiesInternalThunk; 
	ScriptCAnimation::ScriptUpdateFloatPropertiesInternalThunkDef ScriptCAnimation::ScriptUpdateFloatPropertiesInternalThunk; 
	ScriptCAnimation::ScriptOnEventTriggeredInternalThunkDef ScriptCAnimation::ScriptOnEventTriggeredInternalThunk; 

	ScriptCAnimation::ScriptCAnimation(MonoObject* managedInstance, const GameObjectHandle<CAnimation>& value)
		:TScriptComponent(managedInstance, value)
	{
		value->ScriptRebuildFloatPropertiesInternal = std::bind(&ScriptCAnimation::ScriptRebuildFloatPropertiesInternal, this, std::placeholders::_1);
		value->ScriptUpdateFloatPropertiesInternal = std::bind(&ScriptCAnimation::ScriptUpdateFloatPropertiesInternal, this);
		value->ScriptOnEventTriggeredInternal = std::bind(&ScriptCAnimation::ScriptOnEventTriggeredInternal, this, std::placeholders::_1, std::placeholders::_2);
	}

	void ScriptCAnimation::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetDefaultClip", (void*)&ScriptCAnimation::InternalSetDefaultClip);
		metaData.ScriptClass->AddInternalCall("Internal_GetDefaultClip", (void*)&ScriptCAnimation::InternalGetDefaultClip);
		metaData.ScriptClass->AddInternalCall("Internal_SetWrapMode", (void*)&ScriptCAnimation::InternalSetWrapMode);
		metaData.ScriptClass->AddInternalCall("Internal_GetWrapMode", (void*)&ScriptCAnimation::InternalGetWrapMode);
		metaData.ScriptClass->AddInternalCall("Internal_SetSpeed", (void*)&ScriptCAnimation::InternalSetSpeed);
		metaData.ScriptClass->AddInternalCall("Internal_GetSpeed", (void*)&ScriptCAnimation::InternalGetSpeed);
		metaData.ScriptClass->AddInternalCall("Internal_Play", (void*)&ScriptCAnimation::InternalPlay);
		metaData.ScriptClass->AddInternalCall("Internal_BlendAdditive", (void*)&ScriptCAnimation::InternalBlendAdditive);
		metaData.ScriptClass->AddInternalCall("Internal_Blend1D", (void*)&ScriptCAnimation::InternalBlend1D);
		metaData.ScriptClass->AddInternalCall("Internal_Blend2D", (void*)&ScriptCAnimation::InternalBlend2D);
		metaData.ScriptClass->AddInternalCall("Internal_CrossFade", (void*)&ScriptCAnimation::InternalCrossFade);
		metaData.ScriptClass->AddInternalCall("Internal_Sample", (void*)&ScriptCAnimation::InternalSample);
		metaData.ScriptClass->AddInternalCall("Internal_Stop", (void*)&ScriptCAnimation::InternalStop);
		metaData.ScriptClass->AddInternalCall("Internal_StopAll", (void*)&ScriptCAnimation::InternalStopAll);
		metaData.ScriptClass->AddInternalCall("Internal_IsPlaying", (void*)&ScriptCAnimation::InternalIsPlaying);
		metaData.ScriptClass->AddInternalCall("Internal_GetState", (void*)&ScriptCAnimation::InternalGetState);
		metaData.ScriptClass->AddInternalCall("Internal_SetState", (void*)&ScriptCAnimation::InternalSetState);
		metaData.ScriptClass->AddInternalCall("Internal_SetMorphChannelWeight", (void*)&ScriptCAnimation::InternalSetMorphChannelWeight);
		metaData.ScriptClass->AddInternalCall("Internal_SetBounds", (void*)&ScriptCAnimation::InternalSetBounds);
		metaData.ScriptClass->AddInternalCall("Internal_GetBounds", (void*)&ScriptCAnimation::InternalGetBounds);
		metaData.ScriptClass->AddInternalCall("Internal_SetUseBounds", (void*)&ScriptCAnimation::InternalSetUseBounds);
		metaData.ScriptClass->AddInternalCall("Internal_GetUseBounds", (void*)&ScriptCAnimation::InternalGetUseBounds);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnableCull", (void*)&ScriptCAnimation::InternalSetEnableCull);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnableCull", (void*)&ScriptCAnimation::InternalGetEnableCull);
		metaData.ScriptClass->AddInternalCall("Internal_GetNumClips", (void*)&ScriptCAnimation::InternalGetNumClips);
		metaData.ScriptClass->AddInternalCall("Internal_GetClip", (void*)&ScriptCAnimation::InternalGetClip);
		metaData.ScriptClass->AddInternalCall("Internal_RefreshClipMappingsInternal", (void*)&ScriptCAnimation::InternalRefreshClipMappingsInternal);
		metaData.ScriptClass->AddInternalCall("Internal_GetGenericCurveValueInternal", (void*)&ScriptCAnimation::InternalGetGenericCurveValueInternal);
		metaData.ScriptClass->AddInternalCall("Internal_TogglePreviewModeInternal", (void*)&ScriptCAnimation::InternalTogglePreviewModeInternal);

		ScriptRebuildFloatPropertiesInternalThunk = (ScriptRebuildFloatPropertiesInternalThunkDef)metaData.ScriptClass->GetMethodExact("Internal_ScriptRebuildFloatPropertiesInternal", "RRef`1<AnimationClip>")->GetThunk();
		ScriptUpdateFloatPropertiesInternalThunk = (ScriptUpdateFloatPropertiesInternalThunkDef)metaData.ScriptClass->GetMethodExact("Internal_ScriptUpdateFloatPropertiesInternal", "")->GetThunk();
		ScriptOnEventTriggeredInternalThunk = (ScriptOnEventTriggeredInternalThunkDef)metaData.ScriptClass->GetMethodExact("Internal_ScriptOnEventTriggeredInternal", "RRef`1<AnimationClip>,string")->GetThunk();
	}

	void ScriptCAnimation::ScriptRebuildFloatPropertiesInternal(const ResourceHandle<AnimationClip>& p0)
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

	void ScriptCAnimation::ScriptUpdateFloatPropertiesInternal()
	{
		MonoUtil::InvokeThunk(ScriptUpdateFloatPropertiesInternalThunk, GetManagedInstance());
	}

	void ScriptCAnimation::ScriptOnEventTriggeredInternal(const ResourceHandle<AnimationClip>& p0, const String& p1)
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
	void ScriptCAnimation::InternalSetDefaultClip(ScriptCAnimation* thisPtr, MonoObject* clip)
	{
		ResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::ToNative(clip);
		if(scriptclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptclip->GetHandle());
		thisPtr->GetHandle()->SetDefaultClip(tmpclip);
	}

	MonoObject* ScriptCAnimation::InternalGetDefaultClip(ScriptCAnimation* thisPtr)
	{
		ResourceHandle<AnimationClip> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetDefaultClip();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptCAnimation::InternalSetWrapMode(ScriptCAnimation* thisPtr, AnimWrapMode wrapMode)
	{
		thisPtr->GetHandle()->SetWrapMode(wrapMode);
	}

	AnimWrapMode ScriptCAnimation::InternalGetWrapMode(ScriptCAnimation* thisPtr)
	{
		AnimWrapMode tmp__output;
		tmp__output = thisPtr->GetHandle()->GetWrapMode();

		AnimWrapMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAnimation::InternalSetSpeed(ScriptCAnimation* thisPtr, float speed)
	{
		thisPtr->GetHandle()->SetSpeed(speed);
	}

	float ScriptCAnimation::InternalGetSpeed(ScriptCAnimation* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetSpeed();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAnimation::InternalPlay(ScriptCAnimation* thisPtr, MonoObject* clip)
	{
		ResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::ToNative(clip);
		if(scriptclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptclip->GetHandle());
		thisPtr->GetHandle()->Play(tmpclip);
	}

	void ScriptCAnimation::InternalBlendAdditive(ScriptCAnimation* thisPtr, MonoObject* clip, float weight, float fadeLength, uint32_t layer)
	{
		ResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::ToNative(clip);
		if(scriptclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptclip->GetHandle());
		thisPtr->GetHandle()->BlendAdditive(tmpclip, weight, fadeLength, layer);
	}

	void ScriptCAnimation::InternalBlend1D(ScriptCAnimation* thisPtr, __Blend1DInfoInterop* info, float t)
	{
		Blend1DInfo tmpinfo;
		tmpinfo = ScriptBlend1DInfo::FromInterop(*info);
		thisPtr->GetHandle()->Blend1D(tmpinfo, t);
	}

	void ScriptCAnimation::InternalBlend2D(ScriptCAnimation* thisPtr, __Blend2DInfoInterop* info, Vector2* t)
	{
		Blend2DInfo tmpinfo;
		tmpinfo = ScriptBlend2DInfo::FromInterop(*info);
		thisPtr->GetHandle()->Blend2D(tmpinfo, *t);
	}

	void ScriptCAnimation::InternalCrossFade(ScriptCAnimation* thisPtr, MonoObject* clip, float fadeLength)
	{
		ResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::ToNative(clip);
		if(scriptclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptclip->GetHandle());
		thisPtr->GetHandle()->CrossFade(tmpclip, fadeLength);
	}

	void ScriptCAnimation::InternalSample(ScriptCAnimation* thisPtr, MonoObject* clip, float time)
	{
		ResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::ToNative(clip);
		if(scriptclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptclip->GetHandle());
		thisPtr->GetHandle()->Sample(tmpclip, time);
	}

	void ScriptCAnimation::InternalStop(ScriptCAnimation* thisPtr, uint32_t layer)
	{
		thisPtr->GetHandle()->Stop(layer);
	}

	void ScriptCAnimation::InternalStopAll(ScriptCAnimation* thisPtr)
	{
		thisPtr->GetHandle()->StopAll();
	}

	bool ScriptCAnimation::InternalIsPlaying(ScriptCAnimation* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->IsPlaying();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptCAnimation::InternalGetState(ScriptCAnimation* thisPtr, MonoObject* clip, AnimationClipState* state)
	{
		bool tmp__output;
		ResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::ToNative(clip);
		if(scriptclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptclip->GetHandle());
		tmp__output = thisPtr->GetHandle()->GetState(tmpclip, *state);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAnimation::InternalSetState(ScriptCAnimation* thisPtr, MonoObject* clip, AnimationClipState* state)
	{
		ResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::ToNative(clip);
		if(scriptclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptclip->GetHandle());
		thisPtr->GetHandle()->SetState(tmpclip, *state);
	}

	void ScriptCAnimation::InternalSetMorphChannelWeight(ScriptCAnimation* thisPtr, MonoString* name, float weight)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		thisPtr->GetHandle()->SetMorphChannelWeight(tmpname, weight);
	}

	void ScriptCAnimation::InternalSetBounds(ScriptCAnimation* thisPtr, AABox* bounds)
	{
		thisPtr->GetHandle()->SetBounds(*bounds);
	}

	void ScriptCAnimation::InternalGetBounds(ScriptCAnimation* thisPtr, AABox* __output)
	{
		AABox tmp__output;
		tmp__output = thisPtr->GetHandle()->GetBounds();

		*__output = tmp__output;
	}

	void ScriptCAnimation::InternalSetUseBounds(ScriptCAnimation* thisPtr, bool enable)
	{
		thisPtr->GetHandle()->SetUseBounds(enable);
	}

	bool ScriptCAnimation::InternalGetUseBounds(ScriptCAnimation* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->GetUseBounds();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAnimation::InternalSetEnableCull(ScriptCAnimation* thisPtr, bool enable)
	{
		thisPtr->GetHandle()->SetEnableCull(enable);
	}

	bool ScriptCAnimation::InternalGetEnableCull(ScriptCAnimation* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->GetEnableCull();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptCAnimation::InternalGetNumClips(ScriptCAnimation* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetNumClips();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptCAnimation::InternalGetClip(ScriptCAnimation* thisPtr, uint32_t idx)
	{
		ResourceHandle<AnimationClip> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetClip(idx);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptCAnimation::InternalRefreshClipMappingsInternal(ScriptCAnimation* thisPtr)
	{
		thisPtr->GetHandle()->RefreshClipMappingsInternal();
	}

	bool ScriptCAnimation::InternalGetGenericCurveValueInternal(ScriptCAnimation* thisPtr, uint32_t curveIdx, float* value)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->GetGenericCurveValueInternal(curveIdx, *value);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptCAnimation::InternalTogglePreviewModeInternal(ScriptCAnimation* thisPtr, bool enabled)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->TogglePreviewModeInternal(enabled);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
