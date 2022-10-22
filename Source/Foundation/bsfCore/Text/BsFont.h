//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Resources/BsResource.h"
#include "Text/BsFontDesc.h"

namespace bs
{
	/** @addtogroup Text
	 *  @{
	 */

	/**	Contains textures and data about every character for a bitmap font of a specific size. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(GUI_Engine)) FontBitmap : public IReflectable
	{
		/**	Returns a character description for the character with the specified Unicode key. */
		BS_SCRIPT_EXPORT()
		const CharDesc& GetCharDesc(u32 charId) const;

		/** Font size for which the data is contained. */
		BS_SCRIPT_EXPORT()
		u32 Size;
		
		/** Y offset to the baseline on which the characters are placed, in pixels. */
		BS_SCRIPT_EXPORT()
		i32 BaselineOffset;

		/** Height of a single line of the font, in pixels. */
		BS_SCRIPT_EXPORT()
		u32 LineHeight;

		/** Character to use when data for a character is missing. */
		BS_SCRIPT_EXPORT()
		CharDesc MissingGlyph;

		/** Width of a space in pixels. */
		BS_SCRIPT_EXPORT()
		u32 SpaceWidth;

		/** Textures in which the character's pixels are stored. */
		BS_SCRIPT_EXPORT()
		Vector<HTexture> TexturePages;

		/** All characters in the font referenced by character ID. */
		Map<u32, CharDesc> Characters;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class FontBitmapRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**
	 * Font resource containing data about textual characters and how to render text. Contains one or multiple font
	 * bitmaps, each for a specific size.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(GUI_Engine)) Font : public Resource
	{
	public:
		virtual ~Font() = default;

		/**
		 * Returns font bitmap for a specific font size.
		 *
		 * @param[in]	size	Size of the bitmap in points.
		 * @return				Bitmap object if it exists, false otherwise.
		 */
		BS_SCRIPT_EXPORT()
		SPtr<FontBitmap> GetBitmap(u32 size) const;

		/**	
		 * Finds the available font bitmap size closest to the provided size.
		 *
		 * @param[in]	size	Size of the bitmap in points.
		 * @return				Nearest available bitmap size.
		 */
		BS_SCRIPT_EXPORT()
		i32 GetClosestSize(u32 size) const;

		/**	Creates a new font from the provided per-size font data. */
		static HFont Create(const Vector<SPtr<FontBitmap>>& fontInitData);

	public: // ***** INTERNAL ******
		using Resource::Initialize;

		/** @name Internal
		 *  @{
		 */

		/**
		 * Initializes the font with specified per-size font data.
		 *
		 * @note	Internal method. Factory methods will call this automatically for you.
		 */
		void Initialize(const Vector<SPtr<FontBitmap>>& fontData);

		/** Creates a new font as a pointer instead of a resource handle. */
		static SPtr<Font> CreatePtrInternal(const Vector<SPtr<FontBitmap>>& fontInitData);

		/** Creates a Font without initializing it. */
		static SPtr<Font> CreateEmptyInternal();

		/** @} */

	protected:
		friend class FontManager;

		Font();

		/** @copydoc CoreObject::getCoreDependencies */
		void GetCoreDependencies(Vector<CoreObject*>& dependencies) ;

	private:
		Map<u32, SPtr<FontBitmap>> mFontDataPerSize;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class FontRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */
}
