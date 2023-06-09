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

	MonoObject*ScriptNamedFloatCurve::Box(const __TNamedAnimationCurve_float_Interop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TNamedAnimationCurve_float_Interop ScriptNamedFloatCurve::Unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurve_float_Interop*)MonoUtil::Unbox(value);
	}

	TNamedAnimationCurve<float> ScriptNamedFloatCurve::FromInterop(const __TNamedAnimationCurve_float_Interop& value)
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

	__TNamedAnimationCurve_float_Interop ScriptNamedFloatCurve::ToInterop(const TNamedAnimationCurve<float>& value)
	{
		__TNamedAnimationCurve_float_Interop output;
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

	MonoObject*ScriptNamedVector3Curve::Box(const __TNamedAnimationCurve_TVector3_float__Interop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TNamedAnimationCurve_TVector3_float__Interop ScriptNamedVector3Curve::Unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurve_TVector3_float__Interop*)MonoUtil::Unbox(value);
	}

	TNamedAnimationCurve<TVector3<float>> ScriptNamedVector3Curve::FromInterop(const __TNamedAnimationCurve_TVector3_float__Interop& value)
	{
		TNamedAnimationCurve<TVector3<float>> output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		SPtr<TAnimationCurve<TVector3<float>>> tmpCurve;
		ScriptVector3Curve* scriptCurve;
		scriptCurve = ScriptVector3Curve::ToNative(value.Curve);
		if(scriptCurve != nullptr)
			tmpCurve = scriptCurve->GetInternal();
		if(tmpCurve != nullptr)
		output.Curve = *tmpCurve;

		return output;
	}

	__TNamedAnimationCurve_TVector3_float__Interop ScriptNamedVector3Curve::ToInterop(const TNamedAnimationCurve<TVector3<float>>& value)
	{
		__TNamedAnimationCurve_TVector3_float__Interop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		MonoObject* tmpCurve;
		SPtr<TAnimationCurve<TVector3<float>>> tmpCurvecopy;
		tmpCurvecopy = B3DMakeShared<TAnimationCurve<TVector3<float>>>(value.Curve);
		tmpCurve = ScriptVector3Curve::Create(tmpCurvecopy);
		output.Curve = tmpCurve;

		return output;
	}


	ScriptNamedVector2Curve::ScriptNamedVector2Curve(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptNamedVector2Curve::InitRuntimeData()
	{ }

	MonoObject*ScriptNamedVector2Curve::Box(const __TNamedAnimationCurve_TVector2_float__Interop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TNamedAnimationCurve_TVector2_float__Interop ScriptNamedVector2Curve::Unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurve_TVector2_float__Interop*)MonoUtil::Unbox(value);
	}

	TNamedAnimationCurve<TVector2<float>> ScriptNamedVector2Curve::FromInterop(const __TNamedAnimationCurve_TVector2_float__Interop& value)
	{
		TNamedAnimationCurve<TVector2<float>> output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		SPtr<TAnimationCurve<TVector2<float>>> tmpCurve;
		ScriptVector2Curve* scriptCurve;
		scriptCurve = ScriptVector2Curve::ToNative(value.Curve);
		if(scriptCurve != nullptr)
			tmpCurve = scriptCurve->GetInternal();
		if(tmpCurve != nullptr)
		output.Curve = *tmpCurve;

		return output;
	}

	__TNamedAnimationCurve_TVector2_float__Interop ScriptNamedVector2Curve::ToInterop(const TNamedAnimationCurve<TVector2<float>>& value)
	{
		__TNamedAnimationCurve_TVector2_float__Interop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		MonoObject* tmpCurve;
		SPtr<TAnimationCurve<TVector2<float>>> tmpCurvecopy;
		tmpCurvecopy = B3DMakeShared<TAnimationCurve<TVector2<float>>>(value.Curve);
		tmpCurve = ScriptVector2Curve::Create(tmpCurvecopy);
		output.Curve = tmpCurve;

		return output;
	}


	ScriptNamedQuaternionCurve::ScriptNamedQuaternionCurve(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptNamedQuaternionCurve::InitRuntimeData()
	{ }

	MonoObject*ScriptNamedQuaternionCurve::Box(const __TNamedAnimationCurve_Quaternion_Interop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TNamedAnimationCurve_Quaternion_Interop ScriptNamedQuaternionCurve::Unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurve_Quaternion_Interop*)MonoUtil::Unbox(value);
	}

	TNamedAnimationCurve<Quaternion> ScriptNamedQuaternionCurve::FromInterop(const __TNamedAnimationCurve_Quaternion_Interop& value)
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

	__TNamedAnimationCurve_Quaternion_Interop ScriptNamedQuaternionCurve::ToInterop(const TNamedAnimationCurve<Quaternion>& value)
	{
		__TNamedAnimationCurve_Quaternion_Interop output;
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

	MonoObject*ScriptNamedIntegerCurve::Box(const __TNamedAnimationCurve_int32_t_Interop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__TNamedAnimationCurve_int32_t_Interop ScriptNamedIntegerCurve::Unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurve_int32_t_Interop*)MonoUtil::Unbox(value);
	}

	TNamedAnimationCurve<int32_t> ScriptNamedIntegerCurve::FromInterop(const __TNamedAnimationCurve_int32_t_Interop& value)
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

	__TNamedAnimationCurve_int32_t_Interop ScriptNamedIntegerCurve::ToInterop(const TNamedAnimationCurve<int32_t>& value)
	{
		__TNamedAnimationCurve_int32_t_Interop output;
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
