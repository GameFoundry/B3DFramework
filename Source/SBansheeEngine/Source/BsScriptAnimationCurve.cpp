//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptAnimationCurve.h"
#include "BsMonoUtil.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsAnimationCurve.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	ScriptKeyFrame::ScriptKeyFrame(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptKeyFrame::initRuntimeData()
	{
		// Do nothing
	}

	ScriptAnimationCurve::ScriptAnimationCurve(MonoObject* instance, const SPtr<TAnimationCurve<float>>& curve)
		:ScriptObject(instance), mCurve(curve)
	{ }

	void ScriptAnimationCurve::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Create", &ScriptAnimationCurve::internal_Create);
		metaData.scriptClass->addInternalCall("Internal_GetKeyFrames", &ScriptAnimationCurve::internal_GetKeyFrames);
		metaData.scriptClass->addInternalCall("Internal_SetKeyFrames", &ScriptAnimationCurve::internal_SetKeyFrames);
		metaData.scriptClass->addInternalCall("Internal_Evaluate", &ScriptAnimationCurve::internal_Evaluate);
	}

	MonoObject* ScriptAnimationCurve::create(const TAnimationCurve<float>& curve)
	{
		SPtr<TAnimationCurve<float>> curvePtr = bs_shared_ptr_new<TAnimationCurve<float>>();
		*curvePtr = curve;

		MonoObject* instance = metaData.scriptClass->createInstance();
		new (bs_alloc<ScriptAnimationCurve>()) ScriptAnimationCurve(instance, curvePtr);

		return instance;
	}

	void ScriptAnimationCurve::internal_Create(MonoObject* instance, MonoArray* keyFrames)
	{
		ScriptArray inArray(keyFrames);
		
		UINT32 numKeyframes = inArray.size();
		Vector<TKeyframe<float>> nativeKeyframes(numKeyframes);

		memcpy(nativeKeyframes.data(), (UINT8*)inArray.getRawPtr<TKeyframe<float>>(), 
			numKeyframes * sizeof(TKeyframe<float>));

		SPtr<TAnimationCurve<float>> curve = bs_shared_ptr_new<TAnimationCurve<float>>(nativeKeyframes);
		new (bs_alloc<ScriptAnimationCurve>()) ScriptAnimationCurve(instance, curve);
	}

	MonoArray* ScriptAnimationCurve::internal_GetKeyFrames(ScriptAnimationCurve* thisPtr)
	{
		UINT32 numKeyframes = thisPtr->mCurve->getNumKeyFrames();

		ScriptArray output = ScriptArray::create<ScriptKeyFrame>(numKeyframes);
		for (UINT32 i = 0; i < numKeyframes; i++)
			output.set(i, thisPtr->mCurve->getKeyFrame(i));

		return output.getInternal();
	}

	void ScriptAnimationCurve::internal_SetKeyFrames(ScriptAnimationCurve* thisPtr, MonoArray* keyFrames)
	{
		ScriptArray inArray(keyFrames);

		UINT32 numKeyframes = inArray.size();
		Vector<TKeyframe<float>> nativeKeyframes(numKeyframes);

		memcpy(nativeKeyframes.data(), (UINT8*)inArray.getRawPtr<TKeyframe<float>>(),
			numKeyframes * sizeof(TKeyframe<float>));

		std::sort(nativeKeyframes.begin(), nativeKeyframes.end(), 
			[](const TKeyframe<float>& x, const TKeyframe<float>& y)
		{
			return x.time < y.time;
		});

		thisPtr->mCurve = bs_shared_ptr_new<TAnimationCurve<float>>(nativeKeyframes);
	}

	float ScriptAnimationCurve::internal_Evaluate(ScriptAnimationCurve* thisPtr, float time, bool loop)
	{
		return thisPtr->mCurve->evaluate(time, loop);
	}
}