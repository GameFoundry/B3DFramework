//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "GpuBackend/B3DGpuDevice.h"

namespace b3d
{
	namespace render
	{
		class NullGpuDevice;

		/** @addtogroup NullGpuBackend 
		 *  @{
		 */

		/** Null implementation of a GPU queue. */
		class NullGpuQueue : public GpuQueue
		{
		public:
			NullGpuQueue(GpuDevice& device, GpuQueueType type, u32 index);

			void SubmitCommandBuffer(const GpuSubmissionInformation& information, bool flushTransferCommandBuffer = true) override {}
			void WaitUntilIdle() override {}
			void PresentRenderWindow(const SPtr<RenderWindow>& renderWindow, GpuQueueMask syncMask = GpuQueueMask::kAll) override {}
		};

		/** @} */
	} // namespace render
} // namespace b3d
