//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptAnimationCurves.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Animation/B3DAnimationClip.h"
#include "../Extensions/B3DAnimationEx.h"
#include "B3DScriptTAnimationCurve.generated.h"
#include "B3DScriptTAnimationCurve.generated.h"
#include "B3DScriptTAnimationCurve.generated.h"
#include "B3DScriptTNamedAnimationCurve.generated.h"
#include "B3DScriptTNamedAnimationCurve.generated.h"
#include "B3DScriptTNamedAnimationCurve.generated.h"

namespace b3d
{
	ScriptAnimationCurves::ScriptAnimationCurves(const TShared<AnimationCurves>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptAnimationCurves::~ScriptAnimationCurves()
	{
		UnregisterEvents();
	}

	void ScriptAnimationCurves::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_AnimationCurves", (void*)&ScriptAnimationCurves::InternalAnimationCurves);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_AddPositionCurve", (void*)&ScriptAnimationCurves::InternalAddPositionCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_AddRotationCurve", (void*)&ScriptAnimationCurves::InternalAddRotationCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_AddScaleCurve", (void*)&ScriptAnimationCurves::InternalAddScaleCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_AddGenericCurve", (void*)&ScriptAnimationCurves::InternalAddGenericCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RemovePositionCurve", (void*)&ScriptAnimationCurves::InternalRemovePositionCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RemoveRotationCurve", (void*)&ScriptAnimationCurves::InternalRemoveRotationCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RemoveScaleCurve", (void*)&ScriptAnimationCurves::InternalRemoveScaleCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RemoveGenericCurve", (void*)&ScriptAnimationCurves::InternalRemoveGenericCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPositionCurves", (void*)&ScriptAnimationCurves::InternalGetPositionCurves);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPositionCurves", (void*)&ScriptAnimationCurves::InternalSetPositionCurves);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRotationCurves", (void*)&ScriptAnimationCurves::InternalGetRotationCurves);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRotationCurves", (void*)&ScriptAnimationCurves::InternalSetRotationCurves);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetScaleCurves", (void*)&ScriptAnimationCurves::InternalGetScaleCurves);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetScaleCurves", (void*)&ScriptAnimationCurves::InternalSetScaleCurves);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetGenericCurves", (void*)&ScriptAnimationCurves::InternalGetGenericCurves);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetGenericCurves", (void*)&ScriptAnimationCurves::InternalSetGenericCurves);

	}

	MonoObject* ScriptAnimationCurves::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptAnimationCurves::InternalAnimationCurves(MonoObject* scriptObject)
	{
		TShared<AnimationCurves> nativeObject = B3DMakeShared<AnimationCurves>();
		ScriptObjectWrapper::Create<ScriptAnimationCurves>(nativeObject, scriptObject);
	}

	void ScriptAnimationCurves::InternalAddPositionCurve(ScriptAnimationCurves* self, MonoString* name, MonoObject* curve)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TShared<TAnimationCurve<TVector3<float>>> tmpcurve;
		ScriptVector3Curve* scriptObjectWrappercurve;
		scriptObjectWrappercurve = ScriptVector3Curve::GetScriptObjectWrapper(curve);
		if(scriptObjectWrappercurve != nullptr)
			tmpcurve = std::static_pointer_cast<TAnimationCurve<TVector3<float>>>(scriptObjectWrappercurve->GetBaseNativeObjectAsShared());
		static_cast<AnimationCurves*>(self->GetNativeObject())->AddPositionCurve(tmpname, *tmpcurve);
	}

	void ScriptAnimationCurves::InternalAddRotationCurve(ScriptAnimationCurves* self, MonoString* name, MonoObject* curve)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TShared<TAnimationCurve<TQuaternion<float>>> tmpcurve;
		ScriptQuaternionCurve* scriptObjectWrappercurve;
		scriptObjectWrappercurve = ScriptQuaternionCurve::GetScriptObjectWrapper(curve);
		if(scriptObjectWrappercurve != nullptr)
			tmpcurve = std::static_pointer_cast<TAnimationCurve<TQuaternion<float>>>(scriptObjectWrappercurve->GetBaseNativeObjectAsShared());
		static_cast<AnimationCurves*>(self->GetNativeObject())->AddRotationCurve(tmpname, *tmpcurve);
	}

	void ScriptAnimationCurves::InternalAddScaleCurve(ScriptAnimationCurves* self, MonoString* name, MonoObject* curve)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TShared<TAnimationCurve<TVector3<float>>> tmpcurve;
		ScriptVector3Curve* scriptObjectWrappercurve;
		scriptObjectWrappercurve = ScriptVector3Curve::GetScriptObjectWrapper(curve);
		if(scriptObjectWrappercurve != nullptr)
			tmpcurve = std::static_pointer_cast<TAnimationCurve<TVector3<float>>>(scriptObjectWrappercurve->GetBaseNativeObjectAsShared());
		static_cast<AnimationCurves*>(self->GetNativeObject())->AddScaleCurve(tmpname, *tmpcurve);
	}

	void ScriptAnimationCurves::InternalAddGenericCurve(ScriptAnimationCurves* self, MonoString* name, MonoObject* curve)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TShared<TAnimationCurve<float>> tmpcurve;
		ScriptAnimationCurve* scriptObjectWrappercurve;
		scriptObjectWrappercurve = ScriptAnimationCurve::GetScriptObjectWrapper(curve);
		if(scriptObjectWrappercurve != nullptr)
			tmpcurve = std::static_pointer_cast<TAnimationCurve<float>>(scriptObjectWrappercurve->GetBaseNativeObjectAsShared());
		static_cast<AnimationCurves*>(self->GetNativeObject())->AddGenericCurve(tmpname, *tmpcurve);
	}

	void ScriptAnimationCurves::InternalRemovePositionCurve(ScriptAnimationCurves* self, MonoString* name)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<AnimationCurves*>(self->GetNativeObject())->RemovePositionCurve(tmpname);
	}

	void ScriptAnimationCurves::InternalRemoveRotationCurve(ScriptAnimationCurves* self, MonoString* name)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<AnimationCurves*>(self->GetNativeObject())->RemoveRotationCurve(tmpname);
	}

	void ScriptAnimationCurves::InternalRemoveScaleCurve(ScriptAnimationCurves* self, MonoString* name)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<AnimationCurves*>(self->GetNativeObject())->RemoveScaleCurve(tmpname);
	}

	void ScriptAnimationCurves::InternalRemoveGenericCurve(ScriptAnimationCurves* self, MonoString* name)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<AnimationCurves*>(self->GetNativeObject())->RemoveGenericCurve(tmpname);
	}

	MonoArray* ScriptAnimationCurves::InternalGetPositionCurves(ScriptAnimationCurves* self)
	{
		Vector<TNamedAnimationCurve<TVector3<float>>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = AnimationCurvesEx::GetPositionCurves(std::static_pointer_cast<AnimationCurves>(self->GetBaseNativeObjectAsShared()));

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
		if(!self->IsNativeObjectValid())
			return;

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
		AnimationCurvesEx::SetPositionCurves(std::static_pointer_cast<AnimationCurves>(self->GetBaseNativeObjectAsShared()), nativeArrayvalue);
	}

	MonoArray* ScriptAnimationCurves::InternalGetRotationCurves(ScriptAnimationCurves* self)
	{
		Vector<TNamedAnimationCurve<TQuaternion<float>>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = AnimationCurvesEx::GetRotationCurves(std::static_pointer_cast<AnimationCurves>(self->GetBaseNativeObjectAsShared()));

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
		if(!self->IsNativeObjectValid())
			return;

		Vector<TNamedAnimationCurve<TQuaternion<float>>> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = ScriptNamedQuaternionCurve::FromInterop(scriptArrayvalue.Get<__TNamedAnimationCurve_TQuaternion_float__Interop>(elementIndex));
			}
		}
		AnimationCurvesEx::SetRotationCurves(std::static_pointer_cast<AnimationCurves>(self->GetBaseNativeObjectAsShared()), nativeArrayvalue);
	}

	MonoArray* ScriptAnimationCurves::InternalGetScaleCurves(ScriptAnimationCurves* self)
	{
		Vector<TNamedAnimationCurve<TVector3<float>>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = AnimationCurvesEx::GetScaleCurves(std::static_pointer_cast<AnimationCurves>(self->GetBaseNativeObjectAsShared()));

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
		if(!self->IsNativeObjectValid())
			return;

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
		AnimationCurvesEx::SetScaleCurves(std::static_pointer_cast<AnimationCurves>(self->GetBaseNativeObjectAsShared()), nativeArrayvalue);
	}

	MonoArray* ScriptAnimationCurves::InternalGetGenericCurves(ScriptAnimationCurves* self)
	{
		Vector<TNamedAnimationCurve<float>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = AnimationCurvesEx::GetGenericCurves(std::static_pointer_cast<AnimationCurves>(self->GetBaseNativeObjectAsShared()));

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
		if(!self->IsNativeObjectValid())
			return;

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
		AnimationCurvesEx::SetGenericCurves(std::static_pointer_cast<AnimationCurves>(self->GetBaseNativeObjectAsShared()), nativeArrayvalue);
	}
}
