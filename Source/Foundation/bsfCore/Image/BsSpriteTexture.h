//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Image/BsSpriteImage.h"

namespace bs
{
	/** @addtogroup Image
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
	};

	/** Descriptor structure used for initialization of a SpriteTexture. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Rendering)) SpriteTextureCreateInformation : TSpriteTextureCreateInformation<false> 
	{
		using TSpriteTextureCreateInformation::TSpriteTextureCreateInformation;
	};

	/** @} */

	namespace ct
	{
		/** @addtogroup RenderThread
		 *  @{
		 */

		/**
		 * Render proxy counterpart of a bs::SpriteTextureCreateInformation.
		 *
		 * @note	Render thread.
		 */
		struct SpriteTextureCreateInformation : TSpriteTextureCreateInformation<true>
		{
			using TSpriteTextureCreateInformation::TSpriteTextureCreateInformation;
		};

		/** @} */
	} // namespace ct

	/** @addtogroup Image
	 *  @{
	 */

	/** Implementation of SpriteImage that uses a user-provided texture as the atlas texture. */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) SpriteTexture : public CoreVariantType<SpriteImage, false>
	{
	public:
		/**	Sets the atlas texture to utilize. */
		B3D_SCRIPT_EXPORT()
		void SetAtlasTexture(const TextureType& texture);

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

		/**	Checks if the sprite texture and its internal texture have been loaded. */
		static bool CheckIsLoaded(const HSpriteTexture& texture); // TODO - Can this be removed?

	private:
		friend class SpriteTextureRTTI;
		friend class ct::SpriteTexture;
		struct SyncPacket;

		SpriteTexture(const SpriteTextureCreateInformation& createInformation);

		void Initialize() override;
		SPtr<ct::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;
		void GetCoreDependencies(Vector<CoreObject*>& dependencies) override;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/**	Creates a new empty and uninitialized sprite texture. */
		static SPtr<SpriteTexture> CreateEmpty();

	public:
		friend class SpriteTextureRTTI;
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
		 * Render proxy counterpart of a bs::SpriteTexture.
		 *
		 * @note	Render thread.
		 */
		class B3D_CORE_EXPORT SpriteTexture : public CoreVariantType<SpriteImage, true>
		{
		public:
			/**	Sets the atlas texture to utilize. */
			void SetAtlasTexture(const SPtr<Texture>& texture) { mAtlasTexture = texture; }

		private:
			friend class bs::SpriteTexture;

			SpriteTexture(const SpriteTextureCreateInformation& createInformation);

			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;
		};

		/** @} */
	} // namespace ct
} // namespace bs
