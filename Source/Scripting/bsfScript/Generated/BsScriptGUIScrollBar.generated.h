//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"

namespace bs { class GUIScrollBar; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIScrollBarWrapperBase : public ScriptGUIElementWrapper
	{
	public:
		using ScriptGUIElementWrapper::ScriptGUIElementWrapper;

		virtual void RegisterEvents();
		void OnScrollOrResize(float p0, float p1);

		typedef void(B3D_THUNKCALL *OnScrollOrResizeThunkDefinition) (MonoObject*, float p0, float p1, MonoException**);
		static OnScrollOrResizeThunkDefinition OnScrollOrResizeThunk;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIScrollBar : public TScriptGUIElementWrapper<GUIScrollBar, ScriptGUIScrollBar, ScriptGUIScrollBarWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIScrollBar")

		ScriptGUIScrollBar(GUIScrollBar* nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetScrollHandlePosition(ScriptGUIScrollBarWrapperBase* self, float pct);
		static float InternalGetScrollHandlePosition(ScriptGUIScrollBarWrapperBase* self);
		static void InternalSetScrollHandleSize(ScriptGUIScrollBarWrapperBase* self, float pct);
		static float InternalGetScrollHandleSize(ScriptGUIScrollBarWrapperBase* self);
	};
}
