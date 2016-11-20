//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsResource.h"
#include "BsFontDesc.h"

namespace bs
{
	/** @addtogroup Text
	 *  @{
	 */

	/**	Contains textures and data about every character for a bitmap font of a specific size. */
	struct BS_CORE_EXPORT FontBitmap : public IReflectable
	{
		/**	Returns a character description for the character with the specified Unicode key. */
		const CHAR_DESC& getCharDesc(UINT32 charId) const;

		UINT32 size; /**< Font size for which the data is contained. */
		FONT_DESC fontDesc; /**< Font description containing per-character and general font data. */
		Vector<HTexture> texturePages; /**< Textures in which the character's pixels are stored. */

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class FontBitmapRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	// TODO - When saved on disk font currently stores a copy of the texture pages. This should be acceptable
	// if you import a new TrueType or OpenType font since the texture will be generated on the spot
	// but if you use a bitmap texture to initialize the font manually, then you will potentially have duplicate textures.
	// Also, changing the source texture will not automatically update the font because there is no direct link between them.
	// -- This is probably not a large problem, but it is something to keep an eye out.

	/**	Font resource containing data about textual characters and how to render text. */
	class BS_CORE_EXPORT Font : public Resource
	{
	public:
		virtual ~Font();

		/**
		 * Returns font bitmap for a specific size if it exists, null otherwise.
		 *
		 * @param[in]	size	Size of the bitmap in points.
		 */
		SPtr<const FontBitmap> getBitmap(UINT32 size) const;

		/**	Finds the available font bitmap size closest to the provided size. */
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
		virtual RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}