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
	ScriptNamedFloatCurve::ScriptNamedFloatCurve(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptNamedFloatCurve::InitRuntimeData()
	{ }

	MonoObject*ScriptNamedFloatCurve::Box(const __TNamedAnimationCurvefloatInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TNamedAnimationCurvefloatInterop ScriptNamedFloatCurve::Unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurvefloatInterop*)MonoUtil::Unbox(value);
	}

	TNamedAnimationCurve<float> ScriptNamedFloatCurve::FromInterop(const __TNamedAnimationCurvefloatInterop& value)
	{
		TNamedAnimationCurve<float> output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		SPtr<TAnimationCurve<float>> tmpCurve;
		ScriptAnimationCurve* scriptCurve;
		scriptCurve = ScriptAnimationCurve::ToNative(value.Curve);
		if(scriptCurve != nullptr)
			tmpCurve = scriptCurve->GetInternal();
		if(tmpCurve != nullptr)
		output.Curve = *tmpCurve;

		return output;
	}

	__TNamedAnimationCurvefloatInterop ScriptNamedFloatCurve::ToInterop(const TNamedAnimationCurve<float>& value)
	{
		__TNamedAnimationCurvefloatInterop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		MonoObject* tmpCurve;
		SPtr<TAnimationCurve<float>> tmpCurvecopy;
		tmpCurvecopy = B3DMakeShared<TAnimationCurve<float>>(value.Curve);
		tmpCurve = ScriptAnimationCurve::Create(tmpCurvecopy);
		output.Curve = tmpCurve;

		return output;
	}


	ScriptNamedVector3Curve::ScriptNamedVector3Curve(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptNamedVector3Curve::InitRuntimeData()
	{ }

	MonoObject*ScriptNamedVector3Curve::Box(const __TNamedAnimationCurveVector3Interop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TNamedAnimationCurveVector3Interop ScriptNamedVector3Curve::Unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurveVector3Interop*)MonoUtil::Unbox(value);
	}

	TNamedAnimationCurve<Vector3> ScriptNamedVector3Curve::FromInterop(const __TNamedAnimationCurveVector3Interop& value)
	{
		TNamedAnimationCurve<Vector3> output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		SPtr<TAnimationCurve<Vector3>> tmpCurve;
		ScriptVector3Curve* scriptCurve;
		scriptCurve = ScriptVector3Curve::ToNative(value.Curve);
		if(scriptCurve != nullptr)
			tmpCurve = scriptCurve->GetInternal();
		if(tmpCurve != nullptr)
		output.Curve = *tmpCurve;

		return output;
	}

	__TNamedAnimationCurveVector3Interop ScriptNamedVector3Curve::ToInterop(const TNamedAnimationCurve<Vector3>& value)
	{
		__TNamedAnimationCurveVector3Interop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		MonoObject* tmpCurve;
		SPtr<TAnimationCurve<Vector3>> tmpCurvecopy;
		tmpCurvecopy = B3DMakeShared<TAnimationCurve<Vector3>>(value.Curve);
		tmpCurve = ScriptVector3Curve::Create(tmpCurvecopy);
		output.Curve = tmpCurve;

		return output;
	}


	ScriptNamedVector2Curve::ScriptNamedVector2Curve(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptNamedVector2Curve::InitRuntimeData()
	{ }

	MonoObject*ScriptNamedVector2Curve::Box(const __TNamedAnimationCurveVector2Interop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TNamedAnimationCurveVector2Interop ScriptNamedVector2Curve::Unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurveVector2Interop*)MonoUtil::Unbox(value);
	}

	TNamedAnimationCurve<Vector2> ScriptNamedVector2Curve::FromInterop(const __TNamedAnimationCurveVector2Interop& value)
	{
		TNamedAnimationCurve<Vector2> output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		SPtr<TAnimationCurve<Vector2>> tmpCurve;
		ScriptVector2Curve* scriptCurve;
		scriptCurve = ScriptVector2Curve::ToNative(value.Curve);
		if(scriptCurve != nullptr)
			tmpCurve = scriptCurve->GetInternal();
		if(tmpCurve != nullptr)
		output.Curve = *tmpCurve;

		return output;
	}

	__TNamedAnimationCurveVector2Interop ScriptNamedVector2Curve::ToInterop(const TNamedAnimationCurve<Vector2>& value)
	{
		__TNamedAnimationCurveVector2Interop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		MonoObject* tmpCurve;
		SPtr<TAnimationCurve<Vector2>> tmpCurvecopy;
		tmpCurvecopy = B3DMakeShared<TAnimationCurve<Vector2>>(value.Curve);
		tmpCurve = ScriptVector2Curve::Create(tmpCurvecopy);
		output.Curve = tmpCurve;

		return output;
	}


	ScriptNamedQuaternionCurve::ScriptNamedQuaternionCurve(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptNamedQuaternionCurve::InitRuntimeData()
	{ }

	MonoObject*ScriptNamedQuaternionCurve::Box(const __TNamedAnimationCurveQuaternionInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TNamedAnimationCurveQuaternionInterop ScriptNamedQuaternionCurve::Unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurveQuaternionInterop*)MonoUtil::Unbox(value);
	}

	TNamedAnimationCurve<Quaternion> ScriptNamedQuaternionCurve::FromInterop(const __TNamedAnimationCurveQuaternionInterop& value)
	{
		TNamedAnimationCurve<Quaternion> output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		SPtr<TAnimationCurve<Quaternion>> tmpCurve;
		ScriptQuaternionCurve* scriptCurve;
		scriptCurve = ScriptQuaternionCurve::ToNative(value.Curve);
		if(scriptCurve != nullptr)
			tmpCurve = scriptCurve->GetInternal();
		if(tmpCurve != nullptr)
		output.Curve = *tmpCurve;

		return output;
	}

	__TNamedAnimationCurveQuaternionInterop ScriptNamedQuaternionCurve::ToInterop(const TNamedAnimationCurve<Quaternion>& value)
	{
		__TNamedAnimationCurveQuaternionInterop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		MonoObject* tmpCurve;
		SPtr<TAnimationCurve<Quaternion>> tmpCurvecopy;
		tmpCurvecopy = B3DMakeShared<TAnimationCurve<Quaternion>>(value.Curve);
		tmpCurve = ScriptQuaternionCurve::Create(tmpCurvecopy);
		output.Curve = tmpCurve;

		return output;
	}


	ScriptNamedIntegerCurve::ScriptNamedIntegerCurve(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptNamedIntegerCurve::InitRuntimeData()
	{ }

	MonoObject*ScriptNamedIntegerCurve::Box(const __TNamedAnimationCurveint32_tInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TNamedAnimationCurveint32_tInterop ScriptNamedIntegerCurve::Unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurveint32_tInterop*)MonoUtil::Unbox(value);
	}

	TNamedAnimationCurve<int32_t> ScriptNamedIntegerCurve::FromInterop(const __TNamedAnimationCurveint32_tInterop& value)
	{
		TNamedAnimationCurve<int32_t> output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		SPtr<TAnimationCurve<int32_t>> tmpCurve;
		ScriptIntegerCurve* scriptCurve;
		scriptCurve = ScriptIntegerCurve::ToNative(value.Curve);
		if(scriptCurve != nullptr)
			tmpCurve = scriptCurve->GetInternal();
		if(tmpCurve != nullptr)
		output.Curve = *tmpCurve;

		return output;
	}

	__TNamedAnimationCurveint32_tInterop ScriptNamedIntegerCurve::ToInterop(const TNamedAnimationCurve<int32_t>& value)
	{
		__TNamedAnimationCurveint32_tInterop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		MonoObject* tmpCurve;
		SPtr<TAnimationCurve<int32_t>> tmpCurvecopy;
		tmpCurvecopy = B3DMakeShared<TAnimationCurve<int32_t>>(value.Curve);
		tmpCurve = ScriptIntegerCurve::Create(tmpCurvecopy);
		output.Curve = tmpCurve;

		return output;
	}

}
