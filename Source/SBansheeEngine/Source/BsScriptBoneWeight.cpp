//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptBoneWeight.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptBoneWeight::ScriptBoneWeight(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptBoneWeight::initRuntimeData()
	{ }

	MonoObject* ScriptBoneWeight::box(const BoneWeight& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	BoneWeight ScriptBoneWeight::unbox(MonoObject* obj)
	{
		return *(BoneWeight*)MonoUtil::unbox(obj);
	}
}