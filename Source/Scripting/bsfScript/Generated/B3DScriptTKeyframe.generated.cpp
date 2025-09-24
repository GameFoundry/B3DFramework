//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptTKeyframe.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"
#include "B3DScriptTVector3.generated.h"
#include "../../../Foundation/Utility/Math/B3DVector2.h"
#include "B3DScriptTVector2.generated.h"
#include "../../../Foundation/Utility/Math/B3DQuaternion.h"
#include "B3DScriptTQuaternion.generated.h"

namespace b3d
{
	ScriptKeyFrameInt::ScriptKeyFrameInt()
	{ }

	MonoObject* ScriptKeyFrameInt::Box(const TKeyframe<int32_t>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TKeyframe<int32_t> ScriptKeyFrameInt::Unbox(MonoObject* value)
	{
		return *(TKeyframe<int32_t>*)MonoUtil::Unbox(value);
	}


	ScriptKeyFrame::ScriptKeyFrame()
	{ }

	MonoObject* ScriptKeyFrame::Box(const TKeyframe<float>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TKeyframe<float> ScriptKeyFrame::Unbox(MonoObject* value)
	{
		return *(TKeyframe<float>*)MonoUtil::Unbox(value);
	}


	ScriptKeyFrameVec3::ScriptKeyFrameVec3()
	{ }

	MonoObject* ScriptKeyFrameVec3::Box(const __TKeyframe_TVector3_float__Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
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


	ScriptKeyFrameVec2::ScriptKeyFrameVec2()
	{ }

	MonoObject* ScriptKeyFrameVec2::Box(const __TKeyframe_TVector2_float__Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
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


	ScriptKeyFrameQuat::ScriptKeyFrameQuat()
	{ }

	MonoObject* ScriptKeyFrameQuat::Box(const __TKeyframe_TQuaternion_float__Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TKeyframe_TQuaternion_float__Interop ScriptKeyFrameQuat::Unbox(MonoObject* value)
	{
		return *(__TKeyframe_TQuaternion_float__Interop*)MonoUtil::Unbox(value);
	}

	TKeyframe<TQuaternion<float>> ScriptKeyFrameQuat::FromInterop(const __TKeyframe_TQuaternion_float__Interop& value)
	{
		TKeyframe<TQuaternion<float>> output;
		output.Value = value.Value;
		output.InTangent = value.InTangent;
		output.OutTangent = value.OutTangent;
		output.Time = value.Time;

		return output;
	}

	__TKeyframe_TQuaternion_float__Interop ScriptKeyFrameQuat::ToInterop(const TKeyframe<TQuaternion<float>>& value)
	{
		__TKeyframe_TQuaternion_float__Interop output;
		output.Value = value.Value;
		output.InTangent = value.InTangent;
		output.OutTangent = value.OutTangent;
		output.Time = value.Time;

		return output;
	}

}
