//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptTDistribution.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptRandom.generated.h"
#include "B3DScriptTAnimationCurve.generated.h"
#include "B3DScriptTVector3.generated.h"
#include "B3DScriptTAnimationCurve.generated.h"
#include "B3DScriptTVector2.generated.h"
#include "B3DScriptTAnimationCurve.generated.h"

namespace b3d
{
	ScriptFloatDistribution::ScriptFloatDistribution(const TShared<TDistribution<float>>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptFloatDistribution::~ScriptFloatDistribution()
	{
		UnregisterEvents();
	}

	void ScriptFloatDistribution::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TDistribution", (void*)&ScriptFloatDistribution::InternalTDistribution);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TDistribution0", (void*)&ScriptFloatDistribution::InternalTDistribution0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TDistribution1", (void*)&ScriptFloatDistribution::InternalTDistribution1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TDistribution2", (void*)&ScriptFloatDistribution::InternalTDistribution2);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TDistribution3", (void*)&ScriptFloatDistribution::InternalTDistribution3);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptFloatDistribution::InternalGetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMinConstant", (void*)&ScriptFloatDistribution::InternalGetMinConstant);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxConstant", (void*)&ScriptFloatDistribution::InternalGetMaxConstant);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMinCurve", (void*)&ScriptFloatDistribution::InternalGetMinCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxCurve", (void*)&ScriptFloatDistribution::InternalGetMaxCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptFloatDistribution::InternalEvaluate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Evaluate0", (void*)&ScriptFloatDistribution::InternalEvaluate0);

	}

	MonoObject* ScriptFloatDistribution::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptFloatDistribution::InternalTDistribution(MonoObject* scriptObject)
	{
		TShared<TDistribution<float>> nativeObject = B3DMakeShared<TDistribution<float>>();
		ScriptObjectWrapper::Create<ScriptFloatDistribution>(nativeObject, scriptObject);
	}

	void ScriptFloatDistribution::InternalTDistribution0(MonoObject* scriptObject, float value)
	{
		TShared<TDistribution<float>> nativeObject = B3DMakeShared<TDistribution<float>>(value);
		ScriptObjectWrapper::Create<ScriptFloatDistribution>(nativeObject, scriptObject);
	}

	void ScriptFloatDistribution::InternalTDistribution1(MonoObject* scriptObject, float minValue, float maxValue)
	{
		TShared<TDistribution<float>> nativeObject = B3DMakeShared<TDistribution<float>>(minValue, maxValue);
		ScriptObjectWrapper::Create<ScriptFloatDistribution>(nativeObject, scriptObject);
	}

	void ScriptFloatDistribution::InternalTDistribution2(MonoObject* scriptObject, MonoObject* curve)
	{
		TShared<TAnimationCurve<float>> tmpcurve;
		ScriptAnimationCurve* scriptObjectWrappercurve;
		scriptObjectWrappercurve = ScriptAnimationCurve::GetScriptObjectWrapper(curve);
		if(scriptObjectWrappercurve != nullptr)
			tmpcurve = std::static_pointer_cast<TAnimationCurve<float>>(scriptObjectWrappercurve->GetBaseNativeObjectAsShared());
		TShared<TDistribution<float>> nativeObject = B3DMakeShared<TDistribution<float>>(*tmpcurve);
		ScriptObjectWrapper::Create<ScriptFloatDistribution>(nativeObject, scriptObject);
	}

	void ScriptFloatDistribution::InternalTDistribution3(MonoObject* scriptObject, MonoObject* minCurve, MonoObject* maxCurve)
	{
		TShared<TAnimationCurve<float>> tmpminCurve;
		ScriptAnimationCurve* scriptObjectWrapperminCurve;
		scriptObjectWrapperminCurve = ScriptAnimationCurve::GetScriptObjectWrapper(minCurve);
		if(scriptObjectWrapperminCurve != nullptr)
			tmpminCurve = std::static_pointer_cast<TAnimationCurve<float>>(scriptObjectWrapperminCurve->GetBaseNativeObjectAsShared());
		TShared<TAnimationCurve<float>> tmpmaxCurve;
		ScriptAnimationCurve* scriptObjectWrappermaxCurve;
		scriptObjectWrappermaxCurve = ScriptAnimationCurve::GetScriptObjectWrapper(maxCurve);
		if(scriptObjectWrappermaxCurve != nullptr)
			tmpmaxCurve = std::static_pointer_cast<TAnimationCurve<float>>(scriptObjectWrappermaxCurve->GetBaseNativeObjectAsShared());
		TShared<TDistribution<float>> nativeObject = B3DMakeShared<TDistribution<float>>(*tmpminCurve, *tmpmaxCurve);
		ScriptObjectWrapper::Create<ScriptFloatDistribution>(nativeObject, scriptObject);
	}

	PropertyDistributionType ScriptFloatDistribution::InternalGetType(ScriptFloatDistribution* self)
	{
		PropertyDistributionType tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TDistribution<float>*>(self->GetNativeObject())->GetType();

		PropertyDistributionType __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptFloatDistribution::InternalGetMinConstant(ScriptFloatDistribution* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TDistribution<float>*>(self->GetNativeObject())->GetMinConstant();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptFloatDistribution::InternalGetMaxConstant(ScriptFloatDistribution* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TDistribution<float>*>(self->GetNativeObject())->GetMaxConstant();

		float __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptFloatDistribution::InternalGetMinCurve(ScriptFloatDistribution* self)
	{
		TShared<TAnimationCurve<float>> tmp__output = B3DMakeShared<TAnimationCurve<float>>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<TDistribution<float>*>(self->GetNativeObject())->GetMinCurve();

		MonoObject* __output;
		__output = ScriptAnimationCurve::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	MonoObject* ScriptFloatDistribution::InternalGetMaxCurve(ScriptFloatDistribution* self)
	{
		TShared<TAnimationCurve<float>> tmp__output = B3DMakeShared<TAnimationCurve<float>>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<TDistribution<float>*>(self->GetNativeObject())->GetMaxCurve();

		MonoObject* __output;
		__output = ScriptAnimationCurve::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	float ScriptFloatDistribution::InternalEvaluate(ScriptFloatDistribution* self, float t, float factor)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TDistribution<float>*>(self->GetNativeObject())->Evaluate(t, factor);

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptFloatDistribution::InternalEvaluate0(ScriptFloatDistribution* self, float t, MonoObject* factor)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		TShared<Random> tmpfactor;
		ScriptRandom* scriptObjectWrapperfactor;
		scriptObjectWrapperfactor = ScriptRandom::GetScriptObjectWrapper(factor);
		if(scriptObjectWrapperfactor != nullptr)
			tmpfactor = std::static_pointer_cast<Random>(scriptObjectWrapperfactor->GetBaseNativeObjectAsShared());
		tmp__output = static_cast<TDistribution<float>*>(self->GetNativeObject())->Evaluate(t, *tmpfactor);

		float __output;
		__output = tmp__output;

		return __output;
	}

	ScriptVector3Distribution::ScriptVector3Distribution(const TShared<TDistribution<TVector3<float>>>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptVector3Distribution::~ScriptVector3Distribution()
	{
		UnregisterEvents();
	}

	void ScriptVector3Distribution::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TDistribution", (void*)&ScriptVector3Distribution::InternalTDistribution);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TDistribution0", (void*)&ScriptVector3Distribution::InternalTDistribution0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TDistribution1", (void*)&ScriptVector3Distribution::InternalTDistribution1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TDistribution2", (void*)&ScriptVector3Distribution::InternalTDistribution2);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TDistribution3", (void*)&ScriptVector3Distribution::InternalTDistribution3);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptVector3Distribution::InternalGetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMinConstant", (void*)&ScriptVector3Distribution::InternalGetMinConstant);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxConstant", (void*)&ScriptVector3Distribution::InternalGetMaxConstant);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMinCurve", (void*)&ScriptVector3Distribution::InternalGetMinCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxCurve", (void*)&ScriptVector3Distribution::InternalGetMaxCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptVector3Distribution::InternalEvaluate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Evaluate0", (void*)&ScriptVector3Distribution::InternalEvaluate0);

	}

	MonoObject* ScriptVector3Distribution::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptVector3Distribution::InternalTDistribution(MonoObject* scriptObject)
	{
		TShared<TDistribution<TVector3<float>>> nativeObject = B3DMakeShared<TDistribution<TVector3<float>>>();
		ScriptObjectWrapper::Create<ScriptVector3Distribution>(nativeObject, scriptObject);
	}

	void ScriptVector3Distribution::InternalTDistribution0(MonoObject* scriptObject, TVector3<float>* value)
	{
		TShared<TDistribution<TVector3<float>>> nativeObject = B3DMakeShared<TDistribution<TVector3<float>>>(*value);
		ScriptObjectWrapper::Create<ScriptVector3Distribution>(nativeObject, scriptObject);
	}

	void ScriptVector3Distribution::InternalTDistribution1(MonoObject* scriptObject, TVector3<float>* minValue, TVector3<float>* maxValue)
	{
		TShared<TDistribution<TVector3<float>>> nativeObject = B3DMakeShared<TDistribution<TVector3<float>>>(*minValue, *maxValue);
		ScriptObjectWrapper::Create<ScriptVector3Distribution>(nativeObject, scriptObject);
	}

	void ScriptVector3Distribution::InternalTDistribution2(MonoObject* scriptObject, MonoObject* curve)
	{
		TShared<TAnimationCurve<TVector3<float>>> tmpcurve;
		ScriptVector3Curve* scriptObjectWrappercurve;
		scriptObjectWrappercurve = ScriptVector3Curve::GetScriptObjectWrapper(curve);
		if(scriptObjectWrappercurve != nullptr)
			tmpcurve = std::static_pointer_cast<TAnimationCurve<TVector3<float>>>(scriptObjectWrappercurve->GetBaseNativeObjectAsShared());
		TShared<TDistribution<TVector3<float>>> nativeObject = B3DMakeShared<TDistribution<TVector3<float>>>(*tmpcurve);
		ScriptObjectWrapper::Create<ScriptVector3Distribution>(nativeObject, scriptObject);
	}

	void ScriptVector3Distribution::InternalTDistribution3(MonoObject* scriptObject, MonoObject* minCurve, MonoObject* maxCurve)
	{
		TShared<TAnimationCurve<TVector3<float>>> tmpminCurve;
		ScriptVector3Curve* scriptObjectWrapperminCurve;
		scriptObjectWrapperminCurve = ScriptVector3Curve::GetScriptObjectWrapper(minCurve);
		if(scriptObjectWrapperminCurve != nullptr)
			tmpminCurve = std::static_pointer_cast<TAnimationCurve<TVector3<float>>>(scriptObjectWrapperminCurve->GetBaseNativeObjectAsShared());
		TShared<TAnimationCurve<TVector3<float>>> tmpmaxCurve;
		ScriptVector3Curve* scriptObjectWrappermaxCurve;
		scriptObjectWrappermaxCurve = ScriptVector3Curve::GetScriptObjectWrapper(maxCurve);
		if(scriptObjectWrappermaxCurve != nullptr)
			tmpmaxCurve = std::static_pointer_cast<TAnimationCurve<TVector3<float>>>(scriptObjectWrappermaxCurve->GetBaseNativeObjectAsShared());
		TShared<TDistribution<TVector3<float>>> nativeObject = B3DMakeShared<TDistribution<TVector3<float>>>(*tmpminCurve, *tmpmaxCurve);
		ScriptObjectWrapper::Create<ScriptVector3Distribution>(nativeObject, scriptObject);
	}

	PropertyDistributionType ScriptVector3Distribution::InternalGetType(ScriptVector3Distribution* self)
	{
		PropertyDistributionType tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TDistribution<TVector3<float>>*>(self->GetNativeObject())->GetType();

		PropertyDistributionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptVector3Distribution::InternalGetMinConstant(ScriptVector3Distribution* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<TDistribution<TVector3<float>>*>(self->GetNativeObject())->GetMinConstant();

		*__output = tmp__output;
	}

	void ScriptVector3Distribution::InternalGetMaxConstant(ScriptVector3Distribution* self, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<TDistribution<TVector3<float>>*>(self->GetNativeObject())->GetMaxConstant();

		*__output = tmp__output;
	}

	MonoObject* ScriptVector3Distribution::InternalGetMinCurve(ScriptVector3Distribution* self)
	{
		TShared<TAnimationCurve<TVector3<float>>> tmp__output = B3DMakeShared<TAnimationCurve<TVector3<float>>>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<TDistribution<TVector3<float>>*>(self->GetNativeObject())->GetMinCurve();

		MonoObject* __output;
		__output = ScriptVector3Curve::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	MonoObject* ScriptVector3Distribution::InternalGetMaxCurve(ScriptVector3Distribution* self)
	{
		TShared<TAnimationCurve<TVector3<float>>> tmp__output = B3DMakeShared<TAnimationCurve<TVector3<float>>>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<TDistribution<TVector3<float>>*>(self->GetNativeObject())->GetMaxCurve();

		MonoObject* __output;
		__output = ScriptVector3Curve::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptVector3Distribution::InternalEvaluate(ScriptVector3Distribution* self, float t, float factor, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<TDistribution<TVector3<float>>*>(self->GetNativeObject())->Evaluate(t, factor);

		*__output = tmp__output;
	}

	void ScriptVector3Distribution::InternalEvaluate0(ScriptVector3Distribution* self, float t, MonoObject* factor, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TShared<Random> tmpfactor;
		ScriptRandom* scriptObjectWrapperfactor;
		scriptObjectWrapperfactor = ScriptRandom::GetScriptObjectWrapper(factor);
		if(scriptObjectWrapperfactor != nullptr)
			tmpfactor = std::static_pointer_cast<Random>(scriptObjectWrapperfactor->GetBaseNativeObjectAsShared());
		TVector3<float> tmp__output;
		tmp__output = static_cast<TDistribution<TVector3<float>>*>(self->GetNativeObject())->Evaluate(t, *tmpfactor);

		*__output = tmp__output;
	}

	ScriptVector2Distribution::ScriptVector2Distribution(const TShared<TDistribution<TVector2<float>>>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptVector2Distribution::~ScriptVector2Distribution()
	{
		UnregisterEvents();
	}

	void ScriptVector2Distribution::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TDistribution", (void*)&ScriptVector2Distribution::InternalTDistribution);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TDistribution0", (void*)&ScriptVector2Distribution::InternalTDistribution0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TDistribution1", (void*)&ScriptVector2Distribution::InternalTDistribution1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TDistribution2", (void*)&ScriptVector2Distribution::InternalTDistribution2);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TDistribution3", (void*)&ScriptVector2Distribution::InternalTDistribution3);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptVector2Distribution::InternalGetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMinConstant", (void*)&ScriptVector2Distribution::InternalGetMinConstant);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxConstant", (void*)&ScriptVector2Distribution::InternalGetMaxConstant);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMinCurve", (void*)&ScriptVector2Distribution::InternalGetMinCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxCurve", (void*)&ScriptVector2Distribution::InternalGetMaxCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptVector2Distribution::InternalEvaluate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Evaluate0", (void*)&ScriptVector2Distribution::InternalEvaluate0);

	}

	MonoObject* ScriptVector2Distribution::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptVector2Distribution::InternalTDistribution(MonoObject* scriptObject)
	{
		TShared<TDistribution<TVector2<float>>> nativeObject = B3DMakeShared<TDistribution<TVector2<float>>>();
		ScriptObjectWrapper::Create<ScriptVector2Distribution>(nativeObject, scriptObject);
	}

	void ScriptVector2Distribution::InternalTDistribution0(MonoObject* scriptObject, TVector2<float>* value)
	{
		TShared<TDistribution<TVector2<float>>> nativeObject = B3DMakeShared<TDistribution<TVector2<float>>>(*value);
		ScriptObjectWrapper::Create<ScriptVector2Distribution>(nativeObject, scriptObject);
	}

	void ScriptVector2Distribution::InternalTDistribution1(MonoObject* scriptObject, TVector2<float>* minValue, TVector2<float>* maxValue)
	{
		TShared<TDistribution<TVector2<float>>> nativeObject = B3DMakeShared<TDistribution<TVector2<float>>>(*minValue, *maxValue);
		ScriptObjectWrapper::Create<ScriptVector2Distribution>(nativeObject, scriptObject);
	}

	void ScriptVector2Distribution::InternalTDistribution2(MonoObject* scriptObject, MonoObject* curve)
	{
		TShared<TAnimationCurve<TVector2<float>>> tmpcurve;
		ScriptVector2Curve* scriptObjectWrappercurve;
		scriptObjectWrappercurve = ScriptVector2Curve::GetScriptObjectWrapper(curve);
		if(scriptObjectWrappercurve != nullptr)
			tmpcurve = std::static_pointer_cast<TAnimationCurve<TVector2<float>>>(scriptObjectWrappercurve->GetBaseNativeObjectAsShared());
		TShared<TDistribution<TVector2<float>>> nativeObject = B3DMakeShared<TDistribution<TVector2<float>>>(*tmpcurve);
		ScriptObjectWrapper::Create<ScriptVector2Distribution>(nativeObject, scriptObject);
	}

	void ScriptVector2Distribution::InternalTDistribution3(MonoObject* scriptObject, MonoObject* minCurve, MonoObject* maxCurve)
	{
		TShared<TAnimationCurve<TVector2<float>>> tmpminCurve;
		ScriptVector2Curve* scriptObjectWrapperminCurve;
		scriptObjectWrapperminCurve = ScriptVector2Curve::GetScriptObjectWrapper(minCurve);
		if(scriptObjectWrapperminCurve != nullptr)
			tmpminCurve = std::static_pointer_cast<TAnimationCurve<TVector2<float>>>(scriptObjectWrapperminCurve->GetBaseNativeObjectAsShared());
		TShared<TAnimationCurve<TVector2<float>>> tmpmaxCurve;
		ScriptVector2Curve* scriptObjectWrappermaxCurve;
		scriptObjectWrappermaxCurve = ScriptVector2Curve::GetScriptObjectWrapper(maxCurve);
		if(scriptObjectWrappermaxCurve != nullptr)
			tmpmaxCurve = std::static_pointer_cast<TAnimationCurve<TVector2<float>>>(scriptObjectWrappermaxCurve->GetBaseNativeObjectAsShared());
		TShared<TDistribution<TVector2<float>>> nativeObject = B3DMakeShared<TDistribution<TVector2<float>>>(*tmpminCurve, *tmpmaxCurve);
		ScriptObjectWrapper::Create<ScriptVector2Distribution>(nativeObject, scriptObject);
	}

	PropertyDistributionType ScriptVector2Distribution::InternalGetType(ScriptVector2Distribution* self)
	{
		PropertyDistributionType tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TDistribution<TVector2<float>>*>(self->GetNativeObject())->GetType();

		PropertyDistributionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptVector2Distribution::InternalGetMinConstant(ScriptVector2Distribution* self, TVector2<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector2<float> tmp__output;
		tmp__output = static_cast<TDistribution<TVector2<float>>*>(self->GetNativeObject())->GetMinConstant();

		*__output = tmp__output;
	}

	void ScriptVector2Distribution::InternalGetMaxConstant(ScriptVector2Distribution* self, TVector2<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector2<float> tmp__output;
		tmp__output = static_cast<TDistribution<TVector2<float>>*>(self->GetNativeObject())->GetMaxConstant();

		*__output = tmp__output;
	}

	MonoObject* ScriptVector2Distribution::InternalGetMinCurve(ScriptVector2Distribution* self)
	{
		TShared<TAnimationCurve<TVector2<float>>> tmp__output = B3DMakeShared<TAnimationCurve<TVector2<float>>>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<TDistribution<TVector2<float>>*>(self->GetNativeObject())->GetMinCurve();

		MonoObject* __output;
		__output = ScriptVector2Curve::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	MonoObject* ScriptVector2Distribution::InternalGetMaxCurve(ScriptVector2Distribution* self)
	{
		TShared<TAnimationCurve<TVector2<float>>> tmp__output = B3DMakeShared<TAnimationCurve<TVector2<float>>>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = static_cast<TDistribution<TVector2<float>>*>(self->GetNativeObject())->GetMaxCurve();

		MonoObject* __output;
		__output = ScriptVector2Curve::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptVector2Distribution::InternalEvaluate(ScriptVector2Distribution* self, float t, float factor, TVector2<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector2<float> tmp__output;
		tmp__output = static_cast<TDistribution<TVector2<float>>*>(self->GetNativeObject())->Evaluate(t, factor);

		*__output = tmp__output;
	}

	void ScriptVector2Distribution::InternalEvaluate0(ScriptVector2Distribution* self, float t, MonoObject* factor, TVector2<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TShared<Random> tmpfactor;
		ScriptRandom* scriptObjectWrapperfactor;
		scriptObjectWrapperfactor = ScriptRandom::GetScriptObjectWrapper(factor);
		if(scriptObjectWrapperfactor != nullptr)
			tmpfactor = std::static_pointer_cast<Random>(scriptObjectWrapperfactor->GetBaseNativeObjectAsShared());
		TVector2<float> tmp__output;
		tmp__output = static_cast<TDistribution<TVector2<float>>*>(self->GetNativeObject())->Evaluate(t, *tmpfactor);

		*__output = tmp__output;
	}
}
