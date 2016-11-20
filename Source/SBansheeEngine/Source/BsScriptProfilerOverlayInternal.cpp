//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptProfilerOverlayInternal.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsApplication.h"
#include "BsCamera.h"
#include "BsScriptCamera.h"

namespace bs
{
	ScriptProfilerOverlayInternal::ScriptProfilerOverlayInternal(MonoObject* managedInstance, const SPtr<Camera>& camera)
		:ScriptObject(managedInstance), mProfilerOverlayInternal(nullptr)
	{
		if (camera != nullptr)
			mProfilerOverlayInternal = bs_new<ProfilerOverlayInternal>(camera);
	}

	ScriptProfilerOverlayInternal::~ScriptProfilerOverlayInternal()
	{
		if (mProfilerOverlayInternal != nullptr)
			bs_delete(mProfilerOverlayInternal);
	}

	void ScriptProfilerOverlayInternal::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptProfilerOverlayInternal::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_SetType", &ScriptProfilerOverlayInternal::internal_SetType);
		metaData.scriptClass->addInternalCall("Internal_Update", &ScriptProfilerOverlayInternal::internal_Update);
		metaData.scriptClass->addInternalCall("Internal_DestroyInstance", &ScriptProfilerOverlayInternal::internal_DestroyInstance);
	}

	void ScriptProfilerOverlayInternal::internal_CreateInstance(MonoObject* instance, ScriptCamera* camera)
	{
		SPtr<Camera> nativeCamera;
		if (camera != nullptr)
			nativeCamera = camera->getInternal();

		new (bs_alloc<ScriptProfilerOverlayInternal>()) ScriptProfilerOverlayInternal(instance, nativeCamera);
	}

	void ScriptProfilerOverlayInternal::internal_SetType(ScriptProfilerOverlayInternal* thisPtr, ProfilerOverlayType type)
	{
		if (thisPtr->mProfilerOverlayInternal != nullptr)
			thisPtr->mProfilerOverlayInternal->show(type);
	}

	void ScriptProfilerOverlayInternal::internal_Update(ScriptProfilerOverlayInternal* thisPtr)
	{
		if (thisPtr->mProfilerOverlayInternal != nullptr)
			thisPtr->mProfilerOverlayInternal->update();
	}

	void ScriptProfilerOverlayInternal::internal_DestroyInstance(ScriptProfilerOverlayInternal* thisPtr)
	{
		if (thisPtr->mProfilerOverlayInternal != nullptr)
		{
			bs_delete(thisPtr->mProfilerOverlayInternal);
			thisPtr->mProfilerOverlayInternal = nullptr;
		}
	}
}