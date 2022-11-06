//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTNamedAnimationCurve.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptTAnimationCurve.generated.h"

namespace bs
{
	ScriptTNamedAnimationCurvefloat::ScriptTNamedAnimationCurvefloat(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptTNamedAnimationCurvefloat::InitRuntimeData()
	{ }

	MonoObject*ScriptTNamedAnimationCurvefloat::Box(const __TNamedAnimationCurvefloatInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TNamedAnimationCurvefloatInterop ScriptTNamedAnimationCurvefloat::Unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurvefloatInterop*)MonoUtil::Unbox(value);
	}

	TNamedAnimationCurve<float> ScriptTNamedAnimationCurvefloat::FromInterop(const __TNamedAnimationCurvefloatInterop& value)
	{
		TNamedAnimationCurve<float> output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		SPtr<TAnimationCurve<float>> tmpCurve;
		ScriptTAnimationCurvefloat* scriptCurve;
		scriptCurve = ScriptTAnimationCurvefloat::ToNative(value.Curve);
		if(scriptCurve != nullptr)
			tmpCurve = scriptCurve->GetInternal();
		if(tmpCurve != nullptr)
		output.Curve = *tmpCurve;

		return output;
	}

	__TNamedAnimationCurvefloatInterop ScriptTNamedAnimationCurvefloat::ToInterop(const TNamedAnimationCurve<float>& value)
	{
		__TNamedAnimationCurvefloatInterop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		MonoObject* tmpCurve;
		SPtr<TAnimationCurve<float>> tmpCurvecopy;
		tmpCurvecopy = B3DMakeShared<TAnimationCurve<float>>(value.Curve);
		tmpCurve = ScriptTAnimationCurvefloat::Create(tmpCurvecopy);
		output.Curve = tmpCurve;

		return output;
	}


	ScriptTNamedAnimationCurveVector3::ScriptTNamedAnimationCurveVector3(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptTNamedAnimationCurveVector3::InitRuntimeData()
	{ }

	MonoObject*ScriptTNamedAnimationCurveVector3::Box(const __TNamedAnimationCurveVector3Interop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TNamedAnimationCurveVector3Interop ScriptTNamedAnimationCurveVector3::Unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurveVector3Interop*)MonoUtil::Unbox(value);
	}

	TNamedAnimationCurve<Vector3> ScriptTNamedAnimationCurveVector3::FromInterop(const __TNamedAnimationCurveVector3Interop& value)
	{
		TNamedAnimationCurve<Vector3> output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		SPtr<TAnimationCurve<Vector3>> tmpCurve;
		ScriptTAnimationCurveVector3* scriptCurve;
		scriptCurve = ScriptTAnimationCurveVector3::ToNative(value.Curve);
		if(scriptCurve != nullptr)
			tmpCurve = scriptCurve->GetInternal();
		if(tmpCurve != nullptr)
		output.Curve = *tmpCurve;

		return output;
	}

	__TNamedAnimationCurveVector3Interop ScriptTNamedAnimationCurveVector3::ToInterop(const TNamedAnimationCurve<Vector3>& value)
	{
		__TNamedAnimationCurveVector3Interop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		MonoObject* tmpCurve;
		SPtr<TAnimationCurve<Vector3>> tmpCurvecopy;
		tmpCurvecopy = B3DMakeShared<TAnimationCurve<Vector3>>(value.Curve);
		tmpCurve = ScriptTAnimationCurveVector3::Create(tmpCurvecopy);
		output.Curve = tmpCurve;

		return output;
	}


	ScriptTNamedAnimationCurveVector2::ScriptTNamedAnimationCurveVector2(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptTNamedAnimationCurveVector2::InitRuntimeData()
	{ }

	MonoObject*ScriptTNamedAnimationCurveVector2::Box(const __TNamedAnimationCurveVector2Interop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TNamedAnimationCurveVector2Interop ScriptTNamedAnimationCurveVector2::Unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurveVector2Interop*)MonoUtil::Unbox(value);
	}

	TNamedAnimationCurve<Vector2> ScriptTNamedAnimationCurveVector2::FromInterop(const __TNamedAnimationCurveVector2Interop& value)
	{
		TNamedAnimationCurve<Vector2> output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		SPtr<TAnimationCurve<Vector2>> tmpCurve;
		ScriptTAnimationCurveVector2* scriptCurve;
		scriptCurve = ScriptTAnimationCurveVector2::ToNative(value.Curve);
		if(scriptCurve != nullptr)
			tmpCurve = scriptCurve->GetInternal();
		if(tmpCurve != nullptr)
		output.Curve = *tmpCurve;

		return output;
	}

