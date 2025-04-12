//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIScrollArea.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIScrollArea.h"
#include "BsScriptTUnitValue.generated.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"
#include "BsScriptTArea2.generated.h"
#include "BsScriptTUnitValue.generated.h"
#include "BsScriptGUIScrollArea.generated.h"
#include "BsScriptGUIScrollAreaContent.generated.h"
#include "BsScriptGUIOption.generated.h"

namespace bs
{
	ScriptGUIScrollArea::ScriptGUIScrollArea(GUIScrollArea* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptGUIScrollArea::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLayout", (void*)&ScriptGUIScrollArea::InternalGetLayout);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScrollUp", (void*)&ScriptGUIScrollArea::InternalScrollUp);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScrollDown", (void*)&ScriptGUIScrollArea::InternalScrollDown);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScrollLeft", (void*)&ScriptGUIScrollArea::InternalScrollLeft);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScrollRight", (void*)&ScriptGUIScrollArea::InternalScrollRight);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScrollUp0", (void*)&ScriptGUIScrollArea::InternalScrollUp0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScrollDown0", (void*)&ScriptGUIScrollArea::InternalScrollDown0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScrollLeft0", (void*)&ScriptGUIScrollArea::InternalScrollLeft0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScrollRight0", (void*)&ScriptGUIScrollArea::InternalScrollRight0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScrollToVertical", (void*)&ScriptGUIScrollArea::InternalScrollToVertical);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScrollToHorizontal", (void*)&ScriptGUIScrollArea::InternalScrollToHorizontal);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetVerticalScroll", (void*)&ScriptGUIScrollArea::InternalGetVerticalScroll);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHorizontalScroll", (void*)&ScriptGUIScrollArea::InternalGetHorizontalScroll);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetContentBounds", (void*)&ScriptGUIScrollArea::InternalGetContentBounds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnableCulling", (void*)&ScriptGUIScrollArea::InternalSetEnableCulling);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetScrollBarSize", (void*)&ScriptGUIScrollArea::InternalGetScrollBarSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIScrollArea::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIScrollArea::InternalCreate0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create1", (void*)&ScriptGUIScrollArea::InternalCreate1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create2", (void*)&ScriptGUIScrollArea::InternalCreate2);

	}

	MonoObject* ScriptGUIScrollArea::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptGUIScrollArea::InternalGetLayout(ScriptGUIScrollArea* self)
	{
		GUILayout* tmp__output = nullptr;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUIScrollArea*>(self->GetNativeObject())->GetLayout();

		MonoObject* __output;
		__output = ScriptGUILayout::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptGUIScrollArea::InternalScrollUp(ScriptGUIScrollArea* self, TUnitValue<int32_t, PhysicalPixel>* pixels)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIScrollArea*>(self->GetNativeObject())->ScrollUp(*pixels);
	}

	void ScriptGUIScrollArea::InternalScrollDown(ScriptGUIScrollArea* self, TUnitValue<int32_t, PhysicalPixel>* pixels)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIScrollArea*>(self->GetNativeObject())->ScrollDown(*pixels);
	}

	void ScriptGUIScrollArea::InternalScrollLeft(ScriptGUIScrollArea* self, TUnitValue<int32_t, PhysicalPixel>* pixels)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIScrollArea*>(self->GetNativeObject())->ScrollLeft(*pixels);
	}

	void ScriptGUIScrollArea::InternalScrollRight(ScriptGUIScrollArea* self, TUnitValue<int32_t, PhysicalPixel>* pixels)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIScrollArea*>(self->GetNativeObject())->ScrollRight(*pixels);
	}

	void ScriptGUIScrollArea::InternalScrollUp0(ScriptGUIScrollArea* self, float percent)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIScrollArea*>(self->GetNativeObject())->ScrollUp(percent);
	}

	void ScriptGUIScrollArea::InternalScrollDown0(ScriptGUIScrollArea* self, float percent)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIScrollArea*>(self->GetNativeObject())->ScrollDown(percent);
	}

	void ScriptGUIScrollArea::InternalScrollLeft0(ScriptGUIScrollArea* self, float percent)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIScrollArea*>(self->GetNativeObject())->ScrollLeft(percent);
	}

	void ScriptGUIScrollArea::InternalScrollRight0(ScriptGUIScrollArea* self, float percent)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIScrollArea*>(self->GetNativeObject())->ScrollRight(percent);
	}

	void ScriptGUIScrollArea::InternalScrollToVertical(ScriptGUIScrollArea* self, float pct)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIScrollArea*>(self->GetNativeObject())->ScrollToVertical(pct);
	}

	void ScriptGUIScrollArea::InternalScrollToHorizontal(ScriptGUIScrollArea* self, float pct)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIScrollArea*>(self->GetNativeObject())->ScrollToHorizontal(pct);
	}

	float ScriptGUIScrollArea::InternalGetVerticalScroll(ScriptGUIScrollArea* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUIScrollArea*>(self->GetNativeObject())->GetVerticalScroll();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptGUIScrollArea::InternalGetHorizontalScroll(ScriptGUIScrollArea* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUIScrollArea*>(self->GetNativeObject())->GetHorizontalScroll();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIScrollArea::InternalGetContentBounds(ScriptGUIScrollArea* self, __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TArea2<TUnitValue<int32_t, PhysicalPixel>, TUnitValue<int32_t, PhysicalPixel>> tmp__output;
		tmp__output = static_cast<GUIScrollArea*>(self->GetNativeObject())->GetContentBounds();

		__TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop interop__output;
		interop__output = ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptGUIScrollArea::InternalSetEnableCulling(ScriptGUIScrollArea* self, bool enable)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIScrollArea*>(self->GetNativeObject())->SetEnableCulling(enable);
	}

	void ScriptGUIScrollArea::InternalGetScrollBarSize(ScriptGUIScrollArea* self, TUnitValue<int32_t, LogicalPixel>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TUnitValue<int32_t, LogicalPixel> tmp__output;
		tmp__output = static_cast<GUIScrollArea*>(self->GetNativeObject())->GetScrollBarSize();

		*__output = tmp__output;
	}

	void ScriptGUIScrollArea::InternalCreate(MonoObject* scriptObject, GUIScrollAreaContent* contents, MonoString* styleClass, MonoArray* options)
	{
		String tmpstyleClass;
		tmpstyleClass = MonoUtil::MonoToString(styleClass);
		TInlineArray<GUIOption, 4> nativeArrayoptions;
		if(options != nullptr)
		{
			ScriptArray scriptArrayoptions(options);
			nativeArrayoptions.resize(scriptArrayoptions.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayoptions.Size(); elementIndex++)
			{
				nativeArrayoptions[elementIndex] = ScriptGUIOption::FromInterop(scriptArrayoptions.Get<__GUIOptionInterop>(elementIndex));
			}
		}
		GUIScrollArea* nativeObject = GUIScrollArea::Create(*contents, tmpstyleClass, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIScrollArea>(nativeObject, scriptObject);
	}

	void ScriptGUIScrollArea::InternalCreate0(MonoObject* scriptObject, GUIScrollAreaContent* contents, MonoArray* options)
	{
		TInlineArray<GUIOption, 4> nativeArrayoptions;
		if(options != nullptr)
		{
			ScriptArray scriptArrayoptions(options);
			nativeArrayoptions.resize(scriptArrayoptions.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayoptions.Size(); elementIndex++)
			{
				nativeArrayoptions[elementIndex] = ScriptGUIOption::FromInterop(scriptArrayoptions.Get<__GUIOptionInterop>(elementIndex));
			}
		}
		GUIScrollArea* nativeObject = GUIScrollArea::Create(*contents, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIScrollArea>(nativeObject, scriptObject);
	}

	void ScriptGUIScrollArea::InternalCreate1(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options)
	{
		String tmpstyleClass;
		tmpstyleClass = MonoUtil::MonoToString(styleClass);
		TInlineArray<GUIOption, 4> nativeArrayoptions;
		if(options != nullptr)
		{
			ScriptArray scriptArrayoptions(options);
			nativeArrayoptions.resize(scriptArrayoptions.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayoptions.Size(); elementIndex++)
			{
				nativeArrayoptions[elementIndex] = ScriptGUIOption::FromInterop(scriptArrayoptions.Get<__GUIOptionInterop>(elementIndex));
			}
		}
		GUIScrollArea* nativeObject = GUIScrollArea::Create(tmpstyleClass, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIScrollArea>(nativeObject, scriptObject);
	}

	void ScriptGUIScrollArea::InternalCreate2(MonoObject* scriptObject, MonoArray* options)
	{
		TInlineArray<GUIOption, 4> nativeArrayoptions;
		if(options != nullptr)
		{
			ScriptArray scriptArrayoptions(options);
			nativeArrayoptions.resize(scriptArrayoptions.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayoptions.Size(); elementIndex++)
			{
				nativeArrayoptions[elementIndex] = ScriptGUIOption::FromInterop(scriptArrayoptions.Get<__GUIOptionInterop>(elementIndex));
			}
		}
		GUIScrollArea* nativeObject = GUIScrollArea::Create(nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIScrollArea>(nativeObject, scriptObject);
	}
}
