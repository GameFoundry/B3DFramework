//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptHandleSliderPlane.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"

namespace bs
{
	ScriptHandleSliderPlane::ScriptHandleSliderPlane(MonoObject* instance, const Vector3& dir1, const Vector3& dir2, 
		float length, bool fixedScale, UINT64 layer)
		:ScriptObject(instance), mSlider(nullptr)
	{
		mSlider = bs_new<HandleSliderPlane>(dir1, dir2, length, fixedScale, layer);
	}

	ScriptHandleSliderPlane::~ScriptHandleSliderPlane()
	{
		if (mSlider != nullptr)
			bs_delete(mSlider);
	}

	void ScriptHandleSliderPlane::destroyInternal()
	{
		if (mSlider != nullptr)
			bs_delete(mSlider);

		mSlider = nullptr;
	}

	void ScriptHandleSliderPlane::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptHandleSliderPlane::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_GetDelta", &ScriptHandleSliderPlane::internal_GetDelta);
	}

	void ScriptHandleSliderPlane::internal_CreateInstance(MonoObject* instance, Vector3* dir1, Vector3* dir2, 
		float length, bool fixedScale, UINT64 layer)
	{
		new (bs_alloc<ScriptHandleSliderPlane>()) ScriptHandleSliderPlane(instance, *dir1, *dir2, length, fixedScale, layer);
	}

	void ScriptHandleSliderPlane::internal_GetDelta(ScriptHandleSliderPlane* nativeInstance, Vector2* value)
	{
		*value = nativeInstance->mSlider->getDelta();
	}
}