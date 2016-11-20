//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsGUIElementStyle.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Contains native representation of the GUIElementStateStyle structure. */
	struct ScriptGUIElementStateStyleStruct // Note: Must match C# struct GUIElementStateStyle.
	{
		MonoObject* texture;
		Color textColor;
	};

	/**	Performs conversion between managed GUIElementStateStyle and native GUIElementStyle::GUIElementStateStyle. */
	class BS_SCR_BE_EXPORT ScriptGUIElementStateStyle : public ScriptObject<ScriptGUIElementStateStyle>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIElementStateStyle")

		/**
		 * Creates a new managed instance of GUIElementStateStyle.
		 *
		 * @param[in]	state	Native GUI element style state to copy to the managed instance.
		 */
		 static ScriptGUIElementStateStyleStruct toManaged(const GUIElementStyle::GUIElementStateStyle& state);

		/**	Converts a managed instance of GUIElementStateStyle to a native GUI element style state. */
		static GUIElementStyle::GUIElementStateStyle toNative(const ScriptGUIElementStateStyleStruct& instance);

	private:
		ScriptGUIElementStateStyle(MonoObject* instance);
	};

	/** @} */
}