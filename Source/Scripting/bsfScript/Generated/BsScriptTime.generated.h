//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfUtility/Utility/BsTime.h"
#include "BsScriptTypeDefinition.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptTime : public TScriptTypeDefinition<ScriptTime>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Time")

		ScriptTime();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static float InternalGetRealTimeInSeconds();
		static uint64_t InternalGetRealTimeInMilliseconds();
		static float InternalGetSimulationTimeInSeconds();
		static void InternalSetSimulationTimeScale(float scale);
		static float InternalGetSimulationTimeScale();
		static void InternalResetSimulationTime();
		static void InternalSetSimulationTimePaused(bool paused);
		static float InternalGetFrameDelta();
		static uint64_t InternalGetCurrentFrameIndex();
		static uint64_t InternalGetTimePrecise();
	};
}
