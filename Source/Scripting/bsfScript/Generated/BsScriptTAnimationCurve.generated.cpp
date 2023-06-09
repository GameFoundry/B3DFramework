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
		Vector<TKeyframe<float>> veckeyframes;
		if(keyframes != nullptr)
		{
			ScriptArray arraykeyframes(keyframes);
			veckeyframes.resize(arraykeyframes.Size());
			for(int i = 0; i < (int)arraykeyframes.Size(); i++)
			{
				veckeyframes[i] = arraykeyframes.Get<TKeyframe<float>>(i);
			}
		}
		SPtr<TAnimationCurve<float>> instance = B3DMakeShared<TAnimationCurve<float>>(veckeyframes);
		new (B3DAllocate<ScriptAnimationCurve>())ScriptAnimationCurve(managedInstance, instance);
	}

	float ScriptAnimationCurve::InternalEvaluate(ScriptAnimationCurve* thisPtr, float time, bool loop)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Evaluate(time, loop);

		float __output;
		__output = tmp__output;

		return __output;
	}

	MonoArray* ScriptAnimationCurve::InternalGetKeyFrames(ScriptAnimationCurve* thisPtr)
	{
		Vector<TKeyframe<float>> vec__output;
		vec__output = thisPtr->GetInternal()->GetKeyFrames();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptKeyFrame>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

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
		Vector<TKeyframe<TVector3<float>>> veckeyframes;
		if(keyframes != nullptr)
		{
			ScriptArray arraykeyframes(keyframes);
			veckeyframes.resize(arraykeyframes.Size());
			for(int i = 0; i < (int)arraykeyframes.Size(); i++)
			{
				veckeyframes[i] = ScriptKeyFrameVec3::FromInterop(arraykeyframes.Get<__TKeyframe_TVector3_float__Interop>(i));
			}
		}
		SPtr<TAnimationCurve<TVector3<float>>> instance = B3DMakeShared<TAnimationCurve<TVector3<float>>>(veckeyframes);
		new (B3DAllocate<ScriptVector3Curve>())ScriptVector3Curve(managedInstance, instance);
	}

	void ScriptVector3Curve::InternalEvaluate(ScriptVector3Curve* thisPtr, float time, bool loop, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetInternal()->Evaluate(time, loop);

		*__output = tmp__output;
	}

	MonoArray* ScriptVector3Curve::InternalGetKeyFrames(ScriptVector3Curve* thisPtr)
	{
		Vector<TKeyframe<TVector3<float>>> vec__output;
		vec__output = thisPtr->GetInternal()->GetKeyFrames();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptKeyFrameVec3>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptKeyFrameVec3::ToInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

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
		Vector<TKeyframe<TVector2<float>>> veckeyframes;
		if(keyframes != nullptr)
		{
			ScriptArray arraykeyframes(keyframes);
			veckeyframes.resize(arraykeyframes.Size());
			for(int i = 0; i < (int)arraykeyframes.Size(); i++)
			{
				veckeyframes[i] = ScriptKeyFrameVec2::FromInterop(arraykeyframes.Get<__TKeyframe_TVector2_float__Interop>(i));
			}
		}
		SPtr<TAnimationCurve<TVector2<float>>> instance = B3DMakeShared<TAnimationCurve<TVector2<float>>>(veckeyframes);
		new (B3DAllocate<ScriptVector2Curve>())ScriptVector2Curve(managedInstance, instance);
	}

	void ScriptVector2Curve::InternalEvaluate(ScriptVector2Curve* thisPtr, float time, bool loop, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		tmp__output = thisPtr->GetInternal()->Evaluate(time, loop);

		*__output = tmp__output;
	}

	MonoArray* ScriptVector2Curve::InternalGetKeyFrames(ScriptVector2Curve* thisPtr)
	{
		Vector<TKeyframe<TVector2<float>>> vec__output;
		vec__output = thisPtr->GetInternal()->GetKeyFrames();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptKeyFrameVec2>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptKeyFrameVec2::ToInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

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
		Vector<TKeyframe<Quaternion>> veckeyframes;
		if(keyframes != nullptr)
		{
			ScriptArray arraykeyframes(keyframes);
			veckeyframes.resize(arraykeyframes.Size());
			for(int i = 0; i < (int)arraykeyframes.Size(); i++)
			{
				veckeyframes[i] = ScriptKeyFrameQuat::FromInterop(arraykeyframes.Get<__TKeyframe_Quaternion_Interop>(i));
			}
		}
		SPtr<TAnimationCurve<Quaternion>> instance = B3DMakeShared<TAnimationCurve<Quaternion>>(veckeyframes);
		new (B3DAllocate<ScriptQuaternionCurve>())ScriptQuaternionCurve(managedInstance, instance);
	}

	void ScriptQuaternionCurve::InternalEvaluate(ScriptQuaternionCurve* thisPtr, float time, bool loop, Quaternion* __output)
	{
		Quaternion tmp__output;
		tmp__output = thisPtr->GetInternal()->Evaluate(time, loop);

		*__output = tmp__output;
	}

	MonoArray* ScriptQuaternionCurve::InternalGetKeyFrames(ScriptQuaternionCurve* thisPtr)
	{
		Vector<TKeyframe<Quaternion>> vec__output;
		vec__output = thisPtr->GetInternal()->GetKeyFrames();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptKeyFrameQuat>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptKeyFrameQuat::ToInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

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
		Vector<TKeyframe<int32_t>> veckeyframes;
		if(keyframes != nullptr)
		{
			ScriptArray arraykeyframes(keyframes);
			veckeyframes.resize(arraykeyframes.Size());
			for(int i = 0; i < (int)arraykeyframes.Size(); i++)
			{
				veckeyframes[i] = arraykeyframes.Get<TKeyframe<int32_t>>(i);
			}
		}
		SPtr<TAnimationCurve<int32_t>> instance = B3DMakeShared<TAnimationCurve<int32_t>>(veckeyframes);
		new (B3DAllocate<ScriptIntegerCurve>())ScriptIntegerCurve(managedInstance, instance);
	}

	int32_t ScriptIntegerCurve::InternalEvaluate(ScriptIntegerCurve* thisPtr, float time, bool loop)
	{
		int32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->Evaluate(time, loop);

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoArray* ScriptIntegerCurve::InternalGetKeyFrames(ScriptIntegerCurve* thisPtr)
	{
		Vector<TKeyframe<int32_t>> vec__output;
		vec__output = thisPtr->GetInternal()->GetKeyFrames();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptKeyFrameInt>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

		return __output;
	}
}
