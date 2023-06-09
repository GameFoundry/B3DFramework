//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAnimationCurves.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationClip.h"
#include "../Extensions/BsAnimationEx.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "BsScriptTNamedAnimationCurve.generated.h"
#include "BsScriptTNamedAnimationCurve.generated.h"
#include "BsScriptTNamedAnimationCurve.generated.h"

namespace bs
{
	ScriptAnimationCurves::ScriptAnimationCurves(MonoObject* managedInstance, const SPtr<AnimationCurves>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptAnimationCurves::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_AnimationCurves", (void*)&ScriptAnimationCurves::InternalAnimationCurves);
		metaData.ScriptClass->AddInternalCall("Internal_AddPositionCurve", (void*)&ScriptAnimationCurves::InternalAddPositionCurve);
		metaData.ScriptClass->AddInternalCall("Internal_AddRotationCurve", (void*)&ScriptAnimationCurves::InternalAddRotationCurve);
		metaData.ScriptClass->AddInternalCall("Internal_AddScaleCurve", (void*)&ScriptAnimationCurves::InternalAddScaleCurve);
		metaData.ScriptClass->AddInternalCall("Internal_AddGenericCurve", (void*)&ScriptAnimationCurves::InternalAddGenericCurve);
		metaData.ScriptClass->AddInternalCall("Internal_RemovePositionCurve", (void*)&ScriptAnimationCurves::InternalRemovePositionCurve);
		metaData.ScriptClass->AddInternalCall("Internal_RemoveRotationCurve", (void*)&ScriptAnimationCurves::InternalRemoveRotationCurve);
		metaData.ScriptClass->AddInternalCall("Internal_RemoveScaleCurve", (void*)&ScriptAnimationCurves::InternalRemoveScaleCurve);
		metaData.ScriptClass->AddInternalCall("Internal_RemoveGenericCurve", (void*)&ScriptAnimationCurves::InternalRemoveGenericCurve);
		metaData.ScriptClass->AddInternalCall("Internal_GetPositionCurves", (void*)&ScriptAnimationCurves::InternalGetPositionCurves);
		metaData.ScriptClass->AddInternalCall("Internal_SetPositionCurves", (void*)&ScriptAnimationCurves::InternalSetPositionCurves);
		metaData.ScriptClass->AddInternalCall("Internal_GetRotationCurves", (void*)&ScriptAnimationCurves::InternalGetRotationCurves);
		metaData.ScriptClass->AddInternalCall("Internal_SetRotationCurves", (void*)&ScriptAnimationCurves::InternalSetRotationCurves);
		metaData.ScriptClass->AddInternalCall("Internal_GetScaleCurves", (void*)&ScriptAnimationCurves::InternalGetScaleCurves);
		metaData.ScriptClass->AddInternalCall("Internal_SetScaleCurves", (void*)&ScriptAnimationCurves::InternalSetScaleCurves);
		metaData.ScriptClass->AddInternalCall("Internal_GetGenericCurves", (void*)&ScriptAnimationCurves::InternalGetGenericCurves);
		metaData.ScriptClass->AddInternalCall("Internal_SetGenericCurves", (void*)&ScriptAnimationCurves::InternalSetGenericCurves);

	}

