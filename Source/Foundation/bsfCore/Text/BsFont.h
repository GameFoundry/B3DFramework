//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Image/BsTextureAtlasLayout.h"
#include "Resources/BsResource.h"

namespace bs
{
	/** @addtogroup Text
	 *  @{
	 */

	/**	Kerning pair representing larger or smaller offset between a specific pair of characters. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Text)) KerningPair
	{
		u32 OtherCharId;
		float Amount;
	};

	/**	Describes a single character in a font of a specific size. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Text)) CharacterInformation
	{
		u32 CharId; /**< Character ID, corresponding to a Unicode key. */
		u32 Page; /**< Index of the texture the character is located on. */
		float UvX, UvY; /**< Texture coordinates of the character in the page texture. */
		float UvWidth, UvHeight; /**< Width/height of the character in texture coordinates. */
		float Width, Height; /**< Width/height of the character in pixels. */
		float XOffset, YOffset; /**< Offset for the visible portion of the character in pixels. */
		float XAdvance, YAdvance; /**< Determines how much to advance the pen after writing this character, in pixels. */

		/**
		 * Pairs that determine if certain character pairs should be closer or father together. for example "AV"
		 * combination.
		 */
		Vector<KerningPair> KerningPairs;
	};

	/** Information about a single page containing font bitmaps. */
	struct B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Text)) FontBitmapPage : public IReflectable
	{
		HTexture Texture;
		bool IsDynamic = false;

		B3D_SCRIPT_EXPORT(Exclude(true))
		TreeTextureAtlasLayout Layout; /**< Layout that can be used for finding free space in the page texture. Only relevant for dynamic texture maps. */

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class FontBitmapPageRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**	Contains information about font characters rendered into one or multiple bitmaps, for specific font size. */
	struct B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Text)) FontBitmapInformation : public IReflectable
	{
		/**	Returns a character description for the character with the specified Unicode key. */
		B3D_SCRIPT_EXPORT()
		const CharacterInformation& GetCharacterInformation(u32 characterId) const;

		/** Font size for which the bitmaps are rendered. */
		B3D_SCRIPT_EXPORT()
		float Size;

		/** Y offset to the baseline on which the characters are placed, in pixels. */
		B3D_SCRIPT_EXPORT()
		float BaselineOffset;

		/** Height of a single line of the font, in pixels. */
		B3D_SCRIPT_EXPORT()
		float LineHeight;

		/** Character to use when data for a character is missing. */
		B3D_SCRIPT_EXPORT()
		CharacterInformation MissingGlyph;

		/** Width of a space in pixels. */
		B3D_SCRIPT_EXPORT()
		float SpaceWidth;

		/** Textures in which the character's pixels are stored. */
		B3D_SCRIPT_EXPORT()
		Vector<FontBitmapPage> TexturePages;

		/** All characters in the font referenced by character ID. */
		Map<u32, CharacterInformation> Characters;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class FontBitmapInformationRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**	Determines how is a font rendered into the bitmap texture. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Text)) FontRenderMode
	{
		Smooth, /*< Render antialiased fonts without hinting (slightly more blurry). */
		Raster, /*< Render non-antialiased fonts without hinting (slightly more blurry). */
		HintedSmooth, /*< Render antialiased fonts with hinting. */
		HintedRaster /*< Render non-antialiased fonts with hinting. */
	};

	/** Information about a Font. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Text)) FontInformation
	{
		/** Optional name of the font. Used primarily for easier debugging. */
		String Name;

		/**	Determines dots per inch scale that will be used when rendering the characters. */
		B3D_SCRIPT_EXPORT()
		u32 DPI = 96;

		/**	Determines the render mode used for rendering the characters into a bitmap. */
		B3D_SCRIPT_EXPORT()
		FontRenderMode RenderMode = FontRenderMode::HintedSmooth;

		/** Data stream containing the .ttf/.otf font data. */
		B3D_SCRIPT_EXPORT(Exclude(true))
		SPtr<MemoryDataStream> FontData;
	};

	/** Descriptor structure used for initialization of a Font. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Text)) FontCreateInformation : FontInformation 
	{
		FontCreateInformation() = default;
		FontCreateInformation(const FontInformation& other)
			:FontInformation(other)
		{ }
	};

	/**
	 * Font resource containing data about textual characters and how to render text. Contains one or multiple font
	 * bitmaps, each for a specific size.
	 */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Text)) Font : public Resource
	{
	public:
		virtual ~Font();

		/**
		 * Returns font bitmap information for a specific font size.
		 *
		 * @param	size	Size of the font in points.
		 * @return			Bitmap object if it exists, false otherwise.
		 */
		B3D_SCRIPT_EXPORT()
		SPtr<FontBitmapInformation> GetBitmap(float size) const;

		/**
		 * Finds the available font bitmap size closest to the provided size.
		 *
		 * @param[in]	size	Size of the bitmap in points.
		 * @return				Nearest available bitmap size.
		 */
		B3D_SCRIPT_EXPORT()
		float GetClosestSize(float size) const;

		/**
		 * Renders glyphs for particular characters as a particular size (in points). The rendered glyphs will be added to the first
		 * free texture page, or new texture page(s) will be allocated. Returns true if successful.
		 */
		bool RenderGlyphs(float size, const TArrayView<u32>& characterIds);

		/**
		 * Bakes all the currently rasterized glyphs. This ensures that texture pages containing those glyphs
		 * will be serialized when the font is saved.
		 *
		 * @param	clearFontData		Clears the internal font data. This will prevent further glyphs to be rendered
		 *								in the font.
		 */
		void Bake(bool clearFontData);

		/** Creates a new font. */
		static HFont Create(const FontCreateInformation& createInformation);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		void Initialize() override;
		void Destroy() override;

		/** Creates a new font as a pointer instead of a resource handle. */
		static SPtr<Font> CreateShared(const FontCreateInformation& createInformation);

		/** Creates a Font without initializing it. */
		static SPtr<Font> CreateEmpty();

		/** @} */

	protected:
		friend class FontManager;

		Font(const FontCreateInformation& createInformation);

		/**
		 * Creates the font renderer for the currently assigned font data. This must be called before RenderGlyph() is called.
		 * Returns false if the renderer cannot be initialized (usually means the font data is missing, or the renderer is already initialized).
		 */ 
		bool InitializeFontRenderer();

		/** Destroys the font renderer created in InitializeFontRenderer(). */
		void DestroyFontRenderer();

		/** Attempts to retrieve existing bitmap information for particular font size, or creates new bitmap information if one doesn't exist. */
		SPtr<FontBitmapInformation> GetOrCreateBitmapInformationForSize(float size);

		void GetCoreDependencies(Vector<CoreObject*>& dependencies) override;

		/** Retrieves font size that is quantized in a way we can use it to perform bitmap lookup. */
		static float GetQuantizedFontSize(float size);

		static constexpr u32 kFontPageSize = 1024;
		static constexpr u32 kFontQuantizeAmount = 100; // Font sizes with 2 decimal places or lower are treated as unique size bitmaps
	private:
		FontInformation mInformation;
		Map<float, SPtr<FontBitmapInformation>> mFontBitmaps;

		struct Implementation;
		Implementation* mImplementation = nullptr;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class FontRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */
} // namespace bs
