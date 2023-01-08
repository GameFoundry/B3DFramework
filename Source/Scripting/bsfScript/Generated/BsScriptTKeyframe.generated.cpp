//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTKeyframe.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Math/BsQuaternion.h"
#include "Wrappers/BsScriptQuaternion.h"
#include "Math/BsVector3.h"
#include "Wrappers/BsScriptVector.h"
#include "Math/BsVector2.h"
#include "Wrappers/BsScriptVector.h"

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

	MonoObject*ScriptKeyFrameVec3::Box(const __TKeyframeVector3Interop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TKeyframeVector3Interop ScriptKeyFrameVec3::Unbox(MonoObject* value)
	{
		return *(__TKeyframeVector3Interop*)MonoUtil::Unbox(value);
	}

	TKeyframe<Vector3> ScriptKeyFrameVec3::FromInterop(const __TKeyframeVector3Interop& value)
	{
		TKeyframe<Vector3> output;
		output.Value = value.Value;
		output.InTangent = value.InTangent;
		output.OutTangent = value.OutTangent;
		output.Time = value.Time;

		return output;
	}

	__TKeyframeVector3Interop ScriptKeyFrameVec3::ToInterop(const TKeyframe<Vector3>& value)
	{
		__TKeyframeVector3Interop output;
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

	MonoObject*ScriptKeyFrameVec2::Box(const __TKeyframeVector2Interop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TKeyframeVector2Interop ScriptKeyFrameVec2::Unbox(MonoObject* value)
	{
		return *(__TKeyframeVector2Interop*)MonoUtil::Unbox(value);
	}

	TKeyframe<Vector2> ScriptKeyFrameVec2::FromInterop(const __TKeyframeVector2Interop& value)
	{
		TKeyframe<Vector2> output;
		output.Value = value.Value;
		output.InTangent = value.InTangent;
		output.OutTangent = value.OutTangent;
		output.Time = value.Time;

		return output;
	}

	__TKeyframeVector2Interop ScriptKeyFrameVec2::ToInterop(const TKeyframe<Vector2>& value)
	{
		__TKeyframeVector2Interop output;
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

	MonoObject*ScriptKeyFrameQuat::Box(const __TKeyframeQuaternionInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TKeyframeQuaternionInterop ScriptKeyFrameQuat::Unbox(MonoObject* value)
	{
		return *(__TKeyframeQuaternionInterop*)MonoUtil::Unbox(value);
	}

	TKeyframe<Quaternion> ScriptKeyFrameQuat::FromInterop(const __TKeyframeQuaternionInterop& value)
	{
		TKeyframe<Quaternion> output;
		output.Value = value.Value;
		output.InTangent = value.InTangent;
		output.OutTangent = value.OutTangent;
		output.Time = value.Time;

		return output;
	}

	__TKeyframeQuaternionInterop ScriptKeyFrameQuat::ToInterop(const TKeyframe<Quaternion>& value)
	{
		__TKeyframeQuaternionInterop output;
		output.Value = value.Value;
		output.InTangent = value.InTangent;
		output.OutTangent = value.OutTangent;
		output.Time = value.Time;

		return output;
	}

}
