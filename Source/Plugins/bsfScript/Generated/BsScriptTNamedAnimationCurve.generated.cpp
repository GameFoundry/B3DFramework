//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTNamedAnimationCurve.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "../../../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "../../../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "../../../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "../../../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptTAnimationCurve.generated.h"

namespace bs
{
	ScriptTNamedAnimationCurvefloat::ScriptTNamedAnimationCurvefloat(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptTNamedAnimationCurvefloat::initRuntimeData()
	{ }

	MonoObject*ScriptTNamedAnimationCurvefloat::box(const __TNamedAnimationCurvefloatInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__TNamedAnimationCurvefloatInterop ScriptTNamedAnimationCurvefloat::unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurvefloatInterop*)MonoUtil::unbox(value);
	}

	TNamedAnimationCurve<float> ScriptTNamedAnimationCurvefloat::fromInterop(const __TNamedAnimationCurvefloatInterop& value)
	{
		TNamedAnimationCurve<float> output;
		String tmpname;
		tmpname = MonoUtil::monoToString(value.name);
		output.name = tmpname;
		output.flags = value.flags;
		SPtr<TAnimationCurve<float>> tmpcurve;
		ScriptTAnimationCurvefloat* scriptcurve;
		scriptcurve = ScriptTAnimationCurvefloat::toNative(value.curve);
		if(scriptcurve != nullptr)
			tmpcurve = scriptcurve->getInternal();
		if(tmpcurve != nullptr)
		output.curve = *tmpcurve;

		return output;
	}

	__TNamedAnimationCurvefloatInterop ScriptTNamedAnimationCurvefloat::toInterop(const TNamedAnimationCurve<float>& value)
	{
		__TNamedAnimationCurvefloatInterop output;
		MonoString* tmpname;
		tmpname = MonoUtil::stringToMono(value.name);
		output.name = tmpname;
		output.flags = value.flags;
		MonoObject* tmpcurve;
		SPtr<TAnimationCurve<float>> tmpcurvecopy;
		tmpcurvecopy = bs_shared_ptr_new<TAnimationCurve<float>>(value.curve);
		tmpcurve = ScriptTAnimationCurvefloat::create(tmpcurvecopy);
		output.curve = tmpcurve;

		return output;
	}


	ScriptTNamedAnimationCurveVector3::ScriptTNamedAnimationCurveVector3(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptTNamedAnimationCurveVector3::initRuntimeData()
	{ }

	MonoObject*ScriptTNamedAnimationCurveVector3::box(const __TNamedAnimationCurveVector3Interop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__TNamedAnimationCurveVector3Interop ScriptTNamedAnimationCurveVector3::unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurveVector3Interop*)MonoUtil::unbox(value);
	}

	TNamedAnimationCurve<Vector3> ScriptTNamedAnimationCurveVector3::fromInterop(const __TNamedAnimationCurveVector3Interop& value)
	{
		TNamedAnimationCurve<Vector3> output;
		String tmpname;
		tmpname = MonoUtil::monoToString(value.name);
		output.name = tmpname;
		output.flags = value.flags;
		SPtr<TAnimationCurve<Vector3>> tmpcurve;
		ScriptTAnimationCurveVector3* scriptcurve;
		scriptcurve = ScriptTAnimationCurveVector3::toNative(value.curve);
		if(scriptcurve != nullptr)
			tmpcurve = scriptcurve->getInternal();
		if(tmpcurve != nullptr)
		output.curve = *tmpcurve;

		return output;
	}

	__TNamedAnimationCurveVector3Interop ScriptTNamedAnimationCurveVector3::toInterop(const TNamedAnimationCurve<Vector3>& value)
	{
		__TNamedAnimationCurveVector3Interop output;
		MonoString* tmpname;
		tmpname = MonoUtil::stringToMono(value.name);
		output.name = tmpname;
		output.flags = value.flags;
		MonoObject* tmpcurve;
		SPtr<TAnimationCurve<Vector3>> tmpcurvecopy;
		tmpcurvecopy = bs_shared_ptr_new<TAnimationCurve<Vector3>>(value.curve);
		tmpcurve = ScriptTAnimationCurveVector3::create(tmpcurvecopy);
		output.curve = tmpcurve;

		return output;
	}


	ScriptTNamedAnimationCurveVector2::ScriptTNamedAnimationCurveVector2(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptTNamedAnimationCurveVector2::initRuntimeData()
	{ }

	MonoObject*ScriptTNamedAnimationCurveVector2::box(const __TNamedAnimationCurveVector2Interop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__TNamedAnimationCurveVector2Interop ScriptTNamedAnimationCurveVector2::unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurveVector2Interop*)MonoUtil::unbox(value);
	}

	TNamedAnimationCurve<Vector2> ScriptTNamedAnimationCurveVector2::fromInterop(const __TNamedAnimationCurveVector2Interop& value)
	{
		TNamedAnimationCurve<Vector2> output;
		String tmpname;
		tmpname = MonoUtil::monoToString(value.name);
		output.name = tmpname;
		output.flags = value.flags;
		SPtr<TAnimationCurve<Vector2>> tmpcurve;
		ScriptTAnimationCurveVector2* scriptcurve;
		scriptcurve = ScriptTAnimationCurveVector2::toNative(value.curve);
		if(scriptcurve != nullptr)
			tmpcurve = scriptcurve->getInternal();
		if(tmpcurve != nullptr)
		output.curve = *tmpcurve;

		return output;
	}

