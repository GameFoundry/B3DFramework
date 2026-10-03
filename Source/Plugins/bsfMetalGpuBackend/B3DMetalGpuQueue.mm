//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuQueue.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuSubmitThread.h"
#include "GpuBackend/B3DRenderWindow.h"
#include "Debug/B3DLog.h"
#include "Profiling/B3DRenderStats.h"
#include "Threading/B3DThreading.h"

namespace b3d
{
	namespace render
	{
		namespace
		{
			/** Logs the error of @p commandBuffer, if it failed. */
			void LogCommandBufferError(id<MTLCommandBuffer> commandBuffer)
			{
				if ([commandBuffer status] != MTLCommandBufferStatusError)
					return;

				NSError* error = [commandBuffer error];
				B3D_LOG(Fatal, LogRenderBackend, "Metal queue synchronization command buffer failed ({0}, code {1}): {2}",
					error ? String([[error domain] UTF8String]) : String("<unknown domain>"),
					error ? (i64)[error code] : 0,
					error ? String([[error localizedDescription] UTF8String]) : String("No error details were provided."));
			}
		} // namespace

		MetalGpuQueue::MetalGpuQueue(GpuDevice& device, GpuQueueType type, u32 index, id<MTLCommandQueue> commandQueue, id<MTLSharedEvent> sharedEvent)
			: GpuQueue(device, type, index), mCommandQueue(commandQueue), mSharedEvent(sharedEvent)
		{ }

		void MetalGpuQueue::NotifySubmissionCommitted(u64 eventValue, id<MTLCommandBuffer> commandBuffer, const TShared<WaitGroup>& ownerCompletion)
		{
			u64 previousEventValue = mLastCommittedEventValue.load(std::memory_order_relaxed);
			while (eventValue > previousEventValue)
			{
				if (mLastCommittedEventValue.compare_exchange_weak(previousEventValue, eventValue, std::memory_order_release, std::memory_order_relaxed))
				{
					break;
				}
			}

			Lock lock(mSubmissionMutex);
			mActiveSubmissions.push_back({ eventValue, commandBuffer, ownerCompletion });
		}

		void MetalGpuQueue::NotifySubmissionFailed(const TShared<WaitGroup>& ownerCompletion)
		{
			Lock lock(mSubmissionMutex);
			mActiveSubmissions.push_back({ GetLastCommittedEventValue(), nil, ownerCompletion });
		}

		void MetalGpuQueue::SubmitCommandBuffer(const GpuSubmissionInformation& information)
		{
			if (!B3D_ENSURE(information.CommandBuffer))
				return;

			auto metalCommandBuffer = std::static_pointer_cast<MetalGpuCommandBuffer>(information.CommandBuffer);
			if (!B3D_ENSURE(metalCommandBuffer->GetQueueType() == mType))
				return;

			if (metalCommandBuffer->GetState() == GpuCommandBufferState::Executing)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot submit a command buffer that's still executing.");
				return;
			}

			if (!B3D_ENSURE(!metalCommandBuffer->IsInRenderPass()))
				metalCommandBuffer->EndRenderPass();

			if (metalCommandBuffer->IsRecording())
				metalCommandBuffer->End();

			// Set before the hand-off, so the owner thread never observes a submitted buffer as recording
			metalCommandBuffer->SetState(GpuCommandBufferState::Executing);

			mGpuDevice.GetSubmitThread().QueueSubmit(information.CommandBuffer, *this, information.SyncMask, information.SignalFences);
		}

		void MetalGpuQueue::WaitUntilIdle()
		{
			// The submit thread doesn't exist during device initialization and teardown
			auto& metalDevice = static_cast<MetalGpuDevice&>(mGpuDevice);
			if (!metalDevice.HasSubmitThread())
			{
				ExecuteWaitUntilIdle();
				return;
			}

			metalDevice.GetSubmitThread().WaitUntilIdle(*this);
		}

		void MetalGpuQueue::ExecuteWaitUntilIdle()
		{
			if (mCommandQueue == nil)
				return;

			// A failed command buffer may never signal its shared event, so wait on a trailing command buffer instead
			FenceCompletionHandlers();
		}

		void MetalGpuQueue::RefreshCompletionState(bool forceWait, u64 lastEventValue)
		{
			AssertIfNotSubmitThread();

			// Records are in ascending order, so the last command buffer up to lastEventValue covers all earlier ones
			id<MTLCommandBuffer> waitCommandBuffer = nil;
			TInlineArray<TShared<WaitGroup>, 16> ownerCompletions;
			{
				Lock lock(mSubmissionMutex);
				for (const SubmissionRecord& record : mActiveSubmissions)
				{
					if (record.EventValue > lastEventValue)
						break;

					if (record.CommandBuffer != nil)
						waitCommandBuffer = record.CommandBuffer;

					if (record.OwnerCompletion != nullptr)
						ownerCompletions.Add(record.OwnerCompletion);
				}
			}

			// A terminal command buffer status is observable before its completion handler returns, so only forced waits
			// retire submissions
			if (!forceWait)
				return;

			// Also waits for the completion handler, which posts the completion to the owner
			if (waitCommandBuffer != nil)
				[waitCommandBuffer waitUntilCompleted];

			for (const TShared<WaitGroup>& ownerCompletion : ownerCompletions)
				ownerCompletion->Wait();

			Lock lock(mSubmissionMutex);
			size_t retiredCount = 0;
			while (retiredCount < mActiveSubmissions.size() && mActiveSubmissions[retiredCount].EventValue <= lastEventValue)
				retiredCount++;

			mActiveSubmissions.erase(mActiveSubmissions.begin(), mActiveSubmissions.begin() + retiredCount);
		}

		void MetalGpuQueue::FenceCompletionHandlers()
		{
			id<MTLCommandBuffer> fenceCommandBuffer = [mCommandQueue commandBuffer];
			if (fenceCommandBuffer == nil)
				return;

			[fenceCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> completedCommandBuffer)
			{
				LogCommandBufferError(completedCommandBuffer);
			}];
			[fenceCommandBuffer commit];
			[fenceCommandBuffer waitUntilCompleted];
		}

		void MetalGpuQueue::PresentRenderWindow(const TShared<RenderWindow>& renderWindow, GpuQueueMask syncMask)
		{
			if (renderWindow == nullptr)
				return;

			IRenderWindowSurface* surface = renderWindow->GetRenderWindowSurface().get();
			if (surface == nullptr)
				return;

			renderWindow->NotifySwapBuffersRequested();
			surface->SwapBuffers(*this, syncMask);

			B3D_INCREMENT_RENDER_STATISTIC(NumPresents);
		}
	} // namespace render
} // namespace b3d
