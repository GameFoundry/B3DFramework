//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptAnimation.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DAnimation.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "../../../Foundation/Core/Animation/B3DAnimationClip.h"
#include "B3DScriptTAABox.generated.h"
#include "B3DScriptBlend1DInfo.generated.h"
#include "B3DScriptTVector2.generated.h"
#include "B3DScriptBlend2DInfo.generated.h"
#include "B3DScriptAnimationClipState.generated.h"

namespace b3d
{
	ScriptAnimation::ScriptRebuildFloatPropertiesInternalThunkDefinition ScriptAnimation::ScriptRebuildFloatPropertiesInternalThunk; 
	ScriptAnimation::ScriptUpdateFloatPropertiesInternalThunkDefinition ScriptAnimation::ScriptUpdateFloatPropertiesInternalThunk; 
	ScriptAnimation::ScriptOnEventTriggeredInternalThunkDefinition ScriptAnimation::ScriptOnEventTriggeredInternalThunk; 

	ScriptAnimation::ScriptAnimation(const GameObjectHandle<Animation>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptAnimation::~ScriptAnimation()
	{
		UnregisterEvents();
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
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCustomBounds", (void*)&ScriptAnimation::InternalSetCustomBounds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCustomBounds", (void*)&ScriptAnimation::InternalGetCustomBounds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetUseCustomBounds", (void*)&ScriptAnimation::InternalSetUseCustomBounds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUseCustomBounds", (void*)&ScriptAnimation::InternalGetUseCustomBounds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnableCull", (void*)&ScriptAnimation::InternalSetEnableCull);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnableCull", (void*)&ScriptAnimation::InternalGetEnableCull);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetClipCount", (void*)&ScriptAnimation::InternalGetClipCount);
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
			tmpp0 = scriptp0->GetScriptObject();
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
			tmpp0 = scriptp0->GetScriptObject();
		else
			tmpp0 = nullptr;
		MonoString* tmpp1;
		tmpp1 = MonoUtil::StringToMono(p1);
		MonoUtil::InvokeThunk(ScriptOnEventTriggeredInternalThunk, GetScriptObject(), tmpp0, tmpp1);
	}

