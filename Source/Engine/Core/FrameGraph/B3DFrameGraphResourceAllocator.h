//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DTexture.h"
#include "RenderAPI/B3DGpuBuffer.h"

namespace b3d
{
	class GpuDevice;

	namespace render
	{
		/** @addtogroup RenderAPI
		 *  @{
		 */

		/**
		 * Allocates and manages transient resources with pool-based allocation.
		 *
		 * Uses per-type resource pools to avoid per-frame GPU object creation overhead.
		 * Resources are allocated on-demand and returned to the pool for reuse across frames.
		 * This minimizes GPU memory allocation/deallocation overhead while reducing peak memory usage.
		 *
		 * **Allocation Strategy:**
		 * - When a resource is requested, search the pool for a compatible free resource
		 * - If found, mark it as in-use and return it
		 * - If not found, create a new resource and add it to the pool
		 * - When a resource is freed, mark it as available (don't destroy it)
		 * - Reset() at end of frame marks all resources as available
		 *
		 * **Descriptor Compatibility:**
		 * Resources are considered compatible if their creation parameters match exactly.
		 * See AreTextureCreateInformationCompatible() and AreBufferCreateInformationCompatible()
		 * for specific compatibility rules.
		 *
		 * **Phase 4 Scope:**
		 * - No memory aliasing (each resource gets independent allocation)
		 * - Single queue only (no cross-queue resource sharing)
		 * - Phase 5 will add alias group support for memory sharing
		 *
		 * **Performance:**
		 * - Pool allocation overhead target: < 0.1ms per resource
		 * - No GPU object creation/destruction per frame for reused resources
		 * - Cache-friendly linear pool scans
		 */
		class B3D_EXPORT FrameGraphResourceAllocator
		{
		public:
			/**
			 * Creates a new resource allocator.
			 *
			 * @param device    GPU device used to create resources
			 */
			explicit FrameGraphResourceAllocator(GpuDevice& device);
			~FrameGraphResourceAllocator();

			/**
			 * Allocates a transient texture from the pool.
			 *
			 * Searches the pool for a free texture with compatible creation parameters.
			 * If found, marks it as in-use and returns it. Otherwise creates a new texture
			 * and adds it to the pool.
			 *
			 * @param name                  Resource name for debugging/profiling
			 * @param createInformation     Texture creation descriptor
			 * @return                      Allocated texture (never null)
			 *
			 * @note The returned texture is guaranteed to be valid and match the requested descriptor.
			 */
			SPtr<Texture> AllocateTexture(
				const StringView& name,
				const TextureCreateInformation& createInformation);

			/**
			 * Allocates a transient buffer from the pool.
			 *
			 * Searches the pool for a free buffer with compatible creation parameters.
			 * If found, marks it as in-use and returns it. Otherwise creates a new buffer
			 * and adds it to the pool.
			 *
			 * @param name                  Resource name for debugging/profiling
			 * @param createInformation     Buffer creation descriptor
			 * @return                      Allocated buffer (never null)
			 */
			SPtr<GpuBuffer> AllocateBuffer(
				const StringView& name,
				const GpuBufferCreateInformation& createInformation);

			/**
			 * Returns a texture to the pool for reuse.
			 *
			 * The texture is not destroyed, just marked as available for future allocations.
			 * The texture must have been allocated by this allocator.
			 *
			 * @param texture   Texture to free (must not be null)
			 */
			void FreeTexture(const SPtr<Texture>& texture);

			/**
			 * Returns a buffer to the pool for reuse.
			 *
			 * The buffer is not destroyed, just marked as available for future allocations.
			 * The buffer must have been allocated by this allocator.
			 *
			 * @param buffer    Buffer to free (must not be null)
			 */
			void FreeBuffer(const SPtr<GpuBuffer>& buffer);

			/**
			 * Resets all pools at end of frame.
			 *
			 * Marks all resources (textures and buffers) as available for reuse.
			 * Resources are not destroyed - they remain in the pool for next frame.
			 * This should be called after Execute() completes.
			 */
			void Reset();

			/** Statistics about current allocation state for profiling and debugging */
			struct Statistics
			{
				/** Total number of textures in the pool (in-use + available) */
				u32 TotalTexturesAllocated = 0;

				/** Total number of buffers in the pool (in-use + available) */
				u32 TotalBuffersAllocated = 0;

				/** Number of textures currently marked as in-use */
				u32 TexturesInUse = 0;

