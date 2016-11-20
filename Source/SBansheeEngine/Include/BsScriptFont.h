//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResource.h"
#include "BsScriptObject.h"
#include "BsFont.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Font. */
	class BS_SCR_BE_EXPORT ScriptFont : public TScriptResource<ScriptFont, Font>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Font")

		/**	Creates an empty, uninitialized managed instance of the resource interop object. */
		static MonoObject* createInstance();
	private:
		friend class ScriptResourceManager;

		ScriptFont(MonoObject* instance, const HFont& font);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* internal_GetBitmap(ScriptFont* instance, int size);
		static int internal_GetClosestSize(ScriptFont* instance, int size);
	};

	/**	Interop class between C++ & CLR for CharRange. */
	class BS_SCR_BE_EXPORT ScriptCharRange : public ScriptObject <ScriptCharRange>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "CharRange")

	private:
		ScriptCharRange(MonoObject* instance);
	};

	/** @} */
}