//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsGUIContent.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIContent. */
	class BS_SCR_BE_EXPORT ScriptGUIContent : public ScriptObject<ScriptGUIContent>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUIContent")

		/**	Retrieves the title text in a managed GUIContent instance. */
		static const HString& getText(MonoObject* instance);

		/**	Retrieves the tooltip text in a managed GUIContent instance. */
		static const HString& getTooltip(MonoObject* instance);

		/**	Retrieves the content image in a managed GUIContent instance. */
		static GUIContentImages getImage(MonoObject* instance);

	private:
		ScriptGUIContent(MonoObject* instance);

		static MonoField* mTextField;
		static MonoField* mTooltipField;
		static MonoField* mImagesField;
	};

	/** @} */
}