	MonoObject* ScriptAnimationCurves::Create(const SPtr<AnimationCurves>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptAnimationCurves>()) ScriptAnimationCurves(managedInstance, value);
		return managedInstance;
	}
	void ScriptAnimationCurves::InternalAnimationCurves(MonoObject* managedInstance)
	{
		SPtr<AnimationCurves> instance = B3DMakeShared<AnimationCurves>();
		new (B3DAllocate<ScriptAnimationCurves>())ScriptAnimationCurves(managedInstance, instance);
	}

	void ScriptAnimationCurves::InternalAddPositionCurve(ScriptAnimationCurves* thisPtr, MonoString* name, MonoObject* curve)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		SPtr<TAnimationCurve<TVector3<float>>> tmpcurve;
		ScriptVector3Curve* scriptcurve;
		scriptcurve = ScriptVector3Curve::ToNative(curve);
		if(scriptcurve != nullptr)
			tmpcurve = scriptcurve->GetInternal();
		thisPtr->GetInternal()->AddPositionCurve(tmpname, *tmpcurve);
	}

	void ScriptAnimationCurves::InternalAddRotationCurve(ScriptAnimationCurves* thisPtr, MonoString* name, MonoObject* curve)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		SPtr<TAnimationCurve<Quaternion>> tmpcurve;
		ScriptQuaternionCurve* scriptcurve;
		scriptcurve = ScriptQuaternionCurve::ToNative(curve);
		if(scriptcurve != nullptr)
			tmpcurve = scriptcurve->GetInternal();
		thisPtr->GetInternal()->AddRotationCurve(tmpname, *tmpcurve);
	}

	void ScriptAnimationCurves::InternalAddScaleCurve(ScriptAnimationCurves* thisPtr, MonoString* name, MonoObject* curve)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		SPtr<TAnimationCurve<TVector3<float>>> tmpcurve;
		ScriptVector3Curve* scriptcurve;
		scriptcurve = ScriptVector3Curve::ToNative(curve);
		if(scriptcurve != nullptr)
			tmpcurve = scriptcurve->GetInternal();
		thisPtr->GetInternal()->AddScaleCurve(tmpname, *tmpcurve);
	}

	void ScriptAnimationCurves::InternalAddGenericCurve(ScriptAnimationCurves* thisPtr, MonoString* name, MonoObject* curve)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		SPtr<TAnimationCurve<float>> tmpcurve;
		ScriptAnimationCurve* scriptcurve;
		scriptcurve = ScriptAnimationCurve::ToNative(curve);
		if(scriptcurve != nullptr)
			tmpcurve = scriptcurve->GetInternal();
		thisPtr->GetInternal()->AddGenericCurve(tmpname, *tmpcurve);
	}

	void ScriptAnimationCurves::InternalRemovePositionCurve(ScriptAnimationCurves* thisPtr, MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		thisPtr->GetInternal()->RemovePositionCurve(tmpname);
	}

	void ScriptAnimationCurves::InternalRemoveRotationCurve(ScriptAnimationCurves* thisPtr, MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		thisPtr->GetInternal()->RemoveRotationCurve(tmpname);
	}

	void ScriptAnimationCurves::InternalRemoveScaleCurve(ScriptAnimationCurves* thisPtr, MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		thisPtr->GetInternal()->RemoveScaleCurve(tmpname);
	}

	void ScriptAnimationCurves::InternalRemoveGenericCurve(ScriptAnimationCurves* thisPtr, MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		thisPtr->GetInternal()->RemoveGenericCurve(tmpname);
	}

	MonoArray* ScriptAnimationCurves::InternalGetPositionCurves(ScriptAnimationCurves* thisPtr)
	{
		Vector<TNamedAnimationCurve<TVector3<float>>> vec__output;
		vec__output = AnimationCurvesEx::GetPositionCurves(thisPtr->GetInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptNamedVector3Curve>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptNamedVector3Curve::ToInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptAnimationCurves::InternalSetPositionCurves(ScriptAnimationCurves* thisPtr, MonoArray* value)
	{
		Vector<TNamedAnimationCurve<TVector3<float>>> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.Size());
			for(int i = 0; i < (int)arrayvalue.Size(); i++)
			{
				vecvalue[i] = ScriptNamedVector3Curve::FromInterop(arrayvalue.Get<__TNamedAnimationCurve_TVector3_float__Interop>(i));
			}
		}
		AnimationCurvesEx::SetPositionCurves(thisPtr->GetInternal(), vecvalue);
	}

	MonoArray* ScriptAnimationCurves::InternalGetRotationCurves(ScriptAnimationCurves* thisPtr)
	{
		Vector<TNamedAnimationCurve<Quaternion>> vec__output;
		vec__output = AnimationCurvesEx::GetRotationCurves(thisPtr->GetInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptNamedQuaternionCurve>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptNamedQuaternionCurve::ToInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptAnimationCurves::InternalSetRotationCurves(ScriptAnimationCurves* thisPtr, MonoArray* value)
	{
		Vector<TNamedAnimationCurve<Quaternion>> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.Size());
			for(int i = 0; i < (int)arrayvalue.Size(); i++)
			{
				vecvalue[i] = ScriptNamedQuaternionCurve::FromInterop(arrayvalue.Get<__TNamedAnimationCurve_Quaternion_Interop>(i));
			}
		}
		AnimationCurvesEx::SetRotationCurves(thisPtr->GetInternal(), vecvalue);
	}

	MonoArray* ScriptAnimationCurves::InternalGetScaleCurves(ScriptAnimationCurves* thisPtr)
	{
		Vector<TNamedAnimationCurve<TVector3<float>>> vec__output;
		vec__output = AnimationCurvesEx::GetScaleCurves(thisPtr->GetInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptNamedVector3Curve>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptNamedVector3Curve::ToInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptAnimationCurves::InternalSetScaleCurves(ScriptAnimationCurves* thisPtr, MonoArray* value)
	{
		Vector<TNamedAnimationCurve<TVector3<float>>> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.Size());
			for(int i = 0; i < (int)arrayvalue.Size(); i++)
			{
				vecvalue[i] = ScriptNamedVector3Curve::FromInterop(arrayvalue.Get<__TNamedAnimationCurve_TVector3_float__Interop>(i));
			}
		}
		AnimationCurvesEx::SetScaleCurves(thisPtr->GetInternal(), vecvalue);
	}

	MonoArray* ScriptAnimationCurves::InternalGetGenericCurves(ScriptAnimationCurves* thisPtr)
	{
		Vector<TNamedAnimationCurve<float>> vec__output;
		vec__output = AnimationCurvesEx::GetGenericCurves(thisPtr->GetInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptNamedFloatCurve>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptNamedFloatCurve::ToInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptAnimationCurves::InternalSetGenericCurves(ScriptAnimationCurves* thisPtr, MonoArray* value)
	{
		Vector<TNamedAnimationCurve<float>> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.Size());
			for(int i = 0; i < (int)arrayvalue.Size(); i++)
			{
				vecvalue[i] = ScriptNamedFloatCurve::FromInterop(arrayvalue.Get<__TNamedAnimationCurve_float_Interop>(i));
			}
		}
		AnimationCurvesEx::SetGenericCurves(thisPtr->GetInternal(), vecvalue);
	}
}
