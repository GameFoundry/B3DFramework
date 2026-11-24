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
		MetalGpuQueue::MetalGpuQueue(GpuDevice& device, GpuQueueType type, u32 index, id<MTL4CommandQueue> commandQueue, id<MTLSharedEvent> sharedEvent)
			: GpuQueue(device, type, index), mCommandQueue(commandQueue), mSharedEvent(sharedEvent)
		{ }

		void MetalGpuQueue::Commit(const id<MTL4CommandBuffer>* commandBuffers, u32 count)
		{
			if (count == 0)
				return;

			MTL4CommitOptions* options = [[MTL4CommitOptions alloc] init];
			[options addFeedbackHandler:^(id<MTL4CommitFeedback> feedback)
			{
				NSError* error = feedback.error;
				if (error == nil)
					return;

#if B3D_BUILD_TYPE_DEVELOPMENT
				NSArray<id<MTLCommandBufferEncoderInfo>>* encoderInformationArray = [[error userInfo] objectForKey:MTLCommandBufferEncoderInfoErrorKey];
				for (id<MTLCommandBufferEncoderInfo> encoderInformation in encoderInformationArray)
				{
					B3D_LOG(Error, LogRenderBackend, "Metal encoder '{0}' failed with execution state {1}.",
						[encoderInformation label] ? String([[encoderInformation label] UTF8String]) : String("<unnamed>"),
						(u32)[encoderInformation errorState]);

					for (NSString* signpost in [encoderInformation debugSignposts])
						B3D_LOG(Error, LogRenderBackend, "  GPU signpost: {0}", String([signpost UTF8String]));
				}
#endif

				B3D_LOG(Fatal, LogRenderBackend, "Metal command buffer failed during GPU execution ({0}, code {1}): {2}",
					String([[error domain] UTF8String]), (i64)[error code], String([[error localizedDescription] UTF8String]));
			}];

			[mCommandQueue commit:commandBuffers count:count options:options];
		}

		void MetalGpuQueue::WaitForQueues(const GpuQueueFenceValues& waits)
		{
			mGpuDevice.DoForEachQueue([this, &waits](GpuQueue& queue)
			{
				const u64 waitValue = waits[queue.GetId().Id];
				if (waitValue != 0 && &queue != this)
					[mCommandQueue waitForEvent:static_cast<MetalGpuQueue&>(queue).GetSharedEvent() value:waitValue];
			});
		}

		void MetalGpuQueue::WaitForEvent(id<MTLSharedEvent> event, u64 value)
		{
			if (event != nil)
				[mCommandQueue waitForEvent:event value:value];
		}

		void MetalGpuQueue::SignalEvent(id<MTLSharedEvent> event, u64 value)
		{
			if (event != nil)
				[mCommandQueue signalEvent:event value:value];
		}

		void MetalGpuQueue::WaitForDrawable(id<CAMetalDrawable> drawable)
		{
			if (drawable == nil)
				return;

			mWaitedDrawables.erase(std::remove(mWaitedDrawables.begin(), mWaitedDrawables.end(), nil), mWaitedDrawables.end());
			if (std::find(mWaitedDrawables.begin(), mWaitedDrawables.end(), drawable) != mWaitedDrawables.end())
				return;

			[mCommandQueue waitForDrawable:drawable];
			mWaitedDrawables.push_back(drawable);
		}

		void MetalGpuQueue::SignalDrawable(id<CAMetalDrawable> drawable)
		{
			if (drawable == nil)
				return;

			// A drawable nothing rendered to still needs a wait before it may be released
			WaitForDrawable(drawable);
			[mCommandQueue signalDrawable:drawable];

			mWaitedDrawables.erase(std::remove(mWaitedDrawables.begin(), mWaitedDrawables.end(), drawable), mWaitedDrawables.end());
		}

		u64 MetalGpuQueue::SignalSubmission(Function<void()> onCompleted, const TShared<WaitGroup>& ownerCompletion)
		{
			const u64 signalValue = ++mLastReservedEventValue;

			{
				Lock lock(mCompletionMutex);
				mPendingCompletionCount++;
			}

			// The listener delivers notifications in order on a serial queue, so completions are reported in submission order
			MTLSharedEventListener* listener = static_cast<MetalGpuDevice&>(mGpuDevice).GetCompletionListener();
			[mSharedEvent notifyListener:listener atValue:signalValue block:^(id<MTLSharedEvent>, uint64_t)
			{
				if (onCompleted)
					onCompleted();

				{
					Lock lock(mCompletionMutex);
					mPendingCompletionCount--;
				}

				mCompletionCondition.notify_all();
			}];

			[mCommandQueue signalEvent:mSharedEvent value:signalValue];
			mLastCommittedEventValue.store(signalValue, std::memory_order_release);

			Lock lock(mSubmissionMutex);
			mActiveSubmissions.push_back({ signalValue, ownerCompletion });

			return signalValue;
		}

		void MetalGpuQueue::NotifySubmissionFailed(const TShared<WaitGroup>& ownerCompletion)
		{
			const u64 signalValue = ++mLastReservedEventValue;
			[mCommandQueue signalEvent:mSharedEvent value:signalValue];
			mLastCommittedEventValue.store(signalValue, std::memory_order_release);

			Lock lock(mSubmissionMutex);
			mActiveSubmissions.push_back({ signalValue, ownerCompletion });
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
			Lock lock(mCompletionMutex);
			mCompletionCondition.wait(lock, [this]() { return mPendingCompletionCount == 0; });
		}

		void MetalGpuQueue::RefreshCompletionState(bool forceWait, u64 lastEventValue)
		{
			AssertIfNotSubmitThread();

			// Completion is only observable once the owner-side cleanup has run, so only forced waits retire submissions
			if (!forceWait)
				return;

			TInlineArray<TShared<WaitGroup>, 16> ownerCompletions;
			{
				Lock lock(mSubmissionMutex);
				for (const SubmissionRecord& record : mActiveSubmissions)
				{
					if (record.EventValue > lastEventValue)
						break;

					if (record.OwnerCompletion != nullptr)
						ownerCompletions.Add(record.OwnerCompletion);
				}
			}

			for (const TShared<WaitGroup>& ownerCompletion : ownerCompletions)
				ownerCompletion->Wait();

			Lock lock(mSubmissionMutex);
			size_t retiredCount = 0;
			while (retiredCount < mActiveSubmissions.size() && mActiveSubmissions[retiredCount].EventValue <= lastEventValue)
				retiredCount++;

			mActiveSubmissions.erase(mActiveSubmissions.begin(), mActiveSubmissions.begin() + retiredCount);
		}

	} // namespace render
} // namespace b3d
