//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "B3DNullResource.h"

namespace b3d
{
	namespace render
	{
		class NullGpuDevice;

		/** @addtogroup NullGpuBackend 
		 *  @{
		 */

		/**	Null implementation of a GPU buffer. */
		class NullGpuBuffer : public GpuBuffer
		{
		public:
			NullGpuBuffer(NullGpuDevice& device, const GpuBufferCreateInformation& createInformation, const GpuAllocation& allocation);
			~NullGpuBuffer();

			void SetName(const StringView& name) override { mName = name; }
			GpuQueueMask GetUseMask(GpuAccessFlags accessFlags) override { return GpuQueueMask::kNone; }
			u32 GetBoundCount() const override { return 0; }
			u32 GetUseCount() const override { return 0; }
			IGpuResource* GetGpuResource() const override { return mBuffer; }

#if B3D_BUILD_TYPE_DEVELOPMENT
			bool IsRangeBound(u32 offset, u32 size) const override { return false; }
			bool IsRangeInUse(u32 offset, u32 size) const override { return false; }
#endif

		protected:
			friend class NullGpuDevice;

			void RecreateInternalBuffer() override;
			void SwapInternalBuffer(GpuBuffer& other) override;

		private:
			/** Creates a native resource for the buffer. */
			NullBufferResource* CreateBufferResource();

			GpuAllocation mAllocation; /**< Memory the buffer is placed at. Freed on destruction if owned. */
			NullBufferResource* mBuffer = nullptr; /**< Native resource of the buffer. */
		};

		/** @} */
	} // namespace render
} // namespace b3d
