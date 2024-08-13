//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTAnimationCurve.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptTKeyframe.generated.h"
#include "BsScriptTKeyframe.generated.h"
#include "BsScriptTKeyframe.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptTKeyframe.generated.h"
#include "BsScriptTKeyframe.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptQuaternion.h"

namespace bs
{
	ScriptAnimationCurve::ScriptAnimationCurve(MonoObject* managedInstance, const SPtr<TAnimationCurve<float>>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptAnimationCurve::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_TAnimationCurve", (void*)&ScriptAnimationCurve::InternalTAnimationCurve);
		metaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptAnimationCurve::InternalEvaluate);
		metaData.ScriptClass->AddInternalCall("Internal_GetKeyFrames", (void*)&ScriptAnimationCurve::InternalGetKeyFrames);

	}

	MonoObject* ScriptAnimationCurve::Create(const SPtr<TAnimationCurve<float>>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptAnimationCurve>()) ScriptAnimationCurve(managedInstance, value);
		return managedInstance;
	}
	void ScriptAnimationCurve::InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes)
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
		SPtr<TAnimationCurve<float>> nativeObject = B3DMakeShared<TAnimationCurve<float>>(nativeArraykeyframes);
		new (B3DAllocate<ScriptAnimationCurve>())ScriptAnimationCurve(managedInstance, nativeObject);
	}

	float ScriptAnimationCurve::InternalEvaluate(ScriptAnimationCurve* self, float time, bool loop)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Evaluate(time, loop);

		float __output;
		__output = tmp__output;

		return __output;
	}

	MonoArray* ScriptAnimationCurve::InternalGetKeyFrames(ScriptAnimationCurve* self)
	{
		Vector<TKeyframe<float>> nativeArray__output;
		nativeArray__output = self->GetInternal()->GetKeyFrames();

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

	ScriptVector3Curve::ScriptVector3Curve(MonoObject* managedInstance, const SPtr<TAnimationCurve<TVector3<float>>>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptVector3Curve::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_TAnimationCurve", (void*)&ScriptVector3Curve::InternalTAnimationCurve);
		metaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptVector3Curve::InternalEvaluate);
		metaData.ScriptClass->AddInternalCall("Internal_GetKeyFrames", (void*)&ScriptVector3Curve::InternalGetKeyFrames);

	}

	MonoObject* ScriptVector3Curve::Create(const SPtr<TAnimationCurve<TVector3<float>>>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptVector3Curve>()) ScriptVector3Curve(managedInstance, value);
		return managedInstance;
	}
	void ScriptVector3Curve::InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes)
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
		SPtr<TAnimationCurve<TVector3<float>>> nativeObject = B3DMakeShared<TAnimationCurve<TVector3<float>>>(nativeArraykeyframes);
		new (B3DAllocate<ScriptVector3Curve>())ScriptVector3Curve(managedInstance, nativeObject);
	}

	void ScriptVector3Curve::InternalEvaluate(ScriptVector3Curve* self, float time, bool loop, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetInternal()->Evaluate(time, loop);

		*__output = tmp__output;
	}

	MonoArray* ScriptVector3Curve::InternalGetKeyFrames(ScriptVector3Curve* self)
	{
		Vector<TKeyframe<TVector3<float>>> nativeArray__output;
		nativeArray__output = self->GetInternal()->GetKeyFrames();

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

	ScriptVector2Curve::ScriptVector2Curve(MonoObject* managedInstance, const SPtr<TAnimationCurve<TVector2<float>>>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptVector2Curve::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_TAnimationCurve", (void*)&ScriptVector2Curve::InternalTAnimationCurve);
		metaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptVector2Curve::InternalEvaluate);
		metaData.ScriptClass->AddInternalCall("Internal_GetKeyFrames", (void*)&ScriptVector2Curve::InternalGetKeyFrames);

	}

	MonoObject* ScriptVector2Curve::Create(const SPtr<TAnimationCurve<TVector2<float>>>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptVector2Curve>()) ScriptVector2Curve(managedInstance, value);
		return managedInstance;
	}
	void ScriptVector2Curve::InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes)
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
		SPtr<TAnimationCurve<TVector2<float>>> nativeObject = B3DMakeShared<TAnimationCurve<TVector2<float>>>(nativeArraykeyframes);
		new (B3DAllocate<ScriptVector2Curve>())ScriptVector2Curve(managedInstance, nativeObject);
	}

	void ScriptVector2Curve::InternalEvaluate(ScriptVector2Curve* self, float time, bool loop, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		tmp__output = self->GetInternal()->Evaluate(time, loop);

		*__output = tmp__output;
	}

	MonoArray* ScriptVector2Curve::InternalGetKeyFrames(ScriptVector2Curve* self)
	{
		Vector<TKeyframe<TVector2<float>>> nativeArray__output;
		nativeArray__output = self->GetInternal()->GetKeyFrames();

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

	ScriptQuaternionCurve::ScriptQuaternionCurve(MonoObject* managedInstance, const SPtr<TAnimationCurve<Quaternion>>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptQuaternionCurve::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_TAnimationCurve", (void*)&ScriptQuaternionCurve::InternalTAnimationCurve);
		metaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptQuaternionCurve::InternalEvaluate);
		metaData.ScriptClass->AddInternalCall("Internal_GetKeyFrames", (void*)&ScriptQuaternionCurve::InternalGetKeyFrames);

	}

	MonoObject* ScriptQuaternionCurve::Create(const SPtr<TAnimationCurve<Quaternion>>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptQuaternionCurve>()) ScriptQuaternionCurve(managedInstance, value);
		return managedInstance;
	}
	void ScriptQuaternionCurve::InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes)
	{
		Vector<TKeyframe<Quaternion>> nativeArraykeyframes;
		if(keyframes != nullptr)
		{
			ScriptArray scriptArraykeyframes(keyframes);
			nativeArraykeyframes.resize(scriptArraykeyframes.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArraykeyframes.Size(); elementIndex++)
			{
				nativeArraykeyframes[elementIndex] = ScriptKeyFrameQuat::FromInterop(scriptArraykeyframes.Get<__TKeyframe_Quaternion_Interop>(elementIndex));
			}
		}
		SPtr<TAnimationCurve<Quaternion>> nativeObject = B3DMakeShared<TAnimationCurve<Quaternion>>(nativeArraykeyframes);
		new (B3DAllocate<ScriptQuaternionCurve>())ScriptQuaternionCurve(managedInstance, nativeObject);
	}

	void ScriptQuaternionCurve::InternalEvaluate(ScriptQuaternionCurve* self, float time, bool loop, Quaternion* __output)
	{
		Quaternion tmp__output;
		tmp__output = self->GetInternal()->Evaluate(time, loop);

		*__output = tmp__output;
	}

	MonoArray* ScriptQuaternionCurve::InternalGetKeyFrames(ScriptQuaternionCurve* self)
	{
		Vector<TKeyframe<Quaternion>> nativeArray__output;
		nativeArray__output = self->GetInternal()->GetKeyFrames();

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

	ScriptIntegerCurve::ScriptIntegerCurve(MonoObject* managedInstance, const SPtr<TAnimationCurve<int32_t>>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptIntegerCurve::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_TAnimationCurve", (void*)&ScriptIntegerCurve::InternalTAnimationCurve);
		metaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptIntegerCurve::InternalEvaluate);
		metaData.ScriptClass->AddInternalCall("Internal_GetKeyFrames", (void*)&ScriptIntegerCurve::InternalGetKeyFrames);

	}

	MonoObject* ScriptIntegerCurve::Create(const SPtr<TAnimationCurve<int32_t>>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptIntegerCurve>()) ScriptIntegerCurve(managedInstance, value);
		return managedInstance;
	}
	void ScriptIntegerCurve::InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes)
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
		SPtr<TAnimationCurve<int32_t>> nativeObject = B3DMakeShared<TAnimationCurve<int32_t>>(nativeArraykeyframes);
		new (B3DAllocate<ScriptIntegerCurve>())ScriptIntegerCurve(managedInstance, nativeObject);
	}

	int32_t ScriptIntegerCurve::InternalEvaluate(ScriptIntegerCurve* self, float time, bool loop)
	{
		int32_t tmp__output;
		tmp__output = self->GetInternal()->Evaluate(time, loop);

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoArray* ScriptIntegerCurve::InternalGetKeyFrames(ScriptIntegerCurve* self)
	{
		Vector<TKeyframe<int32_t>> nativeArray__output;
		nativeArray__output = self->GetInternal()->GetKeyFrames();

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
