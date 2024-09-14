//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "BsScriptGUIClickable.generated.h"

namespace bs { class GUIToggleable; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIToggleableWrapperBase : public ScriptGUIClickableWrapperBase
	{
	public:
		using ScriptGUIClickableWrapperBase::ScriptGUIClickableWrapperBase;

		virtual void RegisterEvents();
		void OnToggled(bool p0);

		typedef void(B3D_THUNKCALL *OnToggledThunkDefinition) (MonoObject*, bool p0, MonoException**);
		static OnToggledThunkDefinition OnToggledThunk;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIToggleable : public TScriptGUIElementWrapper<GUIToggleable, ScriptGUIToggleable, ScriptGUIToggleableWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIToggleable")

		ScriptGUIToggleable(GUIToggleable* nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetIsToggled(ScriptGUIToggleableWrapperBase* self, bool isToggled);
		static bool InternalIsToggled(ScriptGUIToggleableWrapperBase* self);
	};
}
