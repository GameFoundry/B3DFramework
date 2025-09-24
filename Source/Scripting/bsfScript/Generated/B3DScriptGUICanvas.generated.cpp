//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUICanvas.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Engine/GUI/B3DGUICanvas.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "B3DScriptTVector2.generated.h"
#include "../../../Foundation/Core/Text/B3DFont.h"
#include "B3DScriptColor.generated.h"
#include "B3DScriptGUIOption.generated.h"
#include "../../../Foundation/Core/Image/B3DSpriteImage.h"
#include "B3DScriptGUICanvas.generated.h"
#include "B3DScriptTArea2.generated.h"

namespace b3d
{
	ScriptGUICanvas::ScriptGUICanvas(GUICanvas* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUICanvas::~ScriptGUICanvas()
	{
		UnregisterEvents();
	}

	void ScriptGUICanvas::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_DrawLine", (void*)&ScriptGUICanvas::InternalDrawLine);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_DrawPolyLine", (void*)&ScriptGUICanvas::InternalDrawPolyLine);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_DrawImage", (void*)&ScriptGUICanvas::InternalDrawImage);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_DrawTriangleStrip", (void*)&ScriptGUICanvas::InternalDrawTriangleStrip);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_DrawTriangleList", (void*)&ScriptGUICanvas::InternalDrawTriangleList);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_DrawText", (void*)&ScriptGUICanvas::InternalDrawText);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Clear", (void*)&ScriptGUICanvas::InternalClear);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUICanvas::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUICanvas::InternalCreate0);

	}

	MonoObject* ScriptGUICanvas::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUICanvas::InternalDrawLine(ScriptGUICanvas* self, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* a, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* b, Color* color, uint8_t depth)
	{
		if(!self->IsNativeObjectValid())
			return;

		TVector2<TUnitValue<int32_t, LogicalPixel>> tmpa;
		tmpa = ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::FromInterop(*a);
		TVector2<TUnitValue<int32_t, LogicalPixel>> tmpb;
		tmpb = ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::FromInterop(*b);
		static_cast<GUICanvas*>(self->GetNativeObject())->DrawLine(tmpa, tmpb, *color, depth);
	}

	void ScriptGUICanvas::InternalDrawPolyLine(ScriptGUICanvas* self, MonoArray* vertices, Color* color, uint8_t depth)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<TVector2<TUnitValue<int32_t, LogicalPixel>>> nativeArrayvertices;
		if(vertices != nullptr)
		{
			ScriptArray scriptArrayvertices(vertices);
			nativeArrayvertices.resize(scriptArrayvertices.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvertices.Size(); elementIndex++)
			{
				nativeArrayvertices[elementIndex] = ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::FromInterop(scriptArrayvertices.Get<__TVector2_TUnitValue_int32_t__LogicalPixel__Interop>(elementIndex));
			}

		}
		static_cast<GUICanvas*>(self->GetNativeObject())->DrawPolyLine(nativeArrayvertices, *color, depth);
	}

	void ScriptGUICanvas::InternalDrawImage(ScriptGUICanvas* self, MonoObject* image, __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop* area, Color* color, TextureScaleMode scaleMode, uint8_t depth)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<SpriteImage> tmpimage;
		ScriptRRefBase* scriptObjectWrapperimage;
		scriptObjectWrapperimage = ScriptRRefBase::GetScriptObjectWrapper(image);
		if(scriptObjectWrapperimage != nullptr)
			tmpimage = B3DStaticResourceCast<SpriteImage>(scriptObjectWrapperimage->GetNativeObject());
		TArea2<TUnitValue<int32_t, LogicalPixel>, TUnitValue<int32_t, LogicalPixel>> tmparea;
		tmparea = ScriptTArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__::FromInterop(*area);
		static_cast<GUICanvas*>(self->GetNativeObject())->DrawImage(tmpimage, tmparea, *color, scaleMode, depth);
	}

	void ScriptGUICanvas::InternalDrawTriangleStrip(ScriptGUICanvas* self, MonoArray* vertices, Color* color, uint8_t depth)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<TVector2<TUnitValue<int32_t, LogicalPixel>>> nativeArrayvertices;
		if(vertices != nullptr)
		{
			ScriptArray scriptArrayvertices(vertices);
			nativeArrayvertices.resize(scriptArrayvertices.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvertices.Size(); elementIndex++)
			{
				nativeArrayvertices[elementIndex] = ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::FromInterop(scriptArrayvertices.Get<__TVector2_TUnitValue_int32_t__LogicalPixel__Interop>(elementIndex));
			}

		}
		static_cast<GUICanvas*>(self->GetNativeObject())->DrawTriangleStrip(nativeArrayvertices, *color, depth);
	}

	void ScriptGUICanvas::InternalDrawTriangleList(ScriptGUICanvas* self, MonoArray* vertices, Color* color, uint8_t depth)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<TVector2<TUnitValue<int32_t, LogicalPixel>>> nativeArrayvertices;
		if(vertices != nullptr)
		{
			ScriptArray scriptArrayvertices(vertices);
			nativeArrayvertices.resize(scriptArrayvertices.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvertices.Size(); elementIndex++)
			{
				nativeArrayvertices[elementIndex] = ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::FromInterop(scriptArrayvertices.Get<__TVector2_TUnitValue_int32_t__LogicalPixel__Interop>(elementIndex));
			}

		}
		static_cast<GUICanvas*>(self->GetNativeObject())->DrawTriangleList(nativeArrayvertices, *color, depth);
	}

	void ScriptGUICanvas::InternalDrawText(ScriptGUICanvas* self, MonoString* text, __TVector2_TUnitValue_int32_t__LogicalPixel__Interop* position, MonoObject* font, float size, Color* color, uint8_t depth)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmptext;
		tmptext = MonoUtil::MonoToString(text);
		TVector2<TUnitValue<int32_t, LogicalPixel>> tmpposition;
		tmpposition = ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::FromInterop(*position);
		TResourceHandle<Font> tmpfont;
		ScriptRRefBase* scriptObjectWrapperfont;
		scriptObjectWrapperfont = ScriptRRefBase::GetScriptObjectWrapper(font);
		if(scriptObjectWrapperfont != nullptr)
			tmpfont = B3DStaticResourceCast<Font>(scriptObjectWrapperfont->GetNativeObject());
		static_cast<GUICanvas*>(self->GetNativeObject())->DrawText(tmptext, tmpposition, tmpfont, size, *color, depth);
	}

	void ScriptGUICanvas::InternalClear(ScriptGUICanvas* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUICanvas*>(self->GetNativeObject())->Clear();
	}

	void ScriptGUICanvas::InternalCreate(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options)
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
		GUICanvas* nativeObject = GUICanvas::Create(tmpstyleClass, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUICanvas>(nativeObject, scriptObject);
	}

	void ScriptGUICanvas::InternalCreate0(MonoObject* scriptObject, MonoArray* options)
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
		GUICanvas* nativeObject = GUICanvas::Create(nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUICanvas>(nativeObject, scriptObject);
	}
}
