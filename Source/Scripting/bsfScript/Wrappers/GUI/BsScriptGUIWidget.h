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
	class BS_SCR_BE_EXPORT ScriptGUIWidget : public ScriptObject<ScriptGUIWidget>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "NativeGUIWidget")

		/**	Gets the wrapped native GUIWidget object. */
		SPtr<GUIWidget> GetInternal() const { return mGUIWidget; }

	private:
		ScriptGUIWidget(MonoObject* managedInstance);
		~ScriptGUIWidget();

		/**	Destroys the internal GUI widget handler object. */
		void Destroy(bool destroyPanel = true);

		/** @copydoc ScriptObjectBase::_onManagedInstanceDeleted */
		void OnManagedInstanceDeletedInternal(bool assemblyRefresh) override;

		SPtr<GUIWidget> mGUIWidget;
		ScriptGUILayout* mPanel;

		static MonoField* sGUIPanelField;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreate(MonoObject* instance);
		static void InternalUpdateTransform(ScriptGUIWidget* instance, ScriptSceneObject* parentSO);
		static void InternalUpdateMainCamera(ScriptGUIWidget* instance, ScriptCCamera* camera);
		static void InternalSetSkin(ScriptGUIWidget* instance, ScriptGUISkin* skin);
		static void InternalSetCamera(ScriptGUIWidget* instance, ScriptCCamera* camera);
		static void InternalSetDepth(ScriptGUIWidget* instance, i8 value);
		static i8 InternalGetDepth(ScriptGUIWidget* instance);
		static void InternalDestroy(ScriptGUIWidget* instance);
	};

	/** @} */
} // namespace bs
