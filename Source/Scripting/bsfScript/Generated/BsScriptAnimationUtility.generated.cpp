//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAnimationUtility.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationUtility.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "BsScriptTAnimationCurve.generated.h"

namespace bs
{
	ScriptAnimationUtility::ScriptAnimationUtility(const SPtr<AnimationUtility>& nativeObject, MonoObject* scriptObject)
		:TScriptNonReflectableWrapper(nativeObject, scriptObject)
	{
		RegisterEvents();
	}

	void ScriptAnimationUtility::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_EulerToQuaternionCurve", (void*)&ScriptAnimationUtility::InternalEulerToQuaternionCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_QuaternionToEulerCurve", (void*)&ScriptAnimationUtility::InternalQuaternionToEulerCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SplitCurve3D", (void*)&ScriptAnimationUtility::InternalSplitCurve3D);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CombineCurve3D", (void*)&ScriptAnimationUtility::InternalCombineCurve3D);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SplitCurve2D", (void*)&ScriptAnimationUtility::InternalSplitCurve2D);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CombineCurve2D", (void*)&ScriptAnimationUtility::InternalCombineCurve2D);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CalculateRange", (void*)&ScriptAnimationUtility::InternalCalculateRange);

	}

	MonoObject* ScriptAnimationUtility::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptAnimationUtility::InternalEulerToQuaternionCurve(MonoObject* eulerCurve, EulerAngleOrder order)
	{
		SPtr<TAnimationCurve<Quaternion>> tmp__output;
		SPtr<TAnimationCurve<TVector3<float>>> tmpeulerCurve;
		ScriptVector3Curve* scriptObjectWrappereulerCurve;
		scriptObjectWrappereulerCurve = ScriptVector3Curve::ToNative(eulerCurve);
		if(scriptObjectWrappereulerCurve != nullptr)
			tmpeulerCurve = scriptObjectWrappereulerCurve->GetInternal();
		tmp__output = AnimationUtility::EulerToQuaternionCurve(tmpeulerCurve, order);

		MonoObject* __output;
		__output = ScriptQuaternionCurve::Create(tmp__output);

		return __output;
	}

	MonoObject* ScriptAnimationUtility::InternalQuaternionToEulerCurve(MonoObject* quatCurve)
	{
		SPtr<TAnimationCurve<TVector3<float>>> tmp__output;
		SPtr<TAnimationCurve<Quaternion>> tmpquatCurve;
		ScriptQuaternionCurve* scriptObjectWrapperquatCurve;
		scriptObjectWrapperquatCurve = ScriptQuaternionCurve::ToNative(quatCurve);
		if(scriptObjectWrapperquatCurve != nullptr)
			tmpquatCurve = scriptObjectWrapperquatCurve->GetInternal();
		tmp__output = AnimationUtility::QuaternionToEulerCurve(tmpquatCurve);

		MonoObject* __output;
		__output = ScriptVector3Curve::Create(tmp__output);

		return __output;
	}

	MonoArray* ScriptAnimationUtility::InternalSplitCurve3D(MonoObject* compoundCurve)
	{
		Vector<SPtr<TAnimationCurve<float>>> nativeArray__output;
		SPtr<TAnimationCurve<TVector3<float>>> tmpcompoundCurve;
		ScriptVector3Curve* scriptObjectWrappercompoundCurve;
		scriptObjectWrappercompoundCurve = ScriptVector3Curve::ToNative(compoundCurve);
		if(scriptObjectWrappercompoundCurve != nullptr)
			tmpcompoundCurve = scriptObjectWrappercompoundCurve->GetInternal();
		nativeArray__output = AnimationUtility::SplitCurve3D(tmpcompoundCurve);

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptAnimationCurve>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			SPtr<TAnimationCurve<float>> arrayElementPointer__output = nativeArray__output[elementIndex];
			MonoObject* arrayElement__output;
			arrayElement__output = ScriptAnimationCurve::Create(arrayElementPointer__output);
			scriptArray__output.Set(elementIndex, arrayElement__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	MonoObject* ScriptAnimationUtility::InternalCombineCurve3D(MonoArray* curveComponents)
	{
		SPtr<TAnimationCurve<TVector3<float>>> tmp__output;
		Vector<SPtr<TAnimationCurve<float>>> nativeArraycurveComponents;
		if(curveComponents != nullptr)
		{
			ScriptArray scriptArraycurveComponents(curveComponents);
			nativeArraycurveComponents.resize(scriptArraycurveComponents.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraycurveComponents.Size(); elementIndex++)
			{
				ScriptAnimationCurve* scriptObjectWrappercurveComponents;
				scriptObjectWrappercurveComponents = ScriptAnimationCurve::ToNative(scriptArraycurveComponents.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappercurveComponents != nullptr)
				{
					SPtr<TAnimationCurve<float>> arrayElementPointercurveComponents = scriptObjectWrappercurveComponents->GetInternal();
					nativeArraycurveComponents[elementIndex] = arrayElementPointercurveComponents;
				}
			}
		}
		tmp__output = AnimationUtility::CombineCurve3D(nativeArraycurveComponents);

		MonoObject* __output;
		__output = ScriptVector3Curve::Create(tmp__output);

		return __output;
	}

	MonoArray* ScriptAnimationUtility::InternalSplitCurve2D(MonoObject* compoundCurve)
	{
		Vector<SPtr<TAnimationCurve<float>>> nativeArray__output;
		SPtr<TAnimationCurve<TVector2<float>>> tmpcompoundCurve;
		ScriptVector2Curve* scriptObjectWrappercompoundCurve;
		scriptObjectWrappercompoundCurve = ScriptVector2Curve::ToNative(compoundCurve);
		if(scriptObjectWrappercompoundCurve != nullptr)
			tmpcompoundCurve = scriptObjectWrappercompoundCurve->GetInternal();
		nativeArray__output = AnimationUtility::SplitCurve2D(tmpcompoundCurve);

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptAnimationCurve>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			SPtr<TAnimationCurve<float>> arrayElementPointer__output = nativeArray__output[elementIndex];
			MonoObject* arrayElement__output;
			arrayElement__output = ScriptAnimationCurve::Create(arrayElementPointer__output);
			scriptArray__output.Set(elementIndex, arrayElement__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	MonoObject* ScriptAnimationUtility::InternalCombineCurve2D(MonoArray* curveComponents)
	{
		SPtr<TAnimationCurve<TVector2<float>>> tmp__output;
		Vector<SPtr<TAnimationCurve<float>>> nativeArraycurveComponents;
		if(curveComponents != nullptr)
		{
			ScriptArray scriptArraycurveComponents(curveComponents);
			nativeArraycurveComponents.resize(scriptArraycurveComponents.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraycurveComponents.Size(); elementIndex++)
			{
				ScriptAnimationCurve* scriptObjectWrappercurveComponents;
				scriptObjectWrappercurveComponents = ScriptAnimationCurve::ToNative(scriptArraycurveComponents.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappercurveComponents != nullptr)
				{
					SPtr<TAnimationCurve<float>> arrayElementPointercurveComponents = scriptObjectWrappercurveComponents->GetInternal();
					nativeArraycurveComponents[elementIndex] = arrayElementPointercurveComponents;
				}
			}
		}
		tmp__output = AnimationUtility::CombineCurve2D(nativeArraycurveComponents);

		MonoObject* __output;
		__output = ScriptVector2Curve::Create(tmp__output);

		return __output;
	}

	void ScriptAnimationUtility::InternalCalculateRange(MonoArray* curves, float* xMin, float* xMax, float* yMin, float* yMax)
	{
		Vector<SPtr<TAnimationCurve<float>>> nativeArraycurves;
		if(curves != nullptr)
		{
			ScriptArray scriptArraycurves(curves);
			nativeArraycurves.resize(scriptArraycurves.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraycurves.Size(); elementIndex++)
			{
				ScriptAnimationCurve* scriptObjectWrappercurves;
				scriptObjectWrappercurves = ScriptAnimationCurve::ToNative(scriptArraycurves.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappercurves != nullptr)
				{
					SPtr<TAnimationCurve<float>> arrayElementPointercurves = scriptObjectWrappercurves->GetInternal();
					nativeArraycurves[elementIndex] = arrayElementPointercurves;
				}
			}

		}
		AnimationUtility::CalculateRange(nativeArraycurves, *xMin, *xMax, *yMin, *yMax);
	}
}
