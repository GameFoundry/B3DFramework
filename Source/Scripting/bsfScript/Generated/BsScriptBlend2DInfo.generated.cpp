//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptBlend2DInfo.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationClip.h"
#include "BsScriptAnimationClip.generated.h"

namespace b3d
{
	ScriptBlend2DInfo::ScriptBlend2DInfo()
	{ }

	MonoObject* ScriptBlend2DInfo::Box(const __Blend2DInfoInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__Blend2DInfoInterop ScriptBlend2DInfo::Unbox(MonoObject* value)
	{
		return *(__Blend2DInfoInterop*)MonoUtil::Unbox(value);
	}

	Blend2DInfo ScriptBlend2DInfo::FromInterop(const __Blend2DInfoInterop& value)
	{
		Blend2DInfo output;
		TResourceHandle<AnimationClip> tmpTopLeftClip;
		ScriptRRefBase* scriptObjectWrapperTopLeftClip;
		scriptObjectWrapperTopLeftClip = ScriptRRefBase::GetScriptObjectWrapper(value.TopLeftClip);
		if(scriptObjectWrapperTopLeftClip != nullptr)
			tmpTopLeftClip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperTopLeftClip->GetNativeObject());
		output.TopLeftClip = tmpTopLeftClip;
		TResourceHandle<AnimationClip> tmpTopRightClip;
		ScriptRRefBase* scriptObjectWrapperTopRightClip;
		scriptObjectWrapperTopRightClip = ScriptRRefBase::GetScriptObjectWrapper(value.TopRightClip);
		if(scriptObjectWrapperTopRightClip != nullptr)
			tmpTopRightClip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperTopRightClip->GetNativeObject());
		output.TopRightClip = tmpTopRightClip;
		TResourceHandle<AnimationClip> tmpBottomLeftClip;
		ScriptRRefBase* scriptObjectWrapperBottomLeftClip;
		scriptObjectWrapperBottomLeftClip = ScriptRRefBase::GetScriptObjectWrapper(value.BottomLeftClip);
		if(scriptObjectWrapperBottomLeftClip != nullptr)
			tmpBottomLeftClip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperBottomLeftClip->GetNativeObject());
		output.BottomLeftClip = tmpBottomLeftClip;
		TResourceHandle<AnimationClip> tmpBottomRightClip;
		ScriptRRefBase* scriptObjectWrapperBottomRightClip;
		scriptObjectWrapperBottomRightClip = ScriptRRefBase::GetScriptObjectWrapper(value.BottomRightClip);
		if(scriptObjectWrapperBottomRightClip != nullptr)
			tmpBottomRightClip = B3DStaticResourceCast<AnimationClip>(scriptObjectWrapperBottomRightClip->GetNativeObject());
		output.BottomRightClip = tmpBottomRightClip;

		return output;
	}

	__Blend2DInfoInterop ScriptBlend2DInfo::ToInterop(const Blend2DInfo& value)
	{
		__Blend2DInfoInterop output;
		MonoObject* tmpTopLeftClip;
		ScriptRRefBase* scriptWrapperObjectTopLeftClip;
		scriptWrapperObjectTopLeftClip = ScriptResourceManager::Instance().GetScriptRRef(value.TopLeftClip);
		if(scriptWrapperObjectTopLeftClip != nullptr)
			tmpTopLeftClip = scriptWrapperObjectTopLeftClip->GetScriptObject();
		else
			tmpTopLeftClip = nullptr;
		output.TopLeftClip = tmpTopLeftClip;
		MonoObject* tmpTopRightClip;
		ScriptRRefBase* scriptWrapperObjectTopRightClip;
		scriptWrapperObjectTopRightClip = ScriptResourceManager::Instance().GetScriptRRef(value.TopRightClip);
		if(scriptWrapperObjectTopRightClip != nullptr)
			tmpTopRightClip = scriptWrapperObjectTopRightClip->GetScriptObject();
		else
			tmpTopRightClip = nullptr;
		output.TopRightClip = tmpTopRightClip;
		MonoObject* tmpBottomLeftClip;
		ScriptRRefBase* scriptWrapperObjectBottomLeftClip;
		scriptWrapperObjectBottomLeftClip = ScriptResourceManager::Instance().GetScriptRRef(value.BottomLeftClip);
		if(scriptWrapperObjectBottomLeftClip != nullptr)
			tmpBottomLeftClip = scriptWrapperObjectBottomLeftClip->GetScriptObject();
		else
			tmpBottomLeftClip = nullptr;
		output.BottomLeftClip = tmpBottomLeftClip;
		MonoObject* tmpBottomRightClip;
		ScriptRRefBase* scriptWrapperObjectBottomRightClip;
		scriptWrapperObjectBottomRightClip = ScriptResourceManager::Instance().GetScriptRRef(value.BottomRightClip);
		if(scriptWrapperObjectBottomRightClip != nullptr)
			tmpBottomRightClip = scriptWrapperObjectBottomRightClip->GetScriptObject();
		else
			tmpBottomRightClip = nullptr;
		output.BottomRightClip = tmpBottomRightClip;

		return output;
	}

}
