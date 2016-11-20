//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsModule.h"
#include "BsPixelUtil.h"
#include "BsTexture.h"

namespace bs
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	class RenderTexturePool;
	struct POOLED_RENDER_TEXTURE_DESC;

	/**	Contains data about a single render texture in the texture pool. */
	struct PooledRenderTexture
	{
		PooledRenderTexture(RenderTexturePool* pool);
		~PooledRenderTexture();

		SPtr<TextureCore> texture;
		SPtr<RenderTextureCore> renderTexture;

	private:
		friend class RenderTexturePool;

		RenderTexturePool* mPool;
		bool mIsFree;
	};

	/** Contains a pool of render textures meant to accommodate reuse of render textures of the same size and format. */
	class RenderTexturePool : public Module<RenderTexturePool>
	{
	public:
		~RenderTexturePool();

		/**
		 * Attempts to find the unused render texture with the specified parameters in the pool, or creates a new texture
		 * otherwise. When done with the texture make sure to call release().
		 *
		 * @param[in]	desc		Descriptor structure that describes what kind of texture to retrieve.
		 */
		SPtr<PooledRenderTexture> get(const POOLED_RENDER_TEXTURE_DESC& desc);

		/**
		 * Releases a texture previously allocated with get(). The texture is returned to the pool so that it may be reused
		 * later.
		 *			
		 * @note	
		 * The texture will be removed from the pool if the last reference to it is deleted. Normally you would call 
		 * release() but keep a reference if you plan on using it later on.
		 */
		void release(const SPtr<PooledRenderTexture>& texture);

	private:
		friend struct PooledRenderTexture;

		/**	Registers a newly created render texture in the pool. */
		void _registerTexture(const SPtr<PooledRenderTexture>& texture);

		/**	Unregisters a created render texture in the pool. */
		void _unregisterTexture(PooledRenderTexture* texture);

		/**
		 * Checks does the provided texture match the parameters.
		 * 
		 * @param[in]	desc	Descriptor structure that describes what kind of texture to match.
		 * @return				True if the texture matches the descriptor, false otherwise.
		 */
		static bool matches(const SPtr<TextureCore>& texture, const POOLED_RENDER_TEXTURE_DESC& desc);

		Map<PooledRenderTexture*, std::weak_ptr<PooledRenderTexture>> mTextures;
	};

	/** Structure used for creating a new pooled render texture. */
	struct POOLED_RENDER_TEXTURE_DESC
	{
	public:
		POOLED_RENDER_TEXTURE_DESC() {}

		/**
		 * Creates a descriptor for a two dimensional render texture.
		 *
		 * @param[in]	format		Pixel format used by the texture surface.
		 * @param[in]	width		Width of the render texture, in pixels.
		 * @param[in]	height		Height of the render texture, in pixels.
		 * @param[in]	usage		Usage flags that control in which way is the texture going to be used.
		 * @param[in]	samples		If higher than 1, texture containing multiple samples per pixel is created.
		 * @param[in]	hwGamma		Should the written pixels be gamma corrected.
		 * @return					Descriptor that is accepted by RenderTexturePool.
		 */
		static POOLED_RENDER_TEXTURE_DESC create2D(PixelFormat format, UINT32 width, UINT32 height, 
			INT32 usage = TU_STATIC, UINT32 samples = 0, bool hwGamma = false);

		/**
		 * Creates a descriptor for a three dimensional render texture.
		 *
		 * @param[in]	format		Pixel format used by the texture surface.
		 * @param[in]	width		Width of the render texture, in pixels.
		 * @param[in]	height		Height of the render texture, in pixels.
		 * @param[in]	depth		Depth of the render texture, in pixels.
		 * @param[in]	usage		Usage flags that control in which way is the texture going to be used.
		 * @return					Descriptor that is accepted by RenderTexturePool.
		 */
		static POOLED_RENDER_TEXTURE_DESC create3D(PixelFormat format, UINT32 width, UINT32 height, UINT32 depth,
			INT32 usage = TU_STATIC);

		/**
		 * Creates a descriptor for a cube render texture.
		 *
		 * @param[in]	format		Pixel format used by the texture surface.
		 * @param[in]	width		Width of the render texture, in pixels.
		 * @param[in]	height		Height of the render texture, in pixels.
		 * @param[in]	usage		Usage flags that control in which way is the texture going to be used.
		 * @return					Descriptor that is accepted by RenderTexturePool.
		 */
		static POOLED_RENDER_TEXTURE_DESC createCube(PixelFormat format, UINT32 width, UINT32 height,
			INT32 usage = TU_STATIC);

	private:
		friend class RenderTexturePool;

		UINT32 width;
		UINT32 height;
		UINT32 depth;
		UINT32 numSamples;
		PixelFormat format;
		TextureUsage flag;
		TextureType type;
		bool hwGamma;
	};

	/** @} */
}