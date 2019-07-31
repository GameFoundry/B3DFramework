//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsParticleDistribution.h"

namespace bs
{
	template <class T>
	void addToVector(const T& val, Vector<float>& output)
	{
		output.push_back(val);
	}

	template <>
	void addToVector(const Vector3& val, Vector<float>& output)
	{
		output.push_back(val.x);
		output.push_back(val.y);
		output.push_back(val.z);
	}

	template <>
	void addToVector(const Vector2& val, Vector<float>& output)
	{
		output.push_back(val.x);
		output.push_back(val.y);
	}

	template <>
	void addToVector(const Color& val, Vector<float>& output)
	{
		output.push_back(val.r);
		output.push_back(val.g);
		output.push_back(val.b);
		output.push_back(val.a);
	}

	template<class T>
	LookupTable TColorDistribution<T>::toLookupTable(UINT32 numSamples, bool ignoreRange) const
	{
		numSamples = std::max(1U, numSamples);

		Vector<float> values;
		float minT = 0.0f;
		float maxT = 1.0f;

		const bool useRange = (mType == PDT_RandomRange || mType == PDT_RandomCurveRange) && !ignoreRange;

		switch (mType)
		{
		default:
		case PDT_Constant:
		case PDT_RandomRange:
		{
			addToVector(getMinConstant(), values);

			if(useRange)
				addToVector(getMaxConstant(), values);
		}
			break;
		case PDT_Curve:
		case PDT_RandomCurveRange:
			{
				const std::pair<float, float> minCurveRange = mMinGradient.getTimeRange();
				minT = minCurveRange.first;
				maxT = minCurveRange.second;

				if(useRange)
				{
					const std::pair<float, float> maxCurveRange = mMaxGradient.getTimeRange();
					minT = std::min(minT, maxCurveRange.first);
					maxT = std::max(maxT, maxCurveRange.second);
				}

				float sampleInterval = 0.0f;
				if(numSamples > 1)
					sampleInterval = (maxT - minT) / (numSamples - 1);

				float t = minT;
				for(UINT32 i = 0; i < numSamples; i++)
				{
					addToVector(impl::TGradientHelper<typename T::ColorType>::fromInternalColor(mMinGradient.evaluate(t)), values);

					if(useRange)
						addToVector(impl::TGradientHelper<typename T::ColorType>::fromInternalColor(mMaxGradient.evaluate(t)), values);
					
					t += sampleInterval;
				}
			}
		}

		return LookupTable(std::move(values), minT, maxT, sizeof(Color) / sizeof(float));
	}

	template struct BS_CORE_EXPORT TColorDistribution<ColorGradient>;
	template struct BS_CORE_EXPORT TColorDistribution<ColorGradientHDR>;

	template <class T>
	LookupTable TDistribution<T>::toLookupTable(UINT32 numSamples, bool ignoreRange) const
	{
		numSamples = std::max(1U, numSamples);

		Vector<float> values;
		float minT = 0.0f;
		float maxT = 1.0f;

		const bool useRange = (mType == PDT_RandomRange || mType == PDT_RandomCurveRange) && !ignoreRange;

		switch (mType)
		{
		default:
		case PDT_Constant:
		case PDT_RandomRange:
			addToVector(getMinConstant(), values);

			if(useRange)
				addToVector(getMaxConstant(), values);
			break;
		case PDT_Curve:
		case PDT_RandomCurveRange:
			{
				const std::pair<float, float> minCurveRange = mMinCurve.getTimeRange();
				minT = minCurveRange.first;
				maxT = minCurveRange.second;

				if(useRange)
				{
					const std::pair<float, float> maxCurveRange = mMaxCurve.getTimeRange();
					minT = std::min(minT, maxCurveRange.first);
					maxT = std::max(maxT, maxCurveRange.second);
				}

				float sampleInterval = 0.0f;
				if(numSamples > 1)
					sampleInterval = (maxT - minT) / (numSamples - 1);

				float t = minT;
				for(UINT32 i = 0; i < numSamples; i++)
				{
					T value = mMinCurve.evaluate(t);
					addToVector(value, values);

					if(useRange)
					{
						value = mMaxCurve.evaluate(t);
						addToVector(value, values);
					}
					
					t += sampleInterval;
				}
			}
		}

		return LookupTable(std::move(values), minT, maxT, sizeof(T) / sizeof(float));
	}

	template struct BS_CORE_EXPORT TDistribution<float>;
	template struct BS_CORE_EXPORT TDistribution<Vector3>;
	template struct BS_CORE_EXPORT TDistribution<Vector2>;
}
