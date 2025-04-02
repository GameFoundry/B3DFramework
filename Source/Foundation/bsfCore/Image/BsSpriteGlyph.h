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

		/** Size of the allocated glyph in pixels. */
		const Size2I& GetSizeInPixels() const { return mSizeInPixels; }

	protected:
		TSpriteGlyphAllocation() = default;
		TSpriteGlyphAllocation(const WeakSPtr<SpriteImageType>& owner, const TextureType& texture, const Area2& uvRange, float sizeInPoints)
			:CoreVariantType<SpriteImageAllocation, IsRenderProxy>(owner, texture, uvRange), mSizeInPoints(sizeInPoints)
		{ }

		TSpriteGlyphAllocation(const WeakSPtr<SpriteImageType>& owner, const TextureType& texture, const Area2& uvRange, const Size2I& sizeInPixels)
			:CoreVariantType<SpriteImageAllocation, IsRenderProxy>(owner, texture, uvRange), mSizeInPixels(sizeInPixels)
		{ }

	private:
		float mSizeInPoints;
		Size2I mSizeInPixels;
	};

	/** @copydoc TSpriteGlyphAllocation. */
	class SpriteGlyphAllocation : public TSpriteGlyphAllocation<false>
	{
	public:
		struct SyncPacket;

		/** Creates a new sprite glyph allocation. */
		static SPtr<SpriteGlyphAllocation> Create(const WeakSPtr<SpriteImageType>& owner, const TextureType& texture, const Area2& uvRange, float sizeInPoints);

		/** Creates a new sprite glyph allocation. */
		static SPtr<SpriteGlyphAllocation> Create(const WeakSPtr<SpriteImageType>& owner, const TextureType& texture, const Area2& uvRange, const Size2I& sizeInPixels);

	protected:
		friend class ct::SpriteGlyphAllocation;

		using TSpriteGlyphAllocation::TSpriteGlyphAllocation;

		SPtr<ct::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;
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
			SpriteGlyphCreateInformation(const SpriteImageInformation& spriteImageInformation)
				: SpriteImageInformation(spriteImageInformation)
			{ }
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
		friend class ct::SpriteGlyph;
		struct SyncPacket;

		SpriteGlyph(const SpriteGlyphCreateInformation& createInformation);

		/** Allocates a sprite image using the provided size in points. */
		SPtr<SpriteGlyphAllocation> AllocateImage(float sizeInPoints);

		/** Allocates a sprite image using the provided size in pixels. */
		SPtr<SpriteGlyphAllocation> AllocateImage(const Size2I& sizeInPixels);

		void Initialize() override;
		SPtr<ct::RenderProxy> CreateRenderProxy() const override;
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

	namespace ct
	{
		/** @addtogroup RenderThread
		 *  @{
		 */

		/** @copydoc TSpriteGlyphAllocation. */
		class SpriteGlyphAllocation : public TSpriteGlyphAllocation<true>
		{
		protected:
			friend class bs::SpriteGlyphAllocation;

			SpriteGlyphAllocation() = default;
			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;
		};

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
