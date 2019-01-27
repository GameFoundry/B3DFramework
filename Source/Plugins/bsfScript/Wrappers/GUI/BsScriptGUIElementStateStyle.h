//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "GUI/BsGUIElementStyle.h"

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