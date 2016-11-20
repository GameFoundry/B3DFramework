//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResource.h"
#include "BsScriptObject.h"
#include "BsFont.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Describes a single character in a font of a specific size. */
	struct ScriptCharDesc // Note: Must match C# struct CharDesc
	{
		UINT32 charId; /**< Character ID, corresponding to a Unicode key. */
		UINT32 page; /**< Index of the texture the character is located on. */
		float uvX, uvY; /**< Texture coordinates of the character in the page texture. */
		float uvWidth, uvHeight; /**< Width/height of the character in texture coordinates. */
		UINT32 width, height; /**< Width/height of the character in pixels. */
		INT32 xOffset, yOffset; /**< Offset for the visible portion of the character in pixels. */
		INT32 xAdvance, yAdvance; /**< Determines how much to advance the pen after writing this character, in pixels. */
	};

	/**	Interop class between C++ & CLR for FontBitmap. */
	class BS_SCR_BE_EXPORT ScriptFontBitmap : public ScriptObject<ScriptFontBitmap>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "FontBitmap")

		/**
		 * Creates a new managed instance of a font bitmap.
		 *
		 * @param[in]	bitmap	Native font bitmap to initialize the managed instance with.
		 */
		static MonoObject* create(SPtr<const FontBitmap> bitmap);

	private:
		friend class ScriptResourceManager;

		ScriptFontBitmap(MonoObject* instance, SPtr<const FontBitmap> bitmap);

		/**	Converts the native character description into a script character description. */
		static ScriptCharDesc convertCharDesc(const CHAR_DESC& desc);

		SPtr<const FontBitmap> mBitmap;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static UINT32 internal_GetSize(ScriptFontBitmap* instance);
		static INT32 internal_GetBaselineOffset(ScriptFontBitmap* instance);
		static UINT32 internal_GetLineHeight(ScriptFontBitmap* instance);
		static UINT32 internal_GetSpaceWidth(ScriptFontBitmap* instance);
		static void internal_GetMissingChar(ScriptFontBitmap* instance, ScriptCharDesc* output);
		static MonoArray* internal_GetPages(ScriptFontBitmap* instance);
		static void internal_GetChar(ScriptFontBitmap* instance, UINT32 id, ScriptCharDesc* output);
		static MonoArray* internal_GetKerning(ScriptFontBitmap* instance, UINT32 id);
	};

	/**	Interop class between C++ & CLR for KerningPair. */
	class BS_SCR_BE_EXPORT ScriptKerningPair : public ScriptObject <ScriptKerningPair>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "KerningPair")

	private:
		ScriptKerningPair(MonoObject* instance);
	};

	/** @} */
}