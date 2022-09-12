//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
		metaData.scriptClass->AddInternalCall("Internal_eulerToQuaternionCurve", (void*)&ScriptAnimationUtility::Internal_eulerToQuaternionCurve);
		metaData.scriptClass->AddInternalCall("Internal_quaternionToEulerCurve", (void*)&ScriptAnimationUtility::Internal_quaternionToEulerCurve);
		metaData.scriptClass->AddInternalCall("Internal_splitCurve3D", (void*)&ScriptAnimationUtility::Internal_splitCurve3D);
		metaData.scriptClass->AddInternalCall("Internal_combineCurve3D", (void*)&ScriptAnimationUtility::Internal_combineCurve3D);
		metaData.scriptClass->AddInternalCall("Internal_splitCurve2D", (void*)&ScriptAnimationUtility::Internal_splitCurve2D);
		metaData.scriptClass->AddInternalCall("Internal_combineCurve2D", (void*)&ScriptAnimationUtility::Internal_combineCurve2D);
		metaData.scriptClass->AddInternalCall("Internal_calculateRange", (void*)&ScriptAnimationUtility::Internal_calculateRange);

	}

	MonoObject* ScriptAnimationUtility::create(const SPtr<AnimationUtility>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptAnimationUtility>()) ScriptAnimationUtility(managedInstance, value);
		return managedInstance;
	}
	MonoObject* ScriptAnimationUtility::Internal_eulerToQuaternionCurve(MonoObject* eulerCurve, EulerAngleOrder order)
	{
		SPtr<TAnimationCurve<Quaternion>> tmp__output;
		SPtr<TAnimationCurve<Vector3>> tmpeulerCurve;
		ScriptTAnimationCurveVector3* scripteulerCurve;
		scripteulerCurve = ScriptTAnimationCurveVector3::toNative(eulerCurve);
		if(scripteulerCurve != nullptr)
			tmpeulerCurve = scripteulerCurve->GetInternal();
		tmp__output = AnimationUtility::eulerToQuaternionCurve(tmpeulerCurve, order);

		MonoObject* __output;
		__output = ScriptTAnimationCurveQuaternion::create(tmp__output);

		return __output;
	}

	MonoObject* ScriptAnimationUtility::Internal_quaternionToEulerCurve(MonoObject* quatCurve)
	{
		SPtr<TAnimationCurve<Vector3>> tmp__output;
		SPtr<TAnimationCurve<Quaternion>> tmpquatCurve;
		ScriptTAnimationCurveQuaternion* scriptquatCurve;
		scriptquatCurve = ScriptTAnimationCurveQuaternion::toNative(quatCurve);
		if(scriptquatCurve != nullptr)
			tmpquatCurve = scriptquatCurve->GetInternal();
		tmp__output = AnimationUtility::quaternionToEulerCurve(tmpquatCurve);

		MonoObject* __output;
		__output = ScriptTAnimationCurveVector3::create(tmp__output);

		return __output;
	}

	MonoArray* ScriptAnimationUtility::Internal_splitCurve3D(MonoObject* compoundCurve)
	{
		Vector<SPtr<TAnimationCurve<float>>> vec__output;
		SPtr<TAnimationCurve<Vector3>> tmpcompoundCurve;
		ScriptTAnimationCurveVector3* scriptcompoundCurve;
		scriptcompoundCurve = ScriptTAnimationCurveVector3::toNative(compoundCurve);
		if(scriptcompoundCurve != nullptr)
			tmpcompoundCurve = scriptcompoundCurve->GetInternal();
		vec__output = AnimationUtility::splitCurve3D(tmpcompoundCurve);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptTAnimationCurvefloat>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			SPtr<TAnimationCurve<float>> arrayElemPtr__output = vec__output[i];
			MonoObject* arrayElem__output;
			arrayElem__output = ScriptTAnimationCurvefloat::create(arrayElemPtr__output);
			array__output.Set(i, arrayElem__output);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	MonoObject* ScriptAnimationUtility::Internal_combineCurve3D(MonoArray* curveComponents)
	{
		SPtr<TAnimationCurve<Vector3>> tmp__output;
		Vector<SPtr<TAnimationCurve<float>>> veccurveComponents;
		if(curveComponents != nullptr)
		{
			ScriptArray ArraycurveComponents(curveComponents);
			veccurveComponents.Resize(arraycurveComponents.size());
			for(int i = 0; i < (int)arraycurveComponents.size(); i++)
			{
				ScriptTAnimationCurvefloat* scriptcurveComponents;
				scriptcurveComponents = ScriptTAnimationCurvefloat::toNative(arraycurveComponents.get<MonoObject*>(i));
				if(scriptcurveComponents != nullptr)
				{
					SPtr<TAnimationCurve<float>> arrayElemPtrcurveComponents = scriptcurveComponents->GetInternal();
					veccurveComponents[i] = arrayElemPtrcurveComponents;
				}
			}
		}
		tmp__output = AnimationUtility::combineCurve3D(veccurveComponents);

		MonoObject* __output;
		__output = ScriptTAnimationCurveVector3::create(tmp__output);

		return __output;
	}

	MonoArray* ScriptAnimationUtility::Internal_splitCurve2D(MonoObject* compoundCurve)
	{
		Vector<SPtr<TAnimationCurve<float>>> vec__output;
		SPtr<TAnimationCurve<Vector2>> tmpcompoundCurve;
		ScriptTAnimationCurveVector2* scriptcompoundCurve;
		scriptcompoundCurve = ScriptTAnimationCurveVector2::toNative(compoundCurve);
		if(scriptcompoundCurve != nullptr)
			tmpcompoundCurve = scriptcompoundCurve->GetInternal();
		vec__output = AnimationUtility::splitCurve2D(tmpcompoundCurve);

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptTAnimationCurvefloat>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			SPtr<TAnimationCurve<float>> arrayElemPtr__output = vec__output[i];
			MonoObject* arrayElem__output;
			arrayElem__output = ScriptTAnimationCurvefloat::create(arrayElemPtr__output);
			array__output.Set(i, arrayElem__output);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	MonoObject* ScriptAnimationUtility::Internal_combineCurve2D(MonoArray* curveComponents)
	{
		SPtr<TAnimationCurve<Vector2>> tmp__output;
		Vector<SPtr<TAnimationCurve<float>>> veccurveComponents;
		if(curveComponents != nullptr)
		{
			ScriptArray ArraycurveComponents(curveComponents);
			veccurveComponents.Resize(arraycurveComponents.size());
			for(int i = 0; i < (int)arraycurveComponents.size(); i++)
			{
				ScriptTAnimationCurvefloat* scriptcurveComponents;
				scriptcurveComponents = ScriptTAnimationCurvefloat::toNative(arraycurveComponents.get<MonoObject*>(i));
				if(scriptcurveComponents != nullptr)
				{
					SPtr<TAnimationCurve<float>> arrayElemPtrcurveComponents = scriptcurveComponents->GetInternal();
					veccurveComponents[i] = arrayElemPtrcurveComponents;
				}
			}
		}
		tmp__output = AnimationUtility::combineCurve2D(veccurveComponents);

		MonoObject* __output;
		__output = ScriptTAnimationCurveVector2::create(tmp__output);

		return __output;
	}

	void ScriptAnimationUtility::Internal_calculateRange(MonoArray* curves, float* xMin, float* xMax, float* yMin, float* yMax)
	{
		Vector<SPtr<TAnimationCurve<float>>> veccurves;
		if(curves != nullptr)
		{
			ScriptArray Arraycurves(curves);
			veccurves.Resize(arraycurves.size());
			for(int i = 0; i < (int)arraycurves.size(); i++)
			{
				ScriptTAnimationCurvefloat* scriptcurves;
				scriptcurves = ScriptTAnimationCurvefloat::toNative(arraycurves.get<MonoObject*>(i));
				if(scriptcurves != nullptr)
				{
					SPtr<TAnimationCurve<float>> arrayElemPtrcurves = scriptcurves->GetInternal();
					veccurves[i] = arrayElemPtrcurves;
				}
			}

		}
		AnimationUtility::calculateRange(veccurves, *xMin, *xMax, *yMin, *yMax);
	}
}
