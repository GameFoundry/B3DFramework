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
	ScriptAnimationUtility::ScriptAnimationUtility(MonoObject* managedInstance, const SPtr<AnimationUtility>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptAnimationUtility::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_EulerToQuaternionCurve", (void*)&ScriptAnimationUtility::InternalEulerToQuaternionCurve);
		metaData.ScriptClass->AddInternalCall("Internal_QuaternionToEulerCurve", (void*)&ScriptAnimationUtility::InternalQuaternionToEulerCurve);
		metaData.ScriptClass->AddInternalCall("Internal_SplitCurve3D", (void*)&ScriptAnimationUtility::InternalSplitCurve3D);
		metaData.ScriptClass->AddInternalCall("Internal_CombineCurve3D", (void*)&ScriptAnimationUtility::InternalCombineCurve3D);
		metaData.ScriptClass->AddInternalCall("Internal_SplitCurve2D", (void*)&ScriptAnimationUtility::InternalSplitCurve2D);
		metaData.ScriptClass->AddInternalCall("Internal_CombineCurve2D", (void*)&ScriptAnimationUtility::InternalCombineCurve2D);
		metaData.ScriptClass->AddInternalCall("Internal_CalculateRange", (void*)&ScriptAnimationUtility::InternalCalculateRange);

	}

	MonoObject* ScriptAnimationUtility::Create(const SPtr<AnimationUtility>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptAnimationUtility>()) ScriptAnimationUtility(managedInstance, value);
		return managedInstance;
	}
	MonoObject* ScriptAnimationUtility::InternalEulerToQuaternionCurve(MonoObject* eulerCurve, EulerAngleOrder order)
	{
		SPtr<TAnimationCurve<Quaternion>> tmp__output;
		SPtr<TAnimationCurve<Vector3>> tmpeulerCurve;
		ScriptVector3Curve* scripteulerCurve;
		scripteulerCurve = ScriptVector3Curve::ToNative(eulerCurve);
		if(scripteulerCurve != nullptr)
			tmpeulerCurve = scripteulerCurve->GetInternal();
		tmp__output = AnimationUtility::EulerToQuaternionCurve(tmpeulerCurve, order);

		MonoObject* __output;
		__output = ScriptQuaternionCurve::Create(tmp__output);

		return __output;
	}

	MonoObject* ScriptAnimationUtility::InternalQuaternionToEulerCurve(MonoObject* quatCurve)
	{
		SPtr<TAnimationCurve<Vector3>> tmp__output;
		SPtr<TAnimationCurve<Quaternion>> tmpquatCurve;
		ScriptQuaternionCurve* scriptquatCurve;
		scriptquatCurve = ScriptQuaternionCurve::ToNative(quatCurve);
		if(scriptquatCurve != nullptr)
			tmpquatCurve = scriptquatCurve->GetInternal();
		tmp__output = AnimationUtility::QuaternionToEulerCurve(tmpquatCurve);

		MonoObject* __output;
		__output = ScriptVector3Curve::Create(tmp__output);

		return __output;
	}

	MonoArray* ScriptAnimationUtility::InternalSplitCurve3D(MonoObject* compoundCurve)
	{
		Vector<SPtr<TAnimationCurve<float>>> vec__output;
		SPtr<TAnimationCurve<Vector3>> tmpcompoundCurve;
		ScriptVector3Curve* scriptcompoundCurve;
		scriptcompoundCurve = ScriptVector3Curve::ToNative(compoundCurve);
		if(scriptcompoundCurve != nullptr)
			tmpcompoundCurve = scriptcompoundCurve->GetInternal();
		vec__output = AnimationUtility::SplitCurve3D(tmpcompoundCurve);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptAnimationCurve>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			SPtr<TAnimationCurve<float>> arrayElemPtr__output = vec__output[i];
			MonoObject* arrayElem__output;
			arrayElem__output = ScriptAnimationCurve::Create(arrayElemPtr__output);
			array__output.Set(i, arrayElem__output);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	MonoObject* ScriptAnimationUtility::InternalCombineCurve3D(MonoArray* curveComponents)
	{
		SPtr<TAnimationCurve<Vector3>> tmp__output;
		Vector<SPtr<TAnimationCurve<float>>> veccurveComponents;
		if(curveComponents != nullptr)
		{
			ScriptArray arraycurveComponents(curveComponents);
			veccurveComponents.resize(arraycurveComponents.Size());
			for(int i = 0; i < (int)arraycurveComponents.Size(); i++)
			{
				ScriptAnimationCurve* scriptcurveComponents;
				scriptcurveComponents = ScriptAnimationCurve::ToNative(arraycurveComponents.Get<MonoObject*>(i));
				if(scriptcurveComponents != nullptr)
				{
					SPtr<TAnimationCurve<float>> arrayElemPtrcurveComponents = scriptcurveComponents->GetInternal();
					veccurveComponents[i] = arrayElemPtrcurveComponents;
				}
			}
		}
		tmp__output = AnimationUtility::CombineCurve3D(veccurveComponents);

		MonoObject* __output;
		__output = ScriptVector3Curve::Create(tmp__output);

		return __output;
	}

	MonoArray* ScriptAnimationUtility::InternalSplitCurve2D(MonoObject* compoundCurve)
	{
		Vector<SPtr<TAnimationCurve<float>>> vec__output;
		SPtr<TAnimationCurve<Vector2>> tmpcompoundCurve;
		ScriptVector2Curve* scriptcompoundCurve;
		scriptcompoundCurve = ScriptVector2Curve::ToNative(compoundCurve);
		if(scriptcompoundCurve != nullptr)
			tmpcompoundCurve = scriptcompoundCurve->GetInternal();
		vec__output = AnimationUtility::SplitCurve2D(tmpcompoundCurve);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptAnimationCurve>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			SPtr<TAnimationCurve<float>> arrayElemPtr__output = vec__output[i];
			MonoObject* arrayElem__output;
			arrayElem__output = ScriptAnimationCurve::Create(arrayElemPtr__output);
			array__output.Set(i, arrayElem__output);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	MonoObject* ScriptAnimationUtility::InternalCombineCurve2D(MonoArray* curveComponents)
	{
		SPtr<TAnimationCurve<Vector2>> tmp__output;
		Vector<SPtr<TAnimationCurve<float>>> veccurveComponents;
		if(curveComponents != nullptr)
		{
			ScriptArray arraycurveComponents(curveComponents);
			veccurveComponents.resize(arraycurveComponents.Size());
			for(int i = 0; i < (int)arraycurveComponents.Size(); i++)
			{
				ScriptAnimationCurve* scriptcurveComponents;
				scriptcurveComponents = ScriptAnimationCurve::ToNative(arraycurveComponents.Get<MonoObject*>(i));
				if(scriptcurveComponents != nullptr)
				{
					SPtr<TAnimationCurve<float>> arrayElemPtrcurveComponents = scriptcurveComponents->GetInternal();
					veccurveComponents[i] = arrayElemPtrcurveComponents;
				}
			}
		}
		tmp__output = AnimationUtility::CombineCurve2D(veccurveComponents);

		MonoObject* __output;
		__output = ScriptVector2Curve::Create(tmp__output);

		return __output;
	}

	void ScriptAnimationUtility::InternalCalculateRange(MonoArray* curves, float* xMin, float* xMax, float* yMin, float* yMax)
	{
		Vector<SPtr<TAnimationCurve<float>>> veccurves;
		if(curves != nullptr)
		{
			ScriptArray arraycurves(curves);
			veccurves.resize(arraycurves.Size());
			for(int i = 0; i < (int)arraycurves.Size(); i++)
			{
				ScriptAnimationCurve* scriptcurves;
				scriptcurves = ScriptAnimationCurve::ToNative(arraycurves.Get<MonoObject*>(i));
				if(scriptcurves != nullptr)
				{
					SPtr<TAnimationCurve<float>> arrayElemPtrcurves = scriptcurves->GetInternal();
					veccurves[i] = arrayElemPtrcurves;
				}
			}

		}
		AnimationUtility::CalculateRange(veccurves, *xMin, *xMax, *yMin, *yMax);
	}
}
