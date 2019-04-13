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
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:GUI_Engine) FontBitmap : public IReflectable
	{
		/**	Returns a character description for the character with the specified Unicode key. */
		BS_SCRIPT_EXPORT()
		const CharDesc& getCharDesc(UINT32 charId) const;

		/** Font size for which the data is contained. */
		BS_SCRIPT_EXPORT()
		UINT32 size;
		
		/** Y offset to the baseline on which the characters are placed, in pixels. */
		BS_SCRIPT_EXPORT()
		INT32 baselineOffset;

		/** Height of a single line of the font, in pixels. */
		BS_SCRIPT_EXPORT()
		UINT32 lineHeight;

		/** Character to use when data for a character is missing. */
		BS_SCRIPT_EXPORT()
		CharDesc missingGlyph;

		/** Width of a space in pixels. */
		BS_SCRIPT_EXPORT()
		UINT32 spaceWidth;

		/** Textures in which the character's pixels are stored. */
		BS_SCRIPT_EXPORT()
		Vector<HTexture> texturePages;

		/** All characters in the font referenced by character ID. */
		Map<UINT32, CharDesc> characters;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class FontBitmapRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Font resource containing data about textual characters and how to render text. Contains one or multiple font 
	 * bitmaps, each for a specific size.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:GUI_Engine) Font : public Resource
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
		SPtr<FontBitmap> getBitmap(UINT32 size) const;

		/**	
		 * Finds the available font bitmap size closest to the provided size. 
		 * 
		 * @param[in]	size	Size of the bitmap in points.
		 * @return				Nearest available bitmap size.
		 */
		BS_SCRIPT_EXPORT()
		INT32 getClosestSize(UINT32 size) const;

		/**	Creates a new font from the provided per-size font data. */
		static HFont create(const Vector<SPtr<FontBitmap>>& fontInitData);

	public: // ***** INTERNAL ******
		using Resource::initialize;

		/** @name Internal
		 *  @{
		 */

		/**
		 * Initializes the font with specified per-size font data.
		 *
		 * @note	Internal method. Factory methods will call this automatically for you.
		 */
		void initialize(const Vector<SPtr<FontBitmap>>& fontData);

		/** Creates a new font as a pointer instead of a resource handle. */
		static SPtr<Font> _createPtr(const Vector<SPtr<FontBitmap>>& fontInitData);

		/** Creates a Font without initializing it. */
		static SPtr<Font> _createEmpty();

		/** @} */

	protected:
		friend class FontManager;

		Font();

		/** @copydoc Resource::getResourceDependencies */
		void getResourceDependencies(FrameVector<HResource>& dependencies) const override;

		/** @copydoc CoreObject::getCoreDependencies */
		void getCoreDependencies(Vector<CoreObject*>& dependencies) override;

	private:
		Map<UINT32, SPtr<FontBitmap>> mFontDataPerSize;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class FontRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}