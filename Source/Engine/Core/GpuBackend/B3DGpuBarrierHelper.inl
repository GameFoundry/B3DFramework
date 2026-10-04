//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuBarrierHelper.h"
#include "GpuBackend/B3DGpuBackendUtility.h"

#include <algorithm>

namespace b3d::render
{
	template<class TDerived, class TResourceTracker>
	TGpuBarrierHelper<TDerived, TResourceTracker>::TGpuBarrierHelper(TResourceTracker* resourceTracker)
		: mResourceTracker(resourceTracker)
	{ }

	template<class TDerived, class TResourceTracker>
	void TGpuBarrierHelper<TDerived, TResourceTracker>::QueueResolvedBufferBarrier(IGpuBufferResource* buffer, const GpuBarrierScope& barrier, GpuBarrierFlags barrierFlags)
	{
		if(buffer == nullptr)
			return;

		static_cast<TDerived*>(this)->RecordNativeBufferBarrier(buffer, barrier, barrierFlags);

		BarrierTrackingInfo trackingInfo;
		trackingInfo.Buffer = buffer;
		trackingInfo.Barrier = barrier;
		mBarrierTracking.Add(trackingInfo);
	}

	template<class TDerived, class TResourceTracker>
	void TGpuBarrierHelper<TDerived, TResourceTracker>::QueueResolvedImageBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& subresourceRange, const GpuBarrierScope& barrier, GpuImageLayout oldLayout, GpuImageLayout newLayout, GpuBarrierFlags barrierFlags)
	{
		if(image == nullptr)
			return;

		static_cast<TDerived*>(this)->RecordNativeImageBarrier(image, subresourceRange, barrier, oldLayout, newLayout, barrierFlags);

		auto foundTracking = std::find_if(mImageLayoutTracking.begin(), mImageLayoutTracking.end(), [image, &subresourceRange](const LayoutTrackingInfo& layoutTrackingInfo)
		{
			return layoutTrackingInfo.Image == image && GpuBackendUtility::RangeEquals(layoutTrackingInfo.SubresourceRange, subresourceRange);
		});

		if(foundTracking != mImageLayoutTracking.end())
		{
			B3D_ASSERT(foundTracking->OldLayout == oldLayout);
			// Retain the latest destination even when it returns to the original layout.
			foundTracking->NewLayout = newLayout;
		}
		else if(oldLayout != newLayout)
		{
			LayoutTrackingInfo layoutTrackingInfo;
			layoutTrackingInfo.Image = image;
			layoutTrackingInfo.SubresourceRange = subresourceRange;
			layoutTrackingInfo.OldLayout = oldLayout;
			layoutTrackingInfo.NewLayout = newLayout;
			mImageLayoutTracking.Add(layoutTrackingInfo);
		}

		BarrierTrackingInfo barrierTrackingInfo;
		barrierTrackingInfo.Image = image;
		barrierTrackingInfo.ImageSubresourceRange = subresourceRange;
		barrierTrackingInfo.Barrier = barrier;
		mBarrierTracking.Add(barrierTrackingInfo);
	}

	template<class TDerived, class TResourceTracker>
	void TGpuBarrierHelper<TDerived, TResourceTracker>::ApplyPostBarrierTracking()
	{
		// Update layout for all image barriers
		for(const auto& trackingInfo : mImageLayoutTracking)
		{
			if(trackingInfo.Image == nullptr)
				continue;

			mResourceTracker->UpdateImageLayoutTrackingAfterBarrier(trackingInfo.Image, trackingInfo.SubresourceRange, trackingInfo.OldLayout, trackingInfo.NewLayout);
		}

		// Update hazard summaries for all barriers
		for(const auto& trackingInfo : mBarrierTracking)
		{
			if(trackingInfo.Buffer != nullptr)
				mResourceTracker->UpdateHazardStateAfterBarrier(trackingInfo.Buffer, trackingInfo.Barrier);
			else if(trackingInfo.Image != nullptr)
				mResourceTracker->UpdateHazardStateAfterBarrier(trackingInfo.Image, trackingInfo.ImageSubresourceRange, trackingInfo.Barrier);
		}
	}

	template<class TDerived, class TResourceTracker>
	void TGpuBarrierHelper<TDerived, TResourceTracker>::Clear()
	{
		mImageLayoutTracking.Clear();
		mBarrierTracking.Clear();
	}
}
