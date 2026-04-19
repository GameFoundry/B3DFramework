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

	/** Templated base class used for both main and render thread SpriteTextureCreateInformation counterparts. */
	template <bool IsRenderProxy>
	struct TSpriteTextureCreateInformation : SpriteImageInformation
	{
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;

		TSpriteTextureCreateInformation() = default;
		TSpriteTextureCreateInformation(const SpriteImageInformation& spriteImageInformation, const TextureType& atlasTexture)
			: SpriteImageInformation(spriteImageInformation), AtlasTexture(atlasTexture)
		{ }

		/** Texture used as the atlas. */
		TextureType AtlasTexture;
		Area2 UVRange = Area2(0.0f, 0.0f, 1.0f, 1.0f); /**< Range in the atlas texture that the image is to be read from, in [0, 1] range. */
	};

	/** Descriptor structure used for initialization of a SpriteTexture. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Rendering)) SpriteTextureCreateInformation : TSpriteTextureCreateInformation<false> 
	{
		using TSpriteTextureCreateInformation::TSpriteTextureCreateInformation;
	};

	/** @} */

	namespace render
	{
		/** @addtogroup Renderer
		 *  @{
		 */

		/**
		 * Render proxy counterpart of a b3d::SpriteTextureCreateInformation.
		 *
		 * @note	Render thread.
		 */
		struct SpriteTextureCreateInformation : TSpriteTextureCreateInformation<true>
		{
			using TSpriteTextureCreateInformation::TSpriteTextureCreateInformation;
		};

		/** @} */
	} // namespace render

	/** @addtogroup 2D
	 *  @{
	 */

	/** Implementation of SpriteImage that uses a user-provided texture as the atlas texture. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) SpriteTexture : public CoreVariantType<SpriteImage, false>
	{
	public:
		SPtr<SpriteImageAllocation> FindOrAllocateImageToFitArea(const Size2I& size) override { return mDefaultAllocatedImage; }
		SPtr<SpriteImageAllocation> FindOrAllocateScaledImage(float scale) override { return mDefaultAllocatedImage; }

		/** Retrieves the atlas texture where the image is stored. */
		B3D_SCRIPT_EXPORT(ExportName(Texture), Property(Getter))
		const TextureType& GetAtlasTexture() const { return mAtlasTexture; }

		/** Determines the UV range that the image is referencing. */
		B3D_SCRIPT_EXPORT(ExportName(UVRange), Property(Getter))
		const Area2& GetUVRange() const { return mUVRange; }

		/**	Creates a new sprite texture that references the entire area of the provided texture. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(SpriteTexture))
		static HSpriteTexture Create(const HTexture& texture);

		/**	Creates a new sprite texture that references a sub-area of the provided	texture. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(SpriteTexture))
		static HSpriteTexture Create(const SpriteTextureCreateInformation& createInformation);

		/** Creates a new SpriteTexture without a resource handle. Use Create() for normal use. */
		static SPtr<SpriteTexture> CreateShared(const HTexture& texture);

		/** Creates a new SpriteTexture without a resource handle. Use Create() for normal use. */
		static SPtr<SpriteTexture> CreateShared(const SpriteTextureCreateInformation& createInformation);

	private:
		friend class SpriteTextureRTTI;
		friend class render::SpriteTexture;
		struct SyncPacket;

		SpriteTexture(const SpriteTextureCreateInformation& createInformation);

		void Initialize() override;

		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;
		void GetCoreDependencies(Vector<CoreObject*>& dependencies) override;

		TextureType mAtlasTexture;
		Area2 mUVRange = Area2(0.0f, 0.0f, 1.0f, 1.0f); /**< Range in the atlas texture that the image is to be read from, in [0, 1] range. */

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/**	Creates a new empty and uninitialized sprite texture. */
		static SPtr<SpriteTexture> CreateEmpty();

	public:
		friend class SpriteTextureRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	namespace render
	{
		/** @addtogroup Renderer
		 *  @{
		 */

		/**
		 * Render proxy counterpart of a b3d::SpriteTexture.
		 *
		 * @note	Render thread.
		 */
		class B3D_EXPORT SpriteTexture : public CoreVariantType<SpriteImage, true>
		{
		public:
			/**	Sets the atlas texture to utilize. */
			void SetAtlasTexture(const SPtr<Texture>& texture) { mAtlasTexture = texture; }

		private:
			friend class b3d::SpriteTexture;

			SpriteTexture(const SpriteTextureCreateInformation& createInformation, const SPtr<SpriteImageAllocation>& defaultAllocatedImage);

			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

			TextureType mAtlasTexture;
			Area2 mUVRange = Area2(0.0f, 0.0f, 1.0f, 1.0f); /**< Range in the atlas texture that the image is to be read from, in [0, 1] range. */
		};

		/** @} */
	} // namespace render
} // namespace b3d
