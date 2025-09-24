//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptTime.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Utility/Utility/B3DTime.h"

namespace b3d
{
	ScriptTime::ScriptTime()
		:TScriptTypeDefinition()
	{
	}

	void ScriptTime::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRealTimeInSeconds", (void*)&ScriptTime::InternalGetRealTimeInSeconds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRealTimeInMilliseconds", (void*)&ScriptTime::InternalGetRealTimeInMilliseconds);
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
