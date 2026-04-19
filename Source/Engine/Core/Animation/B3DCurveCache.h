//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	/** @addtogroup Animation-Internal
	 *  @{
	 */

	/**
	 * Holds cached information used for animation curve evaluation so that sequential evaluations can be sped up.
	 * You should not use the same instance of this object for evaluating multiple different animation curves.
	 */
	template <class T>
	struct TCurveCache
	{
	private:
		friend class TAnimationCurve<T>;

		/** Left-most key the curve was last evaluated at. -1 if no cached data. */
		mutable u32 cachedKey = (u32)-1;

		/** Time relative to the animation curve, at which the cached data starts. */
		mutable float cachedCurveStart = std::numeric_limits<float>::infinity();

		/** Time relative to the animation curve, at which the cached data end. */
		mutable float cachedCurveEnd = 0.0f;

		/**
		 * Coefficients of the cubic hermite curve, in order [t^3, t^2, t, 1]. Coefficients assume unnormalized @p t, with
		 * length of @p cachedCurveEnd - @p cachedCurveStart.
		 */
		mutable T cachedCubicCoefficients[4]{};
	};

	/**
	 * Holds cached information used for integrated animation curve evaluation. Evaluations with the cache provided are
	 * significantly faster than non-cached evaluations.
	 */
	template <class T>
	struct TCurveIntegrationCache
	{
		~TCurveIntegrationCache()
		{
			B3DFree(segmentSums);
		}

	private:
		friend class TAnimationCurve<T>;

		/** Initializes the memory required for single integration cache. */
		void Init(u32 numKeys) const
		{
			segmentSums = (T*)B3DAllocate(sizeof(T) * numKeys * 5);
			coeffs = (T(*)[4])(segmentSums + numKeys);
		}

		/** Initializes the memory required for double integration cache. */
		void InitDouble(u32 numKeys) const
		{
			segmentSums = (T*)B3DAllocate(sizeof(T) * numKeys * 6);
			doubleSegmentSums = (T*)(segmentSums + numKeys);
			coeffs = (T(*)[4])(segmentSums + numKeys * 2);
		}

		/**
		 * Contains a list of cumulative areas beneath the curve of each segment. A segment represents the part of the curve
		 * between two keyframes.
		 */
		mutable T* segmentSums = nullptr;

		/**
		 * Contains a list of cumulative doubly integrated areas beneath the curve of each segment. A segment represents
		 * the part of the curve between two keyframes.
		 */
		mutable T* doubleSegmentSums = nullptr;

		/** Contains a list of integrated cubic coefficients for each keyframe. */
		mutable T (*coeffs)[4] = nullptr;
	};

	/** @} */
} // namespace b3d
