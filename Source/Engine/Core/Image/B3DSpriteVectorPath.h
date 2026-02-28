//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DSpriteImage.h"
#include "VectorGraphics/B3DVectorGraphics.h"
#include "VectorGraphics/B3DVectorSpriteAtlas.h"

namespace b3d
{
	class GUIVectorSpriteAtlasAllocation;
	/** @addtogroup Image
	 *  @{
	 */

	/** Descriptor structure used for initialization of a SpriteVectorPath. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Rendering)) SpriteVectorPathCreateInformation : SpriteImageInformation
	{
		SpriteVectorPathCreateInformation() = default;
		SpriteVectorPathCreateInformation(const SpriteImageInformation& spriteImageInformation, const HVectorPath& vectorPath, const Size2I& defaultSize)
			: SpriteImageInformation(spriteImageInformation), VectorPath(vectorPath), DefaultSize(defaultSize)
		{ }

		HVectorPath VectorPath; /**< Vector path to render on the sprite. */
		Size2I DefaultSize; /**< Size of the unscaled rasterized path, in pixels. Actual rendered size might be different depending on DPI scale or other scale factors. */
		VectorGraphicsRasterizationScaling ScalingMode = VectorGraphicsRasterizationScaling::StretchToFit; /**< How to scale the path canvas onto the rasterized destination. */
	};

	/** Provides information about a particular sprite vector path image allocated within a texture atlas. */
	class SpriteVectorPathAllocation : public SpriteImageAllocation
	{
	public:
		struct SyncPacket;

		/** Creates a new sprite vector path allocation. */
		static SPtr<SpriteVectorPathAllocation> Create(const WeakSPtr<SpriteImageType>& owner, const GUIVectorSpriteAtlasAllocation& vectorSpriteAtlasAllocation);

	protected:
		friend class render::SpriteVectorPathAllocation;

		SpriteVectorPathAllocation(const WeakSPtr<SpriteImageType>& owner, const GUIVectorSpriteAtlasAllocation& vectorSpriteAtlasAllocation)
			:SpriteImageAllocation(owner, vectorSpriteAtlasAllocation.AtlasTexture, vectorSpriteAtlasAllocation.UVRange), mVectorSpriteAtlasAllocationHandle(vectorSpriteAtlasAllocation.AllocationHandle)
		{ }

		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

	private:
		/** Allocation handle in the vector path atlas. */
		SPtr<GUIVectorSpriteAtlasAllocationHandle> mVectorSpriteAtlasAllocationHandle;
	};

	/** @} */

	namespace render
	{
		/** @addtogroup RenderThread
		 *  @{
		 */

		/**
		 * Render proxy counterpart of a b3d::SpriteVectorPathCreateInformation.
		 *
		 * @note	Render thread.
		 */
		struct SpriteVectorPathCreateInformation : SpriteImageInformation
		{
			SpriteVectorPathCreateInformation() = default;
			SpriteVectorPathCreateInformation(const SpriteImageInformation& spriteImageInformation)
				: SpriteImageInformation(spriteImageInformation)
			{ }
		};

		/** @} */
	} // namespace render

	/** @addtogroup Image
	 *  @{
	 */

	/** Implementation of SpriteImage that renders a vector path. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) SpriteVectorPath : public CoreVariantType<SpriteImage, false>
	{
	public:
		SPtr<SpriteImageAllocation> FindOrAllocateImageToFitArea(const Size2I& size) override;
		SPtr<SpriteImageAllocation> FindOrAllocateScaledImage(float scale) override;

		/**	Creates a new sprite vector path. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(SpriteVectorPath))
		static HSpriteVectorPath Create(const HVectorPath& vectorPath, const Size2I& defaultSize);

		/**	Creates a new sprite vector path. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(SpriteVectorPath))
		static HSpriteVectorPath Create(const SpriteVectorPathCreateInformation& createInformation);

		/** Creates a new SpriteVectorPath without a resource handle. Use Create() for normal use. */
		static SPtr<SpriteVectorPath> CreateShared(const HVectorPath& vectorPath, const Size2I& defaultSize);

		/** Creates a new SpriteVectorPath without a resource handle. Use Create() for normal use. */
		static SPtr<SpriteVectorPath> CreateShared(const SpriteVectorPathCreateInformation& createInformation);

	private:
		friend class SpriteVectorPathRTTI;
		friend class render::SpriteVectorPath;
		struct SyncPacket;

		SpriteVectorPath(const SpriteVectorPathCreateInformation& createInformation);

		/** Allocates a sprite image using the provided size. */
		SPtr<SpriteVectorPathAllocation> AllocateImage(const Size2I& size);

		void Initialize() override;
		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

		HVectorPath mVectorPath;
		Size2I mDefaultSize{kZeroTag};
		VectorGraphicsRasterizationScaling mScalingMode = VectorGraphicsRasterizationScaling::StretchToFit;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/**	Creates a new empty and uninitialized sprite vector path. */
		static SPtr<SpriteVectorPath> CreateEmpty();

	public:
		friend class SpriteVectorPathRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	namespace render
	{
		/** @addtogroup RenderThread
		 *  @{
		 */

		/** @copydoc b3d::SpriteVectorPathAllocation. */
		class SpriteVectorPathAllocation : public SpriteImageAllocation
		{
		public:
			SpriteVectorPathAllocation() = default;

		private:
			friend class b3d::SpriteVectorPathAllocation;

			SpriteVectorPathAllocation(const WeakSPtr<SpriteImageType>& owner, const TextureType& atlasTexture, const Area2& uvRange, const SPtr<GUIVectorSpriteAtlasAllocationHandle>& vectorSpriteAtlasAllocationHandle);
			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

			/** Allocation handle in the vector path atlas. */
			SPtr<GUIVectorSpriteAtlasAllocationHandle> mVectorSpriteAtlasAllocationHandle;
		};

		/**
		 * Render proxy counterpart of a b3d::SpriteVectorPath.
		 *
		 * @note	Render thread.
		 */
		class B3D_EXPORT SpriteVectorPath : public CoreVariantType<SpriteImage, true>
		{
		private:
			friend class b3d::SpriteVectorPath;

			SpriteVectorPath(const SpriteVectorPathCreateInformation& createInformation, const SPtr<SpriteImageAllocation>& defaultAllocatedImage);

			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;
		};

		/** @} */
	} // namespace render
} // namespace b3d
