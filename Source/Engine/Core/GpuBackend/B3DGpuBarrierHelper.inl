//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuBarrierHelper.h"
#include "GpuBackend/B3DGpuBackendUtility.h"

#include <algorithm>

namespace b3d::render
{
	template<class TDerived>
	TGpuBarrierHelper<TDerived>::TGpuBarrierHelper(TGpuResourceTracker<TDerived>* resourceTracker)
		: mResourceTracker(resourceTracker)
	{ }

	template<class TDerived>
	const typename TGpuBarrierHelper<TDerived>::BarrierTrackingInfo* TGpuBarrierHelper<TDerived>::AddBufferBarrier(IGpuBufferResource* buffer, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccess, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess)
	{
		if(buffer == nullptr)
			return nullptr;

		const GpuStageFlags sourceAccessStageFlags = GpuBackendUtility::GetStageFlags(sourceUsage);
		const GpuStageFlags destinationAccessStageFlags = GpuBackendUtility::GetStageFlags(destinationUsage);

		return AddBufferBarrier(buffer, GpuHazardStageAndAccess(sourceAccessStageFlags, sourceAccess, destinationAccessStageFlags, destinationAccess));
	}

	template<class TDerived>
	const typename TGpuBarrierHelper<TDerived>::BarrierTrackingInfo* TGpuBarrierHelper<TDerived>::AddBufferBarrier(IGpuBufferResource* buffer, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess)
	{
		if(buffer == nullptr)
			return nullptr;

		const GpuBufferTrackingState* bufferTrackingState = mResourceTracker->FindBufferTrackingState(buffer);
		if(bufferTrackingState == nullptr)
			return nullptr;

		return AddBufferBarrier(buffer, *bufferTrackingState, destinationUsage, destinationAccess);
	}

	template<class TDerived>
	const typename TGpuBarrierHelper<TDerived>::BarrierTrackingInfo* TGpuBarrierHelper<TDerived>::AddBufferBarrier(IGpuBufferResource* buffer, const GpuBufferTrackingState& bufferTrackingState, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess)
	{
		if(buffer == nullptr)
			return nullptr;

		const GpuStageFlags destinationAccessStageFlags = GpuBackendUtility::GetStageFlags(destinationUsage);

		GpuStageFlags sourceAccessStageFlags;
		GpuAccessFlags sourceAccessFlags;

		// WAW or RAW hazard
		const GpuStageFlags writeAccessStageFlags = bufferTrackingState.WriteHazardTracking->State.MemoryBarrierTracking.GetUnsafeAccessStages(destinationAccessStageFlags);
		if(destinationAccess.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
		{
			sourceAccessStageFlags |= writeAccessStageFlags;

			if(writeAccessStageFlags != GpuStageFlag::None)
				sourceAccessFlags |= GpuAccessFlag::Write;
		}

		// WAR hazard
		const GpuStageFlags readAccessStageFlags = bufferTrackingState.WriteHazardTracking->State.ExecutionBarrierTracking.GetUnsafeAccessStages(destinationAccessStageFlags);
		if(destinationAccess.IsSet(GpuAccessFlag::Write))
		{
			sourceAccessStageFlags |= readAccessStageFlags;

			if(readAccessStageFlags != GpuStageFlag::None)
				sourceAccessFlags |= GpuAccessFlag::Read;
		}

		if(sourceAccessFlags == GpuAccessFlag::None)
			return nullptr;

		return AddBufferBarrier(buffer, GpuHazardStageAndAccess(sourceAccessStageFlags, sourceAccessFlags, destinationAccessStageFlags, destinationAccess));
	}

	template<class TDerived>
	const typename TGpuBarrierHelper<TDerived>::BarrierTrackingInfo* TGpuBarrierHelper<TDerived>::AddBufferBarrier(IGpuBufferResource* buffer, const GpuHazardStageAndAccess& stageAndAccess)
	{
		if(buffer == nullptr)
			return nullptr;

		static_cast<TDerived*>(this)->RecordBufferBarrier(buffer, stageAndAccess);

		BarrierTrackingInfo trackingInfo;
		trackingInfo.Buffer = buffer;
		trackingInfo.StageAndAccess = stageAndAccess;
		mBarrierTracking.Add(trackingInfo);

		return &mBarrierTracking.back();
	}

	template<class TDerived>
	const typename TGpuBarrierHelper<TDerived>::BarrierTrackingInfo* TGpuBarrierHelper<TDerived>::AddImageBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuResourceUseFlags sourceUsage, GpuAccessFlags sourceAccessFlags, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccessFlags, GpuImageLayout oldLayout, GpuImageLayout newLayout)
	{
		const GpuStageFlags sourceAccessStageFlags = GpuBackendUtility::GetStageFlags(sourceUsage);
		const GpuStageFlags destinationAccessStageFlags = GpuBackendUtility::GetStageFlags(destinationUsage);

		return AddSubresourceBarrier(image, subresourceRange, GpuHazardStageAndAccess(sourceAccessStageFlags, sourceAccessFlags, destinationAccessStageFlags, destinationAccessFlags), oldLayout, newLayout);
	}

