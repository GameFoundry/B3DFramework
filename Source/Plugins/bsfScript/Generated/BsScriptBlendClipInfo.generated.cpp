//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptBlendClipInfo.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../../../Foundation/bsfCore/Animation/BsAnimationClip.h"
#include "BsScriptAnimationClip.generated.h"

namespace bs
{
	ScriptBlendClipInfo::ScriptBlendClipInfo(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptBlendClipInfo::initRuntimeData()
	{ }

	MonoObject*ScriptBlendClipInfo::box(const __BlendClipInfoInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__BlendClipInfoInterop ScriptBlendClipInfo::unbox(MonoObject* value)
	{
		return *(__BlendClipInfoInterop*)MonoUtil::unbox(value);
	}

	BlendClipInfo ScriptBlendClipInfo::fromInterop(const __BlendClipInfoInterop& value)
	{
		BlendClipInfo output;
		ResourceHandle<AnimationClip> tmpclip;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptRRefBase::toNative(value.clip);
		if(scriptclip != nullptr)
			tmpclip = static_resource_cast<AnimationClip>(scriptclip->getHandle());
		output.clip = tmpclip;
		output.position = value.position;

		return output;
	}

	__BlendClipInfoInterop ScriptBlendClipInfo::toInterop(const BlendClipInfo& value)
	{
		__BlendClipInfoInterop output;
		ScriptRRefBase* scriptclip;
		scriptclip = ScriptResourceManager::instance().getScriptRRef(value.clip);
		MonoObject* tmpclip;
		if(scriptclip != nullptr)
			tmpclip = scriptclip->getManagedInstance();
		else
			tmpclip = nullptr;
		output.clip = tmpclip;
		output.position = value.position;

		return output;
	}

}
