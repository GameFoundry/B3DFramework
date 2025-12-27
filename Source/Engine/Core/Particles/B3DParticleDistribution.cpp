//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DParticleDistribution.h"

using namespace b3d;

template <class T>
void AddToVector(const T& val, Vector<float>& output)
{
	output.push_back(val);
}

template <>
void AddToVector(const Vector3& val, Vector<float>& output)
{
	output.push_back(val.X);
	output.push_back(val.Y);
	output.push_back(val.Z);
}

template <>
void AddToVector(const Vector2& val, Vector<float>& output)
{
	output.push_back(val.X);
	output.push_back(val.Y);
}

template <>
void AddToVector(const Color& val, Vector<float>& output)
{
	output.push_back(val.R);
	output.push_back(val.G);
	output.push_back(val.B);
	output.push_back(val.A);
}

template <class T>
LookupTable TColorDistribution<T>::ToLookupTable(u32 numSamples, bool ignoreRange) const
{
	numSamples = std::max(1U, numSamples);

	Vector<float> values;
	float minT = 0.0f;
	float maxT = 1.0f;

	const bool useRange = (mType == PDT_RandomRange || mType == PDT_RandomCurveRange) && !ignoreRange;

	switch(mType)
	{
	default:
	case PDT_Constant:
	case PDT_RandomRange:
		{
			AddToVector(GetMinConstant(), values);

			if(useRange)
				AddToVector(GetMaxConstant(), values);
		}
		break;
	case PDT_Curve:
	case PDT_RandomCurveRange:
		{
			const std::pair<float, float> minCurveRange = mMinGradient.GetTimeRange();
			minT = minCurveRange.first;
			maxT = minCurveRange.second;

			if(useRange)
			{
				const std::pair<float, float> maxCurveRange = mMaxGradient.GetTimeRange();
				minT = std::min(minT, maxCurveRange.first);
				maxT = std::max(maxT, maxCurveRange.second);
			}

			float sampleInterval = 0.0f;
			if(numSamples > 1)
				sampleInterval = (maxT - minT) / (numSamples - 1);

			float t = minT;
			for(u32 i = 0; i < numSamples; i++)
			{
				AddToVector(TGradientHelper<typename T::ColorType>::FromInternalColor(mMinGradient.Evaluate(t)), values);

				if(useRange)
					AddToVector(TGradientHelper<typename T::ColorType>::FromInternalColor(mMaxGradient.Evaluate(t)), values);

				t += sampleInterval;
			}
		}
	}

	return LookupTable(std::move(values), minT, maxT, sizeof(Color) / sizeof(float));
}

template struct B3D_EXPORT TColorDistribution<ColorGradient>;
template struct B3D_EXPORT TColorDistribution<ColorGradientHDR>;

template <class T>
LookupTable TDistribution<T>::ToLookupTable(u32 numSamples, bool ignoreRange) const
{
	numSamples = std::max(1U, numSamples);

	Vector<float> values;
	float minT = 0.0f;
	float maxT = 1.0f;

	const bool useRange = (mType == PDT_RandomRange || mType == PDT_RandomCurveRange) && !ignoreRange;

	switch(mType)
	{
	default:
	case PDT_Constant:
	case PDT_RandomRange:
		AddToVector(GetMinConstant(), values);

		if(useRange)
			AddToVector(GetMaxConstant(), values);
		break;
	case PDT_Curve:
	case PDT_RandomCurveRange:
		{
			const std::pair<float, float> minCurveRange = mMinCurve.GetTimeRange();
			minT = minCurveRange.first;
			maxT = minCurveRange.second;

			if(useRange)
			{
				const std::pair<float, float> maxCurveRange = mMaxCurve.GetTimeRange();
				minT = std::min(minT, maxCurveRange.first);
				maxT = std::max(maxT, maxCurveRange.second);
			}

			float sampleInterval = 0.0f;
			if(numSamples > 1)
				sampleInterval = (maxT - minT) / (numSamples - 1);

			float t = minT;
			for(u32 i = 0; i < numSamples; i++)
			{
				T value = mMinCurve.Evaluate(t);
				AddToVector(value, values);

				if(useRange)
				{
					value = mMaxCurve.Evaluate(t);
					AddToVector(value, values);
				}

				t += sampleInterval;
			}
		}
	}

	return LookupTable(std::move(values), minT, maxT, sizeof(T) / sizeof(float));
}

template struct B3D_EXPORT TDistribution<float>;
template struct B3D_EXPORT TDistribution<Vector3>;
template struct B3D_EXPORT TDistribution<Vector2>;
