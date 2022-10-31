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
ScriptGUICanvas::ScriptGUICanvas(MonoObject* instance, GUICanvas* canvas)
	: TScriptGUIElement(instance, canvas)
{
}

void ScriptGUICanvas::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUICanvas::InternalCreateInstance);
	metaData.ScriptClass->AddInternalCall("Internal_DrawLine", (void*)&ScriptGUICanvas::InternalDrawLine);
	metaData.ScriptClass->AddInternalCall("Internal_DrawPolyLine", (void*)&ScriptGUICanvas::InternalDrawPolyLine);
	metaData.ScriptClass->AddInternalCall("Internal_DrawTexture", (void*)&ScriptGUICanvas::InternalDrawTexture);
	metaData.ScriptClass->AddInternalCall("Internal_DrawTriangleStrip", (void*)&ScriptGUICanvas::InternalDrawTriangleStrip);
	metaData.ScriptClass->AddInternalCall("Internal_DrawTriangleList", (void*)&ScriptGUICanvas::InternalDrawTriangleList);
	metaData.ScriptClass->AddInternalCall("Internal_DrawText", (void*)&ScriptGUICanvas::InternalDrawText);
	metaData.ScriptClass->AddInternalCall("Internal_Clear", (void*)&ScriptGUICanvas::InternalClear);
}

void ScriptGUICanvas::InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUICanvas* guiCanvas = GUICanvas::Create(options, MonoUtil::MonoToString(style));

	new(B3DAllocate<ScriptGUICanvas>()) ScriptGUICanvas(instance, guiCanvas);
}

void ScriptGUICanvas::InternalDrawLine(ScriptGUICanvas* nativeInstance, Vector2I* a, Vector2I* b, Color* color, u8 depth)
{
	GUICanvas* canvas = (GUICanvas*)nativeInstance->GetGuiElement();
	canvas->DrawLine(*a, *b, *color, depth);
}

void ScriptGUICanvas::InternalDrawPolyLine(ScriptGUICanvas* nativeInstance, MonoArray* vertices, Color* color, u8 depth)
{
	GUICanvas* canvas = (GUICanvas*)nativeInstance->GetGuiElement();

	ScriptArray verticesArray(vertices);
	u32 size = verticesArray.Size();

	Vector<Vector2I> nativeVertices(size);
	memcpy(nativeVertices.data(), verticesArray.GetRaw<Vector2I>(), sizeof(Vector2I) * size);

	canvas->DrawPolyLine(nativeVertices, *color, depth);
}

void ScriptGUICanvas::InternalDrawTexture(ScriptGUICanvas* nativeInstance, ScriptSpriteTexture* texture, Rect2I* area, TextureScaleMode scaleMode, Color* color, u8 depth)
{
	GUICanvas* canvas = (GUICanvas*)nativeInstance->GetGuiElement();

	HSpriteTexture nativeTexture;
	if(texture != nullptr)
		nativeTexture = texture->GetHandle();

	canvas->DrawTexture(nativeTexture, *area, scaleMode, *color, depth);
}

void ScriptGUICanvas::InternalDrawTriangleStrip(ScriptGUICanvas* nativeInstance, MonoArray* vertices, Color* color, u8 depth)
{
	GUICanvas* canvas = (GUICanvas*)nativeInstance->GetGuiElement();

	ScriptArray verticesArray(vertices);
	u32 size = verticesArray.Size();

	Vector<Vector2I> nativeVertices(size);
	memcpy(nativeVertices.data(), verticesArray.GetRaw<Vector2I>(), sizeof(Vector2I) * size);

	canvas->DrawTriangleStrip(nativeVertices, *color, depth);
}

void ScriptGUICanvas::InternalDrawTriangleList(ScriptGUICanvas* nativeInstance, MonoArray* vertices, Color* color, u8 depth)
{
	GUICanvas* canvas = (GUICanvas*)nativeInstance->GetGuiElement();

	ScriptArray verticesArray(vertices);
	u32 size = verticesArray.Size();

	Vector<Vector2I> nativeVertices(size);
	memcpy(nativeVertices.data(), verticesArray.GetRaw<Vector2I>(), sizeof(Vector2I) * size);

	canvas->DrawTriangleList(nativeVertices, *color, depth);
}

void ScriptGUICanvas::InternalDrawText(ScriptGUICanvas* nativeInstance, MonoString* text, Vector2I* position, ScriptFont* font, u32 size, Color* color, u8 depth)
{
	GUICanvas* canvas = (GUICanvas*)nativeInstance->GetGuiElement();
	String nativeText = MonoUtil::MonoToString(text);

	HFont nativeFont;
	if(font != nullptr)
		nativeFont = font->GetHandle();

	canvas->DrawText(nativeText, *position, nativeFont, size, *color, depth);
}

void ScriptGUICanvas::InternalClear(ScriptGUICanvas* nativeInstance)
{
	GUICanvas* canvas = (GUICanvas*)nativeInstance->GetGuiElement();
	canvas->Clear();
}
