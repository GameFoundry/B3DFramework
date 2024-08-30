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
	ScriptAnimation::ScriptRebuildFloatPropertiesInternalThunkDefinition ScriptAnimation::ScriptRebuildFloatPropertiesInternalThunk; 
	ScriptAnimation::ScriptUpdateFloatPropertiesInternalThunkDefinition ScriptAnimation::ScriptUpdateFloatPropertiesInternalThunk; 
	ScriptAnimation::ScriptOnEventTriggeredInternalThunkDefinition ScriptAnimation::ScriptOnEventTriggeredInternalThunk; 

	ScriptAnimation::ScriptAnimation(const GameObjectHandle<CAnimation>& nativeObject, MonoObject* scriptObject)
		:TScriptGameObjectWrapper(nativeObject, scriptObject)
	{
		RegisterEvents();
	}

	void ScriptAnimation::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDefaultClip", (void*)&ScriptAnimation::InternalSetDefaultClip);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDefaultClip", (void*)&ScriptAnimation::InternalGetDefaultClip);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetWrapMode", (void*)&ScriptAnimation::InternalSetWrapMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetWrapMode", (void*)&ScriptAnimation::InternalGetWrapMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSpeed", (void*)&ScriptAnimation::InternalSetSpeed);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSpeed", (void*)&ScriptAnimation::InternalGetSpeed);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Play", (void*)&ScriptAnimation::InternalPlay);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_BlendAdditive", (void*)&ScriptAnimation::InternalBlendAdditive);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Blend1D", (void*)&ScriptAnimation::InternalBlend1D);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Blend2D", (void*)&ScriptAnimation::InternalBlend2D);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CrossFade", (void*)&ScriptAnimation::InternalCrossFade);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Sample", (void*)&ScriptAnimation::InternalSample);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Stop", (void*)&ScriptAnimation::InternalStop);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_StopAll", (void*)&ScriptAnimation::InternalStopAll);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsPlaying", (void*)&ScriptAnimation::InternalIsPlaying);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetState", (void*)&ScriptAnimation::InternalGetState);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetState", (void*)&ScriptAnimation::InternalSetState);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMorphChannelWeight", (void*)&ScriptAnimation::InternalSetMorphChannelWeight);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBounds", (void*)&ScriptAnimation::InternalSetBounds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBounds", (void*)&ScriptAnimation::InternalGetBounds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetUseBounds", (void*)&ScriptAnimation::InternalSetUseBounds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUseBounds", (void*)&ScriptAnimation::InternalGetUseBounds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnableCull", (void*)&ScriptAnimation::InternalSetEnableCull);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnableCull", (void*)&ScriptAnimation::InternalGetEnableCull);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetNumClips", (void*)&ScriptAnimation::InternalGetNumClips);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetClip", (void*)&ScriptAnimation::InternalGetClip);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RefreshClipMappingsInternal", (void*)&ScriptAnimation::InternalRefreshClipMappingsInternal);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetGenericCurveValueInternal", (void*)&ScriptAnimation::InternalGetGenericCurveValueInternal);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TogglePreviewModeInternal", (void*)&ScriptAnimation::InternalTogglePreviewModeInternal);

		ScriptRebuildFloatPropertiesInternalThunk = (ScriptRebuildFloatPropertiesInternalThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_ScriptRebuildFloatPropertiesInternal", "RRef`1<AnimationClip>")->GetThunk();
		ScriptUpdateFloatPropertiesInternalThunk = (ScriptUpdateFloatPropertiesInternalThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_ScriptUpdateFloatPropertiesInternal", "")->GetThunk();
		ScriptOnEventTriggeredInternalThunk = (ScriptOnEventTriggeredInternalThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_ScriptOnEventTriggeredInternal", "RRef`1<AnimationClip>,string")->GetThunk();
	}

	MonoObject* ScriptAnimation::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
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
		MonoUtil::InvokeThunk(ScriptRebuildFloatPropertiesInternalThunk, GetScriptObject(), tmpp0);
	}

	void ScriptAnimation::ScriptUpdateFloatPropertiesInternal()
	{
		MonoUtil::InvokeThunk(ScriptUpdateFloatPropertiesInternalThunk, GetScriptObject());
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
		MonoUtil::InvokeThunk(ScriptOnEventTriggeredInternalThunk, GetScriptObject(), tmpp0, tmpp1);
	}

	void ScriptAnimation::RegisterEvents()
	{
		static_cast<CAnimation*>(GetNativeObject())->ScriptRebuildFloatPropertiesInternal = std::bind(&ScriptAnimation::ScriptRebuildFloatPropertiesInternal, this, std::placeholders::_1);
		static_cast<CAnimation*>(GetNativeObject())->ScriptUpdateFloatPropertiesInternal = std::bind(&ScriptAnimation::ScriptUpdateFloatPropertiesInternal, this);
		static_cast<CAnimation*>(GetNativeObject())->ScriptOnEventTriggeredInternal = std::bind(&ScriptAnimation::ScriptOnEventTriggeredInternal, this, std::placeholders::_1, std::placeholders::_2);
	}
	void ScriptAnimation::InternalSetDefaultClip(ScriptAnimation* self, MonoObject* clip)
	{
		TResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::ToNative(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperclip->GetHandle());
		static_cast<CAnimation*>(self->GetNativeObject())->SetDefaultClip(tmpclip);
	}

	MonoObject* ScriptAnimation::InternalGetDefaultClip(ScriptAnimation* self)
	{
		TResourceHandle<AnimationClip> tmp__output;
		tmp__output = static_cast<CAnimation*>(self->GetNativeObject())->GetDefaultClip();

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
		static_cast<CAnimation*>(self->GetNativeObject())->SetWrapMode(wrapMode);
	}

	AnimWrapMode ScriptAnimation::InternalGetWrapMode(ScriptAnimation* self)
	{
		AnimWrapMode tmp__output;
		tmp__output = static_cast<CAnimation*>(self->GetNativeObject())->GetWrapMode();

		AnimWrapMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimation::InternalSetSpeed(ScriptAnimation* self, float speed)
	{
		static_cast<CAnimation*>(self->GetNativeObject())->SetSpeed(speed);
	}

	float ScriptAnimation::InternalGetSpeed(ScriptAnimation* self)
	{
		float tmp__output;
		tmp__output = static_cast<CAnimation*>(self->GetNativeObject())->GetSpeed();

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
		static_cast<CAnimation*>(self->GetNativeObject())->Play(tmpclip);
	}

	void ScriptAnimation::InternalBlendAdditive(ScriptAnimation* self, MonoObject* clip, float weight, float fadeLength, uint32_t layer)
	{
		TResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::ToNative(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperclip->GetHandle());
		static_cast<CAnimation*>(self->GetNativeObject())->BlendAdditive(tmpclip, weight, fadeLength, layer);
	}

	void ScriptAnimation::InternalBlend1D(ScriptAnimation* self, __Blend1DInfoInterop* info, float t)
	{
		Blend1DInfo tmpinfo;
		tmpinfo = ScriptBlend1DInfo::FromInterop(*info);
		static_cast<CAnimation*>(self->GetNativeObject())->Blend1D(tmpinfo, t);
	}

	void ScriptAnimation::InternalBlend2D(ScriptAnimation* self, __Blend2DInfoInterop* info, TVector2<float>* t)
	{
		Blend2DInfo tmpinfo;
		tmpinfo = ScriptBlend2DInfo::FromInterop(*info);
		static_cast<CAnimation*>(self->GetNativeObject())->Blend2D(tmpinfo, *t);
	}

	void ScriptAnimation::InternalCrossFade(ScriptAnimation* self, MonoObject* clip, float fadeLength)
	{
		TResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::ToNative(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperclip->GetHandle());
		static_cast<CAnimation*>(self->GetNativeObject())->CrossFade(tmpclip, fadeLength);
	}

	void ScriptAnimation::InternalSample(ScriptAnimation* self, MonoObject* clip, float time)
	{
		TResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::ToNative(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperclip->GetHandle());
		static_cast<CAnimation*>(self->GetNativeObject())->Sample(tmpclip, time);
	}

	void ScriptAnimation::InternalStop(ScriptAnimation* self, uint32_t layer)
	{
		static_cast<CAnimation*>(self->GetNativeObject())->Stop(layer);
	}

	void ScriptAnimation::InternalStopAll(ScriptAnimation* self)
	{
		static_cast<CAnimation*>(self->GetNativeObject())->StopAll();
	}

	bool ScriptAnimation::InternalIsPlaying(ScriptAnimation* self)
	{
		bool tmp__output;
		tmp__output = static_cast<CAnimation*>(self->GetNativeObject())->IsPlaying();

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
		tmp__output = static_cast<CAnimation*>(self->GetNativeObject())->GetState(tmpclip, *state);

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
		static_cast<CAnimation*>(self->GetNativeObject())->SetState(tmpclip, *state);
	}

	void ScriptAnimation::InternalSetMorphChannelWeight(ScriptAnimation* self, MonoString* name, float weight)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<CAnimation*>(self->GetNativeObject())->SetMorphChannelWeight(tmpname, weight);
	}

	void ScriptAnimation::InternalSetBounds(ScriptAnimation* self, AABox* bounds)
	{
		static_cast<CAnimation*>(self->GetNativeObject())->SetBounds(*bounds);
	}

	void ScriptAnimation::InternalGetBounds(ScriptAnimation* self, AABox* __output)
	{
		AABox tmp__output;
		tmp__output = static_cast<CAnimation*>(self->GetNativeObject())->GetBounds();

		*__output = tmp__output;
	}

	void ScriptAnimation::InternalSetUseBounds(ScriptAnimation* self, bool enable)
	{
		static_cast<CAnimation*>(self->GetNativeObject())->SetUseBounds(enable);
	}

	bool ScriptAnimation::InternalGetUseBounds(ScriptAnimation* self)
	{
		bool tmp__output;
		tmp__output = static_cast<CAnimation*>(self->GetNativeObject())->GetUseBounds();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimation::InternalSetEnableCull(ScriptAnimation* self, bool enable)
	{
		static_cast<CAnimation*>(self->GetNativeObject())->SetEnableCull(enable);
	}

	bool ScriptAnimation::InternalGetEnableCull(ScriptAnimation* self)
	{
		bool tmp__output;
		tmp__output = static_cast<CAnimation*>(self->GetNativeObject())->GetEnableCull();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptAnimation::InternalGetNumClips(ScriptAnimation* self)
	{
		uint32_t tmp__output;
		tmp__output = static_cast<CAnimation*>(self->GetNativeObject())->GetNumClips();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptAnimation::InternalGetClip(ScriptAnimation* self, uint32_t idx)
	{
		TResourceHandle<AnimationClip> tmp__output;
		tmp__output = static_cast<CAnimation*>(self->GetNativeObject())->GetClip(idx);

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
		static_cast<CAnimation*>(self->GetNativeObject())->RefreshClipMappingsInternal();
	}

	bool ScriptAnimation::InternalGetGenericCurveValueInternal(ScriptAnimation* self, uint32_t curveIdx, float* value)
	{
		bool tmp__output;
		tmp__output = static_cast<CAnimation*>(self->GetNativeObject())->GetGenericCurveValueInternal(curveIdx, *value);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptAnimation::InternalTogglePreviewModeInternal(ScriptAnimation* self, bool enabled)
	{
		bool tmp__output;
		tmp__output = static_cast<CAnimation*>(self->GetNativeObject())->TogglePreviewModeInternal(enabled);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
