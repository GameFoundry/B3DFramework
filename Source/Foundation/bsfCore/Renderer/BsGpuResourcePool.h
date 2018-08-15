//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "Image/BsPixelUtil.h"
#include "Image/BsTexture.h"

namespace bs { namespace ct
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	class GpuResourcePool;
	struct POOLED_RENDER_TEXTURE_DESC;
	struct POOLED_STORAGE_BUFFER_DESC;

	/**	Contains data about a single render texture in the GPU resource pool. */
	struct BS_CORE_EXPORT PooledRenderTexture
	{
		PooledRenderTexture(GpuResourcePool* pool);
		~PooledRenderTexture();

		SPtr<Texture> texture;
		SPtr<RenderTexture> renderTexture;

	private:
		friend class GpuResourcePool;

		GpuResourcePool* mPool;
		bool mIsFree;
	};

	/**	Contains data about a single storage buffer in the GPU resource pool. */
	struct BS_CORE_EXPORT PooledStorageBuffer
	{
		PooledStorageBuffer(GpuResourcePool* pool);
		~PooledStorageBuffer();

		SPtr<GpuBuffer> buffer;

	private:
		friend class GpuResourcePool;

		GpuResourcePool* mPool;
		bool mIsFree;
	};

	/** 
	 * Contains a pool of textures and buffers meant to accommodate reuse of such resources for the main purpose of using
	 * them as write targets on the GPU.
	 */
	class BS_CORE_EXPORT GpuResourcePool : public Module<GpuResourcePool>
	{
	public:
		~GpuResourcePool();

		/**
		 * Attempts to find the unused render texture with the specified parameters in the pool, or creates a new texture
		 * otherwise. When done with the texture make sure to call release(const POOLED_RENDER_TEXTURE_DESC&).
		 *
		 * @param[in]	desc		Descriptor structure that describes what kind of texture to retrieve.
		 */
		SPtr<PooledRenderTexture> get(const POOLED_RENDER_TEXTURE_DESC& desc);

		/**
		 * Attempts to find the unused storage buffer with the specified parameters in the pool, or creates a new buffer
		 * otherwise. When done with the buffer make sure to call release(const POOLED_STORAGE_BUFFER_DESC&).
		 *
		 * @param[in]	desc		Descriptor structure that describes what kind of buffer to retrieve.
		 */
		SPtr<PooledStorageBuffer> get(const POOLED_STORAGE_BUFFER_DESC& desc);

		/**
		 * Releases a texture previously allocated with get(const POOLED_RENDER_TEXTURE_DESC&). The texture is returned to
		 * the pool so that it may be reused later.
		 *			
		 * @note	
		 * The texture will be removed from the pool if the last reference to it is deleted. Normally you would call 
		 * release(const POOLED_RENDER_TEXTURE_DESC&) but keep a reference if you plan on using it later on.
		 */
		void release(const SPtr<PooledRenderTexture>& texture);

		/**
		 * Releases a buffer previously allocated with get(const POOLED_STORAGE_BUFFER_DESC&). The buffer is returned to the
		 * pool so that it may be reused later.
		 *			
		 * @note	
		 * The buffer will be removed from the pool if the last reference to it is deleted. Normally you would call 
		 * release(const POOLED_STORAGE_BUFFER_DESC&) but keep a reference if you plan on using it later on.
		 */
		void release(const SPtr<PooledStorageBuffer>& buffer);

	private:
		friend struct PooledRenderTexture;
		friend struct PooledStorageBuffer;

		/**	Registers a newly created render texture in the pool. */
		void _registerTexture(const SPtr<PooledRenderTexture>& texture);

		/**	Unregisters a created render texture in the pool. */
		void _unregisterTexture(PooledRenderTexture* texture);

		/**	Registers a newly created storage buffer in the pool. */
		void _registerBuffer(const SPtr<PooledStorageBuffer>& buffer);

