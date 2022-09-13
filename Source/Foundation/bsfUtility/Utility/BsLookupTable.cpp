//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsLookupTable.h"
#include "Math/BsMath.h"

namespace bs
{
	LookupTable::LookupTable(Vector<float> values, float startTime, float endTime, uint32_t sampleSize)
		: MValues(std::move(values))
		, MSampleSize(std::max(sampleSize, 1U))
		, MNumSamples((uint32_t)mValues.size() / mSampleSize)
		, MTimeStart(startTime)
	{
		if(endTime < startTime)
			endTime = startTime;

		float timeInterval;
		if(mNumSamples > 1)
			timeInterval = (endTime - startTime) / (mNumSamples - 1);
		else
			timeInterval = 0.0f;

		mTimeScale = 1.0f / timeInterval;
	}

	void LookupTable::Evaluate(float t, const float*& left, const float*& right, float& fraction) const
	{
		t -= mTimeStart;
		t *= mTimeScale;

		const auto index = (uint32_t)t;
		fraction =  Math::Frac(t);

		const uint32_t leftIdx = std::min(index, mNumSamples - 1);
		const uint32_t rightIdx = std::min(index + 1, mNumSamples - 1);

		left = &mValues[leftIdx * mSampleSize];
		right = &mValues[rightIdx * mSampleSize];
	}

	const float* LookupTable::getSample(uint32_t idx) const
	{
		if(mNumSamples == 0)
			return nullptr;

		idx = std::min(idx, mNumSamples - 1);
		return &mValues[idx * mSampleSize];
	}
}
