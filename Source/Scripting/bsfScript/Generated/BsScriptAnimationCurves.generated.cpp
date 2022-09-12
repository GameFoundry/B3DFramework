//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
		metaData.scriptClass->AddInternalCall("Internal_AnimationCurves", (void*)&ScriptAnimationCurves::Internal_AnimationCurves);
		metaData.scriptClass->AddInternalCall("Internal_addPositionCurve", (void*)&ScriptAnimationCurves::Internal_addPositionCurve);
		metaData.scriptClass->AddInternalCall("Internal_addRotationCurve", (void*)&ScriptAnimationCurves::Internal_addRotationCurve);
		metaData.scriptClass->AddInternalCall("Internal_addScaleCurve", (void*)&ScriptAnimationCurves::Internal_addScaleCurve);
		metaData.scriptClass->AddInternalCall("Internal_addGenericCurve", (void*)&ScriptAnimationCurves::Internal_addGenericCurve);
		metaData.scriptClass->AddInternalCall("Internal_removePositionCurve", (void*)&ScriptAnimationCurves::Internal_removePositionCurve);
		metaData.scriptClass->AddInternalCall("Internal_removeRotationCurve", (void*)&ScriptAnimationCurves::Internal_removeRotationCurve);
		metaData.scriptClass->AddInternalCall("Internal_removeScaleCurve", (void*)&ScriptAnimationCurves::Internal_removeScaleCurve);
		metaData.scriptClass->AddInternalCall("Internal_removeGenericCurve", (void*)&ScriptAnimationCurves::Internal_removeGenericCurve);
		metaData.scriptClass->AddInternalCall("Internal_getPositionCurves", (void*)&ScriptAnimationCurves::Internal_getPositionCurves);
		metaData.scriptClass->AddInternalCall("Internal_setPositionCurves", (void*)&ScriptAnimationCurves::Internal_setPositionCurves);
		metaData.scriptClass->AddInternalCall("Internal_getRotationCurves", (void*)&ScriptAnimationCurves::Internal_getRotationCurves);
		metaData.scriptClass->AddInternalCall("Internal_setRotationCurves", (void*)&ScriptAnimationCurves::Internal_setRotationCurves);
		metaData.scriptClass->AddInternalCall("Internal_getScaleCurves", (void*)&ScriptAnimationCurves::Internal_getScaleCurves);
		metaData.scriptClass->AddInternalCall("Internal_setScaleCurves", (void*)&ScriptAnimationCurves::Internal_setScaleCurves);
		metaData.scriptClass->AddInternalCall("Internal_getGenericCurves", (void*)&ScriptAnimationCurves::Internal_getGenericCurves);
		metaData.scriptClass->AddInternalCall("Internal_setGenericCurves", (void*)&ScriptAnimationCurves::Internal_setGenericCurves);

	}

	MonoObject* ScriptAnimationCurves::create(const SPtr<AnimationCurves>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptAnimationCurves>()) ScriptAnimationCurves(managedInstance, value);
		return managedInstance;
	}
	void ScriptAnimationCurves::Internal_AnimationCurves(MonoObject* managedInstance)
	{
		SPtr<AnimationCurves> instance = bs_shared_ptr_new<AnimationCurves>();
		new (bs_alloc<ScriptAnimationCurves>())ScriptAnimationCurves(managedInstance, instance);
	}

	void ScriptAnimationCurves::Internal_addPositionCurve(ScriptAnimationCurves* thisPtr, MonoString* name, MonoObject* curve)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		SPtr<TAnimationCurve<Vector3>> tmpcurve;
		ScriptTAnimationCurveVector3* scriptcurve;
		scriptcurve = ScriptTAnimationCurveVector3::toNative(curve);
		if(scriptcurve != nullptr)
			tmpcurve = scriptcurve->GetInternal();
		thisPtr->GetInternal()->addPositionCurve(tmpname, *tmpcurve);
	}

	void ScriptAnimationCurves::Internal_addRotationCurve(ScriptAnimationCurves* thisPtr, MonoString* name, MonoObject* curve)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		SPtr<TAnimationCurve<Quaternion>> tmpcurve;
		ScriptTAnimationCurveQuaternion* scriptcurve;
		scriptcurve = ScriptTAnimationCurveQuaternion::toNative(curve);
		if(scriptcurve != nullptr)
			tmpcurve = scriptcurve->GetInternal();
		thisPtr->GetInternal()->addRotationCurve(tmpname, *tmpcurve);
	}

	void ScriptAnimationCurves::Internal_addScaleCurve(ScriptAnimationCurves* thisPtr, MonoString* name, MonoObject* curve)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		SPtr<TAnimationCurve<Vector3>> tmpcurve;
		ScriptTAnimationCurveVector3* scriptcurve;
		scriptcurve = ScriptTAnimationCurveVector3::toNative(curve);
		if(scriptcurve != nullptr)
			tmpcurve = scriptcurve->GetInternal();
		thisPtr->GetInternal()->addScaleCurve(tmpname, *tmpcurve);
	}

	void ScriptAnimationCurves::Internal_addGenericCurve(ScriptAnimationCurves* thisPtr, MonoString* name, MonoObject* curve)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		SPtr<TAnimationCurve<float>> tmpcurve;
		ScriptTAnimationCurvefloat* scriptcurve;
		scriptcurve = ScriptTAnimationCurvefloat::toNative(curve);
		if(scriptcurve != nullptr)
			tmpcurve = scriptcurve->GetInternal();
		thisPtr->GetInternal()->addGenericCurve(tmpname, *tmpcurve);
	}

	void ScriptAnimationCurves::Internal_removePositionCurve(ScriptAnimationCurves* thisPtr, MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		thisPtr->GetInternal()->removePositionCurve(tmpname);
	}

	void ScriptAnimationCurves::Internal_removeRotationCurve(ScriptAnimationCurves* thisPtr, MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		thisPtr->GetInternal()->removeRotationCurve(tmpname);
	}

	void ScriptAnimationCurves::Internal_removeScaleCurve(ScriptAnimationCurves* thisPtr, MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		thisPtr->GetInternal()->removeScaleCurve(tmpname);
	}

	void ScriptAnimationCurves::Internal_removeGenericCurve(ScriptAnimationCurves* thisPtr, MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		thisPtr->GetInternal()->removeGenericCurve(tmpname);
	}

	MonoArray* ScriptAnimationCurves::Internal_getPositionCurves(ScriptAnimationCurves* thisPtr)
	{
		Vector<TNamedAnimationCurve<Vector3>> vec__output;
		vec__output = AnimationCurvesEx::getPositionCurves(thisPtr->GetInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptTNamedAnimationCurveVector3>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptTNamedAnimationCurveVector3::toInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptAnimationCurves::Internal_setPositionCurves(ScriptAnimationCurves* thisPtr, MonoArray* value)
	{
		Vector<TNamedAnimationCurve<Vector3>> vecvalue;
		if(value != nullptr)
		{
			ScriptArray Arrayvalue(value);
			vecvalue.Resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.Size(); i++)
			{
				vecvalue[i] = ScriptTNamedAnimationCurveVector3::fromInterop(arrayvalue.get<__TNamedAnimationCurveVector3Interop>(i));
			}
		}
		AnimationCurvesEx::setPositionCurves(thisPtr->GetInternal(), vecvalue);
	}

	MonoArray* ScriptAnimationCurves::Internal_getRotationCurves(ScriptAnimationCurves* thisPtr)
	{
		Vector<TNamedAnimationCurve<Quaternion>> vec__output;
		vec__output = AnimationCurvesEx::getRotationCurves(thisPtr->GetInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptTNamedAnimationCurveQuaternion>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptTNamedAnimationCurveQuaternion::toInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptAnimationCurves::Internal_setRotationCurves(ScriptAnimationCurves* thisPtr, MonoArray* value)
	{
		Vector<TNamedAnimationCurve<Quaternion>> vecvalue;
		if(value != nullptr)
		{
			ScriptArray Arrayvalue(value);
			vecvalue.Resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.Size(); i++)
			{
				vecvalue[i] = ScriptTNamedAnimationCurveQuaternion::fromInterop(arrayvalue.get<__TNamedAnimationCurveQuaternionInterop>(i));
			}
		}
		AnimationCurvesEx::setRotationCurves(thisPtr->GetInternal(), vecvalue);
	}

	MonoArray* ScriptAnimationCurves::Internal_getScaleCurves(ScriptAnimationCurves* thisPtr)
	{
		Vector<TNamedAnimationCurve<Vector3>> vec__output;
		vec__output = AnimationCurvesEx::getScaleCurves(thisPtr->GetInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptTNamedAnimationCurveVector3>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptTNamedAnimationCurveVector3::toInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptAnimationCurves::Internal_setScaleCurves(ScriptAnimationCurves* thisPtr, MonoArray* value)
	{
		Vector<TNamedAnimationCurve<Vector3>> vecvalue;
		if(value != nullptr)
		{
			ScriptArray Arrayvalue(value);
			vecvalue.Resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.Size(); i++)
			{
				vecvalue[i] = ScriptTNamedAnimationCurveVector3::fromInterop(arrayvalue.get<__TNamedAnimationCurveVector3Interop>(i));
			}
		}
		AnimationCurvesEx::setScaleCurves(thisPtr->GetInternal(), vecvalue);
	}

	MonoArray* ScriptAnimationCurves::Internal_getGenericCurves(ScriptAnimationCurves* thisPtr)
	{
		Vector<TNamedAnimationCurve<float>> vec__output;
		vec__output = AnimationCurvesEx::getGenericCurves(thisPtr->GetInternal());

		MonoArray* __output;
		int arraySize__output = (int)vec__output.Size();
		ScriptArray array__output = ScriptArray::create<ScriptTNamedAnimationCurvefloat>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptTNamedAnimationCurvefloat::toInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptAnimationCurves::Internal_setGenericCurves(ScriptAnimationCurves* thisPtr, MonoArray* value)
	{
		Vector<TNamedAnimationCurve<float>> vecvalue;
		if(value != nullptr)
		{
			ScriptArray Arrayvalue(value);
			vecvalue.Resize(arrayvalue.size());
			for(int i = 0; i < (int)arrayvalue.Size(); i++)
			{
				vecvalue[i] = ScriptTNamedAnimationCurvefloat::fromInterop(arrayvalue.get<__TNamedAnimationCurvefloatInterop>(i));
			}
		}
		AnimationCurvesEx::setGenericCurves(thisPtr->GetInternal(), vecvalue);
	}
}
