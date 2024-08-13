//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTDistribution.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptRandom.generated.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "Wrappers/BsScriptVector.h"
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
		SPtr<TDistribution<float>> nativeObject = B3DMakeShared<TDistribution<float>>();
		new (B3DAllocate<ScriptFloatDistribution>())ScriptFloatDistribution(managedInstance, nativeObject);
	}

	void ScriptFloatDistribution::InternalTDistribution0(MonoObject* managedInstance, float value)
	{
		SPtr<TDistribution<float>> nativeObject = B3DMakeShared<TDistribution<float>>(value);
		new (B3DAllocate<ScriptFloatDistribution>())ScriptFloatDistribution(managedInstance, nativeObject);
	}

	void ScriptFloatDistribution::InternalTDistribution1(MonoObject* managedInstance, float minValue, float maxValue)
	{
		SPtr<TDistribution<float>> nativeObject = B3DMakeShared<TDistribution<float>>(minValue, maxValue);
		new (B3DAllocate<ScriptFloatDistribution>())ScriptFloatDistribution(managedInstance, nativeObject);
	}

	void ScriptFloatDistribution::InternalTDistribution2(MonoObject* managedInstance, MonoObject* curve)
	{
		SPtr<TAnimationCurve<float>> tmpcurve;
		ScriptAnimationCurve* scriptObjectWrappercurve;
		scriptObjectWrappercurve = ScriptAnimationCurve::ToNative(curve);
		if(scriptObjectWrappercurve != nullptr)
			tmpcurve = scriptObjectWrappercurve->GetInternal();
		SPtr<TDistribution<float>> nativeObject = B3DMakeShared<TDistribution<float>>(*tmpcurve);
		new (B3DAllocate<ScriptFloatDistribution>())ScriptFloatDistribution(managedInstance, nativeObject);
	}

	void ScriptFloatDistribution::InternalTDistribution3(MonoObject* managedInstance, MonoObject* minCurve, MonoObject* maxCurve)
	{
		SPtr<TAnimationCurve<float>> tmpminCurve;
		ScriptAnimationCurve* scriptObjectWrapperminCurve;
		scriptObjectWrapperminCurve = ScriptAnimationCurve::ToNative(minCurve);
		if(scriptObjectWrapperminCurve != nullptr)
			tmpminCurve = scriptObjectWrapperminCurve->GetInternal();
		SPtr<TAnimationCurve<float>> tmpmaxCurve;
		ScriptAnimationCurve* scriptObjectWrappermaxCurve;
		scriptObjectWrappermaxCurve = ScriptAnimationCurve::ToNative(maxCurve);
		if(scriptObjectWrappermaxCurve != nullptr)
			tmpmaxCurve = scriptObjectWrappermaxCurve->GetInternal();
		SPtr<TDistribution<float>> nativeObject = B3DMakeShared<TDistribution<float>>(*tmpminCurve, *tmpmaxCurve);
		new (B3DAllocate<ScriptFloatDistribution>())ScriptFloatDistribution(managedInstance, nativeObject);
	}

	PropertyDistributionType ScriptFloatDistribution::InternalGetType(ScriptFloatDistribution* self)
	{
		PropertyDistributionType tmp__output;
		tmp__output = self->GetInternal()->GetType();

		PropertyDistributionType __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptFloatDistribution::InternalGetMinConstant(ScriptFloatDistribution* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->GetMinConstant();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptFloatDistribution::InternalGetMaxConstant(ScriptFloatDistribution* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->GetMaxConstant();

		float __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptFloatDistribution::InternalGetMinCurve(ScriptFloatDistribution* self)
	{
		SPtr<TAnimationCurve<float>> tmp__output = B3DMakeShared<TAnimationCurve<float>>();
		*tmp__output = self->GetInternal()->GetMinCurve();

		MonoObject* __output;
		__output = ScriptAnimationCurve::Create(tmp__output);

		return __output;
	}

	MonoObject* ScriptFloatDistribution::InternalGetMaxCurve(ScriptFloatDistribution* self)
	{
		SPtr<TAnimationCurve<float>> tmp__output = B3DMakeShared<TAnimationCurve<float>>();
		*tmp__output = self->GetInternal()->GetMaxCurve();

		MonoObject* __output;
		__output = ScriptAnimationCurve::Create(tmp__output);

		return __output;
	}

	float ScriptFloatDistribution::InternalEvaluate(ScriptFloatDistribution* self, float t, float factor)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Evaluate(t, factor);

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptFloatDistribution::InternalEvaluate0(ScriptFloatDistribution* self, float t, MonoObject* factor)
	{
		float tmp__output;
		SPtr<Random> tmpfactor;
		ScriptRandom* scriptObjectWrapperfactor;
		scriptObjectWrapperfactor = ScriptRandom::ToNative(factor);
		if(scriptObjectWrapperfactor != nullptr)
			tmpfactor = scriptObjectWrapperfactor->GetInternal();
		tmp__output = self->GetInternal()->Evaluate(t, *tmpfactor);

		float __output;
		__output = tmp__output;

		return __output;
	}

	ScriptVector3Distribution::ScriptVector3Distribution(MonoObject* managedInstance, const SPtr<TDistribution<TVector3<float>>>& value)
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

	MonoObject* ScriptVector3Distribution::Create(const SPtr<TDistribution<TVector3<float>>>& value)
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
		SPtr<TDistribution<TVector3<float>>> nativeObject = B3DMakeShared<TDistribution<TVector3<float>>>();
		new (B3DAllocate<ScriptVector3Distribution>())ScriptVector3Distribution(managedInstance, nativeObject);
	}

	void ScriptVector3Distribution::InternalTDistribution0(MonoObject* managedInstance, TVector3<float>* value)
	{
		SPtr<TDistribution<TVector3<float>>> nativeObject = B3DMakeShared<TDistribution<TVector3<float>>>(*value);
		new (B3DAllocate<ScriptVector3Distribution>())ScriptVector3Distribution(managedInstance, nativeObject);
	}

	void ScriptVector3Distribution::InternalTDistribution1(MonoObject* managedInstance, TVector3<float>* minValue, TVector3<float>* maxValue)
	{
		SPtr<TDistribution<TVector3<float>>> nativeObject = B3DMakeShared<TDistribution<TVector3<float>>>(*minValue, *maxValue);
		new (B3DAllocate<ScriptVector3Distribution>())ScriptVector3Distribution(managedInstance, nativeObject);
	}

	void ScriptVector3Distribution::InternalTDistribution2(MonoObject* managedInstance, MonoObject* curve)
	{
		SPtr<TAnimationCurve<TVector3<float>>> tmpcurve;
		ScriptVector3Curve* scriptObjectWrappercurve;
		scriptObjectWrappercurve = ScriptVector3Curve::ToNative(curve);
		if(scriptObjectWrappercurve != nullptr)
			tmpcurve = scriptObjectWrappercurve->GetInternal();
		SPtr<TDistribution<TVector3<float>>> nativeObject = B3DMakeShared<TDistribution<TVector3<float>>>(*tmpcurve);
		new (B3DAllocate<ScriptVector3Distribution>())ScriptVector3Distribution(managedInstance, nativeObject);
	}

	void ScriptVector3Distribution::InternalTDistribution3(MonoObject* managedInstance, MonoObject* minCurve, MonoObject* maxCurve)
	{
		SPtr<TAnimationCurve<TVector3<float>>> tmpminCurve;
		ScriptVector3Curve* scriptObjectWrapperminCurve;
		scriptObjectWrapperminCurve = ScriptVector3Curve::ToNative(minCurve);
		if(scriptObjectWrapperminCurve != nullptr)
			tmpminCurve = scriptObjectWrapperminCurve->GetInternal();
		SPtr<TAnimationCurve<TVector3<float>>> tmpmaxCurve;
		ScriptVector3Curve* scriptObjectWrappermaxCurve;
		scriptObjectWrappermaxCurve = ScriptVector3Curve::ToNative(maxCurve);
		if(scriptObjectWrappermaxCurve != nullptr)
			tmpmaxCurve = scriptObjectWrappermaxCurve->GetInternal();
		SPtr<TDistribution<TVector3<float>>> nativeObject = B3DMakeShared<TDistribution<TVector3<float>>>(*tmpminCurve, *tmpmaxCurve);
		new (B3DAllocate<ScriptVector3Distribution>())ScriptVector3Distribution(managedInstance, nativeObject);
	}

	PropertyDistributionType ScriptVector3Distribution::InternalGetType(ScriptVector3Distribution* self)
	{
		PropertyDistributionType tmp__output;
		tmp__output = self->GetInternal()->GetType();

		PropertyDistributionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptVector3Distribution::InternalGetMinConstant(ScriptVector3Distribution* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetInternal()->GetMinConstant();

		*__output = tmp__output;
	}

	void ScriptVector3Distribution::InternalGetMaxConstant(ScriptVector3Distribution* self, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetInternal()->GetMaxConstant();

		*__output = tmp__output;
	}

	MonoObject* ScriptVector3Distribution::InternalGetMinCurve(ScriptVector3Distribution* self)
	{
		SPtr<TAnimationCurve<TVector3<float>>> tmp__output = B3DMakeShared<TAnimationCurve<TVector3<float>>>();
		*tmp__output = self->GetInternal()->GetMinCurve();

		MonoObject* __output;
		__output = ScriptVector3Curve::Create(tmp__output);

		return __output;
	}

	MonoObject* ScriptVector3Distribution::InternalGetMaxCurve(ScriptVector3Distribution* self)
	{
		SPtr<TAnimationCurve<TVector3<float>>> tmp__output = B3DMakeShared<TAnimationCurve<TVector3<float>>>();
		*tmp__output = self->GetInternal()->GetMaxCurve();

		MonoObject* __output;
		__output = ScriptVector3Curve::Create(tmp__output);

		return __output;
	}

	void ScriptVector3Distribution::InternalEvaluate(ScriptVector3Distribution* self, float t, float factor, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetInternal()->Evaluate(t, factor);

		*__output = tmp__output;
	}

	void ScriptVector3Distribution::InternalEvaluate0(ScriptVector3Distribution* self, float t, MonoObject* factor, TVector3<float>* __output)
	{
		SPtr<Random> tmpfactor;
		ScriptRandom* scriptObjectWrapperfactor;
		scriptObjectWrapperfactor = ScriptRandom::ToNative(factor);
		if(scriptObjectWrapperfactor != nullptr)
			tmpfactor = scriptObjectWrapperfactor->GetInternal();
		TVector3<float> tmp__output;
		tmp__output = self->GetInternal()->Evaluate(t, *tmpfactor);

		*__output = tmp__output;
	}

	ScriptVector2Distribution::ScriptVector2Distribution(MonoObject* managedInstance, const SPtr<TDistribution<TVector2<float>>>& value)
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

	MonoObject* ScriptVector2Distribution::Create(const SPtr<TDistribution<TVector2<float>>>& value)
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
		SPtr<TDistribution<TVector2<float>>> nativeObject = B3DMakeShared<TDistribution<TVector2<float>>>();
		new (B3DAllocate<ScriptVector2Distribution>())ScriptVector2Distribution(managedInstance, nativeObject);
	}

	void ScriptVector2Distribution::InternalTDistribution0(MonoObject* managedInstance, TVector2<float>* value)
	{
		SPtr<TDistribution<TVector2<float>>> nativeObject = B3DMakeShared<TDistribution<TVector2<float>>>(*value);
		new (B3DAllocate<ScriptVector2Distribution>())ScriptVector2Distribution(managedInstance, nativeObject);
	}

	void ScriptVector2Distribution::InternalTDistribution1(MonoObject* managedInstance, TVector2<float>* minValue, TVector2<float>* maxValue)
	{
		SPtr<TDistribution<TVector2<float>>> nativeObject = B3DMakeShared<TDistribution<TVector2<float>>>(*minValue, *maxValue);
		new (B3DAllocate<ScriptVector2Distribution>())ScriptVector2Distribution(managedInstance, nativeObject);
	}

	void ScriptVector2Distribution::InternalTDistribution2(MonoObject* managedInstance, MonoObject* curve)
	{
		SPtr<TAnimationCurve<TVector2<float>>> tmpcurve;
		ScriptVector2Curve* scriptObjectWrappercurve;
		scriptObjectWrappercurve = ScriptVector2Curve::ToNative(curve);
		if(scriptObjectWrappercurve != nullptr)
			tmpcurve = scriptObjectWrappercurve->GetInternal();
		SPtr<TDistribution<TVector2<float>>> nativeObject = B3DMakeShared<TDistribution<TVector2<float>>>(*tmpcurve);
		new (B3DAllocate<ScriptVector2Distribution>())ScriptVector2Distribution(managedInstance, nativeObject);
	}

	void ScriptVector2Distribution::InternalTDistribution3(MonoObject* managedInstance, MonoObject* minCurve, MonoObject* maxCurve)
	{
		SPtr<TAnimationCurve<TVector2<float>>> tmpminCurve;
		ScriptVector2Curve* scriptObjectWrapperminCurve;
		scriptObjectWrapperminCurve = ScriptVector2Curve::ToNative(minCurve);
		if(scriptObjectWrapperminCurve != nullptr)
			tmpminCurve = scriptObjectWrapperminCurve->GetInternal();
		SPtr<TAnimationCurve<TVector2<float>>> tmpmaxCurve;
		ScriptVector2Curve* scriptObjectWrappermaxCurve;
		scriptObjectWrappermaxCurve = ScriptVector2Curve::ToNative(maxCurve);
		if(scriptObjectWrappermaxCurve != nullptr)
			tmpmaxCurve = scriptObjectWrappermaxCurve->GetInternal();
		SPtr<TDistribution<TVector2<float>>> nativeObject = B3DMakeShared<TDistribution<TVector2<float>>>(*tmpminCurve, *tmpmaxCurve);
		new (B3DAllocate<ScriptVector2Distribution>())ScriptVector2Distribution(managedInstance, nativeObject);
	}

	PropertyDistributionType ScriptVector2Distribution::InternalGetType(ScriptVector2Distribution* self)
	{
		PropertyDistributionType tmp__output;
		tmp__output = self->GetInternal()->GetType();

		PropertyDistributionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptVector2Distribution::InternalGetMinConstant(ScriptVector2Distribution* self, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		tmp__output = self->GetInternal()->GetMinConstant();

		*__output = tmp__output;
	}

	void ScriptVector2Distribution::InternalGetMaxConstant(ScriptVector2Distribution* self, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		tmp__output = self->GetInternal()->GetMaxConstant();

		*__output = tmp__output;
	}

	MonoObject* ScriptVector2Distribution::InternalGetMinCurve(ScriptVector2Distribution* self)
	{
		SPtr<TAnimationCurve<TVector2<float>>> tmp__output = B3DMakeShared<TAnimationCurve<TVector2<float>>>();
		*tmp__output = self->GetInternal()->GetMinCurve();

		MonoObject* __output;
		__output = ScriptVector2Curve::Create(tmp__output);

		return __output;
	}

	MonoObject* ScriptVector2Distribution::InternalGetMaxCurve(ScriptVector2Distribution* self)
	{
		SPtr<TAnimationCurve<TVector2<float>>> tmp__output = B3DMakeShared<TAnimationCurve<TVector2<float>>>();
		*tmp__output = self->GetInternal()->GetMaxCurve();

		MonoObject* __output;
		__output = ScriptVector2Curve::Create(tmp__output);

		return __output;
	}

	void ScriptVector2Distribution::InternalEvaluate(ScriptVector2Distribution* self, float t, float factor, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		tmp__output = self->GetInternal()->Evaluate(t, factor);

		*__output = tmp__output;
	}

	void ScriptVector2Distribution::InternalEvaluate0(ScriptVector2Distribution* self, float t, MonoObject* factor, TVector2<float>* __output)
	{
		SPtr<Random> tmpfactor;
		ScriptRandom* scriptObjectWrapperfactor;
		scriptObjectWrapperfactor = ScriptRandom::ToNative(factor);
		if(scriptObjectWrapperfactor != nullptr)
			tmpfactor = scriptObjectWrapperfactor->GetInternal();
		TVector2<float> tmp__output;
		tmp__output = self->GetInternal()->Evaluate(t, *tmpfactor);

		*__output = tmp__output;
	}
}
