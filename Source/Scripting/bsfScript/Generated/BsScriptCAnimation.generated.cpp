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

	void ScriptCAnimation::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setDefaultClip", (void*)&ScriptCAnimation::Internal_setDefaultClip);
		metaData.scriptClass->addInternalCall("Internal_getDefaultClip", (void*)&ScriptCAnimation::Internal_getDefaultClip);
		metaData.scriptClass->addInternalCall("Internal_setWrapMode", (void*)&ScriptCAnimation::Internal_setWrapMode);
		metaData.scriptClass->addInternalCall("Internal_getWrapMode", (void*)&ScriptCAnimation::Internal_getWrapMode);
		metaData.scriptClass->addInternalCall("Internal_setSpeed", (void*)&ScriptCAnimation::Internal_setSpeed);
		metaData.scriptClass->addInternalCall("Internal_getSpeed", (void*)&ScriptCAnimation::Internal_getSpeed);
		metaData.scriptClass->addInternalCall("Internal_play", (void*)&ScriptCAnimation::Internal_play);
		metaData.scriptClass->addInternalCall("Internal_blendAdditive", (void*)&ScriptCAnimation::Internal_blendAdditive);
		metaData.scriptClass->addInternalCall("Internal_blend1D", (void*)&ScriptCAnimation::Internal_blend1D);
		metaData.scriptClass->addInternalCall("Internal_blend2D", (void*)&ScriptCAnimation::Internal_blend2D);
		metaData.scriptClass->addInternalCall("Internal_crossFade", (void*)&ScriptCAnimation::Internal_crossFade);
		metaData.scriptClass->addInternalCall("Internal_sample", (void*)&ScriptCAnimation::Internal_sample);
		metaData.scriptClass->addInternalCall("Internal_stop", (void*)&ScriptCAnimation::Internal_stop);
		metaData.scriptClass->addInternalCall("Internal_stopAll", (void*)&ScriptCAnimation::Internal_stopAll);
		metaData.scriptClass->addInternalCall("Internal_isPlaying", (void*)&ScriptCAnimation::Internal_isPlaying);
		metaData.scriptClass->addInternalCall("Internal_getState", (void*)&ScriptCAnimation::Internal_getState);
		metaData.scriptClass->addInternalCall("Internal_setState", (void*)&ScriptCAnimation::Internal_setState);
		metaData.scriptClass->addInternalCall("Internal_setMorphChannelWeight", (void*)&ScriptCAnimation::Internal_setMorphChannelWeight);
		metaData.scriptClass->addInternalCall("Internal_setBounds", (void*)&ScriptCAnimation::Internal_setBounds);
		metaData.scriptClass->addInternalCall("Internal_getBounds", (void*)&ScriptCAnimation::Internal_getBounds);
		metaData.scriptClass->addInternalCall("Internal_setUseBounds", (void*)&ScriptCAnimation::Internal_setUseBounds);
		metaData.scriptClass->addInternalCall("Internal_getUseBounds", (void*)&ScriptCAnimation::Internal_getUseBounds);
		metaData.scriptClass->addInternalCall("Internal_setEnableCull", (void*)&ScriptCAnimation::Internal_setEnableCull);
		metaData.scriptClass->addInternalCall("Internal_getEnableCull", (void*)&ScriptCAnimation::Internal_getEnableCull);
		metaData.scriptClass->addInternalCall("Internal_getNumClips", (void*)&ScriptCAnimation::Internal_getNumClips);
		metaData.scriptClass->addInternalCall("Internal_getClip", (void*)&ScriptCAnimation::Internal_getClip);
		metaData.scriptClass->addInternalCall("Internal__refreshClipMappings", (void*)&ScriptCAnimation::Internal__refreshClipMappings);
		metaData.scriptClass->addInternalCall("Internal__getGenericCurveValue", (void*)&ScriptCAnimation::Internal__getGenericCurveValue);
		metaData.scriptClass->addInternalCall("Internal__togglePreviewMode", (void*)&ScriptCAnimation::Internal__togglePreviewMode);

		_scriptRebuildFloatPropertiesThunk = (_scriptRebuildFloatPropertiesThunkDef)metaData.scriptClass->getMethodExact("Internal__scriptRebuildFloatProperties", "RRef`1<AnimationClip>")->getThunk();
		_scriptUpdateFloatPropertiesThunk = (_scriptUpdateFloatPropertiesThunkDef)metaData.scriptClass->getMethodExact("Internal__scriptUpdateFloatProperties", "")->getThunk();
		_scriptOnEventTriggeredThunk = (_scriptOnEventTriggeredThunkDef)metaData.scriptClass->getMethodExact("Internal__scriptOnEventTriggered", "RRef`1<AnimationClip>,string")->getThunk();
	}

	void ScriptCAnimation::_scriptRebuildFloatProperties(const ResourceHandle<AnimationClip>& p0)
	{
		MonoObject* tmpp0;
		ScriptRRefBase* scriptp0;
		scriptp0 = ScriptResourceManager::instance().getScriptRRef(p0);
		if(scriptp0 != nullptr)
			tmpp0 = scriptp0->getManagedInstance();
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
		scriptp0 = ScriptResourceManager::instance().getScriptRRef(p0);
		if(scriptp0 != nullptr)
			tmpp0 = scriptp0->getManagedInstance();
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
			tmpclip = static_resource_cast<AnimationClip>(scriptclip->getHandle());
		thisPtr->getHandle()->setDefaultClip(tmpclip);
	}

	MonoObject* ScriptCAnimation::Internal_getDefaultClip(ScriptCAnimation* thisPtr)
	{
		ResourceHandle<AnimationClip> tmp__output;
		tmp__output = thisPtr->getHandle()->getDefaultClip();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().getScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->getManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptCAnimation::Internal_setWrapMode(ScriptCAnimation* thisPtr, AnimWrapMode wrapMode)
	{
		thisPtr->getHandle()->setWrapMode(wrapMode);
	}

	AnimWrapMode ScriptCAnimation::Internal_getWrapMode(ScriptCAnimation* thisPtr)
	{
		AnimWrapMode tmp__output;
		tmp__output = thisPtr->getHandle()->getWrapMode();

		AnimWrapMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAnimation::Internal_setSpeed(ScriptCAnimation* thisPtr, float speed)
	{
		thisPtr->getHandle()->setSpeed(speed);
	}

	float ScriptCAnimation::Internal_getSpeed(ScriptCAnimation* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getSpeed();

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
			tmpclip = static_resource_cast<AnimationClip>(scriptclip->getHandle());
		thisPtr->getHandle()->play(tmpclip);
	}

	void ScriptCAnimation::Internal_blendAdditive(ScriptCAnimation* thisPtr, MonoObject* clip, float weight, float fadeLength, uint32_t layer)
	{
		ResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::toNative(clip);
		if(scriptclip != nullptr)
			tmpclip = static_resource_cast<AnimationClip>(scriptclip->getHandle());
		thisPtr->getHandle()->blendAdditive(tmpclip, weight, fadeLength, layer);
	}

	void ScriptCAnimation::Internal_blend1D(ScriptCAnimation* thisPtr, __Blend1DInfoInterop* info, float t)
	{
		Blend1DInfo tmpinfo;
		tmpinfo = ScriptBlend1DInfo::fromInterop(*info);
		thisPtr->getHandle()->blend1D(tmpinfo, t);
	}

	void ScriptCAnimation::Internal_blend2D(ScriptCAnimation* thisPtr, __Blend2DInfoInterop* info, Vector2* t)
	{
		Blend2DInfo tmpinfo;
		tmpinfo = ScriptBlend2DInfo::fromInterop(*info);
		thisPtr->getHandle()->blend2D(tmpinfo, *t);
	}

	void ScriptCAnimation::Internal_crossFade(ScriptCAnimation* thisPtr, MonoObject* clip, float fadeLength)
	{
		ResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::toNative(clip);
		if(scriptclip != nullptr)
			tmpclip = static_resource_cast<AnimationClip>(scriptclip->getHandle());
		thisPtr->getHandle()->crossFade(tmpclip, fadeLength);
	}

	void ScriptCAnimation::Internal_sample(ScriptCAnimation* thisPtr, MonoObject* clip, float time)
	{
		ResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::toNative(clip);
		if(scriptclip != nullptr)
			tmpclip = static_resource_cast<AnimationClip>(scriptclip->getHandle());
		thisPtr->getHandle()->sample(tmpclip, time);
	}

	void ScriptCAnimation::Internal_stop(ScriptCAnimation* thisPtr, uint32_t layer)
	{
		thisPtr->getHandle()->stop(layer);
	}

	void ScriptCAnimation::Internal_stopAll(ScriptCAnimation* thisPtr)
	{
		thisPtr->getHandle()->stopAll();
	}

	bool ScriptCAnimation::Internal_isPlaying(ScriptCAnimation* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->isPlaying();

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
			tmpclip = static_resource_cast<AnimationClip>(scriptclip->getHandle());
		tmp__output = thisPtr->getHandle()->getState(tmpclip, *state);

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
			tmpclip = static_resource_cast<AnimationClip>(scriptclip->getHandle());
		thisPtr->getHandle()->setState(tmpclip, *state);
	}

	void ScriptCAnimation::Internal_setMorphChannelWeight(ScriptCAnimation* thisPtr, MonoString* name, float weight)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		thisPtr->getHandle()->setMorphChannelWeight(tmpname, weight);
	}

	void ScriptCAnimation::Internal_setBounds(ScriptCAnimation* thisPtr, AABox* bounds)
	{
		thisPtr->getHandle()->setBounds(*bounds);
	}

	void ScriptCAnimation::Internal_getBounds(ScriptCAnimation* thisPtr, AABox* __output)
	{
		AABox tmp__output;
		tmp__output = thisPtr->getHandle()->getBounds();

		*__output = tmp__output;
	}

	void ScriptCAnimation::Internal_setUseBounds(ScriptCAnimation* thisPtr, bool enable)
	{
		thisPtr->getHandle()->setUseBounds(enable);
	}

	bool ScriptCAnimation::Internal_getUseBounds(ScriptCAnimation* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->getUseBounds();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCAnimation::Internal_setEnableCull(ScriptCAnimation* thisPtr, bool enable)
	{
		thisPtr->getHandle()->setEnableCull(enable);
	}

	bool ScriptCAnimation::Internal_getEnableCull(ScriptCAnimation* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->getEnableCull();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptCAnimation::Internal_getNumClips(ScriptCAnimation* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getHandle()->getNumClips();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptCAnimation::Internal_getClip(ScriptCAnimation* thisPtr, uint32_t idx)
	{
		ResourceHandle<AnimationClip> tmp__output;
		tmp__output = thisPtr->getHandle()->getClip(idx);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().getScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->getManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptCAnimation::Internal__refreshClipMappings(ScriptCAnimation* thisPtr)
	{
		thisPtr->getHandle()->_refreshClipMappings();
	}

	bool ScriptCAnimation::Internal__getGenericCurveValue(ScriptCAnimation* thisPtr, uint32_t curveIdx, float* value)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->_getGenericCurveValue(curveIdx, *value);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptCAnimation::Internal__togglePreviewMode(ScriptCAnimation* thisPtr, bool enabled)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->_togglePreviewMode(enabled);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
