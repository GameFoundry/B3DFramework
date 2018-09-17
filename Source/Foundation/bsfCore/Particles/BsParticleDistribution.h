//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Image/BsColor.h"
#include "Image/BsColorGradient.h"
#include "Math/BsVector3.h"
#include "Math/BsRandom.h"
#include "Animation/BsAnimationCurve.h"
#include "Utility/BsBitwise.h"
#include "Utility/BsLookupTable.h"

namespace bs 
{
	/** @addtogroup Particles-Internal
	 *  @{
	 */

	/** Determines type of distribution used by distribution properties. */
	enum BS_SCRIPT_EXPORT(m:Particles) PropertyDistributionType
	{
		/** The distribution is a costant value. */
		PDT_Constant			BS_SCRIPT_EXPORT(n:Constant),
		/** The distribution is a random value in a specified constant range. */
		PDT_RandomRange			BS_SCRIPT_EXPORT(n:RandomRange),
		/** The distribution is a time-varying value. */
		PDT_Curve				BS_SCRIPT_EXPORT(n:Curve),
		/** The distribution is a random value in a specified time-varying range. */
		PDT_RandomCurveRange	BS_SCRIPT_EXPORT(n:RandomCurveRange)
	};

	/* @} */

	/** @addtogroup Particles
	 *  @{
	 */

	/** Specifies a color as a distribution, which can include a constant color, random color range or a color gradient. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Particles) ColorDistribution
	{
		/** Creates a new empty distribution. */
		BS_SCRIPT_EXPORT()
		ColorDistribution()
			: mType(PDT_Constant), mMinColor(Color::Black.getAsRGBA())
		{ }

		/** Creates a new distribution that returns a constant color. */
		BS_SCRIPT_EXPORT()
		ColorDistribution(const Color& color)
			: mType(PDT_Constant), mMinColor(color.getAsRGBA())
		{ }

		/** Creates a new distribution that returns a random color in the specified range. */
		BS_SCRIPT_EXPORT()
		ColorDistribution(const Color& minColor, const Color& maxColor)
			: mType(PDT_RandomRange), mMinColor(minColor.getAsRGBA()), mMaxColor(maxColor.getAsRGBA())
		{ }

		/** Creates a new distribution that evaluates a color gradient. */
		BS_SCRIPT_EXPORT()
		ColorDistribution(const ColorGradient& gradient)
			: mType(PDT_Curve), mMinGradient(gradient)
		{ }

		/** Creates a new distribution that returns a random color in a range determined by two gradients. */
		BS_SCRIPT_EXPORT()
		ColorDistribution(const ColorGradient& minGradient, const ColorGradient& maxGradient)
			: mType(PDT_RandomCurveRange), mMinGradient(minGradient), mMaxGradient(maxGradient)
		{ }

		/** Returns the type of the represented distribution. */
		BS_SCRIPT_EXPORT(pr:getter,n:DistributionType)
		PropertyDistributionType getType() const { return mType; }

		/** 
		 * Returns the constant value of the distribution, or the minimal value of a constant range. Undefined if 
		 * the distribution is represented by a gradient. 
		 */
		BS_SCRIPT_EXPORT()
		Color getMinConstant() const { return Color::fromRGBA(mMinColor); }

		/** 
		 * Returns the maximum value of a constant range. Only defined if the distribution represents a non-gradient range.
		 */
		BS_SCRIPT_EXPORT()
		Color getMaxConstant() const { return Color::fromRGBA(mMaxColor); }

		/** 
		 * Returns the gradient representing the distribution, or the first gradient representing a gradient range. 
		 * Undefined if the distribution is represented by a constant or a non-gradient range.
		 */
		BS_SCRIPT_EXPORT()
		const ColorGradient& getMinCurve() const { return mMinGradient; }

		/** 
		 * Returns the curve representing the second curve of a curve range. Only defined if the distribution represents
		 * a curve range.
		 */
		BS_SCRIPT_EXPORT()
		const ColorGradient& getMaxCurve() const { return mMaxGradient; }

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

		/** 
		 * Evaluates the value of the distribution.
		 * 
		 * @param[in]	t		Time at which to evaluate the distribution. This is only relevant if the distribution
		 *						contains gradients.
		 * @param[in]	factor	Random number generator that determines the factor. Factor determines how to interpolate
		 *						between min/max value, if the distribution represents a range.
		 * @return				Evaluated color.
		 *
		 */
		RGBA evaluate(float t, Random& factor) const
		{
			switch(mType)
			{
			default:
			case PDT_Constant:
				return mMinColor;
			case PDT_RandomRange:
			{
				const UINT32 byteFactor = Bitwise::unormToUint<8>(factor.getUNorm());
				return Color::lerp(byteFactor, mMinColor, mMaxColor);
			}
			case PDT_Curve:
				return mMinGradient.evaluate(t);
			case PDT_RandomCurveRange:
				{
					const RGBA minColor = mMinGradient.evaluate(t);
					const RGBA maxColor = mMaxGradient.evaluate(t);

					const UINT32 byteFactor = Bitwise::unormToUint<8>(factor.getUNorm());
					return Color::lerp(byteFactor, minColor, maxColor);
				}
			}
		}

