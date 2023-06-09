//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTKeyframe.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Math/BsVector3.h"
#include "Wrappers/BsScriptVector.h"
#include "Math/BsVector2.h"
#include "Wrappers/BsScriptVector.h"
#include "Math/BsQuaternion.h"
#include "Wrappers/BsScriptQuaternion.h"

namespace bs
{
	ScriptKeyFrameInt::ScriptKeyFrameInt(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptKeyFrameInt::InitRuntimeData()
	{ }

	MonoObject*ScriptKeyFrameInt::Box(const TKeyframe<int32_t>& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	TKeyframe<int32_t> ScriptKeyFrameInt::Unbox(MonoObject* value)
	{
		return *(TKeyframe<int32_t>*)MonoUtil::Unbox(value);
	}


	ScriptKeyFrame::ScriptKeyFrame(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptKeyFrame::InitRuntimeData()
	{ }

	MonoObject*ScriptKeyFrame::Box(const TKeyframe<float>& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	TKeyframe<float> ScriptKeyFrame::Unbox(MonoObject* value)
	{
		return *(TKeyframe<float>*)MonoUtil::Unbox(value);
	}


	ScriptKeyFrameVec3::ScriptKeyFrameVec3(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptKeyFrameVec3::InitRuntimeData()
	{ }

	MonoObject*ScriptKeyFrameVec3::Box(const __TKeyframe_TVector3_float__Interop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TKeyframe_TVector3_float__Interop ScriptKeyFrameVec3::Unbox(MonoObject* value)
	{
		return *(__TKeyframe_TVector3_float__Interop*)MonoUtil::Unbox(value);
	}

	TKeyframe<TVector3<float>> ScriptKeyFrameVec3::FromInterop(const __TKeyframe_TVector3_float__Interop& value)
	{
		TKeyframe<TVector3<float>> output;
		output.Value = value.Value;
		output.InTangent = value.InTangent;
		output.OutTangent = value.OutTangent;
		output.Time = value.Time;

		return output;
	}

	__TKeyframe_TVector3_float__Interop ScriptKeyFrameVec3::ToInterop(const TKeyframe<TVector3<float>>& value)
	{
		__TKeyframe_TVector3_float__Interop output;
		output.Value = value.Value;
		output.InTangent = value.InTangent;
		output.OutTangent = value.OutTangent;
		output.Time = value.Time;

		return output;
	}


	ScriptKeyFrameVec2::ScriptKeyFrameVec2(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptKeyFrameVec2::InitRuntimeData()
	{ }

	MonoObject*ScriptKeyFrameVec2::Box(const __TKeyframe_TVector2_float__Interop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TKeyframe_TVector2_float__Interop ScriptKeyFrameVec2::Unbox(MonoObject* value)
	{
		return *(__TKeyframe_TVector2_float__Interop*)MonoUtil::Unbox(value);
	}

	TKeyframe<TVector2<float>> ScriptKeyFrameVec2::FromInterop(const __TKeyframe_TVector2_float__Interop& value)
	{
		TKeyframe<TVector2<float>> output;
		output.Value = value.Value;
		output.InTangent = value.InTangent;
		output.OutTangent = value.OutTangent;
		output.Time = value.Time;

		return output;
	}

	__TKeyframe_TVector2_float__Interop ScriptKeyFrameVec2::ToInterop(const TKeyframe<TVector2<float>>& value)
	{
		__TKeyframe_TVector2_float__Interop output;
		output.Value = value.Value;
		output.InTangent = value.InTangent;
		output.OutTangent = value.OutTangent;
		output.Time = value.Time;

		return output;
	}


	ScriptKeyFrameQuat::ScriptKeyFrameQuat(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptKeyFrameQuat::InitRuntimeData()
	{ }

	MonoObject*ScriptKeyFrameQuat::Box(const __TKeyframe_Quaternion_Interop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TKeyframe_Quaternion_Interop ScriptKeyFrameQuat::Unbox(MonoObject* value)
	{
		return *(__TKeyframe_Quaternion_Interop*)MonoUtil::Unbox(value);
	}

	TKeyframe<Quaternion> ScriptKeyFrameQuat::FromInterop(const __TKeyframe_Quaternion_Interop& value)
	{
		TKeyframe<Quaternion> output;
		output.Value = value.Value;
		output.InTangent = value.InTangent;
		output.OutTangent = value.OutTangent;
		output.Time = value.Time;

		return output;
	}

	__TKeyframe_Quaternion_Interop ScriptKeyFrameQuat::ToInterop(const TKeyframe<Quaternion>& value)
	{
		__TKeyframe_Quaternion_Interop output;
		output.Value = value.Value;
		output.InTangent = value.InTangent;
		output.OutTangent = value.OutTangent;
		output.Time = value.Time;

		return output;
	}

}
