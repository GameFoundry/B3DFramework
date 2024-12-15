//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "../../../Foundation/bsfUtility/Image/BsColor.h"

namespace bs { class GUIRenderable; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIRenderableWrapperBase : public ScriptGUIElementWrapper
	{
	public:
		using ScriptGUIElementWrapper::ScriptGUIElementWrapper;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIRenderable : public TScriptGUIElementWrapper<GUIRenderable, ScriptGUIRenderable, ScriptGUIRenderableWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIRenderable")

		ScriptGUIRenderable(GUIRenderable* nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoString* InternalGetStyleSheetClass(ScriptGUIRenderableWrapperBase* self);
		static void InternalSetStyleSheetClass(ScriptGUIRenderableWrapperBase* self, MonoString* styleClass);
		static void InternalSetTint(ScriptGUIRenderableWrapperBase* self, Color* color);
		static void InternalGetTint(ScriptGUIRenderableWrapperBase* self, Color* __output);
	};
}
