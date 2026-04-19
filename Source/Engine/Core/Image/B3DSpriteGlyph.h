//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DSpriteImage.h"

namespace b3d
{
	/** @addtogroup 2D
	 *  @{
	 */

	/** Descriptor structure used for initialization of a SpriteGlyph. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Rendering)) SpriteGlyphCreateInformation : SpriteImageInformation
	{
		SpriteGlyphCreateInformation() = default;
		SpriteGlyphCreateInformation(const SpriteImageInformation& spriteImageInformation, const HFont& font, u32 glyph, float defaultSize)
			: SpriteImageInformation(spriteImageInformation), Font(font), Glyph(glyph), DefaultSize(defaultSize)
		{ }

		HFont Font; /**< Font from which to render the glyph from. */
		u32 Glyph = 0; /**< Unicode code for the glyph to render. */
		float DefaultSize = 8.0f; /**< Size of the unscaled glyph in points. Actual rendered size might be different depending on DPI scale or other scale factors. */
	};

	/** Provides information about a particular glyph image allocated within a texture atlas. */
	template<bool IsRenderProxy>
	class TSpriteGlyphAllocation : public CoreVariantType<SpriteImageAllocation, IsRenderProxy>
	{
	public:
		using SpriteImageType = CoreVariantType<SpriteImage, IsRenderProxy>;
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;

		/** Size of the allocated glyph in points. */
		float GetSizeInPoints() const { return mSizeInPoints; }

	protected:
		TSpriteGlyphAllocation() = default;
		TSpriteGlyphAllocation(const WeakSPtr<SpriteImageType>& owner, const TextureType& texture, const Area2& uvRange, float sizeInPoints)
			:CoreVariantType<SpriteImageAllocation, IsRenderProxy>(owner, texture, uvRange), mSizeInPoints(sizeInPoints)
		{ }

	private:
		float mSizeInPoints;
	};

	/** @copydoc TSpriteGlyphAllocation. */
	class SpriteGlyphAllocation : public TSpriteGlyphAllocation<false>
	{
	public:
		struct SyncPacket;

		/** Creates a new sprite glyph allocation. */
		static SPtr<SpriteGlyphAllocation> Create(const WeakSPtr<SpriteImageType>& owner, const TextureType& texture, const Area2& uvRange, float sizeInPoints);

	protected:
		friend class render::SpriteGlyphAllocation;

		using TSpriteGlyphAllocation::TSpriteGlyphAllocation;

		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;
	};

	/** @} */

	namespace render
	{
		/** @addtogroup Renderer
		 *  @{
		 */

		/**
		 * Render proxy counterpart of a b3d::SpriteGlyphCreateInformation.
		 *
		 * @note	Render thread.
		 */
		struct SpriteGlyphCreateInformation : SpriteImageInformation
		{
			SpriteGlyphCreateInformation() = default;
			SpriteGlyphCreateInformation(const SpriteImageInformation& spriteImageInformation)
				: SpriteImageInformation(spriteImageInformation)
			{ }
		};

		/** @} */
	} // namespace render

	/** @addtogroup 2D
	 *  @{
	 */

	/** Implementation of SpriteImage that renders a single glyph from a Font. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) SpriteGlyph : public CoreVariantType<SpriteImage, false>
	{
	public:
		SPtr<SpriteImageAllocation> FindOrAllocateImageToFitArea(const Size2I& size) override;
		SPtr<SpriteImageAllocation> FindOrAllocateScaledImage(float scale) override;

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
		friend class render::SpriteGlyph;
		struct SyncPacket;

		SpriteGlyph(const SpriteGlyphCreateInformation& createInformation);

		/** Allocates a sprite image using the provided size in points. */
		SPtr<SpriteGlyphAllocation> AllocateImage(float sizeInPoints);

		void Initialize() override;
		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

		HFont mFont;
		u32 mGlyph = 0;
		float mDefaultGlyphSize = 8.0f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/**	Creates a new empty and uninitialized sprite glyph. */
		static SPtr<SpriteGlyph> CreateEmpty();

	public:
		friend class SpriteGlyphRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	namespace render
	{
		/** @addtogroup Renderer
		 *  @{
		 */

		/** @copydoc TSpriteGlyphAllocation. */
		class SpriteGlyphAllocation : public TSpriteGlyphAllocation<true>
		{
		protected:
			friend class b3d::SpriteGlyphAllocation;

			SpriteGlyphAllocation(const WeakSPtr<SpriteImageType>& owner, const TextureType& atlasTexture, const Area2& uvRange, float sizeInPoints);
			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;
		};

		/**
		 * Render proxy counterpart of a b3d::SpriteGlyph.
		 *
		 * @note	Render thread.
		 */
		class B3D_EXPORT SpriteGlyph : public CoreVariantType<SpriteImage, true>
		{
		public:
		private:
			friend class b3d::SpriteGlyph;

			SpriteGlyph(const SpriteGlyphCreateInformation& createInformation, const SPtr<SpriteImageAllocation>& defaultAllocatedImage);

			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;
		};

		/** @} */
	} // namespace render
} // namespace b3d
