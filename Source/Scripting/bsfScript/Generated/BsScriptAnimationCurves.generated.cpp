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
		SPtr<AnimationCurves> nativeObject = B3DMakeShared<AnimationCurves>();
		new (B3DAllocate<ScriptAnimationCurves>())ScriptAnimationCurves(managedInstance, nativeObject);
	}

	void ScriptAnimationCurves::InternalAddPositionCurve(ScriptAnimationCurves* self, MonoString* name, MonoObject* curve)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		SPtr<TAnimationCurve<TVector3<float>>> tmpcurve;
		ScriptVector3Curve* scriptObjectWrappercurve;
		scriptObjectWrappercurve = ScriptVector3Curve::ToNative(curve);
		if(scriptObjectWrappercurve != nullptr)
			tmpcurve = scriptObjectWrappercurve->GetInternal();
		self->GetInternal()->AddPositionCurve(tmpname, *tmpcurve);
	}

	void ScriptAnimationCurves::InternalAddRotationCurve(ScriptAnimationCurves* self, MonoString* name, MonoObject* curve)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		SPtr<TAnimationCurve<Quaternion>> tmpcurve;
		ScriptQuaternionCurve* scriptObjectWrappercurve;
		scriptObjectWrappercurve = ScriptQuaternionCurve::ToNative(curve);
		if(scriptObjectWrappercurve != nullptr)
			tmpcurve = scriptObjectWrappercurve->GetInternal();
		self->GetInternal()->AddRotationCurve(tmpname, *tmpcurve);
	}

	void ScriptAnimationCurves::InternalAddScaleCurve(ScriptAnimationCurves* self, MonoString* name, MonoObject* curve)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		SPtr<TAnimationCurve<TVector3<float>>> tmpcurve;
		ScriptVector3Curve* scriptObjectWrappercurve;
		scriptObjectWrappercurve = ScriptVector3Curve::ToNative(curve);
		if(scriptObjectWrappercurve != nullptr)
			tmpcurve = scriptObjectWrappercurve->GetInternal();
		self->GetInternal()->AddScaleCurve(tmpname, *tmpcurve);
	}

	void ScriptAnimationCurves::InternalAddGenericCurve(ScriptAnimationCurves* self, MonoString* name, MonoObject* curve)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		SPtr<TAnimationCurve<float>> tmpcurve;
		ScriptAnimationCurve* scriptObjectWrappercurve;
		scriptObjectWrappercurve = ScriptAnimationCurve::ToNative(curve);
		if(scriptObjectWrappercurve != nullptr)
			tmpcurve = scriptObjectWrappercurve->GetInternal();
		self->GetInternal()->AddGenericCurve(tmpname, *tmpcurve);
	}

	void ScriptAnimationCurves::InternalRemovePositionCurve(ScriptAnimationCurves* self, MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		self->GetInternal()->RemovePositionCurve(tmpname);
	}

	void ScriptAnimationCurves::InternalRemoveRotationCurve(ScriptAnimationCurves* self, MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		self->GetInternal()->RemoveRotationCurve(tmpname);
	}

	void ScriptAnimationCurves::InternalRemoveScaleCurve(ScriptAnimationCurves* self, MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		self->GetInternal()->RemoveScaleCurve(tmpname);
	}

	void ScriptAnimationCurves::InternalRemoveGenericCurve(ScriptAnimationCurves* self, MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		self->GetInternal()->RemoveGenericCurve(tmpname);
	}

	MonoArray* ScriptAnimationCurves::InternalGetPositionCurves(ScriptAnimationCurves* self)
	{
		Vector<TNamedAnimationCurve<TVector3<float>>> nativeArray__output;
		nativeArray__output = AnimationCurvesEx::GetPositionCurves(self->GetInternal());

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptNamedVector3Curve>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptNamedVector3Curve::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptAnimationCurves::InternalSetPositionCurves(ScriptAnimationCurves* self, MonoArray* value)
	{
		Vector<TNamedAnimationCurve<TVector3<float>>> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = ScriptNamedVector3Curve::FromInterop(scriptArrayvalue.Get<__TNamedAnimationCurve_TVector3_float__Interop>(elementIndex));
			}
		}
		AnimationCurvesEx::SetPositionCurves(self->GetInternal(), nativeArrayvalue);
	}

	MonoArray* ScriptAnimationCurves::InternalGetRotationCurves(ScriptAnimationCurves* self)
	{
		Vector<TNamedAnimationCurve<Quaternion>> nativeArray__output;
		nativeArray__output = AnimationCurvesEx::GetRotationCurves(self->GetInternal());

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptNamedQuaternionCurve>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptNamedQuaternionCurve::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptAnimationCurves::InternalSetRotationCurves(ScriptAnimationCurves* self, MonoArray* value)
	{
		Vector<TNamedAnimationCurve<Quaternion>> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = ScriptNamedQuaternionCurve::FromInterop(scriptArrayvalue.Get<__TNamedAnimationCurve_Quaternion_Interop>(elementIndex));
			}
		}
		AnimationCurvesEx::SetRotationCurves(self->GetInternal(), nativeArrayvalue);
	}

	MonoArray* ScriptAnimationCurves::InternalGetScaleCurves(ScriptAnimationCurves* self)
	{
		Vector<TNamedAnimationCurve<TVector3<float>>> nativeArray__output;
		nativeArray__output = AnimationCurvesEx::GetScaleCurves(self->GetInternal());

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptNamedVector3Curve>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptNamedVector3Curve::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptAnimationCurves::InternalSetScaleCurves(ScriptAnimationCurves* self, MonoArray* value)
	{
		Vector<TNamedAnimationCurve<TVector3<float>>> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = ScriptNamedVector3Curve::FromInterop(scriptArrayvalue.Get<__TNamedAnimationCurve_TVector3_float__Interop>(elementIndex));
			}
		}
		AnimationCurvesEx::SetScaleCurves(self->GetInternal(), nativeArrayvalue);
	}

	MonoArray* ScriptAnimationCurves::InternalGetGenericCurves(ScriptAnimationCurves* self)
	{
		Vector<TNamedAnimationCurve<float>> nativeArray__output;
		nativeArray__output = AnimationCurvesEx::GetGenericCurves(self->GetInternal());

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptNamedFloatCurve>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptNamedFloatCurve::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptAnimationCurves::InternalSetGenericCurves(ScriptAnimationCurves* self, MonoArray* value)
	{
		Vector<TNamedAnimationCurve<float>> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = ScriptNamedFloatCurve::FromInterop(scriptArrayvalue.Get<__TNamedAnimationCurve_float_Interop>(elementIndex));
			}
		}
		AnimationCurvesEx::SetGenericCurves(self->GetInternal(), nativeArrayvalue);
	}
}
