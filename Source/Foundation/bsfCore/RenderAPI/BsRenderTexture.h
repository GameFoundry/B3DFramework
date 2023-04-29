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
	struct B3D_CORE_EXPORT RENDER_TEXTURE_DESC
	{
		RENDER_SURFACE_DESC ColorSurfaces[B3D_MAXIMUM_RENDER_TARGET_COUNT];
		RENDER_SURFACE_DESC DepthStencilSurface;
	};

	namespace ct
	{
		struct RENDER_TEXTURE_DESC;
	}

	/**	Contains various properties that describe a render texture. */
	class B3D_CORE_EXPORT RenderTextureProperties : public RenderTargetProperties
	{
	public:
		RenderTextureProperties(const RENDER_TEXTURE_DESC& desc, bool requiresFlipping);
		RenderTextureProperties(const ct::RENDER_TEXTURE_DESC& desc, bool requiresFlipping);

		virtual ~RenderTextureProperties() {}

	private:
		void Construct(const TextureProperties* textureProps, u32 numSlices, u32 mipLevel, bool requiresFlipping, bool hwGamma);

		friend class ct::RenderTexture;
		friend class RenderTexture;
	};

	/**
	 * Render target specialization that allows you to render into one or multiple textures. Such textures can then be used
	 * in other operations as GPU program input.
	 *
	 * @note	Sim thread only. Retrieve core implementation from getCore() for core thread only functionality.
	 */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) RenderTexture : public RenderTarget
	{
	public:
		virtual ~RenderTexture() = default;

		/** @copydoc TextureManager::CreateRenderTexture(const TEXTURE_DESC&, bool, PixelFormat) */
		static SPtr<RenderTexture> Create(const TextureCreateInformation& colorDesc, bool createDepth = true, PixelFormat depthStencilFormat = PF_D32);

		/** @copydoc TextureManager::CreateRenderTexture(const RENDER_TEXTURE_DESC&) */
		static SPtr<RenderTexture> Create(const RENDER_TEXTURE_DESC& desc);

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
		 * Retrieves a core implementation of a render texture usable only from the core thread.
		 *
		 * @note	Core thread only.
		 */
		SPtr<ct::RenderTexture> GetCore() const;

		/**	Returns properties that describe the render texture. */
		const RenderTextureProperties& GetProperties() const;

	protected:
		friend class TextureManager;

		RenderTexture(const RENDER_TEXTURE_DESC& desc);

		SPtr<ct::CoreObject> CreateCore() const override;
		CoreSyncData SyncToCore(FrameAlloc* allocator) override;

	protected:
		HTexture mBindableColorTex[B3D_MAXIMUM_RENDER_TARGET_COUNT];
		HTexture mBindableDepthStencilTex;

		RENDER_TEXTURE_DESC mDesc;

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

		/**
		 * @see		bs::RENDER_TEXTURE_DESC
		 *
		 * @note	References core textures instead of texture handles.
		 */
		struct B3D_CORE_EXPORT RENDER_TEXTURE_DESC
		{
			RENDER_SURFACE_DESC ColorSurfaces[B3D_MAXIMUM_RENDER_TARGET_COUNT];
			RENDER_SURFACE_DESC DepthStencilSurface;
		};

		/**
		 * Provides access to internal render texture implementation usable only from the core thread.
		 *
		 * @note	Core thread only.
		 */
		class B3D_CORE_EXPORT RenderTexture : public RenderTarget
		{
		public:
			RenderTexture(const RENDER_TEXTURE_DESC& desc);
			virtual ~RenderTexture() = default;

			void Initialize() override;

			/** @copydoc TextureManager::CreateRenderTexture(const RENDER_TEXTURE_DESC&, u32) */
			static SPtr<RenderTexture> Create(const RENDER_TEXTURE_DESC& desc);

			/**
			 * Returns a color surface texture you may bind as an input to an GPU program.
			 *
			 * @note	Be aware that you cannot bind a render texture for reading and writing at the same time.
			 */
			SPtr<Texture> GetColorTexture(u32 idx) const { return mDesc.ColorSurfaces[idx].Texture; }

			/**
			 * Returns a depth/stencil surface texture you may bind as an input to an GPU program.
			 *
			 * @note	Be aware that you cannot bind a render texture for reading and writing at the same time.
			 */
			SPtr<Texture> GetDepthStencilTexture() const { return mDesc.DepthStencilSurface.Texture; }

			/**	Returns properties that describe the render texture. */
			const RenderTextureProperties& GetProperties() const;

		protected:
			void SyncToCore(const CoreSyncData& data) override;

		private:
			/**	Throws an exception of the color and depth/stencil buffers aren't compatible. */
			void ThrowIfBuffersDontMatch() const;

		protected:
			friend class bs::RenderTexture;

			SPtr<TextureView> mColorSurfaces[B3D_MAXIMUM_RENDER_TARGET_COUNT];
			SPtr<TextureView> mDepthStencilSurface;

			RENDER_TEXTURE_DESC mDesc;
		};

		/** @} */
	} // namespace ct
} // namespace bs
