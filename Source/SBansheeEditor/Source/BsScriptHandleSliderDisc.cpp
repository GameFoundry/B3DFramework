//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptHandleSliderDisc.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"

namespace bs
{
	ScriptHandleSliderDisc::ScriptHandleSliderDisc(MonoObject* instance, const Vector3& normal, float radius, bool fixedScale, UINT64 layer)
		:ScriptObject(instance), mSlider(nullptr)
	{
		mSlider = bs_new<HandleSliderDisc>(normal, radius, fixedScale, layer);
	}

	ScriptHandleSliderDisc::~ScriptHandleSliderDisc()
	{
		if (mSlider != nullptr)
			bs_delete(mSlider);
	}

	void ScriptHandleSliderDisc::destroyInternal()
	{
		if (mSlider != nullptr)
			bs_delete(mSlider);

		mSlider = nullptr;
	}

	void ScriptHandleSliderDisc::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptHandleSliderDisc::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_GetDelta", &ScriptHandleSliderDisc::internal_GetDelta);
		metaData.scriptClass->addInternalCall("Internal_GetStartAngle", &ScriptHandleSliderDisc::internal_GetStartAngle);
		metaData.scriptClass->addInternalCall("Internal_SetCutoffPlane", &ScriptHandleSliderDisc::internal_SetCutoffPlane);
	}

	void ScriptHandleSliderDisc::internal_CreateInstance(MonoObject* instance, Vector3* normal, float radius, bool fixedScale, UINT64 layer)
	{
		new (bs_alloc<ScriptHandleSliderDisc>()) ScriptHandleSliderDisc(instance, *normal, radius, fixedScale, layer);
	}

	void ScriptHandleSliderDisc::internal_GetDelta(ScriptHandleSliderDisc* nativeInstance, float* value)
	{
		*value = nativeInstance->mSlider->getDelta().valueDegrees();
	}

	void ScriptHandleSliderDisc::internal_GetStartAngle(ScriptHandleSliderDisc* nativeInstance, float* value)
	{
		*value = nativeInstance->mSlider->getStartAngle().valueDegrees();
	}

	void ScriptHandleSliderDisc::internal_SetCutoffPlane(ScriptHandleSliderDisc* nativeInstance, float value, bool enabled)
	{
		nativeInstance->mSlider->setCutoffPlane(Degree(value), enabled);
	}
}