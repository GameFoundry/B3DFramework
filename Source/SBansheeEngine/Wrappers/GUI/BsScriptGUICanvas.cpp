//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/GUI/BsScriptGUICanvas.h"
#include "BsScriptMeta.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUICanvas.h"
#include "GUI/BsGUIOptions.h"

#include "BsScriptFont.generated.h"
#include "BsScriptSpriteTexture.generated.h"

namespace bs
{
	ScriptGUICanvas::ScriptGUICanvas(MonoObject* instance, GUICanvas* canvas)
		:TScriptGUIElement(instance, canvas)
	{

	}

	void ScriptGUICanvas::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUICanvas::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_DrawLine", (void*)&ScriptGUICanvas::internal_drawLine);
		metaData.scriptClass->addInternalCall("Internal_DrawPolyLine", (void*)&ScriptGUICanvas::internal_drawPolyLine);
		metaData.scriptClass->addInternalCall("Internal_DrawTexture", (void*)&ScriptGUICanvas::internal_drawTexture);
		metaData.scriptClass->addInternalCall("Internal_DrawTriangleStrip", (void*)&ScriptGUICanvas::internal_drawTriangleStrip);
		metaData.scriptClass->addInternalCall("Internal_DrawTriangleList", (void*)&ScriptGUICanvas::internal_drawTriangleList);
		metaData.scriptClass->addInternalCall("Internal_DrawText", (void*)&ScriptGUICanvas::internal_drawText);
		metaData.scriptClass->addInternalCall("Internal_Clear", (void*)&ScriptGUICanvas::internal_clear);
	}

	void ScriptGUICanvas::internal_createInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		GUICanvas* guiCanvas = GUICanvas::create(options, toString(MonoUtil::monoToWString(style)));

		new (bs_alloc<ScriptGUICanvas>()) ScriptGUICanvas(instance, guiCanvas);
	}

	void ScriptGUICanvas::internal_drawLine(ScriptGUICanvas* nativeInstance, Vector2I* a, Vector2I* b, Color* color, 
		UINT8 depth)
	{
		GUICanvas* canvas = (GUICanvas*)nativeInstance->getGUIElement();
		canvas->drawLine(*a, *b, *color, depth);
	}

	void ScriptGUICanvas::internal_drawPolyLine(ScriptGUICanvas* nativeInstance, MonoArray* vertices, Color* color, 
		UINT8 depth)
	{
		GUICanvas* canvas = (GUICanvas*)nativeInstance->getGUIElement();

		ScriptArray verticesArray(vertices);
		UINT32 size = verticesArray.size();

		Vector<Vector2I> nativeVertices(size);
		memcpy(nativeVertices.data(), verticesArray.getRaw<Vector2I>(), sizeof(Vector2I) * size);

		canvas->drawPolyLine(nativeVertices, *color, depth);
	}

	void ScriptGUICanvas::internal_drawTexture(ScriptGUICanvas* nativeInstance, ScriptSpriteTexture* texture, Rect2I* area,
		TextureScaleMode scaleMode, Color* color, UINT8 depth)
	{
		GUICanvas* canvas = (GUICanvas*)nativeInstance->getGUIElement();

		HSpriteTexture nativeTexture;
		if (texture != nullptr)
			nativeTexture = texture->getHandle();

		canvas->drawTexture(nativeTexture, *area, scaleMode, *color, depth);
	}
	
	void ScriptGUICanvas::internal_drawTriangleStrip(ScriptGUICanvas* nativeInstance, MonoArray* vertices, Color* color, 
		UINT8 depth)
	{
		GUICanvas* canvas = (GUICanvas*)nativeInstance->getGUIElement();

		ScriptArray verticesArray(vertices);
		UINT32 size = verticesArray.size();

		Vector<Vector2I> nativeVertices(size);
		memcpy(nativeVertices.data(), verticesArray.getRaw<Vector2I>(), sizeof(Vector2I) * size);

		canvas->drawTriangleStrip(nativeVertices, *color, depth);
	}

	void ScriptGUICanvas::internal_drawTriangleList(ScriptGUICanvas* nativeInstance, MonoArray* vertices, Color* color, 
		UINT8 depth)
	{
		GUICanvas* canvas = (GUICanvas*)nativeInstance->getGUIElement();

		ScriptArray verticesArray(vertices);
		UINT32 size = verticesArray.size();

		Vector<Vector2I> nativeVertices(size);
		memcpy(nativeVertices.data(), verticesArray.getRaw<Vector2I>(), sizeof(Vector2I) * size);

		canvas->drawTriangleList(nativeVertices, *color, depth);
	}

	void ScriptGUICanvas::internal_drawText(ScriptGUICanvas* nativeInstance, MonoString* text, Vector2I* position,
		ScriptFont* font, UINT32 size, Color* color, UINT8 depth)
	{
		GUICanvas* canvas = (GUICanvas*)nativeInstance->getGUIElement();
		WString nativeText = MonoUtil::monoToWString(text);

		HFont nativeFont;
		if (font != nullptr)
			nativeFont = font->getHandle();

		canvas->drawText(nativeText, *position, nativeFont, size, *color, depth);
	}

	void ScriptGUICanvas::internal_clear(ScriptGUICanvas* nativeInstance)
	{
		GUICanvas* canvas = (GUICanvas*)nativeInstance->getGUIElement();
		canvas->clear();
	}
}