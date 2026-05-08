//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Script/B3DIScriptExportable.h"
#include "Image/B3DTextureAtlasLayout.h"
#include "Resources/B3DResource.h"

namespace b3d
{namespace render
	{
		class GpuCommandBufferPool;
	}

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
		float PointSize; /**< Size in points that the character was generated from. May be 0 if glyph was generated using pixel width/height. */

		/**
		 * Pairs that determine if certain character pairs should be closer or father together. for example "AV"
		 * combination.
		 */
		Vector<KerningPair> KerningPairs;

		B3D_SCRIPT_EXPORT(Exclude(true))
		TOptional<TreeTextureAtlasLayout::Allocation> DynamicLayoutAllocation; /**< Handle to the character allocation in the texture atlas. Can be used for freeing the allocation. */
	};

	/** Available types of FontBitmapPage. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Text)) FontBitmapPageType
	{
		Runtime, /**< Glyphs in this page can be dynamically allocated at runtime and won't be saved. */
		Baked, /**< Glyphs in this page can be dynamically allocated at runtime and will be saved. Next time they are loaded they will use the Loaded type. */
		Loaded /**< Glyphs in this page can be read, but no new glyphs can be added to the page. */
	};

	/** Information about a single page containing font bitmaps. */
	struct B3D_EXPORT B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Text)) FontBitmapPage : IReflectable
	{
		HTexture Texture;
		FontBitmapPageType Type = FontBitmapPageType::Runtime;

		B3D_SCRIPT_EXPORT(Exclude(true))
		TreeTextureAtlasLayout Layout; /**< Layout that can be used for finding free space in the page texture. Only relevant for dynamic texture maps. */

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class FontBitmapPageRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**	Contains information about font characters rendered into one or multiple bitmaps, for specific font size. */
	struct B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Text)) FontBitmapInformation : IReflectable, IScriptExportable
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

		/** All characters in the font referenced by character ID. */
		UnorderedMap<u32, CharacterInformation> Characters;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class FontBitmapInformationRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
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
		TShared<MemoryDataStream> FontData;
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
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Text)) Font : public Resource
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
		TShared<FontBitmapInformation> GetBitmap(float size) const;

		/**
		 * Finds a rendered bitmap closest to the provided size.
		 *
		 * @param[in]	size	Size of the bitmap in points.
		 * @return				Nearest available bitmap size.
		 */
		B3D_SCRIPT_EXPORT()
		float GetClosestExistingBitmapSize(float size) const;

		/** Calculates the required font size (in points) in order to render a glyph that fits within an area of @p size pixels, without stretching or cropping. */
		float GetPointSizeForGlyphThatFitsArea(u32 glyphId, const Size2I& size) const;

		/** Attempts to find information about a character with the specified id and size. Returns null if character was not yet rendered via RenderGlyphs. */
		const CharacterInformation* FindCharacterInformation(u32 characterId, float sizeInPoints) const;

		/** Returns a font page information for a page at the specified index. */
		const FontBitmapPage& GetPage(u32 pageIndex) const;

		/**
		 * Renders glyphs for particular characters in a particular size. The rendered glyphs will be added to the first free texture page,
		 * or new texture page(s) will be allocated. Returns true if successful. 
		 *
		 * @param	sizeInPoints	Size of the glyph in points.
		 * @param	characterIds	UTF32 character identifiers of the characters to render.
		 * @param	bake			If true the rendered glyph will be saved with the font the next time it is serialized, so it doesn't
		 *							need to be re-rendered the next time its loaded. 
		 */
		bool RenderGlyphs(float sizeInPoints, const TArrayView<u32>& characterIds, bool bake = false);

		/**
		 * Clears all the rendered glyph information.
		 * 
		 * @param onlyRuntime	If true, only data for runtime rendered glyphs will be cleared, baked data will remain.
		 */
		void ClearGlyphs(bool onlyRuntime = true);

		/**
		 * Clears all the rendered glyph information for a specific font size.
		 * 
		 * @param size			Font size in points.
		 * @param onlyRuntime	If true, only data for runtime rendered glyphs will be cleared, baked data will remain.
		 */
		void ClearGlyphs(float size, bool onlyRuntime = true);

		/** Creates a new font. */
		static HFont Create(const FontCreateInformation& createInformation);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Clears all the rendered glyph information from the provided bitmap information.
		 * 
		 * @param bitmapInformation		Structure containing all character and other information for a particular font size.
		 * @param onlyRuntime			If true, only data for runtime rendered glyphs will be cleared, baked data will remain.
		 */
		void ClearGlyphs(FontBitmapInformation& bitmapInformation, bool onlyRuntime = true);

		/**
		 * Removes a page at the specified index and updates all the existing character information to point to the next page. All characters referencing this particular
		 * page must have been removed before calling this method. 
		 */
		void RemovePage(u32 pageIndex);

		void Initialize() override;
		void Destroy() override;

		/** Creates a new font as a pointer instead of a resource handle. */
		static TShared<Font> CreateShared(const FontCreateInformation& createInformation);

		/** Creates a Font without initializing it. */
		static TShared<Font> CreateEmpty();

		/** Retrieves font size that is quantized in a way we can use it to perform bitmap lookup. */
		static float GetQuantizedFontSize(float size);

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

		void GetCoreDependencies(Vector<CoreObject*>& dependencies) override;

		static constexpr u32 kFontPageSize = 1024;
		static constexpr u32 kFontQuantizeAmount = 100; // Font sizes with 2 decimal places or lower are treated as unique size bitmaps
	private:
		FontInformation mInformation;

		UnorderedMap<float, TShared<FontBitmapInformation>> mCharactersByPointSize;
		Vector<FontBitmapPage> mFontPages;

		struct Implementation;
		Implementation* mImplementation = nullptr;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class FontRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Contains a rendered bitmap for a single glyph. */
	struct GlyphBitmap
	{
		TShared<render::Texture> GlyphTexture;
		TShared<render::Texture> AtlasTexture;
		Size2UI Size;
		Vector2I PositionInAtlas;
	};

	/** Handles blitting of individual font glyphs into a texture atlas. */
	class FontAtlasRenderer : public Module<FontAtlasRenderer>
	{
	public:
		void OnStartUp() override;
		void OnShutDown() override;

		/**
		 * Queues bliy operations from the provided source textures to their destination atlases by creating an internal
		 * command buffer and queuing it for execution on the GPU.
		 *
		 * @note Thread safe.
		 */
		void BlitGlyphs(Vector<GlyphBitmap> glyphBitmaps);
		
	private:
		TShared<render::GpuCommandBufferPool> mCommandBufferPool;
	};

	/** @} */
} // namespace b3d
