//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2018 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/BsScriptHandleSlider2D.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"

namespace bs
{
	ScriptHandleSlider2D::ScriptHandleSlider2D(MonoObject* instance, UINT32 width, UINT32 height, 
		Slider2DConstraint constraint, UINT64 layer)
		:ScriptObject(instance), mSlider(nullptr)
	{
		mSlider = bs_new<HandleSlider2D>(width, height, layer, constraint);
	}

	ScriptHandleSlider2D::~ScriptHandleSlider2D()
	{
		if (mSlider != nullptr)
			bs_delete(mSlider);
	}

	void ScriptHandleSlider2D::destroyInternal()
	{
		if (mSlider != nullptr)
			bs_delete(mSlider);

		mSlider = nullptr;
	}

	void ScriptHandleSlider2D::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptHandleSlider2D::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_GetDelta", (void*)&ScriptHandleSlider2D::internal_GetDelta);
	}

	void ScriptHandleSlider2D::internal_CreateInstance(MonoObject* instance, UINT32 width, UINT32 height, 
		Slider2DConstraint constraint, UINT64 layer)
	{
		new (bs_alloc<ScriptHandleSlider2D>()) ScriptHandleSlider2D(instance, width, height, constraint, layer);
	}

	void ScriptHandleSlider2D::internal_GetDelta(ScriptHandleSlider2D* nativeInstance, Vector2I* value)
	{
		*value = nativeInstance->mSlider->getDelta();
	}
}