		/**
		 * Converts the distribution into a lookup table that's faster to access. The distribution will be resampled
		 * using a fixed sample rate with equidistant samples.
		 * 
		 * @param[in]	numSamples			Determines how many samples to output in the lookup table. This value is ignored
		 *									for non-curve distributions in which case there is always just one sample.
		 * @param[in]	ignoreRange			If the curve represents a range (either between constants or curves), this
		 *									determines should the other value of the range be included in the lookup table.
		 *									If true, only the minimum constant/curve will be included, and if false then
		 *									the maximum curve values will follow the minimum curve values of each sample.
		 * @return							Resampled lookup table.
		 */
		LookupTable toLookupTable(UINT32 numSamples = 128, bool ignoreRange = false) const;
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
		/** Creates a new empty distribution. */
		BS_SCRIPT_EXPORT()
		TDistribution()
			: mType(PDT_Constant), mMinValue(T())
		{ }
		/** Creates a new distribution that returns a constant value. */
		BS_SCRIPT_EXPORT()
		TDistribution(T value)
			: mType(PDT_Constant), mMinValue(value)
		{ }

		/** Creates a new distribution that returns a random value in the specified range. */
		BS_SCRIPT_EXPORT()
		TDistribution(T minValue, T maxValue)
			: mType(PDT_RandomRange), mMinValue(minValue), mMaxValue(maxValue)
		{ }

		/** Creates a new distribution that evaluates a curve. */
		BS_SCRIPT_EXPORT()
		TDistribution(const TAnimationCurve<T>& curve)
			: mType(PDT_Curve), mMinCurve(curve)
		{ }

		/** Creates a new distribution that returns a random value in a range determined by two curves. */
		BS_SCRIPT_EXPORT()
		TDistribution(const TAnimationCurve<T>& minCurve, const TAnimationCurve<T>& maxCurve)
			: mType(PDT_RandomCurveRange), mMinCurve(minCurve), mMaxCurve(maxCurve)
		{ }

		/** Returns the type of the represented distribution. */
		BS_SCRIPT_EXPORT(pr:getter,n:DistributionType)
		PropertyDistributionType getType() const { return mType; }

		/** 
		 * Returns the constant value of the distribution, or the minimal value of a constant range. Undefined if 
		 * the distribution is represented by a curve. 
		 */
		BS_SCRIPT_EXPORT()
		const T& getMinConstant() const { return mMinValue; }

		/** 
		 * Returns the maximum value of a constant range. Only defined if the distribution represents a non-curve range.
		 */
		BS_SCRIPT_EXPORT()
		const T& getMaxConstant() const { return mMaxValue; }

		/** 
		 * Returns the curve representing the distribution, or the first curve representing a curve range. Undefined if
		 * the distribution is represented by a constant or a non-curve range.
		 */
		BS_SCRIPT_EXPORT()
		const TAnimationCurve<T>& getMinCurve() const { return mMinCurve; }

		/** 
		 * Returns the curve representing the second curve of a curve range. Only defined if the distribution represents
		 * a curve range.
		 */
		BS_SCRIPT_EXPORT()
		const TAnimationCurve<T>& getMaxCurve() const { return mMaxCurve; }

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
		BS_SCRIPT_EXPORT()
		T evaluate(float t, float factor) const
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
					const T minValue = mMinCurve.evaluate(t);
					const T maxValue = mMaxCurve.evaluate(t);

					return Math::lerp(factor, minValue, maxValue);
				}
			}
		}

		/** 
		 * Evaluates the value of the distribution.
		 * 
		 * @param[in]	t		Time at which to evaluate the distribution. This is only relevant if the distribution
		 *						contains curves.
		 * @param[in]	factor	Random number generator that determines the factor. Factor determines how to interpolate
		 *						between min/max value, if the distribution represents a range.
		 * @return				Evaluated value.
		 *
		 */
		BS_SCRIPT_EXPORT()
		T evaluate(float t, const Random& factor) const
		{
			switch(mType)
			{
			default:
			case PDT_Constant:
				return mMinValue;
			case PDT_RandomRange:
				return Math::lerp(factor.getUNorm(), mMinValue, mMaxValue);
			case PDT_Curve:
				return mMinCurve.evaluate(t);
			case PDT_RandomCurveRange:
				{
					const T minValue = mMinCurve.evaluate(t);
					const T maxValue = mMaxCurve.evaluate(t);

					return Math::lerp(factor.getUNorm(), minValue, maxValue);
				}
			}
		}

		/**
		 * Converts the distribution into a lookup table that's faster to access. The distribution will be resampled
		 * using a fixed sample rate with equidistant samples.
		 * 
		 * @param[in]	numSamples			Determines how many samples to output in the lookup table. This value is ignored
		 *									for non-curve distributions in which case there is always just one sample.
		 * @param[in]	ignoreRange			If the curve represents a range (either between constants or curves), this
		 *									determines should the other value of the range be included in the lookup table.
		 *									If true, only the minimum constant/curve will be included, and if false then
		 *									the maximum curve values will follow the minimum curve values of each sample.
		 * @return							Resampled lookup table.
		 */
		LookupTable toLookupTable(UINT32 numSamples = 128, bool ignoreRange = false) const;
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
	using Vector2Distribution = TDistribution<Vector2>;

#ifdef BS_SBGEN
	template struct BS_SCRIPT_EXPORT(m:Particles,n:FloatDistribution) TDistribution<float>;
	template struct BS_SCRIPT_EXPORT(m:Particles,n:Vector3Distribution) TDistribution<Vector3>;
	template struct BS_SCRIPT_EXPORT(m:Particles,n:Vector2Distribution) TDistribution<Vector2>;
#endif

	/** @} */
}