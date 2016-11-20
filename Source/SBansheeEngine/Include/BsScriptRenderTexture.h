//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptRenderTarget.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for RenderTexture. */
	class BS_SCR_BE_EXPORT ScriptRenderTexture : public ScriptObject <ScriptRenderTexture, ScriptRenderTargetBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "RenderTexture")

	private:
		ScriptRenderTexture(MonoObject* instance);
	};

	/** @} */
}