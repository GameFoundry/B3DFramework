//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DColor.h"
#include "Image/B3DColorGradient.h"
#include "Math/B3DVector3.h"
#include "Math/B3DRandom.h"
#include "Animation/B3DAnimationCurve.h"
#include "Animation/B3DAnimationUtility.h"
#include "Utility/B3DBitwise.h"
#include "Utility/B3DLookupTable.h"

namespace b3d
{
	/** @addtogroup Particles-Internal
	 *  @{
	 */

	/** Determines type of distribution used by distribution properties. */
	enum B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) PropertyDistributionType
	{
		/** The distribution is a costant value. */
		PDT_Constant B3D_SCRIPT_EXPORT(ExportName(Constant)),
		/** The distribution is a random value in a specified constant range. */
		PDT_RandomRange B3D_SCRIPT_EXPORT(ExportName(RandomRange)),
		/** The distribution is a time-varying value. */
		PDT_Curve B3D_SCRIPT_EXPORT(ExportName(Curve)),
		/** The distribution is a random value in a specified time-varying range. */
		PDT_RandomCurveRange B3D_SCRIPT_EXPORT(ExportName(RandomCurveRange))
	};

	/* @} */

	/** @addtogroup Particles
	 *  @{
	 */

	/** Specifies a color as a distribution, which can include a constant color, random color range or a color gradient. */
	template <class T>
	struct TColorDistribution : public IScriptExportable
	{
		/** Creates a new empty distribution. */
		B3D_SCRIPT_EXPORT()

		TColorDistribution()
			: mType(PDT_Constant)
			, mMinGradient({ ColorGradientKey(Color::kBlack, 0.0f) })
			, mMaxGradient({ ColorGradientKey(Color::kBlack, 0.0f) })
		{}

		/** Creates a new distribution that returns a constant color. */
		B3D_SCRIPT_EXPORT()

		TColorDistribution(const Color& color)
			: mType(PDT_Constant)
			, mMinGradient({ ColorGradientKey(color, 0.0f) })
			, mMaxGradient({ ColorGradientKey(color, 0.0f) })
		{}

		/** Creates a new distribution that returns a random color in the specified range. */
		B3D_SCRIPT_EXPORT()

		TColorDistribution(const Color& minColor, const Color& maxColor)
			: mType(PDT_RandomRange)
			, mMinGradient({ ColorGradientKey(minColor, 0.0f) })
			, mMaxGradient({ ColorGradientKey(maxColor, 0.0f) })
		{}

		/** Creates a new distribution that evaluates a color gradient. */
		B3D_SCRIPT_EXPORT()

		TColorDistribution(const T& gradient)
			: mType(PDT_Curve), mMinGradient(gradient), mMaxGradient(gradient)
		{
			if(mMinGradient.GetNumKeys() == 0)
				mMinGradient = T({ ColorGradientKey(Color::kBlack, 0.0f) });

			if(mMaxGradient.GetNumKeys() == 0)
				mMaxGradient = T({ ColorGradientKey(Color::kBlack, 0.0f) });
		}

		/** Creates a new distribution that returns a random color in a range determined by two gradients. */
		B3D_SCRIPT_EXPORT()

		TColorDistribution(const T& minGradient, const T& maxGradient)
			: mType(PDT_RandomCurveRange), mMinGradient(minGradient), mMaxGradient(maxGradient)
		{
			if(mMinGradient.GetNumKeys() == 0)
				mMinGradient = T({ ColorGradientKey(Color::kBlack, 0.0f) });

			if(mMaxGradient.GetNumKeys() == 0)
				mMaxGradient = T({ ColorGradientKey(Color::kBlack, 0.0f) });
		}

		/** Returns the type of the represented distribution. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(DistributionType))

		PropertyDistributionType GetType() const { return mType; }

		/**
		 * Returns the constant value of the distribution, or the minimal value of a constant range. Undefined if
		 * the distribution is represented by a gradient.
		 */
		B3D_SCRIPT_EXPORT()

		Color GetMinConstant() const { return mMinGradient.GetKey(0).Color; }

		/**
		 * Returns the maximum value of a constant range. Only defined if the distribution represents a non-gradient range.
		 */
		B3D_SCRIPT_EXPORT()

		Color GetMaxConstant() const { return mMaxGradient.GetKey(0).Color; }

		/**
		 * Returns the gradient representing the distribution, or the first gradient representing a gradient range.
		 * Undefined if the distribution is represented by a constant or a non-gradient range.
		 */
		B3D_SCRIPT_EXPORT()

		const T& GetMinGradient() const { return mMinGradient; }

		/**
		 * Returns the curve representing the second gradient of a gradient range. Only defined if the distribution
		 * represents a gradient range.
		 */
		B3D_SCRIPT_EXPORT()

		const T& GetMaxGradient() const { return mMaxGradient; }

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
		typename T::ColorType Evaluate(float t, float factor) const
		{
			const auto lerpFactor = TGradientHelper<typename T::ColorType>::ToLerpFactor(factor);
			switch(mType)
			{
			default:
			case PDT_Constant:
				return mMinGradient.Evaluate(0.0f);
			case PDT_RandomRange:
				{
					const auto minColor = mMinGradient.Evaluate(0.0f);
					const auto maxColor = mMaxGradient.Evaluate(0.0f);

					return Color::Lerp(lerpFactor, minColor, maxColor);
				}
			case PDT_Curve:
				return mMinGradient.Evaluate(t);
			case PDT_RandomCurveRange:
				{
					const auto minColor = mMinGradient.Evaluate(t);
					const auto maxColor = mMaxGradient.Evaluate(t);

					return Color::Lerp(lerpFactor, minColor, maxColor);
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
		typename T::ColorType Evaluate(float t, const Random& factor) const
		{
			switch(mType)
			{
			default:
			case PDT_Constant:
				return mMinGradient.Evaluate(0.0f);
			case PDT_RandomRange:
				{
					const auto minColor = mMinGradient.Evaluate(0.0f);
					const auto maxColor = mMaxGradient.Evaluate(0.0f);

					const auto lerpFactor = TGradientHelper<typename T::ColorType>::ToLerpFactor(factor.GetUNorm());
					return Color::Lerp(lerpFactor, minColor, maxColor);
				}
			case PDT_Curve:
				return mMinGradient.Evaluate(t);
			case PDT_RandomCurveRange:
				{
					const auto minColor = mMinGradient.Evaluate(t);
					const auto maxColor = mMaxGradient.Evaluate(t);

					const auto lerpFactor = TGradientHelper<typename T::ColorType>::ToLerpFactor(factor.GetUNorm());
					return Color::Lerp(lerpFactor, minColor, maxColor);
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
		LookupTable ToLookupTable(u32 numSamples = 128, bool ignoreRange = false) const;

		bool operator==(const TColorDistribution<T>& rhs) const
		{
			if(mType != rhs.mType)
				return false;

			if(mType == PDT_Constant || mType == PDT_Curve)
				return mMinGradient == rhs.mMinGradient;
			else
				return mMinGradient == rhs.mMinGradient && mMaxGradient == rhs.mMaxGradient;
		}

		bool operator!=(const TColorDistribution<T>& rhs) const { return !operator==(rhs); }

	private:
		friend struct RTTIPlainType<TColorDistribution<T>>;

		PropertyDistributionType mType;
		T mMinGradient;
		T mMaxGradient;
	};

	using ColorDistribution = TColorDistribution<ColorGradient>;
	using ColorHDRDistribution = TColorDistribution<ColorGradientHDR>;

#ifdef B3D_CODEGEN 
	template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportName(ColorDistribution)) TColorDistribution<ColorGradient>;
	template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportName(ColorHDRDistribution)) TColorDistribution<ColorGradientHDR>;
#endif

	/** Specifies a value as a distribution, which can include a constant value, random range or a curve. */
	template <class T>
	struct TDistribution : public IScriptExportable
	{
		/** Creates a new empty distribution. */
		B3D_SCRIPT_EXPORT()

		TDistribution()
			: mType(PDT_Constant)
			, mMinCurve({ TKeyframe<T>{ T(), TCurveProperties<T>::GetZero(), TCurveProperties<T>::GetZero(), 0.0f } })
			, mMaxCurve({ TKeyframe<T>{ T(), TCurveProperties<T>::GetZero(), TCurveProperties<T>::GetZero(), 0.0f } })
		{}
		/** Creates a new distribution that returns a constant value. */
		B3D_SCRIPT_EXPORT()

		TDistribution(T value)
			: mType(PDT_Constant)
			, mMinCurve({ TKeyframe<T>{ value, TCurveProperties<T>::GetZero(), TCurveProperties<T>::GetZero(), 0.0f } })
			, mMaxCurve({ TKeyframe<T>{ value, TCurveProperties<T>::GetZero(), TCurveProperties<T>::GetZero(), 0.0f } })
		{}

		/** Creates a new distribution that returns a random value in the specified range. */
		B3D_SCRIPT_EXPORT()

		TDistribution(T minValue, T maxValue)
			: mType(PDT_RandomRange)
			, mMinCurve({ TKeyframe<T>{ minValue, TCurveProperties<T>::GetZero(), TCurveProperties<T>::GetZero(), 0.0f } })
			, mMaxCurve({ TKeyframe<T>{ maxValue, TCurveProperties<T>::GetZero(), TCurveProperties<T>::GetZero(), 0.0f } })
		{}

		/** Creates a new distribution that evaluates a curve. */
		B3D_SCRIPT_EXPORT()

		TDistribution(const TAnimationCurve<T>& curve)
			: mType(PDT_Curve), mMinCurve(curve), mMaxCurve(curve)
		{
			if(mMinCurve.GetKeyFrames().empty())
				mMinCurve = TAnimationCurve<T>({ TKeyframe<T>{ T(), TCurveProperties<T>::GetZero(), TCurveProperties<T>::GetZero(), 0.0f } });

			if(mMaxCurve.GetKeyFrames().empty())
				mMaxCurve = TAnimationCurve<T>({ TKeyframe<T>{ T(), TCurveProperties<T>::GetZero(), TCurveProperties<T>::GetZero(), 0.0f } });
		}

		/** Creates a new distribution that returns a random value in a range determined by two curves. */
		B3D_SCRIPT_EXPORT()

		TDistribution(const TAnimationCurve<T>& minCurve, const TAnimationCurve<T>& maxCurve)
			: mType(PDT_RandomCurveRange), mMinCurve(minCurve), mMaxCurve(maxCurve)
		{
			if(mMinCurve.GetKeyFrames().empty())
				mMinCurve = TAnimationCurve<T>({ TKeyframe<T>{ T(), TCurveProperties<T>::GetZero(), TCurveProperties<T>::GetZero(), 0.0f } });

			if(mMaxCurve.GetKeyFrames().empty())
				mMaxCurve = TAnimationCurve<T>({ TKeyframe<T>{ T(), TCurveProperties<T>::GetZero(), TCurveProperties<T>::GetZero(), 0.0f } });
		}

		/** Returns the type of the represented distribution. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(DistributionType))

		PropertyDistributionType GetType() const { return mType; }

		/**
		 * Returns the constant value of the distribution, or the minimal value of a constant range. Undefined if
		 * the distribution is represented by a curve.
		 */
		B3D_SCRIPT_EXPORT()

		const T& GetMinConstant() const { return mMinCurve.GetKeyFrames()[0].Value; }

		/**
		 * Returns the maximum value of a constant range. Only defined if the distribution represents a non-curve range.
		 */
		B3D_SCRIPT_EXPORT()

		const T& GetMaxConstant() const { return mMaxCurve.GetKeyFrames()[0].Value; }

		/**
		 * Returns the curve representing the distribution, or the first curve representing a curve range. Undefined if
		 * the distribution is represented by a constant or a non-curve range.
		 */
		B3D_SCRIPT_EXPORT()

		const TAnimationCurve<T>& GetMinCurve() const { return mMinCurve; }

		/**
		 * Returns the curve representing the second curve of a curve range. Only defined if the distribution represents
		 * a curve range.
		 */
		B3D_SCRIPT_EXPORT()

		const TAnimationCurve<T>& GetMaxCurve() const { return mMaxCurve; }

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
		B3D_SCRIPT_EXPORT()

		T Evaluate(float t, float factor) const
		{
			switch(mType)
			{
			default:
			case PDT_Constant:
				return GetMinConstant();
			case PDT_RandomRange:
				return Math::Lerp(factor, GetMinConstant(), GetMaxConstant());
			case PDT_Curve:
				return mMinCurve.Evaluate(t);
			case PDT_RandomCurveRange:
				{
					const T minValue = mMinCurve.Evaluate(t);
					const T maxValue = mMaxCurve.Evaluate(t);

					return Math::Lerp(factor, minValue, maxValue);
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
		B3D_SCRIPT_EXPORT()

		T Evaluate(float t, const Random& factor) const
		{
			switch(mType)
			{
			default:
			case PDT_Constant:
				return GetMinConstant();
			case PDT_RandomRange:
				return Math::Lerp(factor.GetUNorm(), GetMinConstant(), GetMaxConstant());
			case PDT_Curve:
				return mMinCurve.Evaluate(t);
			case PDT_RandomCurveRange:
				{
					const T minValue = mMinCurve.Evaluate(t);
					const T maxValue = mMaxCurve.Evaluate(t);

					return Math::Lerp(factor.GetUNorm(), minValue, maxValue);
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
		LookupTable ToLookupTable(u32 numSamples = 128, bool ignoreRange = false) const;

		bool operator==(const TDistribution<T>& rhs) const
		{
			if(mType != rhs.mType)
				return false;

			if(mType == PDT_Constant || mType == PDT_Curve)
				return mMinCurve == rhs.mMinCurve;
			else
				return mMinCurve == rhs.mMinCurve && mMaxCurve == rhs.mMaxCurve;
		}

		bool operator!=(const TDistribution<T>& rhs) const { return !operator==(rhs); }

	private:
		friend struct RTTIPlainType<TDistribution<T>>;

		PropertyDistributionType mType;
		TAnimationCurve<T> mMinCurve;
		TAnimationCurve<T> mMaxCurve;
	};

	using FloatDistribution = TDistribution<float>;
	using Vector3Distribution = TDistribution<Vector3>;
	using Vector2Distribution = TDistribution<Vector2>;

#ifdef B3D_CODEGEN 
	template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportName(FloatDistribution)) TDistribution<float>;
	template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportName(Vector3Distribution)) TDistribution<Vector3>;
	template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportName(Vector2Distribution)) TDistribution<Vector2>;
#endif

	/** @} */
} // namespace b3d
