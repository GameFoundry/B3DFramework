//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12Resource.h"
#include "B3DD3D12ResourceTracker.h"
#include "Utility/B3DD3D12BarrierHelper.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "Math/B3DArea2.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/**
		 * Layout compatibility release recorded in a separate ECL on the queue that previously owned an image. Access
		 * visibility is provided by the queue-fence signal and wait around this submission.
		 */
		struct D3D12SourceQueueTransition
		{
			GpuQueueId QueueId; /**< Queue that executes the release command buffer. */
			GpuQueueMask WaitMask = GpuQueueMask::kNone; /**< Queues whose work must precede the release. */
			TShared<D3D12GpuCommandBuffer> CommandBuffer; /**< Command buffer containing the release barriers. */
		};

		/** Command lists and resource-derived waits required for one D3D12 command-buffer submission. */
		struct D3D12GpuCommandBufferSubmitInformation
		{
			TInlineArray<D3D12SourceQueueTransition, 4> SourceQueueTransitions; /**< Releases executed on prior owner queues. */
			TShared<D3D12GpuCommandBuffer> TransitionCommandBuffer; /**< Prologue executed on the destination queue. */
			TShared<D3D12GpuCommandBuffer> PrimaryCommandBuffer; /**< User-recorded command buffer. */
			GpuQueueMask RequiredWaitMask = GpuQueueMask::kNone; /**< Additional queues the destination waits for. */
		};
