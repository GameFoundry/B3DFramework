//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRenderTarget.h"

namespace BansheeEngine
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Descriptor class used for initializing a MultiRenderTexture. Contains descriptors for each individual color render 
	 * surface and their common depth/stencil surface.
	 */
	struct BS_CORE_EXPORT MULTI_RENDER_TEXTURE_DESC
	{
		Vector<RENDER_SURFACE_DESC> colorSurfaces;
		RENDER_SURFACE_DESC depthStencilSurface;
	};

	/**
	 * @see		MULTI_RENDER_TEXTURE_DESC
	 *
	 * @note	References core textures instead of texture handles.
	 */
	struct BS_CORE_EXPORT MULTI_RENDER_TEXTURE_CORE_DESC
	{
		Vector<RENDER_SURFACE_CORE_DESC> colorSurfaces;
		RENDER_SURFACE_CORE_DESC depthStencilSurface;
	};

	/** Contains various properties that describe a render texture. */
	class BS_CORE_EXPORT MultiRenderTextureProperties : public RenderTargetProperties
	{
	public:
		MultiRenderTextureProperties(const MULTI_RENDER_TEXTURE_DESC& desc);
		MultiRenderTextureProperties(const MULTI_RENDER_TEXTURE_CORE_DESC& desc);
		virtual ~MultiRenderTextureProperties() { }

	protected:
		friend class MultiRenderTextureCore;
		friend class MultiRenderTexture;

		void construct(const TextureProperties* props);
	};

	/**
	 * Object representing multiple render textures. You may bind this to the pipeline in order to render to all or some of 
	 * the textures at once.
	 *
	 * @note	Sim thread only.
	 */
	class BS_CORE_EXPORT MultiRenderTexture : public RenderTarget
	{
	public:
		virtual ~MultiRenderTexture() { }

		/**
		 * Returns a color surface texture you may bind as an input to an GPU program.
		 *
		 * @note	Be aware that you cannot bind a render texture for reading and writing at the same time.
		 */
		const HTexture& getBindableColorTexture(UINT32 idx) const { return mBindableColorTex[idx]; }

		/**
		 * Returns a depth/stencil surface texture you may bind as an input to an GPU program.
		 *
		 * @note	Be aware that you cannot bind a render texture for reading and writing at the same time.
		 */
		const HTexture& getBindableDepthStencilTexture() const { return mBindableDepthStencilTex; }

		/** Retrieves a core implementation of a render texture usable only from the core thread. */
		SPtr<MultiRenderTextureCore> getCore() const;

		/** @copydoc TextureManager::createMultiRenderTexture */
		static SPtr<MultiRenderTexture> create(const MULTI_RENDER_TEXTURE_DESC& desc);

		/**	Returns properties that describe the render texture. */
		const MultiRenderTextureProperties& getProperties() const;

		/**	Returns the number of color surfaces used by the render texture. */
		UINT32 getColorSurfaceCount() const;

	protected:
		MultiRenderTexture(const MULTI_RENDER_TEXTURE_DESC& desc);

		/** @copydoc RenderTarget::createCore */
		SPtr<CoreObjectCore> createCore() const override;

		/** @copydoc CoreObjectCore::syncToCore */
		virtual CoreSyncData syncToCore(FrameAlloc* allocator) override;

		MULTI_RENDER_TEXTURE_DESC mDesc;
		Vector<HTexture> mBindableColorTex;
		HTexture mBindableDepthStencilTex;
	};

	/** @} */

	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Object representing multiple render textures. You may bind this to the pipeline in order to render to all or some 
	 * of the textures at once.
	 *
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT MultiRenderTextureCore : public RenderTargetCore
	{
	public:
		virtual ~MultiRenderTextureCore();

		/** @copydoc CoreObjectCore::initialize */
		virtual void initialize() override;

		/** Returns properties that describe the render texture. */
		const MultiRenderTextureProperties& getProperties() const;

		/**
		* Returns a depth/stencil surface texture you may bind as an input to an GPU program.
		*
		* @note	Be aware that you cannot bind a render texture for reading and writing at the same time.
		*/
		const SPtr<TextureView> getBindableDepthStencilTexture() const { return mDepthStencilSurface; }

		/**
		* Returns a color surface texture you may bind as an input to an GPU program.
		*
		* @note	Be aware that you cannot bind a render texture for reading and writing at the same time.
		*/
		const SPtr<TextureView>& getBindableColorTexture(UINT32 idx) const { return mColorSurfaces[idx]; }

		/** @copydoc	TextureManager::createMultiRenderTexture(const MULTI_RENDER_TEXTURE_DESC&) */
		static SPtr<MultiRenderTextureCore> create(const MULTI_RENDER_TEXTURE_CORE_DESC& desc);

	protected:
		MultiRenderTextureCore(const MULTI_RENDER_TEXTURE_CORE_DESC& desc);

		/** @copydoc CoreObjectCore::syncToCore */
		virtual void syncToCore(const CoreSyncData& data) override;

	private:
		/** Checks that all render surfaces and depth/stencil surface match. If they do not match an exception is thrown. */
		void throwIfBuffersDontMatch() const;

		// TODO - Not implemented
		virtual void copyToMemory(PixelData &dst, FrameBuffer buffer = FB_AUTO);

	protected:
		Vector<SPtr<TextureView>> mColorSurfaces;
		SPtr<TextureView> mDepthStencilSurface;

		MULTI_RENDER_TEXTURE_CORE_DESC mDesc;
	};

	/** @} */
}