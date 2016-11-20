//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptHandleSliderLine.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"

namespace bs
{
	ScriptHandleSliderLine::ScriptHandleSliderLine(MonoObject* instance, const Vector3& direction, float length, 
		bool fixedScale, UINT64 layer)
		:ScriptObject(instance), mSlider(nullptr)
	{
		mSlider = bs_new<HandleSliderLine>(direction, length, fixedScale, layer);
	}

	ScriptHandleSliderLine::~ScriptHandleSliderLine()
	{
		if (mSlider != nullptr)
			bs_delete(mSlider);
	}

	void ScriptHandleSliderLine::destroyInternal()
	{
		if (mSlider != nullptr)
			bs_delete(mSlider);

		mSlider = nullptr;
	}

	void ScriptHandleSliderLine::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptHandleSliderLine::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_GetDelta", &ScriptHandleSliderLine::internal_GetDelta);
	}

	void ScriptHandleSliderLine::internal_CreateInstance(MonoObject* instance, Vector3* direction, float length, 
		bool fixedScale, UINT64 layer)
	{
		new (bs_alloc<ScriptHandleSliderLine>()) ScriptHandleSliderLine(instance, *direction, length, fixedScale, layer);
	}

	void ScriptHandleSliderLine::internal_GetDelta(ScriptHandleSliderLine* nativeInstance, float* value)
	{
		*value = nativeInstance->mSlider->getDelta();
	}
}