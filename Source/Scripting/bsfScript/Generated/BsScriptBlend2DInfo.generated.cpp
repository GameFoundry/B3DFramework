//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

	MonoObject*ScriptBlend2DInfo::box(const __Blend2DInfoInterop& value)
	{
		return MonoUtil::Box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__Blend2DInfoInterop ScriptBlend2DInfo::Unbox(MonoObject* value)
	{
		return *(__Blend2DInfoInterop*)MonoUtil::unbox(value);
	}

	Blend2DInfo ScriptBlend2DInfo::FromInterop(const __Blend2DInfoInterop& value)
	{
		Blend2DInfo output;
		ResourceHandle<AnimationClip> tmptopLeftClip;
		ScriptRRefBase* scripttopLeftClip;
		scripttopLeftClip = ScriptRRefBase::toNative(value.topLeftClip);
		if(scripttopLeftClip != nullptr)
			tmptopLeftClip = static_resource_cast<AnimationClip>(scripttopLeftClip->GetHandle());
		output.topLeftClip = tmptopLeftClip;
		ResourceHandle<AnimationClip> tmptopRightClip;
		ScriptRRefBase* scripttopRightClip;
		scripttopRightClip = ScriptRRefBase::toNative(value.topRightClip);
		if(scripttopRightClip != nullptr)
			tmptopRightClip = static_resource_cast<AnimationClip>(scripttopRightClip->GetHandle());
		output.topRightClip = tmptopRightClip;
		ResourceHandle<AnimationClip> tmpbotLeftClip;
		ScriptRRefBase* scriptbotLeftClip;
		scriptbotLeftClip = ScriptRRefBase::toNative(value.botLeftClip);
		if(scriptbotLeftClip != nullptr)
			tmpbotLeftClip = static_resource_cast<AnimationClip>(scriptbotLeftClip->GetHandle());
		output.botLeftClip = tmpbotLeftClip;
		ResourceHandle<AnimationClip> tmpbotRightClip;
		ScriptRRefBase* scriptbotRightClip;
		scriptbotRightClip = ScriptRRefBase::toNative(value.botRightClip);
		if(scriptbotRightClip != nullptr)
			tmpbotRightClip = static_resource_cast<AnimationClip>(scriptbotRightClip->GetHandle());
		output.botRightClip = tmpbotRightClip;

		return output;
	}

	__Blend2DInfoInterop ScriptBlend2DInfo::ToInterop(const Blend2DInfo& value)
	{
		__Blend2DInfoInterop output;
		ScriptRRefBase* scripttopLeftClip;
		scripttopLeftClip = ScriptResourceManager::instance().GetScriptRRef(value.topLeftClip);
		MonoObject* tmptopLeftClip;
		if(scripttopLeftClip != nullptr)
			tmptopLeftClip = scripttopLeftClip->GetManagedInstance();
		else
			tmptopLeftClip = nullptr;
		output.topLeftClip = tmptopLeftClip;
		ScriptRRefBase* scripttopRightClip;
		scripttopRightClip = ScriptResourceManager::instance().GetScriptRRef(value.topRightClip);
		MonoObject* tmptopRightClip;
		if(scripttopRightClip != nullptr)
			tmptopRightClip = scripttopRightClip->GetManagedInstance();
		else
			tmptopRightClip = nullptr;
		output.topRightClip = tmptopRightClip;
		ScriptRRefBase* scriptbotLeftClip;
		scriptbotLeftClip = ScriptResourceManager::instance().GetScriptRRef(value.botLeftClip);
		MonoObject* tmpbotLeftClip;
		if(scriptbotLeftClip != nullptr)
			tmpbotLeftClip = scriptbotLeftClip->GetManagedInstance();
		else
			tmpbotLeftClip = nullptr;
		output.botLeftClip = tmpbotLeftClip;
		ScriptRRefBase* scriptbotRightClip;
		scriptbotRightClip = ScriptResourceManager::instance().GetScriptRRef(value.botRightClip);
		MonoObject* tmpbotRightClip;
		if(scriptbotRightClip != nullptr)
			tmpbotRightClip = scriptbotRightClip->GetManagedInstance();
		else
			tmpbotRightClip = nullptr;
		output.botRightClip = tmpbotRightClip;

		return output;
	}

}
