//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "Image/BsPixelUtil.h"
#include "Image/BsTexture.h"
#include "Utility/BsDynArray.h"

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
		PooledRenderTexture(UINT32 lastUsedFrame)
			:mLastUsedFrame(lastUsedFrame)
		{ }

		SPtr<Texture> texture;
		SPtr<RenderTexture> renderTexture;

	private:
		friend class GpuResourcePool;

		UINT32 mLastUsedFrame = 0;
	};

	/**	Contains data about a single storage buffer in the GPU resource pool. */
	struct BS_CORE_EXPORT PooledStorageBuffer
	{
		PooledStorageBuffer(UINT32 lastUsedFrame)
			:mLastUsedFrame(lastUsedFrame)
		{ }

		SPtr<GpuBuffer> buffer;

	private:
		friend class GpuResourcePool;

		UINT32 mLastUsedFrame = 0;
	};

	/**
	 * Contains a pool of textures and buffers meant to accommodate reuse of such resources for the main purpose of using
	 * them as write targets on the GPU.
	 */
	class BS_CORE_EXPORT GpuResourcePool : public Module<GpuResourcePool>
	{
	public:
		/**
		 * Attempts to find the unused render texture with the specified parameters in the pool, or creates a new texture
		 * otherwise.
		 *
		 * @param[in]	desc		Descriptor structure that describes what kind of texture to retrieve.
		 */
		SPtr<PooledRenderTexture> get(const POOLED_RENDER_TEXTURE_DESC& desc);

		/**
		 * Attempts to find the unused render texture with the specified parameters in the pool, or creates a new texture
		 * otherwise. Use this variant of the method if you are already holding a reference to a pooled texture which
		 * you want to reuse - this is more efficient than releasing the old texture and calling the other get() variant.
		 *
		 * @param[in, out]	texture		Existing reference to a pooled texture that you would prefer to reuse. If it
		 *								matches the provided descriptor the system will return the unchanged texture,
		 *								otherwise it will try to find another unused texture, or allocate a new one. New
		 *								value will be output through this parameter.
		 * @param[in]		desc		Descriptor structure that describes what kind of texture to retrieve.
		 */
		void get(SPtr<PooledRenderTexture>& texture, const POOLED_RENDER_TEXTURE_DESC& desc);

		/**
		 * Attempts to find the unused storage buffer with the specified parameters in the pool, or creates a new buffer
		 * otherwise.
		 *
		 * @param[in]	desc		Descriptor structure that describes what kind of buffer to retrieve.
		 */
		SPtr<PooledStorageBuffer> get(const POOLED_STORAGE_BUFFER_DESC& desc);

		/**
		 * Attempts to find the unused storage buffer with the specified parameters in the pool, or creates a new buffer
		 * otherwise. Use this variant of the method if you are already holding a reference to a pooled buffer which
		 * you want to reuse - this is more efficient than releasing the old buffer and calling the other get() variant.
		 *
		 * @param[in, out]	buffer		Existing reference to a pooled buffer that you would prefer to reuse. If it
		 *								matches the provided descriptor the system will return the unchanged buffer,
		 *								otherwise it will try to find another unused buffer, or allocate a new one. New
		 *								value will be output through this parameter.
		 * @param[in]	desc			Descriptor structure that describes what kind of buffer to retrieve.
		 */
		void get(SPtr<PooledStorageBuffer>& buffer, const POOLED_STORAGE_BUFFER_DESC& desc);

		/** Lets the pool know that another frame has passed. */
		void update();

		/**
		 * Destroys all unreferenced resources with that were last used @p age frames ago. Specify 0 to destroy all
		 * unreferenced resources.
		 */
		void prune(UINT32 age);
	private:
		/**
		 * Checks does the provided texture match the parameters.
		 *
		 * @param[in]	texture		Texture to check.
		 * @param[in]	desc		Descriptor structure that describes what kind of texture to match.
		 * @return					True if the texture matches the descriptor, false otherwise.
		 */
		static bool matches(const SPtr<Texture>& texture, const POOLED_RENDER_TEXTURE_DESC& desc);

		/**
		 * Checks does the provided buffer match the parameters.
		 *
		 * @param[in]	buffer	Buffer to check.
		 * @param[in]	desc	Descriptor structure that describes what kind of buffer to match.
		 * @return				True if the buffer matches the descriptor, false otherwise.
		 */
		static bool matches(const SPtr<GpuBuffer>& buffer, const POOLED_STORAGE_BUFFER_DESC& desc);

		DynArray<SPtr<PooledRenderTexture>> mTextures;
		DynArray<SPtr<PooledStorageBuffer>> mBuffers;

		UINT32 mCurrentFrame = 0;
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

	/**	Provides easy access to the GpuResourcePool. */
	BS_CORE_EXPORT GpuResourcePool& gGpuResourcePool();

	/** @} */
}}
