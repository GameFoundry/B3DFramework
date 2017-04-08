//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptFontBitmap.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsScriptResourceManager.h"
#include "BsScriptTexture.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptFontBitmap::ScriptFontBitmap(MonoObject* instance, SPtr<const FontBitmap> bitmap)
		:ScriptObject(instance), mBitmap(bitmap)
	{

	}

	void ScriptFontBitmap::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetSize", &ScriptFontBitmap::internal_GetSize);
		metaData.scriptClass->addInternalCall("Internal_GetBaselineOffset", &ScriptFontBitmap::internal_GetBaselineOffset);
		metaData.scriptClass->addInternalCall("Internal_GetLineHeight", &ScriptFontBitmap::internal_GetLineHeight);
		metaData.scriptClass->addInternalCall("Internal_GetSpaceWidth", &ScriptFontBitmap::internal_GetSpaceWidth);
		metaData.scriptClass->addInternalCall("Internal_GetMissingChar", &ScriptFontBitmap::internal_GetMissingChar);
		metaData.scriptClass->addInternalCall("Internal_GetPages", &ScriptFontBitmap::internal_GetPages);
		metaData.scriptClass->addInternalCall("Internal_GetChar", &ScriptFontBitmap::internal_GetChar);
		metaData.scriptClass->addInternalCall("Internal_GetKerning", &ScriptFontBitmap::internal_GetKerning);
	}

	ScriptCharDesc ScriptFontBitmap::convertCharDesc(const CHAR_DESC& desc)
	{
		ScriptCharDesc output;

		output.charId = desc.charId;
		output.page = desc.page;
		output.uvX = desc.uvX;
		output.uvY = desc.uvY;
		output.uvWidth = desc.uvWidth;
		output.uvHeight = desc.uvHeight;
		output.width = desc.width;
		output.height = desc.height;
		output.xOffset = desc.xOffset;
		output.yOffset = desc.yOffset;
		output.xAdvance = desc.xAdvance;
		output.yAdvance = desc.yAdvance;

		return output;
	}

	MonoObject* ScriptFontBitmap::create(SPtr<const FontBitmap> bitmap)
	{
		MonoObject* managedInstance = metaData.scriptClass->createInstance();
		new (bs_alloc<ScriptFontBitmap>()) ScriptFontBitmap(managedInstance, bitmap);

		return managedInstance;
	}

	UINT32 ScriptFontBitmap::internal_GetSize(ScriptFontBitmap* instance)
	{
		return instance->mBitmap->size;
	}

	INT32 ScriptFontBitmap::internal_GetBaselineOffset(ScriptFontBitmap* instance)
	{
		return instance->mBitmap->fontDesc.baselineOffset;
	}

	UINT32 ScriptFontBitmap::internal_GetLineHeight(ScriptFontBitmap* instance)
	{
		return instance->mBitmap->fontDesc.lineHeight;
	}

	UINT32 ScriptFontBitmap::internal_GetSpaceWidth(ScriptFontBitmap* instance)
	{
		return instance->mBitmap->fontDesc.spaceWidth;
	}

	void ScriptFontBitmap::internal_GetMissingChar(ScriptFontBitmap* instance, ScriptCharDesc* output)
	{
		*output = convertCharDesc(instance->mBitmap->fontDesc.missingGlyph);
	}

	MonoArray* ScriptFontBitmap::internal_GetPages(ScriptFontBitmap* instance)
	{
		const Vector<HTexture>& texturePages = instance->mBitmap->texturePages;

		UINT32 numPages = (UINT32)texturePages.size();
		ScriptArray output = ScriptArray::create<ScriptTexture>(numPages);
		for (UINT32 i = 0; i < numPages; i++)
		{
			ScriptTexture* scriptTexture = nullptr;
			ScriptResourceManager::instance().getScriptResource(texturePages[i], &scriptTexture, true);

			output.set(i, scriptTexture->getManagedInstance());
		}

		return output.getInternal();
	}

	void ScriptFontBitmap::internal_GetChar(ScriptFontBitmap* instance, UINT32 id, ScriptCharDesc* output)
	{
		*output = convertCharDesc(instance->mBitmap->getCharDesc(id));
	}

	MonoArray* ScriptFontBitmap::internal_GetKerning(ScriptFontBitmap* instance, UINT32 id)
	{
		const CHAR_DESC& charDesc = instance->mBitmap->getCharDesc(id);

		UINT32 numPairs = (UINT32)charDesc.kerningPairs.size();
		ScriptArray outArray = ScriptArray::create<ScriptKerningPair>(numPairs);
		for (UINT32 i = 0; i < numPairs; i++)
			outArray.set(i, charDesc.kerningPairs[i]);

		return outArray.getInternal();
	}

	ScriptKerningPair::ScriptKerningPair(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptKerningPair::initRuntimeData()
	{ }
}