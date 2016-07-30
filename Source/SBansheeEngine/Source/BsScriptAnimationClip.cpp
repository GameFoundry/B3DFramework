//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptAnimationClip.h"
#include "BsScriptAnimationCurves.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"

namespace BansheeEngine
{
	ScriptAnimationClip::ScriptAnimationClip(MonoObject* instance, const HAnimationClip& animationClip)
		:TScriptResource(instance, animationClip)
	{

	}

	void ScriptAnimationClip::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetAnimationCurves", &ScriptAnimationClip::internal_GetAnimationCurves);
		metaData.scriptClass->addInternalCall("Internal_SetAnimationCurves", &ScriptAnimationClip::internal_SetAnimationCurves);
	}
	
	MonoObject* ScriptAnimationClip::createInstance()
	{
		return metaData.scriptClass->createInstance();
	}

	MonoObject* ScriptAnimationClip::internal_GetAnimationCurves(ScriptAnimationClip* thisPtr)
	{
		SPtr<AnimationCurves> curves = thisPtr->getHandle()->getCurves();
		return ScriptAnimationCurves::toManaged(curves);
	}

	void ScriptAnimationClip::internal_SetAnimationCurves(ScriptAnimationClip* thisPtr, MonoObject* curves)
	{
		SPtr<AnimationCurves> nativeCurves = ScriptAnimationCurves::toNative(curves);
		thisPtr->getHandle()->setCurves(*nativeCurves);
	}
}