//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Image/BsColor.h"

namespace bs 
{
	/** @addtogroup Image
	 *  @{
	 */

	/** Single key in a ColorGradient. */
	struct ColorGradientKey
	{
		Color color;
		float time;
	};

	/** 
	 * Represents a range of color values over some parameters, similar to a curve. Internally represented as a set of
	 * keys that get interpolated between.
	 */
	class BS_UTILITY_EXPORT ColorGradient
	{
		static constexpr UINT32 MAX_KEYS = 8;
	public:
		/** Evaluates a color at the specified @p t. */
		RGBA evaluate(float t) const;

		/** Set of keys that control the gradient, sorted by time from first to last. */
		void setKeys(const Vector<ColorGradientKey>& keys);

		/** Specify a "gradient" that represents a single color value. */
		void setConstant(const Color& color);

	private:
		RGBA mColors[MAX_KEYS];
		uint16_t mTimes[MAX_KEYS];
		uint32_t mNumKeys = 0;
		float mDuration = 0.0f;
	};

	/* @} */
}