	__TNamedAnimationCurveVector2Interop ScriptTNamedAnimationCurveVector2::ToInterop(const TNamedAnimationCurve<Vector2>& value)
	{
		__TNamedAnimationCurveVector2Interop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		MonoObject* tmpCurve;
		SPtr<TAnimationCurve<Vector2>> tmpCurvecopy;
		tmpCurvecopy = B3DMakeShared<TAnimationCurve<Vector2>>(value.Curve);
		tmpCurve = ScriptTAnimationCurveVector2::Create(tmpCurvecopy);
		output.Curve = tmpCurve;

		return output;
	}


	ScriptTNamedAnimationCurveQuaternion::ScriptTNamedAnimationCurveQuaternion(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptTNamedAnimationCurveQuaternion::InitRuntimeData()
	{ }

	MonoObject*ScriptTNamedAnimationCurveQuaternion::Box(const __TNamedAnimationCurveQuaternionInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TNamedAnimationCurveQuaternionInterop ScriptTNamedAnimationCurveQuaternion::Unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurveQuaternionInterop*)MonoUtil::Unbox(value);
	}

	TNamedAnimationCurve<Quaternion> ScriptTNamedAnimationCurveQuaternion::FromInterop(const __TNamedAnimationCurveQuaternionInterop& value)
	{
		TNamedAnimationCurve<Quaternion> output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		SPtr<TAnimationCurve<Quaternion>> tmpCurve;
		ScriptTAnimationCurveQuaternion* scriptCurve;
		scriptCurve = ScriptTAnimationCurveQuaternion::ToNative(value.Curve);
		if(scriptCurve != nullptr)
			tmpCurve = scriptCurve->GetInternal();
		if(tmpCurve != nullptr)
		output.Curve = *tmpCurve;

		return output;
	}

	__TNamedAnimationCurveQuaternionInterop ScriptTNamedAnimationCurveQuaternion::ToInterop(const TNamedAnimationCurve<Quaternion>& value)
	{
		__TNamedAnimationCurveQuaternionInterop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		MonoObject* tmpCurve;
		SPtr<TAnimationCurve<Quaternion>> tmpCurvecopy;
		tmpCurvecopy = B3DMakeShared<TAnimationCurve<Quaternion>>(value.Curve);
		tmpCurve = ScriptTAnimationCurveQuaternion::Create(tmpCurvecopy);
		output.Curve = tmpCurve;

		return output;
	}


	ScriptTNamedAnimationCurveint32_t::ScriptTNamedAnimationCurveint32_t(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptTNamedAnimationCurveint32_t::InitRuntimeData()
	{ }

	MonoObject*ScriptTNamedAnimationCurveint32_t::Box(const __TNamedAnimationCurveint32_tInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TNamedAnimationCurveint32_tInterop ScriptTNamedAnimationCurveint32_t::Unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurveint32_tInterop*)MonoUtil::Unbox(value);
	}

	TNamedAnimationCurve<int32_t> ScriptTNamedAnimationCurveint32_t::FromInterop(const __TNamedAnimationCurveint32_tInterop& value)
	{
		TNamedAnimationCurve<int32_t> output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		SPtr<TAnimationCurve<int32_t>> tmpCurve;
		ScriptTAnimationCurveint32_t* scriptCurve;
		scriptCurve = ScriptTAnimationCurveint32_t::ToNative(value.Curve);
		if(scriptCurve != nullptr)
			tmpCurve = scriptCurve->GetInternal();
		if(tmpCurve != nullptr)
		output.Curve = *tmpCurve;

		return output;
	}

	__TNamedAnimationCurveint32_tInterop ScriptTNamedAnimationCurveint32_t::ToInterop(const TNamedAnimationCurve<int32_t>& value)
	{
		__TNamedAnimationCurveint32_tInterop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		MonoObject* tmpCurve;
		SPtr<TAnimationCurve<int32_t>> tmpCurvecopy;
		tmpCurvecopy = B3DMakeShared<TAnimationCurve<int32_t>>(value.Curve);
		tmpCurve = ScriptTAnimationCurveint32_t::Create(tmpCurvecopy);
		output.Curve = tmpCurve;

		return output;
	}

}
