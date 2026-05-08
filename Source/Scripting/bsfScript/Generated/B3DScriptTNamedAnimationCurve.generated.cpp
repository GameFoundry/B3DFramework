//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptTNamedAnimationCurve.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "B3DScriptTAnimationCurve.generated.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "B3DScriptTAnimationCurve.generated.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "B3DScriptTAnimationCurve.generated.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "B3DScriptTAnimationCurve.generated.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "B3DScriptTAnimationCurve.generated.h"

namespace b3d
{
	ScriptNamedFloatCurve::ScriptNamedFloatCurve()
	{ }

	MonoObject* ScriptNamedFloatCurve::Box(const __TNamedAnimationCurve_float_Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
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
		TShared<TAnimationCurve<float>> tmpCurve;
		ScriptAnimationCurve* scriptObjectWrapperCurve;
		scriptObjectWrapperCurve = ScriptAnimationCurve::GetScriptObjectWrapper(value.Curve);
		if(scriptObjectWrapperCurve != nullptr)
			tmpCurve = std::static_pointer_cast<TAnimationCurve<float>>(scriptObjectWrapperCurve->GetBaseNativeObjectAsShared());
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
		TShared<TAnimationCurve<float>> tmpCurvecopy;
		tmpCurvecopy = B3DMakeShared<TAnimationCurve<float>>(value.Curve);
		tmpCurve = ScriptAnimationCurve::GetOrCreateScriptObject(tmpCurvecopy);
		output.Curve = tmpCurve;

		return output;
	}


	ScriptNamedVector3Curve::ScriptNamedVector3Curve()
	{ }

	MonoObject* ScriptNamedVector3Curve::Box(const __TNamedAnimationCurve_TVector3_float__Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
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
		TShared<TAnimationCurve<TVector3<float>>> tmpCurve;
		ScriptVector3Curve* scriptObjectWrapperCurve;
		scriptObjectWrapperCurve = ScriptVector3Curve::GetScriptObjectWrapper(value.Curve);
		if(scriptObjectWrapperCurve != nullptr)
			tmpCurve = std::static_pointer_cast<TAnimationCurve<TVector3<float>>>(scriptObjectWrapperCurve->GetBaseNativeObjectAsShared());
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
		TShared<TAnimationCurve<TVector3<float>>> tmpCurvecopy;
		tmpCurvecopy = B3DMakeShared<TAnimationCurve<TVector3<float>>>(value.Curve);
		tmpCurve = ScriptVector3Curve::GetOrCreateScriptObject(tmpCurvecopy);
		output.Curve = tmpCurve;

		return output;
	}


	ScriptNamedVector2Curve::ScriptNamedVector2Curve()
	{ }

	MonoObject* ScriptNamedVector2Curve::Box(const __TNamedAnimationCurve_TVector2_float__Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
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
		TShared<TAnimationCurve<TVector2<float>>> tmpCurve;
		ScriptVector2Curve* scriptObjectWrapperCurve;
		scriptObjectWrapperCurve = ScriptVector2Curve::GetScriptObjectWrapper(value.Curve);
		if(scriptObjectWrapperCurve != nullptr)
			tmpCurve = std::static_pointer_cast<TAnimationCurve<TVector2<float>>>(scriptObjectWrapperCurve->GetBaseNativeObjectAsShared());
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
		TShared<TAnimationCurve<TVector2<float>>> tmpCurvecopy;
		tmpCurvecopy = B3DMakeShared<TAnimationCurve<TVector2<float>>>(value.Curve);
		tmpCurve = ScriptVector2Curve::GetOrCreateScriptObject(tmpCurvecopy);
		output.Curve = tmpCurve;

		return output;
	}


	ScriptNamedQuaternionCurve::ScriptNamedQuaternionCurve()
	{ }

	MonoObject* ScriptNamedQuaternionCurve::Box(const __TNamedAnimationCurve_TQuaternion_float__Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TNamedAnimationCurve_TQuaternion_float__Interop ScriptNamedQuaternionCurve::Unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurve_TQuaternion_float__Interop*)MonoUtil::Unbox(value);
	}

	TNamedAnimationCurve<TQuaternion<float>> ScriptNamedQuaternionCurve::FromInterop(const __TNamedAnimationCurve_TQuaternion_float__Interop& value)
	{
		TNamedAnimationCurve<TQuaternion<float>> output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		TShared<TAnimationCurve<TQuaternion<float>>> tmpCurve;
		ScriptQuaternionCurve* scriptObjectWrapperCurve;
		scriptObjectWrapperCurve = ScriptQuaternionCurve::GetScriptObjectWrapper(value.Curve);
		if(scriptObjectWrapperCurve != nullptr)
			tmpCurve = std::static_pointer_cast<TAnimationCurve<TQuaternion<float>>>(scriptObjectWrapperCurve->GetBaseNativeObjectAsShared());
		if(tmpCurve != nullptr)
		output.Curve = *tmpCurve;

		return output;
	}

	__TNamedAnimationCurve_TQuaternion_float__Interop ScriptNamedQuaternionCurve::ToInterop(const TNamedAnimationCurve<TQuaternion<float>>& value)
	{
		__TNamedAnimationCurve_TQuaternion_float__Interop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		output.Flags = value.Flags;
		MonoObject* tmpCurve;
		TShared<TAnimationCurve<TQuaternion<float>>> tmpCurvecopy;
		tmpCurvecopy = B3DMakeShared<TAnimationCurve<TQuaternion<float>>>(value.Curve);
		tmpCurve = ScriptQuaternionCurve::GetOrCreateScriptObject(tmpCurvecopy);
		output.Curve = tmpCurve;

		return output;
	}


	ScriptNamedIntegerCurve::ScriptNamedIntegerCurve()
	{ }

	MonoObject* ScriptNamedIntegerCurve::Box(const __TNamedAnimationCurve_int32_t_Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
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
		TShared<TAnimationCurve<int32_t>> tmpCurve;
		ScriptIntegerCurve* scriptObjectWrapperCurve;
		scriptObjectWrapperCurve = ScriptIntegerCurve::GetScriptObjectWrapper(value.Curve);
		if(scriptObjectWrapperCurve != nullptr)
			tmpCurve = std::static_pointer_cast<TAnimationCurve<int32_t>>(scriptObjectWrapperCurve->GetBaseNativeObjectAsShared());
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
		TShared<TAnimationCurve<int32_t>> tmpCurvecopy;
		tmpCurvecopy = B3DMakeShared<TAnimationCurve<int32_t>>(value.Curve);
		tmpCurve = ScriptIntegerCurve::GetOrCreateScriptObject(tmpCurvecopy);
		output.Curve = tmpCurve;

		return output;
	}

}
