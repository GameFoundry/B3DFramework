//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
	ScriptTKeyframeint32_t::ScriptTKeyframeint32_t(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptTKeyframeint32_t::InitRuntimeData()
	{}

	MonoObject* ScriptTKeyframeint32_t::Box(const TKeyframe<int32_t>& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	TKeyframe<int32_t> ScriptTKeyframeint32_t::Unbox(MonoObject* value)
	{
		return *(TKeyframe<int32_t>*)MonoUtil::Unbox(value);
	}

	ScriptTKeyframefloat::ScriptTKeyframefloat(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptTKeyframefloat::InitRuntimeData()
	{}

	MonoObject* ScriptTKeyframefloat::Box(const TKeyframe<float>& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	TKeyframe<float> ScriptTKeyframefloat::Unbox(MonoObject* value)
	{
		return *(TKeyframe<float>*)MonoUtil::Unbox(value);
	}

	ScriptTKeyframeVector3::ScriptTKeyframeVector3(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptTKeyframeVector3::InitRuntimeData()
	{}

	MonoObject* ScriptTKeyframeVector3::Box(const __TKeyframeVector3Interop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TKeyframeVector3Interop ScriptTKeyframeVector3::Unbox(MonoObject* value)
	{
		return *(__TKeyframeVector3Interop*)MonoUtil::Unbox(value);
	}

	TKeyframe<Vector3> ScriptTKeyframeVector3::FromInterop(const __TKeyframeVector3Interop& value)
	{
		TKeyframe<Vector3> output;
		output.Value = value.Value;
		output.InTangent = value.InTangent;
		output.OutTangent = value.OutTangent;
		output.Time = value.Time;

		return output;
	}

	__TKeyframeVector3Interop ScriptTKeyframeVector3::ToInterop(const TKeyframe<Vector3>& value)
	{
		__TKeyframeVector3Interop output;
		output.Value = value.Value;
		output.InTangent = value.InTangent;
		output.OutTangent = value.OutTangent;
		output.Time = value.Time;

		return output;
	}

	ScriptTKeyframeVector2::ScriptTKeyframeVector2(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptTKeyframeVector2::InitRuntimeData()
	{}

	MonoObject* ScriptTKeyframeVector2::Box(const __TKeyframeVector2Interop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TKeyframeVector2Interop ScriptTKeyframeVector2::Unbox(MonoObject* value)
	{
		return *(__TKeyframeVector2Interop*)MonoUtil::Unbox(value);
	}

	TKeyframe<Vector2> ScriptTKeyframeVector2::FromInterop(const __TKeyframeVector2Interop& value)
	{
		TKeyframe<Vector2> output;
		output.Value = value.Value;
		output.InTangent = value.InTangent;
		output.OutTangent = value.OutTangent;
		output.Time = value.Time;

		return output;
	}

	__TKeyframeVector2Interop ScriptTKeyframeVector2::ToInterop(const TKeyframe<Vector2>& value)
	{
		__TKeyframeVector2Interop output;
		output.Value = value.Value;
		output.InTangent = value.InTangent;
		output.OutTangent = value.OutTangent;
		output.Time = value.Time;

		return output;
	}

	ScriptTKeyframeQuaternion::ScriptTKeyframeQuaternion(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptTKeyframeQuaternion::InitRuntimeData()
	{}

	MonoObject* ScriptTKeyframeQuaternion::Box(const __TKeyframeQuaternionInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TKeyframeQuaternionInterop ScriptTKeyframeQuaternion::Unbox(MonoObject* value)
	{
		return *(__TKeyframeQuaternionInterop*)MonoUtil::Unbox(value);
	}

	TKeyframe<Quaternion> ScriptTKeyframeQuaternion::FromInterop(const __TKeyframeQuaternionInterop& value)
	{
		TKeyframe<Quaternion> output;
		output.Value = value.Value;
		output.InTangent = value.InTangent;
		output.OutTangent = value.OutTangent;
		output.Time = value.Time;

		return output;
	}

	__TKeyframeQuaternionInterop ScriptTKeyframeQuaternion::ToInterop(const TKeyframe<Quaternion>& value)
	{
		__TKeyframeQuaternionInterop output;
		output.Value = value.Value;
		output.InTangent = value.InTangent;
		output.OutTangent = value.OutTangent;
		output.Time = value.Time;

		return output;
	}

} // namespace bs
