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

namespace bs
{
	ScriptGUICanvas::ScriptGUICanvas(MonoObject* instance, GUICanvas* canvas)
		:TScriptGUIElement(instance, canvas)
	{

	}

	void ScriptGUICanvas::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUICanvas::internal_createInstance);
		metaData.scriptClass->AddInternalCall("Internal_DrawLine", (void*)&ScriptGUICanvas::internal_drawLine);
		metaData.scriptClass->AddInternalCall("Internal_DrawPolyLine", (void*)&ScriptGUICanvas::internal_drawPolyLine);
		metaData.scriptClass->AddInternalCall("Internal_DrawTexture", (void*)&ScriptGUICanvas::internal_drawTexture);
		metaData.scriptClass->AddInternalCall("Internal_DrawTriangleStrip", (void*)&ScriptGUICanvas::internal_drawTriangleStrip);
		metaData.scriptClass->AddInternalCall("Internal_DrawTriangleList", (void*)&ScriptGUICanvas::internal_drawTriangleList);
		metaData.scriptClass->AddInternalCall("Internal_DrawText", (void*)&ScriptGUICanvas::internal_drawText);
		metaData.scriptClass->AddInternalCall("Internal_Clear", (void*)&ScriptGUICanvas::internal_clear);
	}

	void ScriptGUICanvas::internal_createInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray ScriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.AddOption(scriptArray.get<GUIOption>(i));

		GUICanvas* guiCanvas = GUICanvas::create(options, MonoUtil::monoToString(style));

		new (bs_alloc<ScriptGUICanvas>()) ScriptGUICanvas(instance, guiCanvas);
	}

	void ScriptGUICanvas::internal_drawLine(ScriptGUICanvas* nativeInstance, Vector2I* a, Vector2I* b, Color* color,
		UINT8 depth)
	{
		GUICanvas* canvas = (GUICanvas*)nativeInstance->GetGUIElement();
		canvas->DrawLine(*a, *b, *color, depth);
	}

	void ScriptGUICanvas::internal_drawPolyLine(ScriptGUICanvas* nativeInstance, MonoArray* vertices, Color* color,
		UINT8 depth)
	{
		GUICanvas* canvas = (GUICanvas*)nativeInstance->GetGUIElement();

		ScriptArray VerticesArray(vertices);
		UINT32 size = verticesArray.size();

		Vector<Vector2I> NativeVertices(size);
		memcpy(nativeVertices.Data(), verticesArray.getRaw<Vector2I>(), sizeof(Vector2I) * size);

		canvas->DrawPolyLine(nativeVertices, *color, depth);
	}

	void ScriptGUICanvas::internal_drawTexture(ScriptGUICanvas* nativeInstance, ScriptSpriteTexture* texture, Rect2I* area,
		TextureScaleMode scaleMode, Color* color, UINT8 depth)
	{
		GUICanvas* canvas = (GUICanvas*)nativeInstance->GetGUIElement();

		HSpriteTexture nativeTexture;
		if (texture != nullptr)
			nativeTexture = texture->GetHandle();

		canvas->DrawTexture(nativeTexture, *area, scaleMode, *color, depth);
	}
	
	void ScriptGUICanvas::internal_drawTriangleStrip(ScriptGUICanvas* nativeInstance, MonoArray* vertices, Color* color,
		UINT8 depth)
	{
		GUICanvas* canvas = (GUICanvas*)nativeInstance->GetGUIElement();

		ScriptArray VerticesArray(vertices);
		UINT32 size = verticesArray.size();

		Vector<Vector2I> NativeVertices(size);
		memcpy(nativeVertices.Data(), verticesArray.getRaw<Vector2I>(), sizeof(Vector2I) * size);

		canvas->DrawTriangleStrip(nativeVertices, *color, depth);
	}

	void ScriptGUICanvas::internal_drawTriangleList(ScriptGUICanvas* nativeInstance, MonoArray* vertices, Color* color,
		UINT8 depth)
	{
		GUICanvas* canvas = (GUICanvas*)nativeInstance->GetGUIElement();

		ScriptArray VerticesArray(vertices);
		UINT32 size = verticesArray.size();

		Vector<Vector2I> NativeVertices(size);
		memcpy(nativeVertices.Data(), verticesArray.getRaw<Vector2I>(), sizeof(Vector2I) * size);

		canvas->DrawTriangleList(nativeVertices, *color, depth);
	}

	void ScriptGUICanvas::internal_drawText(ScriptGUICanvas* nativeInstance, MonoString* text, Vector2I* position,
		ScriptFont* font, UINT32 size, Color* color, UINT8 depth)
	{
		GUICanvas* canvas = (GUICanvas*)nativeInstance->GetGUIElement();
		String nativeText = MonoUtil::monoToString(text);

		HFont nativeFont;
		if (font != nullptr)
			nativeFont = font->GetHandle();

		canvas->DrawText(nativeText, *position, nativeFont, size, *color, depth);
	}

	void ScriptGUICanvas::internal_clear(ScriptGUICanvas* nativeInstance)
	{
		GUICanvas* canvas = (GUICanvas*)nativeInstance->GetGUIElement();
		canvas->Clear();
	}
}
