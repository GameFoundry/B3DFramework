//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCGUIWidget.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsCGUIWidget.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"
#include "BsScriptTVector2I.generated.h"

namespace bs
{
	ScriptGUIWidget::ScriptGUIWidget(const GameObjectHandle<CGUIWidget>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptGUIWidget::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPanel", (void*)&ScriptGUIWidget::InternalGetPanel);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDepth", (void*)&ScriptGUIWidget::InternalGetDepth);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDepth", (void*)&ScriptGUIWidget::InternalSetDepth);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_InBounds", (void*)&ScriptGUIWidget::InternalInBounds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBounds", (void*)&ScriptGUIWidget::InternalGetBounds);

	}

	MonoObject* ScriptGUIWidget::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptGUIWidget::InternalGetPanel(ScriptGUIWidget* self)
	{
		GUIPanel* tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CGUIWidget*>(self->GetNativeObject())->GetPanel();

		MonoObject* __output;
		__output = ScriptGUIPanel::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	uint8_t ScriptGUIWidget::InternalGetDepth(ScriptGUIWidget* self)
	{
		uint8_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CGUIWidget*>(self->GetNativeObject())->GetDepth();

		uint8_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIWidget::InternalSetDepth(ScriptGUIWidget* self, uint8_t depth)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<CGUIWidget*>(self->GetNativeObject())->SetDepth(depth);
	}

	bool ScriptGUIWidget::InternalInBounds(ScriptGUIWidget* self, TVector2I<int32_t>* position)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CGUIWidget*>(self->GetNativeObject())->InBounds(*position);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIWidget::InternalGetBounds(ScriptGUIWidget* self, Rect2I* __output)
	{
		if(!self->IsNativeObjectValid())
			{
				__output = {};
				return;
			}

		Rect2I tmp__output;
		tmp__output = static_cast<CGUIWidget*>(self->GetNativeObject())->GetBounds();

		*__output = tmp__output;
	}
}
