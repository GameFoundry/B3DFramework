//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTime.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfUtility/Utility/BsTime.h"

namespace bs
{
	ScriptTime::ScriptTime()
		:TScriptTypeDefinition()
	{
	}

	void ScriptTime::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRealTimeInSeconds", (void*)&ScriptTime::InternalGetRealTimeInSeconds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRealTimeInMilliseconds", (void*)&ScriptTime::InternalGetRealTimeInMilliseconds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSimulationTimeInSeconds", (void*)&ScriptTime::InternalGetSimulationTimeInSeconds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSimulationTimeScale", (void*)&ScriptTime::InternalSetSimulationTimeScale);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSimulationTimeScale", (void*)&ScriptTime::InternalGetSimulationTimeScale);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ResetSimulationTime", (void*)&ScriptTime::InternalResetSimulationTime);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSimulationTimePaused", (void*)&ScriptTime::InternalSetSimulationTimePaused);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFrameDelta", (void*)&ScriptTime::InternalGetFrameDelta);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCurrentFrameIndex", (void*)&ScriptTime::InternalGetCurrentFrameIndex);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTimePrecise", (void*)&ScriptTime::InternalGetTimePrecise);

	}

	float ScriptTime::InternalGetRealTimeInSeconds()
	{
		float tmp__output;
		tmp__output = Time::Instance().GetRealTimeInSeconds();

		float __output;
		__output = tmp__output;

		return __output;
	}

	uint64_t ScriptTime::InternalGetRealTimeInMilliseconds()
	{
		uint64_t tmp__output;
		tmp__output = Time::Instance().GetRealTimeInMilliseconds();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptTime::InternalGetSimulationTimeInSeconds()
	{
		float tmp__output;
		tmp__output = Time::Instance().GetSimulationTimeInSeconds();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTime::InternalSetSimulationTimeScale(float scale)
	{
		Time::Instance().SetSimulationTimeScale(scale);
	}

	float ScriptTime::InternalGetSimulationTimeScale()
	{
		float tmp__output;
		tmp__output = Time::Instance().GetSimulationTimeScale();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptTime::InternalResetSimulationTime()
	{
		Time::Instance().ResetSimulationTime();
	}

	void ScriptTime::InternalSetSimulationTimePaused(bool paused)
	{
		Time::Instance().SetSimulationTimePaused(paused);
	}

	float ScriptTime::InternalGetFrameDelta()
	{
		float tmp__output;
		tmp__output = Time::Instance().GetFrameDelta();

		float __output;
		__output = tmp__output;

		return __output;
	}

	uint64_t ScriptTime::InternalGetCurrentFrameIndex()
	{
		uint64_t tmp__output;
		tmp__output = Time::Instance().GetCurrentFrameIndex();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	uint64_t ScriptTime::InternalGetTimePrecise()
	{
		uint64_t tmp__output;
		tmp__output = Time::Instance().GetTimePrecise();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}
}
