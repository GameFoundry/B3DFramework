//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUIElement.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Engine/GUI/B3DGUIElement.h"
#include "B3DScriptTUnitValue.generated.h"
#include "B3DScriptTVector2.generated.h"
#include "B3DScriptTSize2.generated.h"
#include "B3DScriptGUIElement.generated.h"
#include "B3DScriptTArea2.generated.h"
#include "B3DScriptTVector2.generated.h"
#include "B3DScriptTArea2.generated.h"

namespace b3d
{
	ScriptGUIElement::ScriptGUIElement(GUIElement* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUIElement::~ScriptGUIElement()
	{
		UnregisterEvents();
	}

	void ScriptGUIElement::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPosition", (void*)&ScriptGUIElement::InternalSetPosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPosition0", (void*)&ScriptGUIElement::InternalSetPosition0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetWidth", (void*)&ScriptGUIElement::InternalSetWidth);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFlexibleWidth", (void*)&ScriptGUIElement::InternalSetFlexibleWidth);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHeight", (void*)&ScriptGUIElement::InternalSetHeight);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFlexibleHeight", (void*)&ScriptGUIElement::InternalSetFlexibleHeight);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSize", (void*)&ScriptGUIElement::InternalSetSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ResetSizeConstraints", (void*)&ScriptGUIElement::InternalResetSizeConstraints);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHidden", (void*)&ScriptGUIElement::InternalSetHidden);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetActive", (void*)&ScriptGUIElement::InternalSetActive);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDisabled", (void*)&ScriptGUIElement::InternalSetDisabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CalculateSizeInLayout", (void*)&ScriptGUIElement::InternalCalculateSizeInLayout);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CalculatePositionRelativeTo", (void*)&ScriptGUIElement::InternalCalculatePositionRelativeTo);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CalculateAbsoluteBoundsRelativeTo", (void*)&ScriptGUIElement::InternalCalculateAbsoluteBoundsRelativeTo);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CalculateAbsoluteBounds", (void*)&ScriptGUIElement::InternalCalculateAbsoluteBounds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_CalculateScreenBounds", (void*)&ScriptGUIElement::InternalCalculateScreenBounds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_WidgetToElementSpace", (void*)&ScriptGUIElement::InternalWidgetToElementSpace);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ElementToWidgetSpace", (void*)&ScriptGUIElement::InternalElementToWidgetSpace);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_WidgetToElementSpace0", (void*)&ScriptGUIElement::InternalWidgetToElementSpace0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ElementToWidgetSpace0", (void*)&ScriptGUIElement::InternalElementToWidgetSpace0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Destroy", (void*)&ScriptGUIElement::InternalDestroy);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_UpdateLayoutIfDirty", (void*)&ScriptGUIElement::InternalUpdateLayoutIfDirty);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetParent", (void*)&ScriptGUIElement::InternalGetParent);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsHidden", (void*)&ScriptGUIElement::InternalIsHidden);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsActive", (void*)&ScriptGUIElement::InternalIsActive);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsDisabled", (void*)&ScriptGUIElement::InternalIsDisabled);

	}

	MonoObject* ScriptGUIElement::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUIElement::InternalSetPosition(ScriptGUIElement* self, TUnitValue<int32_t, LogicalPixel>* x, TUnitValue<int32_t, LogicalPixel>* y)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIElement*>(self->GetNativeObject())->SetPosition(*x, *y);
	}

	void ScriptGUIElement::InternalSetPosition0(ScriptGUIElement* self, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* position)
	{
		if(!self->IsNativeObjectValid())
			return;

		TVector2<TUnitValue<int32_t, LogicalPixel>> tmpposition;
		tmpposition = ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::FromInterop(*position);
		static_cast<GUIElement*>(self->GetNativeObject())->SetPosition(tmpposition);
	}

	void ScriptGUIElement::InternalSetWidth(ScriptGUIElement* self, TUnitValue<int32_t, LogicalPixel>* width)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIElement*>(self->GetNativeObject())->SetWidth(*width);
	}

	void ScriptGUIElement::InternalSetFlexibleWidth(ScriptGUIElement* self, TUnitValue<int32_t, LogicalPixel>* minWidth, TUnitValue<int32_t, LogicalPixel>* maxWidth)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIElement*>(self->GetNativeObject())->SetFlexibleWidth(*minWidth, *maxWidth);
	}

	void ScriptGUIElement::InternalSetHeight(ScriptGUIElement* self, TUnitValue<int32_t, LogicalPixel>* height)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIElement*>(self->GetNativeObject())->SetHeight(*height);
	}

	void ScriptGUIElement::InternalSetFlexibleHeight(ScriptGUIElement* self, TUnitValue<int32_t, LogicalPixel>* minHeight, TUnitValue<int32_t, LogicalPixel>* maxHeight)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIElement*>(self->GetNativeObject())->SetFlexibleHeight(*minHeight, *maxHeight);
	}

	void ScriptGUIElement::InternalSetSize(ScriptGUIElement* self, __TSize2_TUnitValue_int32_t__LogicalPixel__Interop* size)
	{
		if(!self->IsNativeObjectValid())
			return;

		TSize2<TUnitValue<int32_t, LogicalPixel>> tmpsize;
		tmpsize = ScriptTSize2_TUnitValue_int32_t__LogicalPixel__::FromInterop(*size);
		static_cast<GUIElement*>(self->GetNativeObject())->SetSize(tmpsize);
	}

	void ScriptGUIElement::InternalResetSizeConstraints(ScriptGUIElement* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIElement*>(self->GetNativeObject())->ResetSizeConstraints();
	}

	void ScriptGUIElement::InternalSetHidden(ScriptGUIElement* self, bool hidden)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIElement*>(self->GetNativeObject())->SetHidden(hidden);
	}

	void ScriptGUIElement::InternalSetActive(ScriptGUIElement* self, bool active)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIElement*>(self->GetNativeObject())->SetActive(active);
	}

	void ScriptGUIElement::InternalSetDisabled(ScriptGUIElement* self, bool disabled)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIElement*>(self->GetNativeObject())->SetDisabled(disabled);
	}

	void ScriptGUIElement::InternalCalculateSizeInLayout(ScriptGUIElement* self, __TSize2_TUnitValue_int32_t__LogicalPixel__Interop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TSize2<TUnitValue<int32_t, LogicalPixel>> tmp__output;
		tmp__output = static_cast<GUIElement*>(self->GetNativeObject())->CalculateSizeInLayout();

		__TSize2_TUnitValue_int32_t__LogicalPixel__Interop interop__output;
		interop__output = ScriptTSize2_TUnitValue_int32_t__LogicalPixel__::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptTSize2_TUnitValue_int32_t__LogicalPixel__::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptGUIElement::InternalCalculatePositionRelativeTo(ScriptGUIElement* self, MonoObject* relativeTo, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		GUIElement* tmprelativeTo = nullptr;
		ScriptGUIElement* scriptObjectWrapperrelativeTo;
		scriptObjectWrapperrelativeTo = ScriptGUIElement::GetScriptObjectWrapper(relativeTo);
		if(scriptObjectWrapperrelativeTo != nullptr)
			tmprelativeTo = static_cast<GUIElement*>(scriptObjectWrapperrelativeTo->GetNativeObject());
		TVector2<TUnitValue<int32_t, LogicalPixel>> tmp__output;
		tmp__output = static_cast<GUIElement*>(self->GetNativeObject())->CalculatePositionRelativeTo(tmprelativeTo);

		__TVector2_TUnitValue_int32_t__LogicalPixel__Interop interop__output;
		interop__output = ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptGUIElement::InternalCalculateAbsoluteBoundsRelativeTo(ScriptGUIElement* self, MonoObject* relativeTo, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		GUIElement* tmprelativeTo = nullptr;
		ScriptGUIElement* scriptObjectWrapperrelativeTo;
		scriptObjectWrapperrelativeTo = ScriptGUIElement::GetScriptObjectWrapper(relativeTo);
		if(scriptObjectWrapperrelativeTo != nullptr)
			tmprelativeTo = static_cast<GUIElement*>(scriptObjectWrapperrelativeTo->GetNativeObject());
		TArea2<TUnitValue<int32_t, PhysicalPixel>, TUnitValue<int32_t, PhysicalPixel>> tmp__output;
		tmp__output = static_cast<GUIElement*>(self->GetNativeObject())->CalculateAbsoluteBoundsRelativeTo(tmprelativeTo);

		__TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop interop__output;
		interop__output = ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptGUIElement::InternalCalculateAbsoluteBounds(ScriptGUIElement* self, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TArea2<TUnitValue<int32_t, PhysicalPixel>, TUnitValue<int32_t, PhysicalPixel>> tmp__output;
		tmp__output = static_cast<GUIElement*>(self->GetNativeObject())->CalculateAbsoluteBounds();

		__TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop interop__output;
		interop__output = ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptGUIElement::InternalCalculateScreenBounds(ScriptGUIElement* self, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TArea2<TUnitValue<int32_t, PhysicalPixel>, TUnitValue<int32_t, PhysicalPixel>> tmp__output;
		tmp__output = static_cast<GUIElement*>(self->GetNativeObject())->CalculateScreenBounds();

		__TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop interop__output;
		interop__output = ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptGUIElement::InternalWidgetToElementSpace(ScriptGUIElement* self, __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop* point, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector2<TUnitValue<int32_t, PhysicalPixel>> tmppoint;
		tmppoint = ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__::FromInterop(*point);
		TVector2<TUnitValue<int32_t, LogicalPixel>> tmp__output;
		tmp__output = static_cast<GUIElement*>(self->GetNativeObject())->WidgetToElementSpace(tmppoint);

		__TVector2_TUnitValue_int32_t__LogicalPixel__Interop interop__output;
		interop__output = ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptGUIElement::InternalElementToWidgetSpace(ScriptGUIElement* self, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* point, __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector2<TUnitValue<int32_t, LogicalPixel>> tmppoint;
		tmppoint = ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::FromInterop(*point);
		TVector2<TUnitValue<int32_t, PhysicalPixel>> tmp__output;
		tmp__output = static_cast<GUIElement*>(self->GetNativeObject())->ElementToWidgetSpace(tmppoint);

		__TVector2_TUnitValue_int32_t__PhysicalPixel__Interop interop__output;
		interop__output = ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptGUIElement::InternalWidgetToElementSpace0(ScriptGUIElement* self, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* area, __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TArea2<TUnitValue<int32_t, PhysicalPixel>, TUnitValue<int32_t, PhysicalPixel>> tmparea;
		tmparea = ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::FromInterop(*area);
		TArea2<TUnitValue<int32_t, LogicalPixel>, TUnitValue<int32_t, LogicalPixel>> tmp__output;
		tmp__output = static_cast<GUIElement*>(self->GetNativeObject())->WidgetToElementSpace(tmparea);

		__TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop interop__output;
		interop__output = ScriptTArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptTArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptGUIElement::InternalElementToWidgetSpace0(ScriptGUIElement* self, __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop* area, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TArea2<TUnitValue<int32_t, LogicalPixel>, TUnitValue<int32_t, LogicalPixel>> tmparea;
		tmparea = ScriptTArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__::FromInterop(*area);
		TArea2<TUnitValue<int32_t, PhysicalPixel>, TUnitValue<int32_t, PhysicalPixel>> tmp__output;
		tmp__output = static_cast<GUIElement*>(self->GetNativeObject())->ElementToWidgetSpace(tmparea);

		__TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop interop__output;
		interop__output = ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptGUIElement::InternalDestroy(ScriptGUIElement* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIElement*>(self->GetNativeObject())->Destroy();
	}

	void ScriptGUIElement::InternalUpdateLayoutIfDirty(ScriptGUIElement* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIElement*>(self->GetNativeObject())->UpdateLayoutIfDirty();
	}

	MonoObject* ScriptGUIElement::InternalGetParent(ScriptGUIElement* self)
	{
		GUIElement* tmp__output = nullptr;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUIElement*>(self->GetNativeObject())->GetParent();

		MonoObject* __output;
		__output = ScriptGUIElement::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	bool ScriptGUIElement::InternalIsHidden(ScriptGUIElement* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUIElement*>(self->GetNativeObject())->IsHidden();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptGUIElement::InternalIsActive(ScriptGUIElement* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUIElement*>(self->GetNativeObject())->IsActive();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptGUIElement::InternalIsDisabled(ScriptGUIElement* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUIElement*>(self->GetNativeObject())->IsDisabled();

		bool __output;
		__output = tmp__output;

		return __output;
	}

}
