//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Image/BsSpriteImage.h"

namespace bs
{
	/** @addtogroup Image
	 *  @{
	 */

	/** Descriptor structure used for initialization of a SpriteGlyph. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Rendering)) SpriteGlyphCreateInformation : SpriteImageInformation
	{
		SpriteGlyphCreateInformation() = default;
		SpriteGlyphCreateInformation(const SpriteImageInformation& spriteImageInformation, const HFont& font, u32 glyph, float size)
			: SpriteImageInformation(spriteImageInformation), Font(font), Glyph(glyph), Size(size)
		{ }

		HFont Font; /**< Font from which to render the glyph from. */
		u32 Glyph = 0; /**< Unicode code for the glyph to render. */
		float Size = 8.0f; /**< Size of the glyph in points. */
	};

	/** @} */

	namespace ct
	{
		/** @addtogroup RenderThread
		 *  @{
		 */

		/**
		 * Render proxy counterpart of a bs::SpriteGlyphCreateInformation.
		 *
		 * @note	Render thread.
		 */
		struct SpriteGlyphCreateInformation : SpriteImageInformation
		{
			SpriteGlyphCreateInformation() = default;
			SpriteGlyphCreateInformation(const SpriteImageInformation& spriteImageInformation, const SPtr<Texture>& atlasTexture)
				: SpriteImageInformation(spriteImageInformation), AtlasTexture(atlasTexture)
			{ }

			/** Texture used as the atlas. */
			SPtr<Texture> AtlasTexture;
		};

		/** @} */
	} // namespace ct

	/** @addtogroup Image
	 *  @{
	 */

	/** Implementation of SpriteImage that renders a single glyph from a Font. */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) SpriteGlyph : public CoreVariantType<SpriteImage, false>
	{
	public:
		/**	Sets the font to render the glyph from. */
		B3D_SCRIPT_EXPORT()
		void SetFont(const HFont& font);

		/**	Sets the unicode code of the glyph to render. */
		B3D_SCRIPT_EXPORT()
		void SetGlyph(u32 glyph);

		/**	Sets the size of the glyph in points. */
		B3D_SCRIPT_EXPORT()
		void SetGlyphSize(float size);

		/**	Creates a new sprite glyph. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(SpriteGlyph))
		static HSpriteGlyph Create(const HFont& font, u32 glyph, float size = 8.0f);

		/**	Creates a new sprite glyph. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(SpriteGlyph))
		static HSpriteGlyph Create(const SpriteGlyphCreateInformation& createInformation);

		/** Creates a new SpriteGlyph without a resource handle. Use Create() for normal use. */
		static SPtr<SpriteGlyph> CreateShared(const HFont& font, u32 glyph, float size = 8.0f);

		/** Creates a new SpriteGlyph without a resource handle. Use Create() for normal use. */
		static SPtr<SpriteGlyph> CreateShared(const SpriteGlyphCreateInformation& createInformation);

	private:
		friend class SpriteGlyphRTTI;
		friend class ct::SpriteGlyph;
		struct SyncPacket;

		SpriteGlyph(const SpriteGlyphCreateInformation& createInformation);

		/** Updates information about the atlas the glyph is stored in. Call this after glyph, font or size of the glyph changes. */
		void UpdateGlyphAtlasInformation();

		void Initialize() override;
		SPtr<ct::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;
		void GetCoreDependencies(Vector<CoreObject*>& dependencies) override;

		HFont mFont;
		u32 mGlyph = 0;
		float mGlyphSize = 8.0f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/**	Creates a new empty and uninitialized sprite glyph. */
		static SPtr<SpriteGlyph> CreateEmpty();

	public:
		friend class SpriteGlyphRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */

	namespace ct
	{
		/** @addtogroup RenderThread
		 *  @{
		 */

		/**
		 * Render proxy counterpart of a bs::SpriteGlyph.
		 *
		 * @note	Render thread.
		 */
		class B3D_CORE_EXPORT SpriteGlyph : public CoreVariantType<SpriteImage, true>
		{
		public:
		private:
			friend class bs::SpriteGlyph;

			SpriteGlyph(const SpriteGlyphCreateInformation& createInformation);

			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;
		};

		/** @} */
	} // namespace ct
} // namespace bs
