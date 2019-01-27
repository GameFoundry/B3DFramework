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
		:ScriptObject(managedInstance)
	{ }

	void ScriptTKeyframeint32_t::initRuntimeData()
	{ }

	MonoObject*ScriptTKeyframeint32_t::box(const TKeyframe<int32_t>& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	TKeyframe<int32_t> ScriptTKeyframeint32_t::unbox(MonoObject* value)
	{
		return *(TKeyframe<int32_t>*)MonoUtil::unbox(value);
	}


	ScriptTKeyframefloat::ScriptTKeyframefloat(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptTKeyframefloat::initRuntimeData()
	{ }

	MonoObject*ScriptTKeyframefloat::box(const TKeyframe<float>& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	TKeyframe<float> ScriptTKeyframefloat::unbox(MonoObject* value)
	{
		return *(TKeyframe<float>*)MonoUtil::unbox(value);
	}


	ScriptTKeyframeVector3::ScriptTKeyframeVector3(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptTKeyframeVector3::initRuntimeData()
	{ }

	MonoObject*ScriptTKeyframeVector3::box(const __TKeyframeVector3Interop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__TKeyframeVector3Interop ScriptTKeyframeVector3::unbox(MonoObject* value)
	{
		return *(__TKeyframeVector3Interop*)MonoUtil::unbox(value);
	}

	TKeyframe<Vector3> ScriptTKeyframeVector3::fromInterop(const __TKeyframeVector3Interop& value)
	{
		TKeyframe<Vector3> output;
		output.value = value.value;
		output.inTangent = value.inTangent;
		output.outTangent = value.outTangent;
		output.time = value.time;

		return output;
	}

	__TKeyframeVector3Interop ScriptTKeyframeVector3::toInterop(const TKeyframe<Vector3>& value)
	{
		__TKeyframeVector3Interop output;
		output.value = value.value;
		output.inTangent = value.inTangent;
		output.outTangent = value.outTangent;
		output.time = value.time;

		return output;
	}


	ScriptTKeyframeVector2::ScriptTKeyframeVector2(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptTKeyframeVector2::initRuntimeData()
	{ }

	MonoObject*ScriptTKeyframeVector2::box(const __TKeyframeVector2Interop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__TKeyframeVector2Interop ScriptTKeyframeVector2::unbox(MonoObject* value)
	{
		return *(__TKeyframeVector2Interop*)MonoUtil::unbox(value);
	}

	TKeyframe<Vector2> ScriptTKeyframeVector2::fromInterop(const __TKeyframeVector2Interop& value)
	{
		TKeyframe<Vector2> output;
		output.value = value.value;
		output.inTangent = value.inTangent;
		output.outTangent = value.outTangent;
		output.time = value.time;

		return output;
	}

	__TKeyframeVector2Interop ScriptTKeyframeVector2::toInterop(const TKeyframe<Vector2>& value)
	{
		__TKeyframeVector2Interop output;
		output.value = value.value;
		output.inTangent = value.inTangent;
		output.outTangent = value.outTangent;
		output.time = value.time;

		return output;
	}


	ScriptTKeyframeQuaternion::ScriptTKeyframeQuaternion(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptTKeyframeQuaternion::initRuntimeData()
	{ }

	MonoObject*ScriptTKeyframeQuaternion::box(const __TKeyframeQuaternionInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__TKeyframeQuaternionInterop ScriptTKeyframeQuaternion::unbox(MonoObject* value)
	{
		return *(__TKeyframeQuaternionInterop*)MonoUtil::unbox(value);
	}

	TKeyframe<Quaternion> ScriptTKeyframeQuaternion::fromInterop(const __TKeyframeQuaternionInterop& value)
	{
		TKeyframe<Quaternion> output;
		output.value = value.value;
		output.inTangent = value.inTangent;
		output.outTangent = value.outTangent;
		output.time = value.time;

		return output;
	}

	__TKeyframeQuaternionInterop ScriptTKeyframeQuaternion::toInterop(const TKeyframe<Quaternion>& value)
	{
		__TKeyframeQuaternionInterop output;
		output.value = value.value;
		output.inTangent = value.inTangent;
		output.outTangent = value.outTangent;
		output.time = value.time;

		return output;
	}

}
