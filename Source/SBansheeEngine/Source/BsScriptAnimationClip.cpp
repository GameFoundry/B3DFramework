//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptAnimationClip.h"
#include "BsScriptResourceManager.h"
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
		
	}
	
	MonoObject* ScriptAnimationClip::createInstance()
	{
		return metaData.scriptClass->createInstance();
	}
}