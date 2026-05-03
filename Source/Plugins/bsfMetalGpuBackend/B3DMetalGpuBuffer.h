//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuBuffer.h"

namespace b3d
{
	namespace render
	{
		class MetalGpuDevice;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**
		 * Metal implementation of a GPU buffer.
		 *
		 * Wraps a single @c MTLBuffer. Storage mode follows the engine flags: buffers created with
		 * @c GpuBufferFlag::StoreOnCPUWithGPUAccess (or typed as @c StagingRead / @c StagingWrite) use
		 * shared storage so @c Map returns @c [buffer contents] directly; all other buffers use
		 * private storage, which makes them CPU-invisible so @c GpuBufferUtility routes their CPU
		 * traffic through a staging buffer + @c CopyBufferToBuffer on the transfer queue.
		 */
		class MetalGpuBuffer : public GpuBuffer
		{
		public:
			MetalGpuBuffer(MetalGpuDevice& device, const GpuBufferCreateInformation& createInformation);
			~MetalGpuBuffer();

			void SetName(const StringView& name) override;
			GpuQueueMask GetUseMask(GpuAccessFlags accessFlags) override { return GpuQueueMask::kNone; }
			u32 GetBoundCount() const override { return 0; }
			u32 GetUseCount() const override { return 0; }

#if B3D_BUILD_TYPE_DEVELOPMENT
			bool IsRangeBound(u32 offset, u32 size) const override { return false; }
			bool IsRangeInUse(u32 offset, u32 size) const override { return false; }
#endif

#ifdef __OBJC__
			/** Returns the underlying MTLBuffer. May be nil before Initialize() has been called. */
			id<MTLBuffer> GetMetalBuffer() const;
#endif

		protected:
			friend class MetalGpuDevice;

			void Initialize() override;
			void RecreateInternalBuffer() override;

		private:
			void CreateInternalBuffer();
			void ReleaseInternalBuffer();

			struct Impl;

			MetalGpuDevice& mGpuDevice;
			UPtr<Impl> mImpl;
			// Note: no local @c mName — we inherit @c GpuBuffer::mName and its @c GetName accessor,
			// delegating the store through @c GpuBuffer::SetName in our override.
		};

		/** @} */
	} // namespace render
} // namespace b3d