	__TNamedAnimationCurveVector2Interop ScriptTNamedAnimationCurveVector2::toInterop(const TNamedAnimationCurve<Vector2>& value)
	{
		__TNamedAnimationCurveVector2Interop output;
		MonoString* tmpname;
		tmpname = MonoUtil::stringToMono(value.name);
		output.name = tmpname;
		output.flags = value.flags;
		MonoObject* tmpcurve;
		SPtr<TAnimationCurve<Vector2>> tmpcurvecopy;
		tmpcurvecopy = bs_shared_ptr_new<TAnimationCurve<Vector2>>(value.curve);
		tmpcurve = ScriptTAnimationCurveVector2::create(tmpcurvecopy);
		output.curve = tmpcurve;

		return output;
	}


	ScriptTNamedAnimationCurveQuaternion::ScriptTNamedAnimationCurveQuaternion(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptTNamedAnimationCurveQuaternion::initRuntimeData()
	{ }

	MonoObject*ScriptTNamedAnimationCurveQuaternion::box(const __TNamedAnimationCurveQuaternionInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__TNamedAnimationCurveQuaternionInterop ScriptTNamedAnimationCurveQuaternion::unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurveQuaternionInterop*)MonoUtil::unbox(value);
	}

	TNamedAnimationCurve<Quaternion> ScriptTNamedAnimationCurveQuaternion::fromInterop(const __TNamedAnimationCurveQuaternionInterop& value)
	{
		TNamedAnimationCurve<Quaternion> output;
		String tmpname;
		tmpname = MonoUtil::monoToString(value.name);
		output.name = tmpname;
		output.flags = value.flags;
		SPtr<TAnimationCurve<Quaternion>> tmpcurve;
		ScriptTAnimationCurveQuaternion* scriptcurve;
		scriptcurve = ScriptTAnimationCurveQuaternion::toNative(value.curve);
		if(scriptcurve != nullptr)
			tmpcurve = scriptcurve->getInternal();
		if(tmpcurve != nullptr)
		output.curve = *tmpcurve;

		return output;
	}

	__TNamedAnimationCurveQuaternionInterop ScriptTNamedAnimationCurveQuaternion::toInterop(const TNamedAnimationCurve<Quaternion>& value)
	{
		__TNamedAnimationCurveQuaternionInterop output;
		MonoString* tmpname;
		tmpname = MonoUtil::stringToMono(value.name);
		output.name = tmpname;
		output.flags = value.flags;
		MonoObject* tmpcurve;
		SPtr<TAnimationCurve<Quaternion>> tmpcurvecopy;
		tmpcurvecopy = bs_shared_ptr_new<TAnimationCurve<Quaternion>>(value.curve);
		tmpcurve = ScriptTAnimationCurveQuaternion::create(tmpcurvecopy);
		output.curve = tmpcurve;

		return output;
	}


	ScriptTNamedAnimationCurveint32_t::ScriptTNamedAnimationCurveint32_t(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptTNamedAnimationCurveint32_t::initRuntimeData()
	{ }

	MonoObject*ScriptTNamedAnimationCurveint32_t::box(const __TNamedAnimationCurveint32_tInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__TNamedAnimationCurveint32_tInterop ScriptTNamedAnimationCurveint32_t::unbox(MonoObject* value)
	{
		return *(__TNamedAnimationCurveint32_tInterop*)MonoUtil::unbox(value);
	}

	TNamedAnimationCurve<int32_t> ScriptTNamedAnimationCurveint32_t::fromInterop(const __TNamedAnimationCurveint32_tInterop& value)
	{
		TNamedAnimationCurve<int32_t> output;
		String tmpname;
		tmpname = MonoUtil::monoToString(value.name);
		output.name = tmpname;
		output.flags = value.flags;
		SPtr<TAnimationCurve<int32_t>> tmpcurve;
		ScriptTAnimationCurveint32_t* scriptcurve;
		scriptcurve = ScriptTAnimationCurveint32_t::toNative(value.curve);
		if(scriptcurve != nullptr)
			tmpcurve = scriptcurve->getInternal();
		if(tmpcurve != nullptr)
		output.curve = *tmpcurve;

		return output;
	}

	__TNamedAnimationCurveint32_tInterop ScriptTNamedAnimationCurveint32_t::toInterop(const TNamedAnimationCurve<int32_t>& value)
	{
		__TNamedAnimationCurveint32_tInterop output;
		MonoString* tmpname;
		tmpname = MonoUtil::stringToMono(value.name);
		output.name = tmpname;
		output.flags = value.flags;
		MonoObject* tmpcurve;
		SPtr<TAnimationCurve<int32_t>> tmpcurvecopy;
		tmpcurvecopy = bs_shared_ptr_new<TAnimationCurve<int32_t>>(value.curve);
		tmpcurve = ScriptTAnimationCurveint32_t::create(tmpcurvecopy);
		output.curve = tmpcurve;

		return output;
	}

}
