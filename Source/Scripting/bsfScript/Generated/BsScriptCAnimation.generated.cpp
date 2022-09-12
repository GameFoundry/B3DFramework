//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
	ScriptCAnimation::_scriptRebuildFloatPropertiesThunkDef ScriptCAnimation::_scriptRebuildFloatPropertiesThunk; 
	ScriptCAnimation::_scriptUpdateFloatPropertiesThunkDef ScriptCAnimation::_scriptUpdateFloatPropertiesThunk; 
	ScriptCAnimation::_scriptOnEventTriggeredThunkDef ScriptCAnimation::_scriptOnEventTriggeredThunk; 

	ScriptCAnimation::ScriptCAnimation(MonoObject* managedInstance, const GameObjectHandle<CAnimation>& value)
		:TScriptComponent(managedInstance, value)
	{
		value->_scriptRebuildFloatProperties = std::bind(&ScriptCAnimation::_scriptRebuildFloatProperties, this, std::placeholders::_1);
		value->_scriptUpdateFloatProperties = std::bind(&ScriptCAnimation::_scriptUpdateFloatProperties, this);
		value->_scriptOnEventTriggered = std::bind(&ScriptCAnimation::_scriptOnEventTriggered, this, std::placeholders::_1, std::placeholders::_2);
	}

	void ScriptCAnimation::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_setDefaultClip", (void*)&ScriptCAnimation::Internal_setDefaultClip);
		metaData.scriptClass->AddInternalCall("Internal_getDefaultClip", (void*)&ScriptCAnimation::Internal_getDefaultClip);
		metaData.scriptClass->AddInternalCall("Internal_setWrapMode", (void*)&ScriptCAnimation::Internal_setWrapMode);
		metaData.scriptClass->AddInternalCall("Internal_getWrapMode", (void*)&ScriptCAnimation::Internal_getWrapMode);
		metaData.scriptClass->AddInternalCall("Internal_setSpeed", (void*)&ScriptCAnimation::Internal_setSpeed);
		metaData.scriptClass->AddInternalCall("Internal_getSpeed", (void*)&ScriptCAnimation::Internal_getSpeed);
		metaData.scriptClass->AddInternalCall("Internal_play", (void*)&ScriptCAnimation::Internal_play);
		metaData.scriptClass->AddInternalCall("Internal_blendAdditive", (void*)&ScriptCAnimation::Internal_blendAdditive);
		metaData.scriptClass->AddInternalCall("Internal_blend1D", (void*)&ScriptCAnimation::Internal_blend1D);
		metaData.scriptClass->AddInternalCall("Internal_blend2D", (void*)&ScriptCAnimation::Internal_blend2D);
		metaData.scriptClass->AddInternalCall("Internal_crossFade", (void*)&ScriptCAnimation::Internal_crossFade);
		metaData.scriptClass->AddInternalCall("Internal_sample", (void*)&ScriptCAnimation::Internal_sample);
		metaData.scriptClass->AddInternalCall("Internal_stop", (void*)&ScriptCAnimation::Internal_stop);
		metaData.scriptClass->AddInternalCall("Internal_stopAll", (void*)&ScriptCAnimation::Internal_stopAll);
		metaData.scriptClass->AddInternalCall("Internal_isPlaying", (void*)&ScriptCAnimation::Internal_isPlaying);
		metaData.scriptClass->AddInternalCall("Internal_getState", (void*)&ScriptCAnimation::Internal_getState);
		metaData.scriptClass->AddInternalCall("Internal_setState", (void*)&ScriptCAnimation::Internal_setState);
		metaData.scriptClass->AddInternalCall("Internal_setMorphChannelWeight", (void*)&ScriptCAnimation::Internal_setMorphChannelWeight);
		metaData.scriptClass->AddInternalCall("Internal_setBounds", (void*)&ScriptCAnimation::Internal_setBounds);
		metaData.scriptClass->AddInternalCall("Internal_getBounds", (void*)&ScriptCAnimation::Internal_getBounds);
		metaData.scriptClass->AddInternalCall("Internal_setUseBounds", (void*)&ScriptCAnimation::Internal_setUseBounds);
		metaData.scriptClass->AddInternalCall("Internal_getUseBounds", (void*)&ScriptCAnimation::Internal_getUseBounds);
		metaData.scriptClass->AddInternalCall("Internal_setEnableCull", (void*)&ScriptCAnimation::Internal_setEnableCull);
		metaData.scriptClass->AddInternalCall("Internal_getEnableCull", (void*)&ScriptCAnimation::Internal_getEnableCull);
		metaData.scriptClass->AddInternalCall("Internal_getNumClips", (void*)&ScriptCAnimation::Internal_getNumClips);
		metaData.scriptClass->AddInternalCall("Internal_getClip", (void*)&ScriptCAnimation::Internal_getClip);
		metaData.scriptClass->AddInternalCall("Internal__refreshClipMappings", (void*)&ScriptCAnimation::Internal__refreshClipMappings);
		metaData.scriptClass->AddInternalCall("Internal__getGenericCurveValue", (void*)&ScriptCAnimation::Internal__getGenericCurveValue);
		metaData.scriptClass->AddInternalCall("Internal__togglePreviewMode", (void*)&ScriptCAnimation::Internal__togglePreviewMode);

		_scriptRebuildFloatPropertiesThunk = (_scriptRebuildFloatPropertiesThunkDef)metaData.scriptClass->GetMethodExact("Internal__scriptRebuildFloatProperties", "RRef`1<AnimationClip>")->getThunk();
		_scriptUpdateFloatPropertiesThunk = (_scriptUpdateFloatPropertiesThunkDef)metaData.scriptClass->GetMethodExact("Internal__scriptUpdateFloatProperties", "")->getThunk();
		_scriptOnEventTriggeredThunk = (_scriptOnEventTriggeredThunkDef)metaData.scriptClass->GetMethodExact("Internal__scriptOnEventTriggered", "RRef`1<AnimationClip>,string")->getThunk();
	}

	void ScriptCAnimation::_scriptRebuildFloatProperties(const ResourceHandle<AnimationClip>& p0)
	{
		MonoObject* tmpp0;
		ScriptRRefBase* scriptp0;
		scriptp0 = ScriptResourceManager::instance().GetScriptRRef(p0);
		if(scriptp0 != nullptr)
			tmpp0 = scriptp0->GetManagedInstance();
		else
			tmpp0 = nullptr;
		MonoUtil::invokeThunk(_scriptRebuildFloatPropertiesThunk, getManagedInstance(), tmpp0);
	}

	void ScriptCAnimation::_scriptUpdateFloatProperties()
	{
		MonoUtil::invokeThunk(_scriptUpdateFloatPropertiesThunk, getManagedInstance());
	}

	void ScriptCAnimation::_scriptOnEventTriggered(const ResourceHandle<AnimationClip>& p0, const String& p1)
	{
		MonoObject* tmpp0;
		ScriptRRefBase* scriptp0;
		scriptp0 = ScriptResourceManager::instance().GetScriptRRef(p0);
		if(scriptp0 != nullptr)
			tmpp0 = scriptp0->GetManagedInstance();
		else
			tmpp0 = nullptr;
		MonoString* tmpp1;
		tmpp1 = MonoUtil::stringToMono(p1);
		MonoUtil::invokeThunk(_scriptOnEventTriggeredThunk, getManagedInstance(), tmpp0, tmpp1);
	}
	void ScriptCAnimation::Internal_setDefaultClip(ScriptCAnimation* thisPtr, MonoObject* clip)
	{
		ResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::toNative(clip);
		if(scriptclip != nullptr)
			tmpclip = static_resource_cast<AnimationClip>(scriptclip->GetHandle());
		thisPtr->GetHandle()->setDefaultClip(tmpclip);
	}

	MonoObject* ScriptCAnimation::Internal_getDefaultClip(ScriptCAnimation* thisPtr)
	{
		ResourceHandle<AnimationClip> tmp__output;
		tmp__output = thisPtr->GetHandle()->getDefaultClip();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptCAnimation::Internal_setWrapMode(ScriptCAnimation* thisPtr, AnimWrapMode wrapMode)
	{
		thisPtr->GetHandle()->setWrapMode(wrapMode);
	}

	AnimWrapMode ScriptCAnimation::Internal_getWrapMode(ScriptCAnimation* thisPtr)
	{
		AnimWrapMode tmp__output;
		tmp__output = thisPtr->GetHandle()->getWrapMode();

		AnimWrapMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAnimation::Internal_setSpeed(ScriptCAnimation* thisPtr, float speed)
	{
		thisPtr->GetHandle()->setSpeed(speed);
	}

	float ScriptCAnimation::Internal_getSpeed(ScriptCAnimation* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getSpeed();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAnimation::Internal_play(ScriptCAnimation* thisPtr, MonoObject* clip)
	{
		ResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::toNative(clip);
		if(scriptclip != nullptr)
			tmpclip = static_resource_cast<AnimationClip>(scriptclip->GetHandle());
		thisPtr->GetHandle()->play(tmpclip);
	}

	void ScriptCAnimation::Internal_blendAdditive(ScriptCAnimation* thisPtr, MonoObject* clip, float weight, float fadeLength, uint32_t layer)
	{
		ResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::toNative(clip);
		if(scriptclip != nullptr)
			tmpclip = static_resource_cast<AnimationClip>(scriptclip->GetHandle());
		thisPtr->GetHandle()->blendAdditive(tmpclip, weight, fadeLength, layer);
	}

	void ScriptCAnimation::Internal_blend1D(ScriptCAnimation* thisPtr, __Blend1DInfoInterop* info, float t)
	{
		Blend1DInfo tmpinfo;
		tmpinfo = ScriptBlend1DInfo::fromInterop(*info);
		thisPtr->GetHandle()->blend1D(tmpinfo, t);
	}

	void ScriptCAnimation::Internal_blend2D(ScriptCAnimation* thisPtr, __Blend2DInfoInterop* info, Vector2* t)
	{
		Blend2DInfo tmpinfo;
		tmpinfo = ScriptBlend2DInfo::fromInterop(*info);
		thisPtr->GetHandle()->blend2D(tmpinfo, *t);
	}

	void ScriptCAnimation::Internal_crossFade(ScriptCAnimation* thisPtr, MonoObject* clip, float fadeLength)
	{
		ResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::toNative(clip);
		if(scriptclip != nullptr)
			tmpclip = static_resource_cast<AnimationClip>(scriptclip->GetHandle());
		thisPtr->GetHandle()->crossFade(tmpclip, fadeLength);
	}

	void ScriptCAnimation::Internal_sample(ScriptCAnimation* thisPtr, MonoObject* clip, float time)
	{
		ResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::toNative(clip);
		if(scriptclip != nullptr)
			tmpclip = static_resource_cast<AnimationClip>(scriptclip->GetHandle());
		thisPtr->GetHandle()->sample(tmpclip, time);
	}

	void ScriptCAnimation::Internal_stop(ScriptCAnimation* thisPtr, uint32_t layer)
	{
		thisPtr->GetHandle()->stop(layer);
	}

	void ScriptCAnimation::Internal_stopAll(ScriptCAnimation* thisPtr)
	{
		thisPtr->GetHandle()->stopAll();
	}

	bool ScriptCAnimation::Internal_isPlaying(ScriptCAnimation* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->isPlaying();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptCAnimation::Internal_getState(ScriptCAnimation* thisPtr, MonoObject* clip, AnimationClipState* state)
	{
		bool tmp__output;
		ResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::toNative(clip);
		if(scriptclip != nullptr)
			tmpclip = static_resource_cast<AnimationClip>(scriptclip->GetHandle());
		tmp__output = thisPtr->GetHandle()->getState(tmpclip, *state);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAnimation::Internal_setState(ScriptCAnimation* thisPtr, MonoObject* clip, AnimationClipState* state)
	{
		ResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::toNative(clip);
		if(scriptclip != nullptr)
			tmpclip = static_resource_cast<AnimationClip>(scriptclip->GetHandle());
		thisPtr->GetHandle()->setState(tmpclip, *state);
	}

	void ScriptCAnimation::Internal_setMorphChannelWeight(ScriptCAnimation* thisPtr, MonoString* name, float weight)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		thisPtr->GetHandle()->setMorphChannelWeight(tmpname, weight);
	}

	void ScriptCAnimation::Internal_setBounds(ScriptCAnimation* thisPtr, AABox* bounds)
	{
		thisPtr->GetHandle()->setBounds(*bounds);
	}

	void ScriptCAnimation::Internal_getBounds(ScriptCAnimation* thisPtr, AABox* __output)
	{
		AABox tmp__output;
		tmp__output = thisPtr->GetHandle()->getBounds();

		*__output = tmp__output;
	}

	void ScriptCAnimation::Internal_setUseBounds(ScriptCAnimation* thisPtr, bool enable)
	{
		thisPtr->GetHandle()->setUseBounds(enable);
	}

	bool ScriptCAnimation::Internal_getUseBounds(ScriptCAnimation* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->getUseBounds();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAnimation::Internal_setEnableCull(ScriptCAnimation* thisPtr, bool enable)
	{
		thisPtr->GetHandle()->setEnableCull(enable);
	}

	bool ScriptCAnimation::Internal_getEnableCull(ScriptCAnimation* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->getEnableCull();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptCAnimation::Internal_getNumClips(ScriptCAnimation* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->getNumClips();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptCAnimation::Internal_getClip(ScriptCAnimation* thisPtr, uint32_t idx)
	{
		ResourceHandle<AnimationClip> tmp__output;
		tmp__output = thisPtr->GetHandle()->getClip(idx);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptCAnimation::Internal__refreshClipMappings(ScriptCAnimation* thisPtr)
	{
		thisPtr->GetHandle()->_refreshClipMappings();
	}

	bool ScriptCAnimation::Internal__getGenericCurveValue(ScriptCAnimation* thisPtr, uint32_t curveIdx, float* value)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->_getGenericCurveValue(curveIdx, *value);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptCAnimation::Internal__togglePreviewMode(ScriptCAnimation* thisPtr, bool enabled)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->_togglePreviewMode(enabled);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
