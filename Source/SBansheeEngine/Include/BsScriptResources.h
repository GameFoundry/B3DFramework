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

	/**	Interop class between C++ & CLR for Resources & GameResourceManager. */
	class BS_SCR_BE_EXPORT ScriptResources : public ScriptObject<ScriptResources>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Resources")

	private:
		ScriptResources(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* internal_Load(MonoString* path, bool keepLoaded);
		static MonoObject* internal_LoadRef(MonoObject* reference, bool keepLoaded);
		static void internal_Release(ScriptResourceBase* resource);
		static void internal_ReleaseRef(ScriptResourceRef* resource);
		static void internal_UnloadUnused();
	};

	/** @} */
}