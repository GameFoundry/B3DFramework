//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DTexture.h"
#include "RenderAPI/B3DRenderTarget.h"

namespace b3d
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**	Structure that describes a render texture color and depth/stencil surfaces. */
	struct B3D_EXPORT RenderTextureInformation
	{
		RenderSurfaceInformation ColorSurfaces[B3D_MAXIMUM_RENDER_TARGET_COUNT];
		RenderSurfaceInformation DepthStencilSurface;
	};

	/** Descriptor structure used for initialization of a RenderTexture. */
	struct B3D_EXPORT RenderTextureCreateInformation : RenderTextureInformation
	{
		RenderTextureCreateInformation() = default;
		RenderTextureCreateInformation(const RenderTextureInformation& other)
			: RenderTextureInformation(other)
		{ }
	};

	namespace render
	{
		struct RenderTextureCreateInformation;
	}

	/**
	 * Render target specialization that allows you to render into one or multiple textures. Such textures can then be used
	 * in other operations as GPU program input.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) RenderTexture : public RenderTarget
	{
	public:
		virtual ~RenderTexture() = default;

		/** @copydoc TextureManager::CreateRenderTexture(const TextureCreateInformation&, bool, PixelFormat) */
		static SPtr<RenderTexture> Create(const TextureCreateInformation& textureCreateInformation, bool createDepth = true, PixelFormat depthStencilFormat = PF_D32);

		/** @copydoc TextureManager::CreateRenderTexture(const RenderTextureCreateInformation&) */
		static SPtr<RenderTexture> Create(const RenderTextureCreateInformation& createInformation);

		/**
		 * Returns a color surface texture you may bind as an input to an GPU program.
		 *
		 * @note	Be aware that you cannot bind a render texture for reading and writing at the same time.
		 */
		const HTexture& GetColorTexture(u32 idx) const { return mBindableColorTex[idx]; }

		/**
		 * Returns a depth/stencil surface texture you may bind as an input to an GPU program.
		 *
		 * @note	Be aware that you cannot bind a render texture for reading and writing at the same time.
		 */
		const HTexture& GetDepthStencilTexture() const { return mBindableDepthStencilTex; }

	protected:
		friend class TextureManager;
		friend class render::RenderTexture;
		struct SyncPacket;

		RenderTexture(const RenderTextureCreateInformation& createInformation);

		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

	protected:
		HTexture mBindableColorTex[B3D_MAXIMUM_RENDER_TARGET_COUNT];
		HTexture mBindableDepthStencilTex;

		RenderTextureInformation mInformation;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class RenderTextureRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	namespace render
	{
		/** @addtogroup GpuBackend
		 *  @{
		 */

		/** @copydoc b3d::RenderTextureInformation */
		struct B3D_EXPORT RenderTextureInformation
		{
			RenderSurfaceInformation ColorSurfaces[B3D_MAXIMUM_RENDER_TARGET_COUNT];
			RenderSurfaceInformation DepthStencilSurface;
		};

		/** @copydoc b3d::RenderTextureCreateInformation */
		struct B3D_EXPORT RenderTextureCreateInformation : RenderTextureInformation
		{
			RenderTextureCreateInformation() = default;
			RenderTextureCreateInformation(const RenderTextureInformation& other)
				: RenderTextureInformation(other)
			{ }
		};

		/**
		 * Render proxy variant of render texture.
		 *
		 * @note	Render thread only.
		 */
		class B3D_EXPORT RenderTexture : public RenderTarget
		{
		public:
			RenderTexture(const RenderTextureCreateInformation& createInformation);
			virtual ~RenderTexture() = default;

			void Initialize() override;
			TAsyncOp<SPtr<PixelData>> ReadAsync(GpuCommandBuffer& commandBuffer, u32 colorSurfaceIndex = 0, u32 mipLevel = 0, u32 arrayLayer = 0) override;

			/** @copydoc TextureManager::CreateRenderTexture(const RenderTextureCreateInformation&, u32) */
			static SPtr<RenderTexture> Create(const RenderTextureCreateInformation& createInformation);

			/**
			 * Returns a color surface texture you may bind as an input to an GPU program.
			 *
			 * @note	Be aware that you cannot bind a render texture for reading and writing at the same time.
			 */
			SPtr<Texture> GetColorTexture(u32 idx) const { return mInformation.ColorSurfaces[idx].Texture; }

			/**
			 * Returns a depth/stencil surface texture you may bind as an input to an GPU program.
			 *
			 * @note	Be aware that you cannot bind a render texture for reading and writing at the same time.
			 */
			SPtr<Texture> GetDepthStencilTexture() const { return mInformation.DepthStencilSurface.Texture; }

		protected:
			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

		private:
			/**	Throws an exception of the color and depth/stencil buffers aren't compatible. */
			void ReportIfBuffersDontMatch() const;

		protected:
			friend class b3d::RenderTexture;

			SPtr<TextureView> mColorSurfaces[B3D_MAXIMUM_RENDER_TARGET_COUNT];
			SPtr<TextureView> mDepthStencilSurface;

			RenderTextureInformation mInformation;
		};

		/** @} */
	} // namespace render
} // namespace b3d
