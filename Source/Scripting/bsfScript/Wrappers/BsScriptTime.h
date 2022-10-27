//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Time. */
	class BS_SCR_BE_EXPORT ScriptTime : public ScriptObject<ScriptTime>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Time")

	private:
		ScriptTime(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static float InternalGetRealElapsed();
		static float InternalGetElapsed();
		static float InternalGetFrameDelta();
		static u64 InternalGetFrameNumber();
		static u64 InternalGetPrecise();
	};

	/** @} */
} // namespace bs
