//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "GUI/BsProfilerOverlay.h"

namespace bs
{
	class ScriptCCamera;

	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for ProfilerOverlayInternal. */
	class BS_SCR_BE_EXPORT ScriptProfilerOverlayInternal : public ScriptObject < ScriptProfilerOverlayInternal >
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "ProfilerOverlayInternal")

		/**	Returns the native profiler overlay instance. */
		ProfilerOverlayInternal* getInternal() const { return mProfilerOverlayInternal; }

	private:
		ScriptProfilerOverlayInternal(MonoObject* managedInstance, const SPtr<Camera>& camera);
		~ScriptProfilerOverlayInternal();

		ProfilerOverlayInternal* mProfilerOverlayInternal;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, ScriptCCamera* camera);
		static void internal_SetType(ScriptProfilerOverlayInternal* thisPtr, ProfilerOverlayType type);
		static void internal_Update(ScriptProfilerOverlayInternal* thisPtr);
		static void internal_DestroyInstance(ScriptProfilerOverlayInternal* thisPtr);
	};

	/** @} */
}