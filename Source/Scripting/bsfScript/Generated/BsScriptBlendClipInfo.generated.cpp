//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptBlendClipInfo.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationClip.h"
#include "BsScriptAnimationClip.generated.h"

namespace bs
{
	ScriptBlendClipInfo::ScriptBlendClipInfo(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptBlendClipInfo::InitRuntimeData()
	{ }

	MonoObject*ScriptBlendClipInfo::Box(const __BlendClipInfoInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__BlendClipInfoInterop ScriptBlendClipInfo::Unbox(MonoObject* value)
	{
		return *(__BlendClipInfoInterop*)MonoUtil::Unbox(value);
	}

	BlendClipInfo ScriptBlendClipInfo::FromInterop(const __BlendClipInfoInterop& value)
	{
		BlendClipInfo output;
		ResourceHandle<AnimationClip> tmpClip;
		ScriptRRefBase* scriptClip;
		scriptClip = ScriptRRefBase::ToNative(value.Clip);
		if(scriptClip != nullptr)
			tmpClip = static_resource_cast<AnimationClip>(scriptClip->GetHandle());
		output.Clip = tmpClip;
		output.Position = value.Position;

		return output;
	}

	__BlendClipInfoInterop ScriptBlendClipInfo::ToInterop(const BlendClipInfo& value)
	{
		__BlendClipInfoInterop output;
		MonoObject* tmpClip;
		ScriptRRefBase* scriptClip;
		scriptClip = ScriptResourceManager::Instance().GetScriptRRef(value.Clip);
		if(scriptClip != nullptr)
			tmpClip = scriptClip->GetManagedInstance();
		else
			tmpClip = nullptr;
		output.Clip = tmpClip;
		output.Position = value.Position;

		return output;
	}

}
