//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "B3DMetalResource.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "Allocators/B3DPoolAlloc.h"

namespace b3d
{
	namespace render
	{
		class MetalGpuDevice;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

#ifdef __OBJC__
		/** Native Metal buffer handle. Aliased to void* in plain C++ TUs so class layouts stay identical (id is a pointer). */
		using MetalBufferNativeHandle = id<MTLBuffer>;
#else
		using MetalBufferNativeHandle = void*;
#endif

		/** Descriptor used to create a MetalBuffer. */
		struct MetalBufferCreateInformation
		{
			GpuBufferType Type = GpuBufferType::Vertex; /**< Type of the buffer being created. */
			GpuBufferFlags Flags; /**< Flags that specify how the buffer is intended to be used. */
			StringView DebugName; /**< Optional name of the resource, for debugging purposes. */
		};

		/** Wrapper around a Metal buffer object that manages its usage and lifetime. */
		class MetalBuffer : public TMetalResource<IGpuBufferResource>
		{
		public:
			/**
			 * @param	owner				Manager that takes care of tracking and releasing of this object.
			 * @param	createInformation	Describes the buffer being wrapped.
			 * @param	buffer				Native MTLBuffer handle the wrapper takes ownership of (+1 reference under MRC).
			 * @param	allocation			Engine allocator span backing the buffer's memory, or an invalid location for
			 *								direct (non-sub-allocated) device allocations.
			 * @param	mappedMemory		CPU-visible contents pointer for shared-storage buffers, null otherwise.
			 */
			MetalBuffer(MetalResourceManager* owner, const MetalBufferCreateInformation& createInformation, MetalBufferNativeHandle buffer, const GpuResourceLocation& allocation, void* mappedMemory);
			~MetalBuffer();

			/** Assigns a name to the buffer, primarily used for easier debugging. */
			void SetName(const StringView& name);

			/** Returns a pointer to persistently mapped memory of the buffer, or null pointer if the buffer is not mappable. */
			void* GetMappedMemory() const { return mMappedMemory; }

#ifdef __OBJC__
			/** Returns the internal handle to the Metal object. */
			id<MTLBuffer> GetMetalHandle() const { return mBuffer; }
#endif

		private:
			GpuBufferType mType;
			GpuBufferFlags mFlags;
			MetalBufferNativeHandle mBuffer = nullptr;
			GpuResourceLocation mAllocation;
			void* mMappedMemory = nullptr;
		};

		/**
		 * Metal implementation of a GPU buffer.
		 *
		 * High-level proxy over a MetalBuffer wrapper. Storage mode follows the engine flags:
		 * buffers created with @c GpuBufferFlag::StoreOnCPUWithGPUAccess (or typed as
		 * @c StagingRead / @c StagingWrite) use shared storage so @c Map returns
		 * @c [buffer contents] directly; all other buffers use private storage, which makes them
		 * CPU-invisible so @c GpuBufferUtility routes their CPU traffic through a staging buffer +
		 * @c CopyBufferToBuffer on the transfer queue. Persistent buffers use the device's
		 * per-memory-type TLSF allocator; work-context transient buffers use a completion-tracked
		 * linear allocator (see MetalHeapAllocator).
		 */
		class MetalGpuBuffer : public GpuBuffer
		{
		public:
			MetalGpuBuffer(MetalGpuDevice& device, const GpuBufferCreateInformation& createInformation);
			MetalGpuBuffer(MetalGpuDevice& device, const GpuBufferCreateInformation& createInformation,
				IGpuAllocator& allocator);
			~MetalGpuBuffer();

			void SetName(const StringView& name) override;
			GpuQueueMask GetUseMask(GpuAccessFlags accessFlags) override;
			u32 GetBoundCount() const override;
			u32 GetUseCount() const override;

#if B3D_BUILD_TYPE_DEVELOPMENT
			bool IsRangeBound(u32 offset, u32 size) const override;
			bool IsRangeInUse(u32 offset, u32 size) const override;
#endif

			/** Gets the resource wrapping the buffer object. */
			MetalBuffer* GetMetalResource() const { return mBuffer; }

#ifdef __OBJC__
			/** Returns the underlying MTLBuffer. May be nil before Initialize() has been called, or if creation failed. */
			id<MTLBuffer> GetMetalBuffer() const;
#endif

		protected:
			friend class MetalGpuDevice;

			void Initialize() override;
			void RecreateInternalBuffer() override;

		private:
			/** Creates a new low-level buffer wrapper matching the current buffer properties. Returns null on failure. */
			MetalBuffer* CreateBuffer();

			MetalGpuDevice& mGpuDevice;
			MetalBuffer* mBuffer = nullptr;

			/** Memory type the buffer's backing memory allocates from. Pure function of the create information, resolved once. */
			u32 mMemoryType = 0;

			/** Explicit backing allocator for transient buffers; null selects the device's persistent allocator. */
			IGpuAllocator* mAllocator = nullptr;

			bool mDirectlyMappable : 1;
			bool mSupportsGPUWrites : 1;
		};

		/** @} */
	} // namespace render
} // namespace b3d

namespace b3d
{
	B3D_IMPLEMENT_GLOBAL_POOL(render::MetalGpuBuffer, 32)
}
