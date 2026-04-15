//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "RenderAPI/B3DGpuBuffer.h"

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
			NullGpuBuffer(NullGpuDevice& device, const GpuBufferCreateInformation& createInformation);
			~NullGpuBuffer();

			void SetName(const StringView& name) override { mName = name; }
			GpuQueueMask GetUseMask(GpuAccessFlags accessFlags) override { return GpuQueueMask::kNone; }
			u32 GetBoundCount() const override { return 0; }
			u32 GetUseCount() const override { return 0; }

#if B3D_BUILD_TYPE_DEVELOPMENT
			bool IsRangeBound(u32 offset, u32 size) const override { return false; }
			bool IsRangeInUse(u32 offset, u32 size) const override { return false; }
#endif

		protected:
			friend class NullGpuDevice;

			void Initialize() override {}
			void RecreateInternalBuffer() override {}
		};

		/** @} */
	} // namespace render
} // namespace b3d
