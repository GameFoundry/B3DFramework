//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptBlend2DInfo.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationClip.h"
#include "BsScriptAnimationClip.generated.h"

namespace bs
{
	ScriptBlend2DInfo::ScriptBlend2DInfo(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptBlend2DInfo::InitRuntimeData()
	{ }

	MonoObject*ScriptBlend2DInfo::Box(const __Blend2DInfoInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__Blend2DInfoInterop ScriptBlend2DInfo::Unbox(MonoObject* value)
	{
		return *(__Blend2DInfoInterop*)MonoUtil::Unbox(value);
	}

	Blend2DInfo ScriptBlend2DInfo::FromInterop(const __Blend2DInfoInterop& value)
	{
		Blend2DInfo output;
		ResourceHandle<AnimationClip> tmpTopLeftClip;
		ScriptRRefBase* scriptTopLeftClip;
		scriptTopLeftClip = ScriptRRefBase::ToNative(value.TopLeftClip);
		if(scriptTopLeftClip != nullptr)
			tmpTopLeftClip = B3DStaticResourceCast<AnimationClip>(scriptTopLeftClip->GetHandle());
		output.TopLeftClip = tmpTopLeftClip;
		ResourceHandle<AnimationClip> tmpTopRightClip;
		ScriptRRefBase* scriptTopRightClip;
		scriptTopRightClip = ScriptRRefBase::ToNative(value.TopRightClip);
		if(scriptTopRightClip != nullptr)
			tmpTopRightClip = B3DStaticResourceCast<AnimationClip>(scriptTopRightClip->GetHandle());
		output.TopRightClip = tmpTopRightClip;
		ResourceHandle<AnimationClip> tmpBotLeftClip;
		ScriptRRefBase* scriptBotLeftClip;
		scriptBotLeftClip = ScriptRRefBase::ToNative(value.BotLeftClip);
		if(scriptBotLeftClip != nullptr)
			tmpBotLeftClip = B3DStaticResourceCast<AnimationClip>(scriptBotLeftClip->GetHandle());
		output.BotLeftClip = tmpBotLeftClip;
		ResourceHandle<AnimationClip> tmpBotRightClip;
		ScriptRRefBase* scriptBotRightClip;
		scriptBotRightClip = ScriptRRefBase::ToNative(value.BotRightClip);
		if(scriptBotRightClip != nullptr)
			tmpBotRightClip = B3DStaticResourceCast<AnimationClip>(scriptBotRightClip->GetHandle());
		output.BotRightClip = tmpBotRightClip;

		return output;
	}

	__Blend2DInfoInterop ScriptBlend2DInfo::ToInterop(const Blend2DInfo& value)
	{
		__Blend2DInfoInterop output;
		MonoObject* tmpTopLeftClip;
		ScriptRRefBase* scriptTopLeftClip;
		scriptTopLeftClip = ScriptResourceManager::Instance().GetScriptRRef(value.TopLeftClip);
		if(scriptTopLeftClip != nullptr)
			tmpTopLeftClip = scriptTopLeftClip->GetManagedInstance();
		else
			tmpTopLeftClip = nullptr;
		output.TopLeftClip = tmpTopLeftClip;
		MonoObject* tmpTopRightClip;
		ScriptRRefBase* scriptTopRightClip;
		scriptTopRightClip = ScriptResourceManager::Instance().GetScriptRRef(value.TopRightClip);
		if(scriptTopRightClip != nullptr)
			tmpTopRightClip = scriptTopRightClip->GetManagedInstance();
		else
			tmpTopRightClip = nullptr;
		output.TopRightClip = tmpTopRightClip;
		MonoObject* tmpBotLeftClip;
		ScriptRRefBase* scriptBotLeftClip;
		scriptBotLeftClip = ScriptResourceManager::Instance().GetScriptRRef(value.BotLeftClip);
		if(scriptBotLeftClip != nullptr)
			tmpBotLeftClip = scriptBotLeftClip->GetManagedInstance();
		else
			tmpBotLeftClip = nullptr;
		output.BotLeftClip = tmpBotLeftClip;
		MonoObject* tmpBotRightClip;
		ScriptRRefBase* scriptBotRightClip;
		scriptBotRightClip = ScriptResourceManager::Instance().GetScriptRRef(value.BotRightClip);
		if(scriptBotRightClip != nullptr)
			tmpBotRightClip = scriptBotRightClip->GetManagedInstance();
		else
			tmpBotRightClip = nullptr;
		output.BotRightClip = tmpBotRightClip;

		return output;
	}

}
