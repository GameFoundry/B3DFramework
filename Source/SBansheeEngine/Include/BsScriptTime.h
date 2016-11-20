//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Time. */
	class BS_SCR_BE_EXPORT ScriptTime : public ScriptObject <ScriptTime>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Time")

	private:
		ScriptTime(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static float internal_getRealElapsed();
		static float internal_getElapsed();
		static float internal_getFrameDelta();
		static UINT64 internal_getFrameNumber();
		static UINT64 internal_getPrecise();
	};

	/** @} */
}