	void ScriptAnimation::RegisterEvents()
	{
		static_cast<Animation*>(GetNativeObject())->ScriptRebuildFloatPropertiesInternal = std::bind(&ScriptAnimation::ScriptRebuildFloatPropertiesInternal, this, std::placeholders::_1);
		static_cast<Animation*>(GetNativeObject())->ScriptUpdateFloatPropertiesInternal = std::bind(&ScriptAnimation::ScriptUpdateFloatPropertiesInternal, this);
		static_cast<Animation*>(GetNativeObject())->ScriptOnEventTriggeredInternal = std::bind(&ScriptAnimation::ScriptOnEventTriggeredInternal, this, std::placeholders::_1, std::placeholders::_2);
	}
	void ScriptAnimation::UnregisterEvents()
	{
	}
	void ScriptAnimation::InternalSetDefaultClip(ScriptAnimation* self, MonoObject* clip)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::GetScriptObjectWrapper(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperclip->GetNativeObject());
		static_cast<Animation*>(self->GetNativeObject())->SetDefaultClip(tmpclip);
	}

	MonoObject* ScriptAnimation::InternalGetDefaultClip(ScriptAnimation* self)
	{
		TResourceHandle<AnimationClip> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Animation*>(self->GetNativeObject())->GetDefaultClip();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptAnimation::InternalSetWrapMode(ScriptAnimation* self, AnimationWrapMode wrapMode)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Animation*>(self->GetNativeObject())->SetWrapMode(wrapMode);
	}

	AnimationWrapMode ScriptAnimation::InternalGetWrapMode(ScriptAnimation* self)
	{
		AnimationWrapMode tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Animation*>(self->GetNativeObject())->GetWrapMode();

		AnimationWrapMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimation::InternalSetSpeed(ScriptAnimation* self, float speed)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Animation*>(self->GetNativeObject())->SetSpeed(speed);
	}

	float ScriptAnimation::InternalGetSpeed(ScriptAnimation* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Animation*>(self->GetNativeObject())->GetSpeed();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimation::InternalPlay(ScriptAnimation* self, MonoObject* clip)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::GetScriptObjectWrapper(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperclip->GetNativeObject());
		static_cast<Animation*>(self->GetNativeObject())->Play(tmpclip);
	}

	void ScriptAnimation::InternalBlendAdditive(ScriptAnimation* self, MonoObject* clip, float weight, float fadeLength, uint32_t layer)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::GetScriptObjectWrapper(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperclip->GetNativeObject());
		static_cast<Animation*>(self->GetNativeObject())->BlendAdditive(tmpclip, weight, fadeLength, layer);
	}

	void ScriptAnimation::InternalBlend1D(ScriptAnimation* self, __Blend1DInfoInterop* info, float alpha)
	{
		if(!self->IsNativeObjectValid())
			return;

		Blend1DInfo tmpinfo;
		tmpinfo = ScriptBlend1DInfo::FromInterop(*info);
		static_cast<Animation*>(self->GetNativeObject())->Blend1D(tmpinfo, alpha);
	}

	void ScriptAnimation::InternalBlend2D(ScriptAnimation* self, __Blend2DInfoInterop* info, TVector2<float>* alpha)
	{
		if(!self->IsNativeObjectValid())
			return;

		Blend2DInfo tmpinfo;
		tmpinfo = ScriptBlend2DInfo::FromInterop(*info);
		static_cast<Animation*>(self->GetNativeObject())->Blend2D(tmpinfo, *alpha);
	}

	void ScriptAnimation::InternalCrossFade(ScriptAnimation* self, MonoObject* clip, float fadeLength)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::GetScriptObjectWrapper(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperclip->GetNativeObject());
		static_cast<Animation*>(self->GetNativeObject())->CrossFade(tmpclip, fadeLength);
	}

	void ScriptAnimation::InternalSample(ScriptAnimation* self, MonoObject* clip, float time)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::GetScriptObjectWrapper(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperclip->GetNativeObject());
		static_cast<Animation*>(self->GetNativeObject())->Sample(tmpclip, time);
	}

	void ScriptAnimation::InternalStop(ScriptAnimation* self, uint32_t layer)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Animation*>(self->GetNativeObject())->Stop(layer);
	}

	void ScriptAnimation::InternalStopAll(ScriptAnimation* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Animation*>(self->GetNativeObject())->StopAll();
	}

	bool ScriptAnimation::InternalIsPlaying(ScriptAnimation* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Animation*>(self->GetNativeObject())->IsPlaying();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptAnimation::InternalGetState(ScriptAnimation* self, MonoObject* clip, AnimationClipState* outState)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		TResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::GetScriptObjectWrapper(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperclip->GetNativeObject());
		tmp__output = static_cast<Animation*>(self->GetNativeObject())->GetState(tmpclip, *outState);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimation::InternalSetState(ScriptAnimation* self, MonoObject* clip, AnimationClipState* state)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptObjectWrapperclip;
		scriptObjectWrapperclip = ScriptRRefBase::GetScriptObjectWrapper(clip);
		if(scriptObjectWrapperclip != nullptr)
			tmpclip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperclip->GetNativeObject());
		static_cast<Animation*>(self->GetNativeObject())->SetState(tmpclip, *state);
	}

	void ScriptAnimation::InternalSetMorphChannelWeight(ScriptAnimation* self, MonoString* name, float weight)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<Animation*>(self->GetNativeObject())->SetMorphChannelWeight(tmpname, weight);
	}

	void ScriptAnimation::InternalSetCustomBounds(ScriptAnimation* self, __TAABox_float_Interop* bounds)
	{
		if(!self->IsNativeObjectValid())
			return;

		TAABox<float> tmpbounds;
		tmpbounds = ScriptAABox::FromInterop(*bounds);
		static_cast<Animation*>(self->GetNativeObject())->SetCustomBounds(tmpbounds);
	}

	void ScriptAnimation::InternalGetCustomBounds(ScriptAnimation* self, __TAABox_float_Interop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TAABox<float> tmp__output;
		tmp__output = static_cast<Animation*>(self->GetNativeObject())->GetCustomBounds();

		__TAABox_float_Interop interop__output;
		interop__output = ScriptAABox::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptAABox::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptAnimation::InternalSetUseCustomBounds(ScriptAnimation* self, bool enable)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Animation*>(self->GetNativeObject())->SetUseCustomBounds(enable);
	}

	bool ScriptAnimation::InternalGetUseCustomBounds(ScriptAnimation* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Animation*>(self->GetNativeObject())->GetUseCustomBounds();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimation::InternalSetEnableCull(ScriptAnimation* self, bool enable)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Animation*>(self->GetNativeObject())->SetEnableCull(enable);
	}

	bool ScriptAnimation::InternalGetEnableCull(ScriptAnimation* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Animation*>(self->GetNativeObject())->GetEnableCull();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptAnimation::InternalGetClipCount(ScriptAnimation* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Animation*>(self->GetNativeObject())->GetClipCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptAnimation::InternalGetClip(ScriptAnimation* self, uint32_t index)
	{
		TResourceHandle<AnimationClip> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Animation*>(self->GetNativeObject())->GetClip(index);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptAnimation::InternalRefreshClipMappingsInternal(ScriptAnimation* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Animation*>(self->GetNativeObject())->RefreshClipMappingsInternal();
	}

	bool ScriptAnimation::InternalGetGenericCurveValueInternal(ScriptAnimation* self, uint32_t curveIndex, float* outValue)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Animation*>(self->GetNativeObject())->GetGenericCurveValueInternal(curveIndex, *outValue);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptAnimation::InternalTogglePreviewModeInternal(ScriptAnimation* self, bool enabled)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Animation*>(self->GetNativeObject())->TogglePreviewModeInternal(enabled);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
