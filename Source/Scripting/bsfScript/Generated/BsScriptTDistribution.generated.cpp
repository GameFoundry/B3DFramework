//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTDistribution.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptRandom.generated.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "BsScriptTAnimationCurve.generated.h"

namespace bs
{
	ScriptFloatDistribution::ScriptFloatDistribution(MonoObject* managedInstance, const SPtr<TDistribution<float>>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptFloatDistribution::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_TDistribution", (void*)&ScriptFloatDistribution::InternalTDistribution);
		metaData.ScriptClass->AddInternalCall("Internal_TDistribution0", (void*)&ScriptFloatDistribution::InternalTDistribution0);
		metaData.ScriptClass->AddInternalCall("Internal_TDistribution1", (void*)&ScriptFloatDistribution::InternalTDistribution1);
		metaData.ScriptClass->AddInternalCall("Internal_TDistribution2", (void*)&ScriptFloatDistribution::InternalTDistribution2);
		metaData.ScriptClass->AddInternalCall("Internal_TDistribution3", (void*)&ScriptFloatDistribution::InternalTDistribution3);
		metaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptFloatDistribution::InternalGetType);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinConstant", (void*)&ScriptFloatDistribution::InternalGetMinConstant);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxConstant", (void*)&ScriptFloatDistribution::InternalGetMaxConstant);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinCurve", (void*)&ScriptFloatDistribution::InternalGetMinCurve);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxCurve", (void*)&ScriptFloatDistribution::InternalGetMaxCurve);
		metaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptFloatDistribution::InternalEvaluate);
		metaData.ScriptClass->AddInternalCall("Internal_Evaluate0", (void*)&ScriptFloatDistribution::InternalEvaluate0);

	}

	MonoObject* ScriptFloatDistribution::Create(const SPtr<TDistribution<float>>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptFloatDistribution>()) ScriptFloatDistribution(managedInstance, value);
		return managedInstance;
	}
	void ScriptFloatDistribution::InternalTDistribution(MonoObject* managedInstance)
	{
		SPtr<TDistribution<float>> instance = B3DMakeShared<TDistribution<float>>();
		new (B3DAllocate<ScriptFloatDistribution>())ScriptFloatDistribution(managedInstance, instance);
	}

	void ScriptFloatDistribution::InternalTDistribution0(MonoObject* managedInstance, float value)
	{
		SPtr<TDistribution<float>> instance = B3DMakeShared<TDistribution<float>>(value);
		new (B3DAllocate<ScriptFloatDistribution>())ScriptFloatDistribution(managedInstance, instance);
	}

	void ScriptFloatDistribution::InternalTDistribution1(MonoObject* managedInstance, float minValue, float maxValue)
	{
		SPtr<TDistribution<float>> instance = B3DMakeShared<TDistribution<float>>(minValue, maxValue);
		new (B3DAllocate<ScriptFloatDistribution>())ScriptFloatDistribution(managedInstance, instance);
	}

	void ScriptFloatDistribution::InternalTDistribution2(MonoObject* managedInstance, MonoObject* curve)
	{
		SPtr<TAnimationCurve<float>> tmpcurve;
		ScriptAnimationCurve* scriptcurve;
		scriptcurve = ScriptAnimationCurve::ToNative(curve);
		if(scriptcurve != nullptr)
			tmpcurve = scriptcurve->GetInternal();
		SPtr<TDistribution<float>> instance = B3DMakeShared<TDistribution<float>>(*tmpcurve);
		new (B3DAllocate<ScriptFloatDistribution>())ScriptFloatDistribution(managedInstance, instance);
	}

	void ScriptFloatDistribution::InternalTDistribution3(MonoObject* managedInstance, MonoObject* minCurve, MonoObject* maxCurve)
	{
		SPtr<TAnimationCurve<float>> tmpminCurve;
		ScriptAnimationCurve* scriptminCurve;
		scriptminCurve = ScriptAnimationCurve::ToNative(minCurve);
		if(scriptminCurve != nullptr)
			tmpminCurve = scriptminCurve->GetInternal();
		SPtr<TAnimationCurve<float>> tmpmaxCurve;
		ScriptAnimationCurve* scriptmaxCurve;
		scriptmaxCurve = ScriptAnimationCurve::ToNative(maxCurve);
		if(scriptmaxCurve != nullptr)
			tmpmaxCurve = scriptmaxCurve->GetInternal();
		SPtr<TDistribution<float>> instance = B3DMakeShared<TDistribution<float>>(*tmpminCurve, *tmpmaxCurve);
		new (B3DAllocate<ScriptFloatDistribution>())ScriptFloatDistribution(managedInstance, instance);
	}

	PropertyDistributionType ScriptFloatDistribution::InternalGetType(ScriptFloatDistribution* thisPtr)
	{
		PropertyDistributionType tmp__output;
		tmp__output = thisPtr->GetInternal()->GetType();

		PropertyDistributionType __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptFloatDistribution::InternalGetMinConstant(ScriptFloatDistribution* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->GetMinConstant();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptFloatDistribution::InternalGetMaxConstant(ScriptFloatDistribution* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->GetMaxConstant();

		float __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptFloatDistribution::InternalGetMinCurve(ScriptFloatDistribution* thisPtr)
	{
		SPtr<TAnimationCurve<float>> tmp__output = B3DMakeShared<TAnimationCurve<float>>();
		*tmp__output = thisPtr->GetInternal()->GetMinCurve();

		MonoObject* __output;
		__output = ScriptAnimationCurve::Create(tmp__output);

		return __output;
	}

	MonoObject* ScriptFloatDistribution::InternalGetMaxCurve(ScriptFloatDistribution* thisPtr)
	{
		SPtr<TAnimationCurve<float>> tmp__output = B3DMakeShared<TAnimationCurve<float>>();
		*tmp__output = thisPtr->GetInternal()->GetMaxCurve();

		MonoObject* __output;
		__output = ScriptAnimationCurve::Create(tmp__output);

		return __output;
	}

	float ScriptFloatDistribution::InternalEvaluate(ScriptFloatDistribution* thisPtr, float t, float factor)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Evaluate(t, factor);

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptFloatDistribution::InternalEvaluate0(ScriptFloatDistribution* thisPtr, float t, MonoObject* factor)
	{
		float tmp__output;
		SPtr<Random> tmpfactor;
		ScriptRandom* scriptfactor;
		scriptfactor = ScriptRandom::ToNative(factor);
		if(scriptfactor != nullptr)
			tmpfactor = scriptfactor->GetInternal();
		tmp__output = thisPtr->GetInternal()->Evaluate(t, *tmpfactor);

		float __output;
		__output = tmp__output;

		return __output;
	}

	ScriptVector3Distribution::ScriptVector3Distribution(MonoObject* managedInstance, const SPtr<TDistribution<Vector3>>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptVector3Distribution::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_TDistribution", (void*)&ScriptVector3Distribution::InternalTDistribution);
		metaData.ScriptClass->AddInternalCall("Internal_TDistribution0", (void*)&ScriptVector3Distribution::InternalTDistribution0);
		metaData.ScriptClass->AddInternalCall("Internal_TDistribution1", (void*)&ScriptVector3Distribution::InternalTDistribution1);
		metaData.ScriptClass->AddInternalCall("Internal_TDistribution2", (void*)&ScriptVector3Distribution::InternalTDistribution2);
		metaData.ScriptClass->AddInternalCall("Internal_TDistribution3", (void*)&ScriptVector3Distribution::InternalTDistribution3);
		metaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptVector3Distribution::InternalGetType);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinConstant", (void*)&ScriptVector3Distribution::InternalGetMinConstant);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxConstant", (void*)&ScriptVector3Distribution::InternalGetMaxConstant);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinCurve", (void*)&ScriptVector3Distribution::InternalGetMinCurve);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxCurve", (void*)&ScriptVector3Distribution::InternalGetMaxCurve);
		metaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptVector3Distribution::InternalEvaluate);
		metaData.ScriptClass->AddInternalCall("Internal_Evaluate0", (void*)&ScriptVector3Distribution::InternalEvaluate0);

	}

	MonoObject* ScriptVector3Distribution::Create(const SPtr<TDistribution<Vector3>>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptVector3Distribution>()) ScriptVector3Distribution(managedInstance, value);
		return managedInstance;
	}
	void ScriptVector3Distribution::InternalTDistribution(MonoObject* managedInstance)
	{
		SPtr<TDistribution<Vector3>> instance = B3DMakeShared<TDistribution<Vector3>>();
		new (B3DAllocate<ScriptVector3Distribution>())ScriptVector3Distribution(managedInstance, instance);
	}

	void ScriptVector3Distribution::InternalTDistribution0(MonoObject* managedInstance, Vector3* value)
	{
		SPtr<TDistribution<Vector3>> instance = B3DMakeShared<TDistribution<Vector3>>(*value);
		new (B3DAllocate<ScriptVector3Distribution>())ScriptVector3Distribution(managedInstance, instance);
	}

	void ScriptVector3Distribution::InternalTDistribution1(MonoObject* managedInstance, Vector3* minValue, Vector3* maxValue)
	{
		SPtr<TDistribution<Vector3>> instance = B3DMakeShared<TDistribution<Vector3>>(*minValue, *maxValue);
		new (B3DAllocate<ScriptVector3Distribution>())ScriptVector3Distribution(managedInstance, instance);
	}

	void ScriptVector3Distribution::InternalTDistribution2(MonoObject* managedInstance, MonoObject* curve)
	{
		SPtr<TAnimationCurve<Vector3>> tmpcurve;
		ScriptVector3Curve* scriptcurve;
		scriptcurve = ScriptVector3Curve::ToNative(curve);
		if(scriptcurve != nullptr)
			tmpcurve = scriptcurve->GetInternal();
		SPtr<TDistribution<Vector3>> instance = B3DMakeShared<TDistribution<Vector3>>(*tmpcurve);
		new (B3DAllocate<ScriptVector3Distribution>())ScriptVector3Distribution(managedInstance, instance);
	}

	void ScriptVector3Distribution::InternalTDistribution3(MonoObject* managedInstance, MonoObject* minCurve, MonoObject* maxCurve)
	{
		SPtr<TAnimationCurve<Vector3>> tmpminCurve;
		ScriptVector3Curve* scriptminCurve;
		scriptminCurve = ScriptVector3Curve::ToNative(minCurve);
		if(scriptminCurve != nullptr)
			tmpminCurve = scriptminCurve->GetInternal();
		SPtr<TAnimationCurve<Vector3>> tmpmaxCurve;
		ScriptVector3Curve* scriptmaxCurve;
		scriptmaxCurve = ScriptVector3Curve::ToNative(maxCurve);
		if(scriptmaxCurve != nullptr)
			tmpmaxCurve = scriptmaxCurve->GetInternal();
		SPtr<TDistribution<Vector3>> instance = B3DMakeShared<TDistribution<Vector3>>(*tmpminCurve, *tmpmaxCurve);
		new (B3DAllocate<ScriptVector3Distribution>())ScriptVector3Distribution(managedInstance, instance);
	}

	PropertyDistributionType ScriptVector3Distribution::InternalGetType(ScriptVector3Distribution* thisPtr)
	{
		PropertyDistributionType tmp__output;
		tmp__output = thisPtr->GetInternal()->GetType();

		PropertyDistributionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptVector3Distribution::InternalGetMinConstant(ScriptVector3Distribution* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->GetMinConstant();

		*__output = tmp__output;
	}

	void ScriptVector3Distribution::InternalGetMaxConstant(ScriptVector3Distribution* thisPtr, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->GetMaxConstant();

		*__output = tmp__output;
	}

	MonoObject* ScriptVector3Distribution::InternalGetMinCurve(ScriptVector3Distribution* thisPtr)
	{
		SPtr<TAnimationCurve<Vector3>> tmp__output = B3DMakeShared<TAnimationCurve<Vector3>>();
		*tmp__output = thisPtr->GetInternal()->GetMinCurve();

		MonoObject* __output;
		__output = ScriptVector3Curve::Create(tmp__output);

		return __output;
	}

	MonoObject* ScriptVector3Distribution::InternalGetMaxCurve(ScriptVector3Distribution* thisPtr)
	{
		SPtr<TAnimationCurve<Vector3>> tmp__output = B3DMakeShared<TAnimationCurve<Vector3>>();
		*tmp__output = thisPtr->GetInternal()->GetMaxCurve();

		MonoObject* __output;
		__output = ScriptVector3Curve::Create(tmp__output);

		return __output;
	}

	void ScriptVector3Distribution::InternalEvaluate(ScriptVector3Distribution* thisPtr, float t, float factor, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->Evaluate(t, factor);

		*__output = tmp__output;
	}

	void ScriptVector3Distribution::InternalEvaluate0(ScriptVector3Distribution* thisPtr, float t, MonoObject* factor, Vector3* __output)
	{
		SPtr<Random> tmpfactor;
		ScriptRandom* scriptfactor;
		scriptfactor = ScriptRandom::ToNative(factor);
		if(scriptfactor != nullptr)
			tmpfactor = scriptfactor->GetInternal();
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->Evaluate(t, *tmpfactor);

		*__output = tmp__output;
	}

	ScriptVector2Distribution::ScriptVector2Distribution(MonoObject* managedInstance, const SPtr<TDistribution<Vector2>>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptVector2Distribution::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_TDistribution", (void*)&ScriptVector2Distribution::InternalTDistribution);
		metaData.ScriptClass->AddInternalCall("Internal_TDistribution0", (void*)&ScriptVector2Distribution::InternalTDistribution0);
		metaData.ScriptClass->AddInternalCall("Internal_TDistribution1", (void*)&ScriptVector2Distribution::InternalTDistribution1);
		metaData.ScriptClass->AddInternalCall("Internal_TDistribution2", (void*)&ScriptVector2Distribution::InternalTDistribution2);
		metaData.ScriptClass->AddInternalCall("Internal_TDistribution3", (void*)&ScriptVector2Distribution::InternalTDistribution3);
		metaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptVector2Distribution::InternalGetType);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinConstant", (void*)&ScriptVector2Distribution::InternalGetMinConstant);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxConstant", (void*)&ScriptVector2Distribution::InternalGetMaxConstant);
		metaData.ScriptClass->AddInternalCall("Internal_GetMinCurve", (void*)&ScriptVector2Distribution::InternalGetMinCurve);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxCurve", (void*)&ScriptVector2Distribution::InternalGetMaxCurve);
		metaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptVector2Distribution::InternalEvaluate);
		metaData.ScriptClass->AddInternalCall("Internal_Evaluate0", (void*)&ScriptVector2Distribution::InternalEvaluate0);

	}

	MonoObject* ScriptVector2Distribution::Create(const SPtr<TDistribution<Vector2>>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptVector2Distribution>()) ScriptVector2Distribution(managedInstance, value);
		return managedInstance;
	}
	void ScriptVector2Distribution::InternalTDistribution(MonoObject* managedInstance)
	{
		SPtr<TDistribution<Vector2>> instance = B3DMakeShared<TDistribution<Vector2>>();
		new (B3DAllocate<ScriptVector2Distribution>())ScriptVector2Distribution(managedInstance, instance);
	}

	void ScriptVector2Distribution::InternalTDistribution0(MonoObject* managedInstance, Vector2* value)
	{
		SPtr<TDistribution<Vector2>> instance = B3DMakeShared<TDistribution<Vector2>>(*value);
		new (B3DAllocate<ScriptVector2Distribution>())ScriptVector2Distribution(managedInstance, instance);
	}

	void ScriptVector2Distribution::InternalTDistribution1(MonoObject* managedInstance, Vector2* minValue, Vector2* maxValue)
	{
		SPtr<TDistribution<Vector2>> instance = B3DMakeShared<TDistribution<Vector2>>(*minValue, *maxValue);
		new (B3DAllocate<ScriptVector2Distribution>())ScriptVector2Distribution(managedInstance, instance);
	}

	void ScriptVector2Distribution::InternalTDistribution2(MonoObject* managedInstance, MonoObject* curve)
	{
		SPtr<TAnimationCurve<Vector2>> tmpcurve;
		ScriptVector2Curve* scriptcurve;
		scriptcurve = ScriptVector2Curve::ToNative(curve);
		if(scriptcurve != nullptr)
			tmpcurve = scriptcurve->GetInternal();
		SPtr<TDistribution<Vector2>> instance = B3DMakeShared<TDistribution<Vector2>>(*tmpcurve);
		new (B3DAllocate<ScriptVector2Distribution>())ScriptVector2Distribution(managedInstance, instance);
	}

	void ScriptVector2Distribution::InternalTDistribution3(MonoObject* managedInstance, MonoObject* minCurve, MonoObject* maxCurve)
	{
		SPtr<TAnimationCurve<Vector2>> tmpminCurve;
		ScriptVector2Curve* scriptminCurve;
		scriptminCurve = ScriptVector2Curve::ToNative(minCurve);
		if(scriptminCurve != nullptr)
			tmpminCurve = scriptminCurve->GetInternal();
		SPtr<TAnimationCurve<Vector2>> tmpmaxCurve;
		ScriptVector2Curve* scriptmaxCurve;
		scriptmaxCurve = ScriptVector2Curve::ToNative(maxCurve);
		if(scriptmaxCurve != nullptr)
			tmpmaxCurve = scriptmaxCurve->GetInternal();
		SPtr<TDistribution<Vector2>> instance = B3DMakeShared<TDistribution<Vector2>>(*tmpminCurve, *tmpmaxCurve);
		new (B3DAllocate<ScriptVector2Distribution>())ScriptVector2Distribution(managedInstance, instance);
	}

	PropertyDistributionType ScriptVector2Distribution::InternalGetType(ScriptVector2Distribution* thisPtr)
	{
		PropertyDistributionType tmp__output;
		tmp__output = thisPtr->GetInternal()->GetType();

		PropertyDistributionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptVector2Distribution::InternalGetMinConstant(ScriptVector2Distribution* thisPtr, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetInternal()->GetMinConstant();

		*__output = tmp__output;
	}

	void ScriptVector2Distribution::InternalGetMaxConstant(ScriptVector2Distribution* thisPtr, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetInternal()->GetMaxConstant();

		*__output = tmp__output;
	}

	MonoObject* ScriptVector2Distribution::InternalGetMinCurve(ScriptVector2Distribution* thisPtr)
	{
		SPtr<TAnimationCurve<Vector2>> tmp__output = B3DMakeShared<TAnimationCurve<Vector2>>();
		*tmp__output = thisPtr->GetInternal()->GetMinCurve();

		MonoObject* __output;
		__output = ScriptVector2Curve::Create(tmp__output);

		return __output;
	}

	MonoObject* ScriptVector2Distribution::InternalGetMaxCurve(ScriptVector2Distribution* thisPtr)
	{
		SPtr<TAnimationCurve<Vector2>> tmp__output = B3DMakeShared<TAnimationCurve<Vector2>>();
		*tmp__output = thisPtr->GetInternal()->GetMaxCurve();

		MonoObject* __output;
		__output = ScriptVector2Curve::Create(tmp__output);

		return __output;
	}

	void ScriptVector2Distribution::InternalEvaluate(ScriptVector2Distribution* thisPtr, float t, float factor, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetInternal()->Evaluate(t, factor);

		*__output = tmp__output;
	}

	void ScriptVector2Distribution::InternalEvaluate0(ScriptVector2Distribution* thisPtr, float t, MonoObject* factor, Vector2* __output)
	{
		SPtr<Random> tmpfactor;
		ScriptRandom* scriptfactor;
		scriptfactor = ScriptRandom::ToNative(factor);
		if(scriptfactor != nullptr)
			tmpfactor = scriptfactor->GetInternal();
		Vector2 tmp__output;
		tmp__output = thisPtr->GetInternal()->Evaluate(t, *tmpfactor);

		*__output = tmp__output;
	}
}