	template<class TDerived>
	const typename TGpuBarrierHelper<TDerived>::BarrierTrackingInfo* TGpuBarrierHelper<TDerived>::AddImageBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, GpuImageLayout newLayout)
	{
		if(image == nullptr)
			return nullptr;

		const GpuImageTrackingState* imageTrackingState = mResourceTracker->FindImageTrackingState(image);
		if(imageTrackingState == nullptr)
			return nullptr;

		// The provided range may straddle several tracked subresource blocks; subdivide so each fully-overlapping block
		// is barriered with its own current layout.
		struct CallbackParameters
		{
			TGpuBarrierHelper* BarrierHelper;
			TGpuResourceTracker<TDerived>* ResourceTracker;
			IGpuImageResource* Image;
			GpuResourceUseFlags DestinationUsage;
			GpuAccessFlags DestinationAccess;
			GpuImageLayout NewLayout;
			const BarrierTrackingInfo* OutTrackingInfo;
		};

		CallbackParameters callbackParameters { this, mResourceTracker, image, destinationUsage, destinationAccess, newLayout, nullptr };
		mResourceTracker->IterateAndCreateOverlappingImageSubresourceTrackingState(image, subresourceRange, [](u32 globalSubresourceIndex, void* userData)
		{
			CallbackParameters* const callbackParameters = static_cast<CallbackParameters*>(userData);

			TGpuResourceTracker<TDerived>& resourceTracker = *callbackParameters->ResourceTracker;
			const GpuImageSubresourceTrackingState& subresourceTrackingState = resourceTracker.GetSubresourceTrackingStateAtIndex(globalSubresourceIndex);

			TGpuBarrierHelper& barrierHelper = *callbackParameters->BarrierHelper;
			callbackParameters->OutTrackingInfo = barrierHelper.AddSubresourceBarrier(callbackParameters->Image, subresourceTrackingState, callbackParameters->DestinationUsage, callbackParameters->DestinationAccess, callbackParameters->NewLayout);
		}, &callbackParameters);

		return callbackParameters.OutTrackingInfo;
	}

	template<class TDerived>
	const typename TGpuBarrierHelper<TDerived>::BarrierTrackingInfo* TGpuBarrierHelper<TDerived>::AddSubresourceBarrier(IGpuImageResource* image, const GpuImageSubresourceTrackingState& subresourceTrackingState, GpuResourceUseFlags destinationUsage, GpuAccessFlags destinationAccess, GpuImageLayout newLayout)
	{
		if(image == nullptr)
			return nullptr;

		const GpuStageFlags destinationAccessStageFlags = GpuBackendUtility::GetStageFlags(destinationUsage);

		GpuStageFlags sourceAccessStageFlags;
		GpuAccessFlags sourceAccessFlags;

		// WAW or RAW hazard
		const GpuStageFlags writeAccessStageFlags = subresourceTrackingState.WriteHazardTracking->State.MemoryBarrierTracking.GetUnsafeAccessStages(destinationAccessStageFlags);
		if(destinationAccess.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
		{
			sourceAccessStageFlags |= writeAccessStageFlags;

			if(writeAccessStageFlags != GpuStageFlag::None)
				sourceAccessFlags |= GpuAccessFlag::Write;
		}

		// WAR hazard
		const GpuStageFlags readAccessStageFlags = subresourceTrackingState.WriteHazardTracking->State.ExecutionBarrierTracking.GetUnsafeAccessStages(destinationAccessStageFlags);
		if(destinationAccess.IsSet(GpuAccessFlag::Write))
		{
			sourceAccessStageFlags |= readAccessStageFlags;

			if(readAccessStageFlags != GpuStageFlag::None)
				sourceAccessFlags |= GpuAccessFlag::Read;
		}

		// No layout transition if destination layout is undefined
		if(newLayout == GpuImageLayout::Undefined)
			newLayout = subresourceTrackingState.CurrentLayout;

		if(sourceAccessFlags == GpuAccessFlag::None && subresourceTrackingState.CurrentLayout == newLayout)
			return nullptr;

		return AddSubresourceBarrier(image, subresourceTrackingState.Range, GpuHazardStageAndAccess(sourceAccessStageFlags, sourceAccessFlags, destinationAccessStageFlags, destinationAccess), subresourceTrackingState.CurrentLayout, newLayout);
	}

	template<class TDerived>
	const typename TGpuBarrierHelper<TDerived>::BarrierTrackingInfo* TGpuBarrierHelper<TDerived>::AddSubresourceBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, const GpuHazardStageAndAccess& stageAndAccess, GpuImageLayout oldLayout, GpuImageLayout newLayout)
	{
		if(image == nullptr)
			return nullptr;

		// Accumulate the native barrier. The backend may reconcile oldLayout from an already-merged barrier (e.g. Vulkan),
		// in which case the layout-tracking bookkeeping below must observe the reconciled value.
		static_cast<TDerived*>(this)->RecordSubresourceBarrier(image, subresourceRange, stageAndAccess, oldLayout, newLayout);

		if(oldLayout != newLayout)
		{
			auto foundTracking = std::find_if(mImageLayoutTracking.begin(), mImageLayoutTracking.end(), [image, &subresourceRange](const LayoutTrackingInfo& layoutTrackingInfo)
			{
				return layoutTrackingInfo.Image == image && GpuBackendUtility::RangeEquals(layoutTrackingInfo.SubresourceRange, subresourceRange);
			});

			if(foundTracking == mImageLayoutTracking.end())
			{
				LayoutTrackingInfo layoutTrackingInfo;
				layoutTrackingInfo.Image = image;
				layoutTrackingInfo.SubresourceRange = subresourceRange;
				layoutTrackingInfo.OldLayout = oldLayout;
				layoutTrackingInfo.NewLayout = newLayout;
				mImageLayoutTracking.Add(layoutTrackingInfo);
			}
			else
			{
				B3D_ASSERT(foundTracking->OldLayout == oldLayout);
				foundTracking->NewLayout = newLayout;
			}
		}

		BarrierTrackingInfo barrierTrackingInfo;
		barrierTrackingInfo.Image = image;
		barrierTrackingInfo.ImageSubresourceRange = subresourceRange;
		barrierTrackingInfo.StageAndAccess = stageAndAccess;
		mBarrierTracking.Add(barrierTrackingInfo);

		return &mBarrierTracking.back();
	}

	template<class TDerived>
	void TGpuBarrierHelper<TDerived>::ApplyPostBarrierTracking()
	{
		// Update layout for all image barriers
		for(const auto& trackingInfo : mImageLayoutTracking)
		{
			if(trackingInfo.Image == nullptr)
				continue;

			mResourceTracker->UpdateImageLayoutTrackingAfterBarrier(trackingInfo.Image, trackingInfo.SubresourceRange, trackingInfo.OldLayout, trackingInfo.NewLayout);
		}

		// Update hazard tracking for all barriers
		for(const auto& trackingInfo : mBarrierTracking)
		{
			if(trackingInfo.Buffer != nullptr)
				mResourceTracker->UpdateWriteHazardTrackingAfterBarrier(trackingInfo.Buffer, trackingInfo.StageAndAccess);
			else if(trackingInfo.Image != nullptr)
				mResourceTracker->UpdateWriteHazardTrackingAfterBarrier( trackingInfo.Image, trackingInfo.ImageSubresourceRange, trackingInfo.StageAndAccess);
		}
	}

	template<class TDerived>
	void TGpuBarrierHelper<TDerived>::Clear()
	{
		mImageLayoutTracking.Clear();
		mBarrierTracking.Clear();
	}
}
