//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptAnimationUtility.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Animation/B3DAnimationUtility.h"
#include "B3DScriptTAnimationCurve.generated.h"
#include "B3DScriptTAnimationCurve.generated.h"
#include "B3DScriptTAnimationCurve.generated.h"
#include "B3DScriptTAnimationCurve.generated.h"

namespace b3d
{
	ScriptAnimationUtility::ScriptAnimationUtility(const TShared<AnimationUtility>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptAnimationUtility::~ScriptAnimationUtility()
	{
		UnregisterEvents();
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
		TShared<TAnimationCurve<TQuaternion<float>>> tmp__output;
		TShared<TAnimationCurve<TVector3<float>>> tmpeulerCurve;
		ScriptVector3Curve* scriptObjectWrappereulerCurve;
		scriptObjectWrappereulerCurve = ScriptVector3Curve::GetScriptObjectWrapper(eulerCurve);
		if(scriptObjectWrappereulerCurve != nullptr)
			tmpeulerCurve = std::static_pointer_cast<TAnimationCurve<TVector3<float>>>(scriptObjectWrappereulerCurve->GetBaseNativeObjectAsShared());
		tmp__output = AnimationUtility::EulerToQuaternionCurve(tmpeulerCurve, order);

		MonoObject* __output;
		__output = ScriptQuaternionCurve::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	MonoObject* ScriptAnimationUtility::InternalQuaternionToEulerCurve(MonoObject* quatCurve)
	{
		TShared<TAnimationCurve<TVector3<float>>> tmp__output;
		TShared<TAnimationCurve<TQuaternion<float>>> tmpquatCurve;
		ScriptQuaternionCurve* scriptObjectWrapperquatCurve;
		scriptObjectWrapperquatCurve = ScriptQuaternionCurve::GetScriptObjectWrapper(quatCurve);
		if(scriptObjectWrapperquatCurve != nullptr)
			tmpquatCurve = std::static_pointer_cast<TAnimationCurve<TQuaternion<float>>>(scriptObjectWrapperquatCurve->GetBaseNativeObjectAsShared());
		tmp__output = AnimationUtility::QuaternionToEulerCurve(tmpquatCurve);

		MonoObject* __output;
		__output = ScriptVector3Curve::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	MonoArray* ScriptAnimationUtility::InternalSplitCurve3D(MonoObject* compoundCurve)
	{
		Vector<TShared<TAnimationCurve<float>>> nativeArray__output;
		TShared<TAnimationCurve<TVector3<float>>> tmpcompoundCurve;
		ScriptVector3Curve* scriptObjectWrappercompoundCurve;
		scriptObjectWrappercompoundCurve = ScriptVector3Curve::GetScriptObjectWrapper(compoundCurve);
		if(scriptObjectWrappercompoundCurve != nullptr)
			tmpcompoundCurve = std::static_pointer_cast<TAnimationCurve<TVector3<float>>>(scriptObjectWrappercompoundCurve->GetBaseNativeObjectAsShared());
		nativeArray__output = AnimationUtility::SplitCurve3D(tmpcompoundCurve);

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptAnimationCurve>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			TShared<TAnimationCurve<float>> arrayElementPointer__output = nativeArray__output[elementIndex];
			MonoObject* arrayElement__output;
			arrayElement__output = ScriptAnimationCurve::GetOrCreateScriptObject(arrayElementPointer__output);
			scriptArray__output.Set(elementIndex, arrayElement__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	MonoObject* ScriptAnimationUtility::InternalCombineCurve3D(MonoArray* curveComponents)
	{
		TShared<TAnimationCurve<TVector3<float>>> tmp__output;
		Vector<TShared<TAnimationCurve<float>>> nativeArraycurveComponents;
		if(curveComponents != nullptr)
		{
			ScriptArray scriptArraycurveComponents(curveComponents);
			nativeArraycurveComponents.resize(scriptArraycurveComponents.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraycurveComponents.Size(); elementIndex++)
			{
				TShared<TAnimationCurve<float>> arrayElementPointercurveComponents;
				ScriptAnimationCurve* scriptObjectWrappercurveComponents;
				scriptObjectWrappercurveComponents = ScriptAnimationCurve::GetScriptObjectWrapper(scriptArraycurveComponents.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappercurveComponents != nullptr)
				{
					arrayElementPointercurveComponents = std::static_pointer_cast<TAnimationCurve<float>>(scriptObjectWrappercurveComponents->GetBaseNativeObjectAsShared());
					nativeArraycurveComponents[elementIndex] = arrayElementPointercurveComponents;
				}
			}
		}
		tmp__output = AnimationUtility::CombineCurve3D(nativeArraycurveComponents);

		MonoObject* __output;
		__output = ScriptVector3Curve::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	MonoArray* ScriptAnimationUtility::InternalSplitCurve2D(MonoObject* compoundCurve)
	{
		Vector<TShared<TAnimationCurve<float>>> nativeArray__output;
		TShared<TAnimationCurve<TVector2<float>>> tmpcompoundCurve;
		ScriptVector2Curve* scriptObjectWrappercompoundCurve;
		scriptObjectWrappercompoundCurve = ScriptVector2Curve::GetScriptObjectWrapper(compoundCurve);
		if(scriptObjectWrappercompoundCurve != nullptr)
			tmpcompoundCurve = std::static_pointer_cast<TAnimationCurve<TVector2<float>>>(scriptObjectWrappercompoundCurve->GetBaseNativeObjectAsShared());
		nativeArray__output = AnimationUtility::SplitCurve2D(tmpcompoundCurve);

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptAnimationCurve>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			TShared<TAnimationCurve<float>> arrayElementPointer__output = nativeArray__output[elementIndex];
			MonoObject* arrayElement__output;
			arrayElement__output = ScriptAnimationCurve::GetOrCreateScriptObject(arrayElementPointer__output);
			scriptArray__output.Set(elementIndex, arrayElement__output);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	MonoObject* ScriptAnimationUtility::InternalCombineCurve2D(MonoArray* curveComponents)
	{
		TShared<TAnimationCurve<TVector2<float>>> tmp__output;
		Vector<TShared<TAnimationCurve<float>>> nativeArraycurveComponents;
		if(curveComponents != nullptr)
		{
			ScriptArray scriptArraycurveComponents(curveComponents);
			nativeArraycurveComponents.resize(scriptArraycurveComponents.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraycurveComponents.Size(); elementIndex++)
			{
				TShared<TAnimationCurve<float>> arrayElementPointercurveComponents;
				ScriptAnimationCurve* scriptObjectWrappercurveComponents;
				scriptObjectWrappercurveComponents = ScriptAnimationCurve::GetScriptObjectWrapper(scriptArraycurveComponents.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappercurveComponents != nullptr)
				{
					arrayElementPointercurveComponents = std::static_pointer_cast<TAnimationCurve<float>>(scriptObjectWrappercurveComponents->GetBaseNativeObjectAsShared());
					nativeArraycurveComponents[elementIndex] = arrayElementPointercurveComponents;
				}
			}
		}
		tmp__output = AnimationUtility::CombineCurve2D(nativeArraycurveComponents);

		MonoObject* __output;
		__output = ScriptVector2Curve::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptAnimationUtility::InternalCalculateRange(MonoArray* curves, float* outXMin, float* outXMax, float* outYMin, float* outYMax)
	{
		Vector<TShared<TAnimationCurve<float>>> nativeArraycurves;
		if(curves != nullptr)
		{
			ScriptArray scriptArraycurves(curves);
			nativeArraycurves.resize(scriptArraycurves.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraycurves.Size(); elementIndex++)
			{
				TShared<TAnimationCurve<float>> arrayElementPointercurves;
				ScriptAnimationCurve* scriptObjectWrappercurves;
				scriptObjectWrappercurves = ScriptAnimationCurve::GetScriptObjectWrapper(scriptArraycurves.Get<MonoObject*>(elementIndex));
				if(scriptObjectWrappercurves != nullptr)
				{
					arrayElementPointercurves = std::static_pointer_cast<TAnimationCurve<float>>(scriptObjectWrappercurves->GetBaseNativeObjectAsShared());
					nativeArraycurves[elementIndex] = arrayElementPointercurves;
				}
			}

		}
		AnimationUtility::CalculateRange(nativeArraycurves, *outXMin, *outXMax, *outYMin, *outYMax);
	}
}
