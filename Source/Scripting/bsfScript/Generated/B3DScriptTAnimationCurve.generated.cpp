//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptTAnimationCurve.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptTQuaternion.generated.h"
#include "B3DScriptTKeyframe.generated.h"
#include "B3DScriptTKeyframe.generated.h"
#include "B3DScriptTVector3.generated.h"
#include "B3DScriptTKeyframe.generated.h"
#include "B3DScriptTKeyframe.generated.h"
#include "B3DScriptTVector2.generated.h"
#include "B3DScriptTKeyframe.generated.h"

namespace b3d
{
	ScriptAnimationCurve::ScriptAnimationCurve(const TShared<TAnimationCurve<float>>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptAnimationCurve::~ScriptAnimationCurve()
	{
		UnregisterEvents();
	}

	void ScriptAnimationCurve::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TAnimationCurve", (void*)&ScriptAnimationCurve::InternalTAnimationCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptAnimationCurve::InternalEvaluate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetKeyFrames", (void*)&ScriptAnimationCurve::InternalGetKeyFrames);

	}

	MonoObject* ScriptAnimationCurve::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptAnimationCurve::InternalTAnimationCurve(MonoObject* scriptObject, MonoArray* keyframes)
	{
		Vector<TKeyframe<float>> nativeArraykeyframes;
		if(keyframes != nullptr)
		{
			ScriptArray scriptArraykeyframes(keyframes);
			nativeArraykeyframes.resize(scriptArraykeyframes.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraykeyframes.Size(); elementIndex++)
			{
				nativeArraykeyframes[elementIndex] = scriptArraykeyframes.Get<TKeyframe<float>>(elementIndex);
			}
		}
		TShared<TAnimationCurve<float>> nativeObject = B3DMakeShared<TAnimationCurve<float>>(nativeArraykeyframes);
		ScriptObjectWrapper::Create<ScriptAnimationCurve>(nativeObject, scriptObject);
	}

	float ScriptAnimationCurve::InternalEvaluate(ScriptAnimationCurve* self, float time, bool loop)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TAnimationCurve<float>*>(self->GetNativeObject())->Evaluate(time, loop);

		float __output;
		__output = tmp__output;

		return __output;
	}

	MonoArray* ScriptAnimationCurve::InternalGetKeyFrames(ScriptAnimationCurve* self)
	{
		Vector<TKeyframe<float>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<TAnimationCurve<float>*>(self->GetNativeObject())->GetKeyFrames();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptKeyFrame>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	ScriptVector3Curve::ScriptVector3Curve(const TShared<TAnimationCurve<TVector3<float>>>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptVector3Curve::~ScriptVector3Curve()
	{
		UnregisterEvents();
	}

	void ScriptVector3Curve::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TAnimationCurve", (void*)&ScriptVector3Curve::InternalTAnimationCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptVector3Curve::InternalEvaluate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetKeyFrames", (void*)&ScriptVector3Curve::InternalGetKeyFrames);

	}

	MonoObject* ScriptVector3Curve::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptVector3Curve::InternalTAnimationCurve(MonoObject* scriptObject, MonoArray* keyframes)
	{
		Vector<TKeyframe<TVector3<float>>> nativeArraykeyframes;
		if(keyframes != nullptr)
		{
			ScriptArray scriptArraykeyframes(keyframes);
			nativeArraykeyframes.resize(scriptArraykeyframes.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraykeyframes.Size(); elementIndex++)
			{
				nativeArraykeyframes[elementIndex] = ScriptKeyFrameVec3::FromInterop(scriptArraykeyframes.Get<__TKeyframe_TVector3_float__Interop>(elementIndex));
			}
		}
		TShared<TAnimationCurve<TVector3<float>>> nativeObject = B3DMakeShared<TAnimationCurve<TVector3<float>>>(nativeArraykeyframes);
		ScriptObjectWrapper::Create<ScriptVector3Curve>(nativeObject, scriptObject);
	}

	void ScriptVector3Curve::InternalEvaluate(ScriptVector3Curve* self, float time, bool loop, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<TAnimationCurve<TVector3<float>>*>(self->GetNativeObject())->Evaluate(time, loop);

		*__output = tmp__output;
	}

	MonoArray* ScriptVector3Curve::InternalGetKeyFrames(ScriptVector3Curve* self)
	{
		Vector<TKeyframe<TVector3<float>>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<TAnimationCurve<TVector3<float>>*>(self->GetNativeObject())->GetKeyFrames();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptKeyFrameVec3>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptKeyFrameVec3::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	ScriptVector2Curve::ScriptVector2Curve(const TShared<TAnimationCurve<TVector2<float>>>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptVector2Curve::~ScriptVector2Curve()
	{
		UnregisterEvents();
	}

	void ScriptVector2Curve::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TAnimationCurve", (void*)&ScriptVector2Curve::InternalTAnimationCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptVector2Curve::InternalEvaluate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetKeyFrames", (void*)&ScriptVector2Curve::InternalGetKeyFrames);

	}

	MonoObject* ScriptVector2Curve::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptVector2Curve::InternalTAnimationCurve(MonoObject* scriptObject, MonoArray* keyframes)
	{
		Vector<TKeyframe<TVector2<float>>> nativeArraykeyframes;
		if(keyframes != nullptr)
		{
			ScriptArray scriptArraykeyframes(keyframes);
			nativeArraykeyframes.resize(scriptArraykeyframes.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraykeyframes.Size(); elementIndex++)
			{
				nativeArraykeyframes[elementIndex] = ScriptKeyFrameVec2::FromInterop(scriptArraykeyframes.Get<__TKeyframe_TVector2_float__Interop>(elementIndex));
			}
		}
		TShared<TAnimationCurve<TVector2<float>>> nativeObject = B3DMakeShared<TAnimationCurve<TVector2<float>>>(nativeArraykeyframes);
		ScriptObjectWrapper::Create<ScriptVector2Curve>(nativeObject, scriptObject);
	}

	void ScriptVector2Curve::InternalEvaluate(ScriptVector2Curve* self, float time, bool loop, TVector2<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector2<float> tmp__output;
		tmp__output = static_cast<TAnimationCurve<TVector2<float>>*>(self->GetNativeObject())->Evaluate(time, loop);

		*__output = tmp__output;
	}

	MonoArray* ScriptVector2Curve::InternalGetKeyFrames(ScriptVector2Curve* self)
	{
		Vector<TKeyframe<TVector2<float>>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<TAnimationCurve<TVector2<float>>*>(self->GetNativeObject())->GetKeyFrames();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptKeyFrameVec2>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptKeyFrameVec2::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	ScriptQuaternionCurve::ScriptQuaternionCurve(const TShared<TAnimationCurve<TQuaternion<float>>>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptQuaternionCurve::~ScriptQuaternionCurve()
	{
		UnregisterEvents();
	}

	void ScriptQuaternionCurve::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TAnimationCurve", (void*)&ScriptQuaternionCurve::InternalTAnimationCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptQuaternionCurve::InternalEvaluate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetKeyFrames", (void*)&ScriptQuaternionCurve::InternalGetKeyFrames);

	}

	MonoObject* ScriptQuaternionCurve::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptQuaternionCurve::InternalTAnimationCurve(MonoObject* scriptObject, MonoArray* keyframes)
	{
		Vector<TKeyframe<TQuaternion<float>>> nativeArraykeyframes;
		if(keyframes != nullptr)
		{
			ScriptArray scriptArraykeyframes(keyframes);
			nativeArraykeyframes.resize(scriptArraykeyframes.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraykeyframes.Size(); elementIndex++)
			{
				nativeArraykeyframes[elementIndex] = ScriptKeyFrameQuat::FromInterop(scriptArraykeyframes.Get<__TKeyframe_TQuaternion_float__Interop>(elementIndex));
			}
		}
		TShared<TAnimationCurve<TQuaternion<float>>> nativeObject = B3DMakeShared<TAnimationCurve<TQuaternion<float>>>(nativeArraykeyframes);
		ScriptObjectWrapper::Create<ScriptQuaternionCurve>(nativeObject, scriptObject);
	}

	void ScriptQuaternionCurve::InternalEvaluate(ScriptQuaternionCurve* self, float time, bool loop, TQuaternion<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TQuaternion<float> tmp__output;
		tmp__output = static_cast<TAnimationCurve<TQuaternion<float>>*>(self->GetNativeObject())->Evaluate(time, loop);

		*__output = tmp__output;
	}

	MonoArray* ScriptQuaternionCurve::InternalGetKeyFrames(ScriptQuaternionCurve* self)
	{
		Vector<TKeyframe<TQuaternion<float>>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<TAnimationCurve<TQuaternion<float>>*>(self->GetNativeObject())->GetKeyFrames();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptKeyFrameQuat>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptKeyFrameQuat::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	ScriptIntegerCurve::ScriptIntegerCurve(const TShared<TAnimationCurve<int32_t>>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptIntegerCurve::~ScriptIntegerCurve()
	{
		UnregisterEvents();
	}

	void ScriptIntegerCurve::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_TAnimationCurve", (void*)&ScriptIntegerCurve::InternalTAnimationCurve);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptIntegerCurve::InternalEvaluate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetKeyFrames", (void*)&ScriptIntegerCurve::InternalGetKeyFrames);

	}

	MonoObject* ScriptIntegerCurve::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptIntegerCurve::InternalTAnimationCurve(MonoObject* scriptObject, MonoArray* keyframes)
	{
		Vector<TKeyframe<int32_t>> nativeArraykeyframes;
		if(keyframes != nullptr)
		{
			ScriptArray scriptArraykeyframes(keyframes);
			nativeArraykeyframes.resize(scriptArraykeyframes.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraykeyframes.Size(); elementIndex++)
			{
				nativeArraykeyframes[elementIndex] = scriptArraykeyframes.Get<TKeyframe<int32_t>>(elementIndex);
			}
		}
		TShared<TAnimationCurve<int32_t>> nativeObject = B3DMakeShared<TAnimationCurve<int32_t>>(nativeArraykeyframes);
		ScriptObjectWrapper::Create<ScriptIntegerCurve>(nativeObject, scriptObject);
	}

	int32_t ScriptIntegerCurve::InternalEvaluate(ScriptIntegerCurve* self, float time, bool loop)
	{
		int32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<TAnimationCurve<int32_t>*>(self->GetNativeObject())->Evaluate(time, loop);

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoArray* ScriptIntegerCurve::InternalGetKeyFrames(ScriptIntegerCurve* self)
	{
		Vector<TKeyframe<int32_t>> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<TAnimationCurve<int32_t>*>(self->GetNativeObject())->GetKeyFrames();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptKeyFrameInt>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}
}
