//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	class ScriptCCamera;

	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GUIWidget. */
	class BS_SCR_BE_EXPORT ScriptGUIWidget : public ScriptObject <ScriptGUIWidget>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "NativeGUIWidget")

		/**	Gets the wrapped native GUIWidget object. */
		SPtr<GUIWidget> getInternal() const { return mGUIWidget; }

	private:
		ScriptGUIWidget(MonoObject* managedInstance);
		~ScriptGUIWidget();

		/**	Destroys the internal GUI widget handler object. */
		void destroy(bool destroyPanel = true);

		/** @copydoc ScriptObjectBase::_onManagedInstanceDeleted */
		void _onManagedInstanceDeleted(bool assemblyRefresh) override;

		SPtr<GUIWidget> mGUIWidget;
		ScriptGUILayout* mPanel;

		static MonoField* sGUIPanelField;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_Create(MonoObject* instance);
		static void internal_UpdateTransform(ScriptGUIWidget* instance, ScriptSceneObject* parentSO);
		static void internal_UpdateMainCamera(ScriptGUIWidget* instance, ScriptCCamera* camera);
		static void internal_SetSkin(ScriptGUIWidget* instance, ScriptGUISkin* skin);
		static void internal_SetCamera(ScriptGUIWidget* instance, ScriptCCamera* camera);
		static void internal_SetDepth(ScriptGUIWidget* instance, INT8 value);
		static INT8 internal_GetDepth(ScriptGUIWidget* instance);
		static void internal_Destroy(ScriptGUIWidget* instance);
	};

	/** @} */
}
