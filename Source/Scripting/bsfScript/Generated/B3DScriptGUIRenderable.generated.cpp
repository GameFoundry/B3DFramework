//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUIRenderable.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Engine/GUI/B3DGUIRenderable.h"
#include "B3DScriptColor.generated.h"

namespace b3d
{
	ScriptGUIRenderable::ScriptGUIRenderable(GUIRenderable* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUIRenderable::~ScriptGUIRenderable()
	{
		UnregisterEvents();
	}

	void ScriptGUIRenderable::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetStyleSheetClass", (void*)&ScriptGUIRenderable::InternalGetStyleSheetClass);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetStyleSheetClass", (void*)&ScriptGUIRenderable::InternalSetStyleSheetClass);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptGUIRenderable::InternalSetTint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTint", (void*)&ScriptGUIRenderable::InternalGetTint);

	}

	MonoObject* ScriptGUIRenderable::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoString* ScriptGUIRenderable::InternalGetStyleSheetClass(ScriptGUIRenderableWrapperBase* self)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUIRenderable*>(self->GetNativeObject())->GetStyleSheetClass();

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptGUIRenderable::InternalSetStyleSheetClass(ScriptGUIRenderableWrapperBase* self, MonoString* styleClass)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpstyleClass;
		tmpstyleClass = MonoUtil::MonoToString(styleClass);
		static_cast<GUIRenderable*>(self->GetNativeObject())->SetStyleSheetClass(tmpstyleClass);
	}

	void ScriptGUIRenderable::InternalSetTint(ScriptGUIRenderableWrapperBase* self, Color* color)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIRenderable*>(self->GetNativeObject())->SetTint(*color);
	}

	void ScriptGUIRenderable::InternalGetTint(ScriptGUIRenderableWrapperBase* self, Color* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		Color tmp__output;
		tmp__output = static_cast<GUIRenderable*>(self->GetNativeObject())->GetTint();

		*__output = tmp__output;
	}
}