				/** Number of buffers currently marked as in-use */
				u32 BuffersInUse = 0;

				/** Estimated total GPU memory used by textures in the pool (bytes) */
				u64 TotalTextureMemoryBytes = 0;

				/** Estimated total GPU memory used by buffers in the pool (bytes) */
				u64 TotalBufferMemoryBytes = 0;
			};

			/**
			 * Returns statistics about current allocation state.
			 *
			 * @return  Current allocation statistics (counts, memory usage, etc.)
			 */
			Statistics GetStatistics() const;

		private:
			/** Pool entry for a texture resource */
			struct TexturePoolEntry
			{
				/** The actual texture resource */
				SPtr<Texture> TextureResource;

				/** Creation parameters for compatibility checking */
				TextureCreateInformation CreateInformation;

				/** True if currently allocated to a transient resource */
				bool InUse = false;

				/** Estimated GPU memory size in bytes */
				u64 MemorySize = 0;
			};

			/** Pool entry for a buffer resource */
			struct BufferPoolEntry
			{
				/** The actual buffer resource */
				SPtr<GpuBuffer> BufferResource;

				/** Creation parameters for compatibility checking */
				GpuBufferCreateInformation CreateInformation;

				/** True if currently allocated to a transient resource */
				bool InUse = false;

				/** Estimated GPU memory size in bytes */
				u64 MemorySize = 0;
			};

			/**
			 * Finds a free texture in the pool with compatible creation parameters.
			 *
			 * @param createInformation     Desired texture parameters
			 * @return                      Compatible free texture or null if none found
			 */
			SPtr<Texture> FindFreeTexture(const TextureCreateInformation& createInformation);

			/**
			 * Finds a free buffer in the pool with compatible creation parameters.
			 *
			 * @param createInformation     Desired buffer parameters
			 * @return                      Compatible free buffer or null if none found
			 */
			SPtr<GpuBuffer> FindFreeBuffer(const GpuBufferCreateInformation& createInformation);

			/**
			 * Checks if two texture creation descriptors are compatible for reuse.
			 *
			 * Textures are compatible if they have identical:
			 * - Type (2D, 3D, Cube, etc.)
			 * - Width, Height, Depth
			 * - Format (pixel format)
			 * - Mipmap count
			 * - Sample count (MSAA)
			 * - Array slice count
			 * - Usage flags
			 * - SRGB setting
			 *
			 * @param a     First descriptor
			 * @param b     Second descriptor
			 * @return      True if descriptors are compatible for resource reuse
			 */
			bool AreTextureCreateInformationCompatible(
				const TextureCreateInformation& a,
				const TextureCreateInformation& b) const;

			/**
			 * Checks if two buffer creation descriptors are compatible for reuse.
			 *
			 * Buffers are compatible if they have identical:
			 * - Type (Vertex, Index, Uniform, etc.)
			 * - Element count
			 * - Element size
			 * - Format (for storage buffers)
			 * - Flags (storage location, access patterns)
			 * - Suballocation count
			 *
			 * @param a     First descriptor
			 * @param b     Second descriptor
			 * @return      True if descriptors are compatible for resource reuse
			 */
			bool AreBufferCreateInformationCompatible(
				const GpuBufferCreateInformation& a,
				const GpuBufferCreateInformation& b) const;

			/**
			 * Calculates estimated GPU memory size for a texture.
			 *
			 * @param createInformation     Texture creation parameters
			 * @return                      Estimated memory size in bytes
			 */
			u64 CalculateTextureMemorySize(const TextureCreateInformation& createInformation) const;

			/**
			 * Calculates estimated GPU memory size for a buffer.
			 *
			 * @param createInformation     Buffer creation parameters
			 * @return                      Estimated memory size in bytes
			 */
			u64 CalculateBufferMemorySize(const GpuBufferCreateInformation& createInformation) const;

			/** GPU device used to create resources */
			GpuDevice& mDevice;

			/** Pool of texture resources (in-use and available) */
			Vector<TexturePoolEntry> mTexturePool;

			/** Pool of buffer resources (in-use and available) */
			Vector<BufferPoolEntry> mBufferPool;

			/** Cached statistics updated on allocate/free */
			mutable Statistics mStatistics;

			/** Flag indicating statistics need recalculation */
			mutable bool mStatisticsDirty = true;
		};

		/** @} */
	}
}
