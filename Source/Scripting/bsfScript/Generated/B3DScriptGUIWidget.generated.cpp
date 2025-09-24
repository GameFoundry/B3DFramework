//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIWidget.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIWidget.h"
#include "BsScriptGUIPanel.generated.h"
#include "BsScriptTVector2.generated.h"
#include "BsScriptTArea2.generated.h"

namespace b3d
{
	ScriptGUIWidget::ScriptGUIWidget(const GameObjectHandle<GUIWidget>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUIWidget::~ScriptGUIWidget()
	{
		UnregisterEvents();
	}

	void ScriptGUIWidget::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPanel", (void*)&ScriptGUIWidget::InternalGetPanel);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDepth", (void*)&ScriptGUIWidget::InternalSetDepth);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDepth", (void*)&ScriptGUIWidget::InternalGetDepth);
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
		GUIPanel* tmp__output = nullptr;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUIWidget*>(self->GetNativeObject())->GetPanel();

		MonoObject* __output;
		__output = ScriptGUIPanel::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptGUIWidget::InternalSetDepth(ScriptGUIWidget* self, uint8_t depth)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIWidget*>(self->GetNativeObject())->SetDepth(depth);
	}

	uint8_t ScriptGUIWidget::InternalGetDepth(ScriptGUIWidget* self)
	{
		uint8_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUIWidget*>(self->GetNativeObject())->GetDepth();

		uint8_t __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptGUIWidget::InternalInBounds(ScriptGUIWidget* self, __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop* position)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		TVector2<TUnitValue<int32_t, PhysicalPixel>> tmpposition;
		tmpposition = ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__::FromInterop(*position);
		tmp__output = static_cast<GUIWidget*>(self->GetNativeObject())->InBounds(tmpposition);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIWidget::InternalGetBounds(ScriptGUIWidget* self, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TArea2<TUnitValue<int32_t, PhysicalPixel>, TUnitValue<int32_t, PhysicalPixel>> tmp__output;
		tmp__output = static_cast<GUIWidget*>(self->GetNativeObject())->GetBounds();

		__TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop interop__output;
		interop__output = ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::GetMetaData()->ScriptClass->GetInternalClass());
	}
}