		/**	Unregisters a created storage buffer in the pool. */
		void _unregisterBuffer(PooledStorageBuffer* buffer);

		/**
		 * Checks does the provided texture match the parameters.
		 * 
		 * @param[in]	desc	Descriptor structure that describes what kind of texture to match.
		 * @return				True if the texture matches the descriptor, false otherwise.
		 */
		static bool matches(const SPtr<Texture>& texture, const POOLED_RENDER_TEXTURE_DESC& desc);

		/**
		 * Checks does the provided buffer match the parameters.
		 * 
		 * @param[in]	desc	Descriptor structure that describes what kind of buffer to match.
		 * @return				True if the buffer matches the descriptor, false otherwise.
		 */
		static bool matches(const SPtr<GpuBuffer>& buffer, const POOLED_STORAGE_BUFFER_DESC& desc);

		Map<PooledRenderTexture*, std::weak_ptr<PooledRenderTexture>> mTextures;
		Map<PooledStorageBuffer*, std::weak_ptr<PooledStorageBuffer>> mBuffers;
	};

	/** Structure used for creating a new pooled render texture. */
	struct BS_CORE_EXPORT POOLED_RENDER_TEXTURE_DESC
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
		 * @param[in]	arraySize	Number of textures in a texture array. Specify 1 for no array.
		 * @param[in]	mipCount	Number of mip levels, excluding the root mip level.
		 * @return					Descriptor that is accepted by RenderTexturePool.
		 */
		static POOLED_RENDER_TEXTURE_DESC create2D(PixelFormat format, UINT32 width, UINT32 height, 
			INT32 usage = TU_STATIC, UINT32 samples = 0, bool hwGamma = false, UINT32 arraySize = 1, UINT32 mipCount = 0);

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
		 * @param[in]	arraySize	Number of textures in a texture array. Specify 1 for no array.
		 * @return					Descriptor that is accepted by RenderTexturePool.
		 */
		static POOLED_RENDER_TEXTURE_DESC createCube(PixelFormat format, UINT32 width, UINT32 height,
			INT32 usage = TU_STATIC, UINT32 arraySize = 1);

	private:
		friend class GpuResourcePool;

		UINT32 width;
		UINT32 height;
		UINT32 depth;
		UINT32 numSamples;
		PixelFormat format;
		TextureUsage flag;
		TextureType type;
		bool hwGamma;
		UINT32 arraySize;
		UINT32 numMipLevels;
	};

	/** Structure used for describing a pooled storage buffer. */
	struct BS_CORE_EXPORT POOLED_STORAGE_BUFFER_DESC
	{
	public:
		POOLED_STORAGE_BUFFER_DESC() {}

		/**
		 * Creates a descriptor for a storage buffer containing primitive data types.
		 *
		 * @param[in]	format		Format of individual buffer entries.
		 * @param[in]	numElements	Number of elements in the buffer.
		 * @param[in]	usage		Usage flag hinting the driver how is buffer going to be used.
		 */
		static POOLED_STORAGE_BUFFER_DESC createStandard(GpuBufferFormat format, UINT32 numElements, 
			GpuBufferUsage usage = GBU_LOADSTORE);

		/**
		 * Creates a descriptor for a storage buffer containing structures.
		 *
		 * @param[in]	elementSize		Size of a single structure in the buffer.
		 * @param[in]	numElements		Number of elements in the buffer.
		 * @param[in]	usage		Usage flag hinting the driver how is buffer going to be used.
		 */
		static POOLED_STORAGE_BUFFER_DESC createStructured(UINT32 elementSize, UINT32 numElements,
			GpuBufferUsage usage = GBU_LOADSTORE);

	private:
		friend class GpuResourcePool;

		GpuBufferType type;
		GpuBufferFormat format;
		GpuBufferUsage usage;
		UINT32 numElements;
		UINT32 elementSize;
	};

	/** @} */
}}