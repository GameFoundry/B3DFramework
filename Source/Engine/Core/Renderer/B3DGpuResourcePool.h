//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Image/B3DPixelUtility.h"
#include "Image/B3DTexture.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup RenderBeast
		 *  @{
		 */

		class GpuResourcePool;
		struct PooledRenderTextureCreateInformation;
		struct POOLED_STORAGE_BUFFER_DESC;

		/**	Contains data about a single render texture in the GPU resource pool. */
		struct B3D_EXPORT PooledRenderTexture
		{
			PooledRenderTexture(u32 lastUsedFrame)
				: mLastUsedFrame(lastUsedFrame)
			{}

			SPtr<Texture> Texture;
			SPtr<RenderTexture> RenderTexture;

		private:
			friend class GpuResourcePool;

			u32 mLastUsedFrame = 0;
		};

		/**	Contains data about a single storage buffer in the GPU resource pool. */
		struct B3D_EXPORT PooledStorageBuffer
		{
			PooledStorageBuffer(u32 lastUsedFrame)
				: mLastUsedFrame(lastUsedFrame)
			{}

			SPtr<GpuBuffer> Buffer;

		private:
			friend class GpuResourcePool;

			u32 mLastUsedFrame = 0;
		};

		/**
		 * Contains a pool of textures and buffers meant to accommodate reuse of such resources for the main purpose of using
		 * them as write targets on the GPU.
		 */
		class B3D_EXPORT GpuResourcePool : public Module<GpuResourcePool>
		{
		public:
			GpuResourcePool();

			/**
			 * Attempts to find the unused render texture with the specified parameters in the pool, or creates a new texture
			 * otherwise.
			 *
			 * @param[in]	desc		Descriptor structure that describes what kind of texture to retrieve.
			 */
			SPtr<PooledRenderTexture> Get(const PooledRenderTextureCreateInformation& desc);

			/**
			 * Attempts to find the unused render texture with the specified parameters in the pool, or creates a new texture
			 * otherwise. Use this variant of the method if you are already holding a reference to a pooled texture which
			 * you want to reuse - this is more efficient than releasing the old texture and calling the other Get() variant.
			 *
			 * @param[in, out]	texture		Existing reference to a pooled texture that you would prefer to reuse. If it
			 *								matches the provided descriptor the system will return the unchanged texture,
			 *								otherwise it will try to find another unused texture, or allocate a new one. New
			 *								value will be output through this parameter.
			 * @param[in]		desc		Descriptor structure that describes what kind of texture to retrieve.
			 */
			void Get(SPtr<PooledRenderTexture>& texture, const PooledRenderTextureCreateInformation& desc);

			/**
			 * Attempts to find the unused storage buffer with the specified parameters in the pool, or creates a new buffer
			 * otherwise.
			 *
			 * @param[in]	desc		Descriptor structure that describes what kind of buffer to retrieve.
			 */
			SPtr<PooledStorageBuffer> Get(const POOLED_STORAGE_BUFFER_DESC& desc);

			/**
			 * Attempts to find the unused storage buffer with the specified parameters in the pool, or creates a new buffer
			 * otherwise. Use this variant of the method if you are already holding a reference to a pooled buffer which
			 * you want to reuse - this is more efficient than releasing the old buffer and calling the other Get() variant.
			 *
			 * @param[in, out]	buffer		Existing reference to a pooled buffer that you would prefer to reuse. If it
			 *								matches the provided descriptor the system will return the unchanged buffer,
			 *								otherwise it will try to find another unused buffer, or allocate a new one. New
			 *								value will be output through this parameter.
			 * @param[in]	desc			Descriptor structure that describes what kind of buffer to retrieve.
			 */
			void Get(SPtr<PooledStorageBuffer>& buffer, const POOLED_STORAGE_BUFFER_DESC& desc);

			/** Lets the pool know that another frame has passed. */
			void Update();

			/**
			 * Destroys all unreferenced resources with that were last used @p age frames ago. Specify 0 to destroy all
			 * unreferenced resources.
			 */
			void Prune(u32 age);

		private:
			/**
			 * Checks does the provided texture match the parameters.
			 *
			 * @param[in]	texture		Texture to check.
			 * @param[in]	desc		Descriptor structure that describes what kind of texture to match.
			 * @return					True if the texture matches the descriptor, false otherwise.
			 */
			static bool Matches(const SPtr<Texture>& texture, const PooledRenderTextureCreateInformation& desc);

			/**
			 * Checks does the provided buffer match the parameters.
			 *
			 * @param[in]	buffer	Buffer to check.
			 * @param[in]	desc	Descriptor structure that describes what kind of buffer to match.
			 * @return				True if the buffer matches the descriptor, false otherwise.
			 */
			static bool Matches(const SPtr<GpuBuffer>& buffer, const POOLED_STORAGE_BUFFER_DESC& desc);

			SPtr<GpuDevice> mDevice;
			TArray<SPtr<PooledRenderTexture>> mTextures;
			TArray<SPtr<PooledStorageBuffer>> mBuffers;

			u32 mCurrentFrame = 0;
		};

		/** Structure used for creating a new pooled render texture. */
		struct B3D_EXPORT PooledRenderTextureCreateInformation
		{
			// TODO - Add a required Name parameter to each Create method, and propagate it to created textures
		public:
			PooledRenderTextureCreateInformation() {}

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
			static PooledRenderTextureCreateInformation Create2D(PixelFormat format, u32 width, u32 height, TextureUsageFlags usage = TextureUsageFlag::Default, u32 samples = 0, bool hwGamma = false, u32 arraySize = 1, u32 mipCount = 0);

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
			static PooledRenderTextureCreateInformation Create3D(PixelFormat format, u32 width, u32 height, u32 depth, TextureUsageFlags usage = TextureUsageFlag::Default);

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
			static PooledRenderTextureCreateInformation CreateCube(PixelFormat format, u32 width, u32 height, TextureUsageFlags usage = TextureUsageFlag::Default, u32 arraySize = 1);

		private:
			friend class GpuResourcePool;

			u32 width;
			u32 height;
			u32 depth;
			u32 numSamples;
			PixelFormat format;
			TextureUsageFlags flag;
			TextureType type;
			bool hwGamma;
			u32 arraySize;
			u32 numMipLevels;
		};

		/** Structure used for describing a pooled storage buffer. */
		struct B3D_EXPORT POOLED_STORAGE_BUFFER_DESC
		{
		public:
			POOLED_STORAGE_BUFFER_DESC() {}

			/**
			 * Creates a descriptor for a storage buffer containing primitive data types.
			 *
			 * @param	format		Format of individual buffer entries.
			 * @param	numElements	Number of elements in the buffer.
			 * @param	flags		Flags that control the behaviour of the buffer.
			 */
			static POOLED_STORAGE_BUFFER_DESC CreateStandard(GpuBufferFormat format, u32 numElements, GpuBufferFlags flags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU);

			/**
			 * Creates a descriptor for a storage buffer containing structures.
			 *
			 * @param	elementSize		Size of a single structure in the buffer.
			 * @param	numElements		Number of elements in the buffer.
			 * @param	flags		Flags that control the behaviour of the buffer.
			 */
			static POOLED_STORAGE_BUFFER_DESC CreateStructured(u32 elementSize, u32 numElements, GpuBufferFlags flags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU);

		private:
			friend class GpuResourcePool;

			GpuBufferType type;
			GpuBufferFormat format;
			GpuBufferFlags flags;
			u32 numElements;
			u32 elementSize;
		};

		/**	Provides easy access to the GpuResourcePool. */
		B3D_EXPORT GpuResourcePool& GetGpuResourcePool();

		/** @} */
	} // namespace render
} // namespace b3d
