//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Image/BsColor.h"
#include "Image/BsColorGradient.h"
#include "Math/BsVector3.h"
#include "Animation/BsAnimationCurve.h"
#include "Utility/BsBitwise.h"

namespace bs 
{
	/** @addtogroup Particles-Internal
	 *  @{
	 */

	/** Determines type of distribution used by distribution properties. */
	enum PropertyDistributionType
	{
		/** The distribution is a costant value. */
		PDT_Constant,
		/** The distribution is a random value in a specified constant range. */
		PDT_RandomRange,
		/** The distribution is a time-varying value. */
		PDT_Curve,
		/** The distribution is a random value in a specified time-varying range. */
		PDT_RandomCurveRange
	};

	/* @} */

	/** @addtogroup Particles
	 *  @{
	 */

	/** Specifies a color as a distribution, which can include a constant color, random color range or a color gradient. */
	struct ColorDistribution
	{
		/** Creates a new distribution that returns a constant color. */
		ColorDistribution(const Color& color = Color::Black)
			: mType(PDT_Constant), mMinColor(color.getAsRGBA())
		{ }

		/** Creates a new distribution that returns a random color in the specified range. */
		ColorDistribution(const Color& minColor, const Color& maxColor)
			: mType(PDT_RandomRange), mMinColor(minColor.getAsRGBA()), mMaxColor(maxColor.getAsRGBA())
		{ }

		/** Creates a new distribution that evaluates a color gradient. */
		ColorDistribution(const ColorGradient& gradient)
			: mType(PDT_Curve), mMinGradient(gradient)
		{ }

		/** Creates a new distribution that returns a random color in a range determined by two gradients. */
		ColorDistribution(const ColorGradient& minGradient, const ColorGradient& maxGradient)
			: mType(PDT_RandomCurveRange), mMinGradient(minGradient), mMaxGradient(maxGradient)
		{ }

		/** 
		 * Evaluates the value of the distribution.
		 * 
		 * @param[in]	t		Time at which to evaluate the distribution. This is only relevant if the distribution
		 *						contains gradients.
		 * @param[in]	factor	Value in range [0, 1] that determines how to interpolate between min/max value, if the
		 *						distribution represents a range. Value of 0 will return the minimum value, while value of 1
		 *						will return the maximum value, and interpolate the values in-between.
		 * @return				Evaluated color.
		 *
		 */
		RGBA evaluate(float t, float factor) const
		{
			const UINT32 byteFactor = Bitwise::unormToUint<8>(factor);
			switch(mType)
			{
			default:
			case PDT_Constant:
				return mMinColor;
			case PDT_RandomRange:
				return Color::lerp(byteFactor, mMinColor, mMaxColor);
			case PDT_Curve:
				return mMinGradient.evaluate(t);
			case PDT_RandomCurveRange:
				{
					const RGBA minColor = mMinGradient.evaluate(t);
					const RGBA maxColor = mMaxGradient.evaluate(t);

					return Color::lerp(byteFactor, minColor, maxColor);
				}
			}
		}

	private:
		friend struct RTTIPlainType<ColorDistribution>;

		PropertyDistributionType mType;
		RGBA mMinColor;
		RGBA mMaxColor;
		ColorGradient mMinGradient;
		ColorGradient mMaxGradient;
	};

	/** Specifies a value as a distribution, which can include a constant value, random range or a curve. */
	template<class T>
	struct TDistribution
	{
		/** Creates a new distribution that returns a constant value. */
		TDistribution(T value = T())
			: mType(PDT_Constant), mMinValue(value)
		{ }

		/** Creates a new distribution that returns a random value in the specified range. */
		TDistribution(T minValue, T maxValue)
			: mType(PDT_RandomRange), mMinValue(minValue), mMaxValue(maxValue)
		{ }

		/** Creates a new distribution that evaluates a curve. */
		TDistribution(const TAnimationCurve<T>& curve)
			: mType(PDT_Curve), mMinCurve(curve)
		{ }

		/** Creates a new distribution that returns a random value in a range determined by two curves. */
		TDistribution(const TAnimationCurve<T>& minCurve, const TAnimationCurve<T>& maxCurve)
			: mType(PDT_RandomCurveRange), mMinCurve(minCurve), mMaxCurve(maxCurve)
		{ }

		/** 
		 * Evaluates the value of the distribution.
		 * 
		 * @param[in]	t		Time at which to evaluate the distribution. This is only relevant if the distribution
		 *						contains curves.
		 * @param[in]	factor	Value in range [0, 1] that determines how to interpolate between min/max value, if the
		 *						distribution represents a range. Value of 0 will return the minimum value, while value of 1
		 *						will return the maximum value, and interpolate the values in-between.
		 * @return				Evaluated value.
		 *
		 */
		T evaluate(T t, float factor) const
		{
			switch(mType)
			{
			default:
			case PDT_Constant:
				return mMinValue;
			case PDT_RandomRange:
				return Math::lerp(factor, mMinValue, mMaxValue);
			case PDT_Curve:
				return mMinCurve.evaluate(t);
			case PDT_RandomCurveRange:
				{
					const float minValue = mMinCurve.evaluate(t);
					const float maxValue = mMaxCurve.evaluate(t);

					return Math::lerp(factor, minValue, maxValue);
				}
			}
		}
	private:
		friend struct RTTIPlainType<TDistribution<T>>;

		PropertyDistributionType mType;
		T mMinValue;
		T mMaxValue;
		TAnimationCurve<T> mMinCurve;
		TAnimationCurve<T> mMaxCurve;
	};

	using FloatDistribution = TDistribution<float>;
	using Vector3Distribution = TDistribution<Vector3>;

	/** @} */
}