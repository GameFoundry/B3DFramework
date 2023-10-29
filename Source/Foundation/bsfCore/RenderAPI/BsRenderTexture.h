//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Image/BsTexture.h"
#include "RenderAPI/BsRenderTarget.h"

namespace bs
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**	Structure that describes a render texture color and depth/stencil surfaces. */
	struct B3D_CORE_EXPORT RenderTextureInformation
	{
		RenderSurfaceInformation ColorSurfaces[B3D_MAXIMUM_RENDER_TARGET_COUNT];
		RenderSurfaceInformation DepthStencilSurface;
	};

	/** Descriptor structure used for initialization of a RenderTexture. */
	struct B3D_CORE_EXPORT RenderTextureCreateInformation : RenderTextureInformation 
	{
		RenderTextureCreateInformation() = default;
		RenderTextureCreateInformation(const RenderTextureInformation& other)
			: RenderTextureInformation(other)
		{ }
	};

	namespace ct
	{
		struct RenderTextureCreateInformation;
	}

	/**	Contains various properties that describe a render texture. */
	class B3D_CORE_EXPORT RenderTextureProperties : public RenderTargetProperties
	{
	public:
		RenderTextureProperties() = default;
		RenderTextureProperties(const RenderTextureCreateInformation& createInformation, bool requiresFlipping);
		RenderTextureProperties(const ct::RenderTextureCreateInformation& createInformation, bool requiresFlipping);

		virtual ~RenderTextureProperties() {}

	private:
		void Construct(const TextureProperties* textureProps, u32 numSlices, u32 mipLevel, bool requiresFlipping, bool hwGamma);

		friend class ct::RenderTexture;
		friend class RenderTexture;
	};

	/**
	 * Render target specialization that allows you to render into one or multiple textures. Such textures can then be used
	 * in other operations as GPU program input.
	 */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) RenderTexture : public RenderTarget
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

		/**
		 * Retrieves the render proxy.
		 *
		 * @note	Render thread only.
		 */
		SPtr<ct::RenderTexture> GetCore() const;

		/**	Returns properties that describe the render texture. */
		const RenderTextureProperties& GetProperties() const;

	protected:
		friend class TextureManager;
		friend class ct::RenderTexture;
		struct SyncPacket;

		RenderTexture(const RenderTextureCreateInformation& createInformation);

		SPtr<ct::RenderProxy> CreateRenderProxy() const override;
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
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */

	namespace ct
	{
		/** @addtogroup RenderAPI-Internal
		 *  @{
		 */

		/** @copydoc bs::RenderTextureInformation */
		struct B3D_CORE_EXPORT RenderTextureInformation
		{
			RenderSurfaceInformation ColorSurfaces[B3D_MAXIMUM_RENDER_TARGET_COUNT];
			RenderSurfaceInformation DepthStencilSurface;
		};

		/** @copydoc bs::RenderTextureCreateInformation */
		struct B3D_CORE_EXPORT RenderTextureCreateInformation : RenderTextureInformation 
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
		class B3D_CORE_EXPORT RenderTexture : public RenderTarget
		{
		public:
			RenderTexture(const RenderTextureCreateInformation& createInformation);
			virtual ~RenderTexture() = default;

			void Initialize() override;

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

			/**	Returns properties that describe the render texture. */
			const RenderTextureProperties& GetProperties() const;

		protected:
			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

		private:
			/**	Throws an exception of the color and depth/stencil buffers aren't compatible. */
			void ThrowIfBuffersDontMatch() const;

		protected:
			friend class bs::RenderTexture;

			SPtr<TextureView> mColorSurfaces[B3D_MAXIMUM_RENDER_TARGET_COUNT];
			SPtr<TextureView> mDepthStencilSurface;

			RenderTextureInformation mInformation;
		};

		/** @} */
	} // namespace ct
} // namespace bs
