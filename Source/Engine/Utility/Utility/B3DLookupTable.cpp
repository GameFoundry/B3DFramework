//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DLookupTable.h"
#include "Math/B3DMath.h"

using namespace b3d;

LookupTable::LookupTable(Vector<float> values, float startTime, float endTime, uint32_t sampleSize)
	: mValues(std::move(values))
	, mSampleSize(std::max(sampleSize, 1U))
	, mSampleCount((uint32_t)mValues.size() / mSampleSize)
	, mTimeStart(startTime)
{
	if(endTime < startTime)
		endTime = startTime;

	float timeInterval;
	if(mSampleCount > 1)
		timeInterval = (endTime - startTime) / (mSampleCount - 1);
	else
		timeInterval = 0.0f;

	mTimeScale = 1.0f / timeInterval;
}

void LookupTable::Evaluate(float t, const float*& outLeft, const float*& outRight, float& outFraction) const
{
	t -= mTimeStart;
	t *= mTimeScale;

	const auto index = (uint32_t)t;
	outFraction = Math::Frac(t);

	const uint32_t leftIndex = std::min(index, mSampleCount - 1);
	const uint32_t rightIndex = std::min(index + 1, mSampleCount - 1);

	outLeft = &mValues[leftIndex * mSampleSize];
	outRight = &mValues[rightIndex * mSampleSize];
}

const float* LookupTable::GetSample(uint32_t index) const
{
	if(mSampleCount == 0)
		return nullptr;

	index = std::min(index, mSampleCount - 1);
	return &mValues[index * mSampleSize];
}
