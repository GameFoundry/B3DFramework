//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptFont.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsScriptResourceManager.h"
#include "BsScriptFontBitmap.h"

namespace bs
{
	ScriptFont::ScriptFont(MonoObject* instance, const HFont& font)
		:TScriptResource(instance, font)
	{

	}

	void ScriptFont::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetBitmap", &ScriptFont::internal_GetBitmap);
		metaData.scriptClass->addInternalCall("Internal_GetClosestSize", &ScriptFont::internal_GetClosestSize);
	}

	MonoObject* ScriptFont::internal_GetBitmap(ScriptFont* instance, int size)
	{
		HFont font = instance->getHandle();

		SPtr<const FontBitmap> bitmap = font->getBitmap(size);
		if (bitmap != nullptr)
			return ScriptFontBitmap::create(bitmap);

		return nullptr;
	}

	int ScriptFont::internal_GetClosestSize(ScriptFont* instance, int size)
	{
		HFont font = instance->getHandle();

		return font->getClosestSize(size);
	}

	MonoObject* ScriptFont::createInstance()
	{
		return metaData.scriptClass->createInstance();
	}

	ScriptCharRange::ScriptCharRange(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptCharRange::initRuntimeData()
	{ }
}