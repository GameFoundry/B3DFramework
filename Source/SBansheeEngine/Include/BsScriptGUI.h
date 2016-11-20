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