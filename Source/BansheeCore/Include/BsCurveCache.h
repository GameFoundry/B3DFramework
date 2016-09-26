//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace BansheeEngine
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
	public:
		TCurveCache()
			: cachedKey((UINT32)-1), cachedCurveStart(std::numeric_limits<float>::infinity()), cachedCurveEnd(0.0f)
			, cachedCubicCoefficients()
		{ }

	private:
		friend class TAnimationCurve<T>;

		mutable UINT32 cachedKey; /**< Left-most key the curve was last evaluated at. -1 if no cached data. */
		mutable float cachedCurveStart; /**< Time relative to the animation curve, at which the cached data starts. */
		mutable float cachedCurveEnd; /**< Time relative to the animation curve, at which the cached data end. */
		/** 
		 * Coefficients of the cubic hermite curve, in order [t^3, t^2, t, 1]. Coefficients assume unnormalized @p t, with
		 * length of @p cachedCurveEnd - @p cachedCurveStart.
		 */
		mutable T cachedCubicCoefficients[4]; 
	};

	/** @} */
}