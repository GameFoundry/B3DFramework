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

	/**	Interop class between C++ & CLR for a global always-accessible GUIWidget. */
	class BS_SCR_BE_EXPORT ScriptGUI : public ScriptObject <ScriptGUI>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "GUI")

		/**	Creates the globally accessible GUIWidget and makes the system ready to use. */
		static void startUp();

		/** Updates the main camera the widget renders to, if it changes. Should be called every frame. */
		static void update();

		/** Destroys the globally accessible GUIWidget and all its GUI elements. */
		static void shutDown();

	private:
		ScriptGUI(MonoObject* managedInstance);
		~ScriptGUI();

		static SPtr<GUIWidget> sGUIWidget;
		static ScriptGUILayout* sPanel;
		static MonoMethod* sGUIPanelMethod;
		static HEvent sDomainUnloadConn;
		static HEvent sDomainLoadConn;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_SetSkin(ScriptGUISkin* skin);
	};

	/** @} */
}