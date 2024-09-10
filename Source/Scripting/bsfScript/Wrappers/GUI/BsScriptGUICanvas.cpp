//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUICanvas.h"
#include "BsScriptMeta.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUICanvas.h"
#include "GUI/BsGUIOptions.h"

#include "Generated/BsScriptFont.generated.h"
#include "Generated/BsScriptSpriteTexture.generated.h"

using namespace bs;
ScriptGUICanvas::ScriptGUICanvas(GUICanvas* nativeObject)
	: TScriptGUIElementWrapper(nativeObject)
{
}

void ScriptGUICanvas::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUICanvas::InternalCreateInstance);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_DrawLine", (void*)&ScriptGUICanvas::InternalDrawLine);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_DrawPolyLine", (void*)&ScriptGUICanvas::InternalDrawPolyLine);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_DrawImage", (void*)&ScriptGUICanvas::InternalDrawImage);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_DrawTriangleStrip", (void*)&ScriptGUICanvas::InternalDrawTriangleStrip);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_DrawTriangleList", (void*)&ScriptGUICanvas::InternalDrawTriangleList);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_DrawText", (void*)&ScriptGUICanvas::InternalDrawText);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_Clear", (void*)&ScriptGUICanvas::InternalClear);
}

MonoObject* ScriptGUICanvas::CreateScriptObject(bool construct)
{
	// TODO - Add a ctor in C# we can call if needed
	return nullptr;
}

void ScriptGUICanvas::InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUICanvas* guiCanvas = GUICanvas::Create(options, MonoUtil::MonoToString(style));

	ScriptObjectWrapper::Create<ScriptGUICanvas>(guiCanvas, instance);
}

void ScriptGUICanvas::InternalDrawLine(ScriptGUICanvas* self, Vector2I* a, Vector2I* b, Color* color, u8 depth)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->DrawLine(*a, *b, *color, depth);
}

void ScriptGUICanvas::InternalDrawPolyLine(ScriptGUICanvas* self, MonoArray* vertices, Color* color, u8 depth)
{
	if(!self->IsNativeObjectValid())
		return;

	ScriptArray verticesArray(vertices);
	u32 size = verticesArray.Size();

	Vector<Vector2I> nativeVertices(size);
	memcpy(nativeVertices.data(), verticesArray.GetRaw<Vector2I>(), sizeof(Vector2I) * size);

	self->GetNativeObject()->DrawPolyLine(nativeVertices, *color, depth);
}

void ScriptGUICanvas::InternalDrawImage(ScriptGUICanvas* self, ScriptSpriteImage* texture, Rect2I* area, TextureScaleMode scaleMode, Color* color, u8 depth)
{
	if(!self->IsNativeObjectValid())
		return;

	HSpriteImage nativeTexture;
	if(texture != nullptr)
		nativeTexture = texture->GetNativeObjectAsHandle();

	self->GetNativeObject()->DrawImage(nativeTexture, *area, scaleMode, *color, depth);
}

void ScriptGUICanvas::InternalDrawTriangleStrip(ScriptGUICanvas* self, MonoArray* vertices, Color* color, u8 depth)
{
	if(!self->IsNativeObjectValid())
		return;

	ScriptArray verticesArray(vertices);
	u32 size = verticesArray.Size();

	Vector<Vector2I> nativeVertices(size);
	memcpy(nativeVertices.data(), verticesArray.GetRaw<Vector2I>(), sizeof(Vector2I) * size);

	self->GetNativeObject()->DrawTriangleStrip(nativeVertices, *color, depth);
}

void ScriptGUICanvas::InternalDrawTriangleList(ScriptGUICanvas* self, MonoArray* vertices, Color* color, u8 depth)
{
	if(!self->IsNativeObjectValid())
		return;

	ScriptArray verticesArray(vertices);
	u32 size = verticesArray.Size();

	Vector<Vector2I> nativeVertices(size);
	memcpy(nativeVertices.data(), verticesArray.GetRaw<Vector2I>(), sizeof(Vector2I) * size);

	self->GetNativeObject()->DrawTriangleList(nativeVertices, *color, depth);
}

void ScriptGUICanvas::InternalDrawText(ScriptGUICanvas* self, MonoString* text, Vector2I* position, ScriptFont* font, float size, Color* color, u8 depth)
{
	if(!self->IsNativeObjectValid())
		return;

	String nativeText = MonoUtil::MonoToString(text);

	HFont nativeFont;
	if(font != nullptr)
		nativeFont = font->GetNativeObjectAsHandle();

	self->GetNativeObject()->DrawText(nativeText, *position, nativeFont, size, *color, depth);
}

void ScriptGUICanvas::InternalClear(ScriptGUICanvas* self)
{
	if(!self->IsNativeObjectValid())
		return;

	self->GetNativeObject()->Clear();
}
