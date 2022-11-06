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
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptQuaternion.h"
#include "BsScriptTKeyframe.generated.h"

namespace bs
{
	ScriptTAnimationCurvefloat::ScriptTAnimationCurvefloat(MonoObject* managedInstance, const SPtr<TAnimationCurve<float>>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptTAnimationCurvefloat::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_TAnimationCurve", (void*)&ScriptTAnimationCurvefloat::InternalTAnimationCurve);
		metaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptTAnimationCurvefloat::InternalEvaluate);
		metaData.ScriptClass->AddInternalCall("Internal_GetKeyFrames", (void*)&ScriptTAnimationCurvefloat::InternalGetKeyFrames);

	}

	MonoObject* ScriptTAnimationCurvefloat::Create(const SPtr<TAnimationCurve<float>>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptTAnimationCurvefloat>()) ScriptTAnimationCurvefloat(managedInstance, value);
		return managedInstance;
	}
	void ScriptTAnimationCurvefloat::InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes)
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
		new (B3DAllocate<ScriptTAnimationCurvefloat>())ScriptTAnimationCurvefloat(managedInstance, instance);
	}

	float ScriptTAnimationCurvefloat::InternalEvaluate(ScriptTAnimationCurvefloat* thisPtr, float time, bool loop)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Evaluate(time, loop);

		float __output;
		__output = tmp__output;

		return __output;
	}

	MonoArray* ScriptTAnimationCurvefloat::InternalGetKeyFrames(ScriptTAnimationCurvefloat* thisPtr)
	{
		Vector<TKeyframe<float>> vec__output;
		vec__output = thisPtr->GetInternal()->GetKeyFrames();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptTKeyframefloat>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

		return __output;
	}

	ScriptTAnimationCurveVector3::ScriptTAnimationCurveVector3(MonoObject* managedInstance, const SPtr<TAnimationCurve<Vector3>>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptTAnimationCurveVector3::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_TAnimationCurve", (void*)&ScriptTAnimationCurveVector3::InternalTAnimationCurve);
		metaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptTAnimationCurveVector3::InternalEvaluate);
		metaData.ScriptClass->AddInternalCall("Internal_GetKeyFrames", (void*)&ScriptTAnimationCurveVector3::InternalGetKeyFrames);

	}

	MonoObject* ScriptTAnimationCurveVector3::Create(const SPtr<TAnimationCurve<Vector3>>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptTAnimationCurveVector3>()) ScriptTAnimationCurveVector3(managedInstance, value);
		return managedInstance;
	}
	void ScriptTAnimationCurveVector3::InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes)
	{
		Vector<TKeyframe<Vector3>> veckeyframes;
		if(keyframes != nullptr)
		{
			ScriptArray arraykeyframes(keyframes);
			veckeyframes.resize(arraykeyframes.Size());
			for(int i = 0; i < (int)arraykeyframes.Size(); i++)
			{
				veckeyframes[i] = ScriptTKeyframeVector3::FromInterop(arraykeyframes.Get<__TKeyframeVector3Interop>(i));
			}
		}
		SPtr<TAnimationCurve<Vector3>> instance = B3DMakeShared<TAnimationCurve<Vector3>>(veckeyframes);
		new (B3DAllocate<ScriptTAnimationCurveVector3>())ScriptTAnimationCurveVector3(managedInstance, instance);
	}

	void ScriptTAnimationCurveVector3::InternalEvaluate(ScriptTAnimationCurveVector3* thisPtr, float time, bool loop, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetInternal()->Evaluate(time, loop);

		*__output = tmp__output;
	}

	MonoArray* ScriptTAnimationCurveVector3::InternalGetKeyFrames(ScriptTAnimationCurveVector3* thisPtr)
	{
		Vector<TKeyframe<Vector3>> vec__output;
		vec__output = thisPtr->GetInternal()->GetKeyFrames();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptTKeyframeVector3>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptTKeyframeVector3::ToInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	ScriptTAnimationCurveVector2::ScriptTAnimationCurveVector2(MonoObject* managedInstance, const SPtr<TAnimationCurve<Vector2>>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptTAnimationCurveVector2::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_TAnimationCurve", (void*)&ScriptTAnimationCurveVector2::InternalTAnimationCurve);
		metaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptTAnimationCurveVector2::InternalEvaluate);
		metaData.ScriptClass->AddInternalCall("Internal_GetKeyFrames", (void*)&ScriptTAnimationCurveVector2::InternalGetKeyFrames);

	}

	MonoObject* ScriptTAnimationCurveVector2::Create(const SPtr<TAnimationCurve<Vector2>>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptTAnimationCurveVector2>()) ScriptTAnimationCurveVector2(managedInstance, value);
		return managedInstance;
	}
	void ScriptTAnimationCurveVector2::InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes)
	{
		Vector<TKeyframe<Vector2>> veckeyframes;
		if(keyframes != nullptr)
		{
			ScriptArray arraykeyframes(keyframes);
			veckeyframes.resize(arraykeyframes.Size());
			for(int i = 0; i < (int)arraykeyframes.Size(); i++)
			{
				veckeyframes[i] = ScriptTKeyframeVector2::FromInterop(arraykeyframes.Get<__TKeyframeVector2Interop>(i));
			}
		}
		SPtr<TAnimationCurve<Vector2>> instance = B3DMakeShared<TAnimationCurve<Vector2>>(veckeyframes);
		new (B3DAllocate<ScriptTAnimationCurveVector2>())ScriptTAnimationCurveVector2(managedInstance, instance);
	}

	void ScriptTAnimationCurveVector2::InternalEvaluate(ScriptTAnimationCurveVector2* thisPtr, float time, bool loop, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetInternal()->Evaluate(time, loop);

		*__output = tmp__output;
	}

	MonoArray* ScriptTAnimationCurveVector2::InternalGetKeyFrames(ScriptTAnimationCurveVector2* thisPtr)
	{
		Vector<TKeyframe<Vector2>> vec__output;
		vec__output = thisPtr->GetInternal()->GetKeyFrames();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptTKeyframeVector2>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptTKeyframeVector2::ToInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	ScriptTAnimationCurveQuaternion::ScriptTAnimationCurveQuaternion(MonoObject* managedInstance, const SPtr<TAnimationCurve<Quaternion>>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptTAnimationCurveQuaternion::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_TAnimationCurve", (void*)&ScriptTAnimationCurveQuaternion::InternalTAnimationCurve);
		metaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptTAnimationCurveQuaternion::InternalEvaluate);
		metaData.ScriptClass->AddInternalCall("Internal_GetKeyFrames", (void*)&ScriptTAnimationCurveQuaternion::InternalGetKeyFrames);

	}

	MonoObject* ScriptTAnimationCurveQuaternion::Create(const SPtr<TAnimationCurve<Quaternion>>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptTAnimationCurveQuaternion>()) ScriptTAnimationCurveQuaternion(managedInstance, value);
		return managedInstance;
	}
	void ScriptTAnimationCurveQuaternion::InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes)
	{
		Vector<TKeyframe<Quaternion>> veckeyframes;
		if(keyframes != nullptr)
		{
			ScriptArray arraykeyframes(keyframes);
			veckeyframes.resize(arraykeyframes.Size());
			for(int i = 0; i < (int)arraykeyframes.Size(); i++)
			{
				veckeyframes[i] = ScriptTKeyframeQuaternion::FromInterop(arraykeyframes.Get<__TKeyframeQuaternionInterop>(i));
			}
		}
		SPtr<TAnimationCurve<Quaternion>> instance = B3DMakeShared<TAnimationCurve<Quaternion>>(veckeyframes);
		new (B3DAllocate<ScriptTAnimationCurveQuaternion>())ScriptTAnimationCurveQuaternion(managedInstance, instance);
	}

	void ScriptTAnimationCurveQuaternion::InternalEvaluate(ScriptTAnimationCurveQuaternion* thisPtr, float time, bool loop, Quaternion* __output)
	{
		Quaternion tmp__output;
		tmp__output = thisPtr->GetInternal()->Evaluate(time, loop);

		*__output = tmp__output;
	}

	MonoArray* ScriptTAnimationCurveQuaternion::InternalGetKeyFrames(ScriptTAnimationCurveQuaternion* thisPtr)
	{
		Vector<TKeyframe<Quaternion>> vec__output;
		vec__output = thisPtr->GetInternal()->GetKeyFrames();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptTKeyframeQuaternion>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptTKeyframeQuaternion::ToInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	ScriptTAnimationCurveint32_t::ScriptTAnimationCurveint32_t(MonoObject* managedInstance, const SPtr<TAnimationCurve<int32_t>>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptTAnimationCurveint32_t::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_TAnimationCurve", (void*)&ScriptTAnimationCurveint32_t::InternalTAnimationCurve);
		metaData.ScriptClass->AddInternalCall("Internal_Evaluate", (void*)&ScriptTAnimationCurveint32_t::InternalEvaluate);
		metaData.ScriptClass->AddInternalCall("Internal_GetKeyFrames", (void*)&ScriptTAnimationCurveint32_t::InternalGetKeyFrames);

	}

	MonoObject* ScriptTAnimationCurveint32_t::Create(const SPtr<TAnimationCurve<int32_t>>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptTAnimationCurveint32_t>()) ScriptTAnimationCurveint32_t(managedInstance, value);
		return managedInstance;
	}
	void ScriptTAnimationCurveint32_t::InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes)
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
		new (B3DAllocate<ScriptTAnimationCurveint32_t>())ScriptTAnimationCurveint32_t(managedInstance, instance);
	}

	int32_t ScriptTAnimationCurveint32_t::InternalEvaluate(ScriptTAnimationCurveint32_t* thisPtr, float time, bool loop)
	{
		int32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->Evaluate(time, loop);

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoArray* ScriptTAnimationCurveint32_t::InternalGetKeyFrames(ScriptTAnimationCurveint32_t* thisPtr)
	{
		Vector<TKeyframe<int32_t>> vec__output;
		vec__output = thisPtr->GetInternal()->GetKeyFrames();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptTKeyframeint32_t>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, vec__output[i]);
		}
		__output = array__output.GetInternal();

		return __output;
	}
}
