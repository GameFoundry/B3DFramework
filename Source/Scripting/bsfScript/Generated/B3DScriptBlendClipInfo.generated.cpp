//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptBlendClipInfo.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationClip.h"
#include "BsScriptAnimationClip.generated.h"

namespace b3d
{
	ScriptBlendClipInfo::ScriptBlendClipInfo()
	{ }

	MonoObject* ScriptBlendClipInfo::Box(const __BlendClipInfoInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__BlendClipInfoInterop ScriptBlendClipInfo::Unbox(MonoObject* value)
	{
		return *(__BlendClipInfoInterop*)MonoUtil::Unbox(value);
	}

	BlendClipInfo ScriptBlendClipInfo::FromInterop(const __BlendClipInfoInterop& value)
	{
		BlendClipInfo output;
		TResourceHandle<AnimationClip> tmpClip;
		ScriptRRefBase* scriptObjectWrapperClip;
		scriptObjectWrapperClip = ScriptRRefBase::GetScriptObjectWrapper(value.Clip);
		if(scriptObjectWrapperClip != nullptr)
			tmpClip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperClip->GetNativeObject());
		output.Clip = tmpClip;
		output.Position = value.Position;

		return output;
	}

	__BlendClipInfoInterop ScriptBlendClipInfo::ToInterop(const BlendClipInfo& value)
	{
		__BlendClipInfoInterop output;
		MonoObject* tmpClip;
		ScriptRRefBase* scriptWrapperObjectClip;
		scriptWrapperObjectClip = ScriptResourceManager::Instance().GetScriptRRef(value.Clip);
		if(scriptWrapperObjectClip != nullptr)
			tmpClip = scriptWrapperObjectClip->GetScriptObject();
		else
			tmpClip = nullptr;
		output.Clip = tmpClip;
		output.Position = value.Position;

		return output;
	}

}
