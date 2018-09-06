//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	/** 
	 * Contains a set of samples resulting from sampling some function at equal intervals. The table can then be used
	 * for sampling that function at arbitrary time intervals. The sampling is fast but precision is limited to the number
	 * of samples.
	 */
	class BS_UTILITY_EXPORT LookupTable
	{
	public:
		/**
		 * Constructs a lookup table from the provided set of values.
		 * 
		 * @param[in]	values		Buffer containing information about all the samples. Total buffer size must be divisble
		 *							by @p sampleSize.
		 * @param[in]	startTime	Time at which the first provided sample has been evaluated at.
		 * @param[in]	endTime		Time at which the last provided sample has been evaluate at. All samples in-between
		 *							first and last are assumed to be evaluated to equal intervals in the 
		 *							[startTime, endTime] range.
		 * @param[in]	sampleSize	Number of 'float's each sample requires. This number must divide the number of elements
		 *							in the @p values buffer.
		 */
		LookupTable(Vector<float> values, float startTime = 0.0f, float endTime = 1.0f, uint32_t sampleSize = 1);

		/** 
		 * Evaluates the lookup table at the specified time. 
		 *
		 * @param[in]	t			Time to evaluate the lookup table at.
		 * @param[out]	left		Pointer to the set of values contained in the sample left to the time value.
		 * @param[out]	right		Pointer to the set of values contained in the sample right to the time value.
		 * @param[out]	fraction	Fraction that determines how to interpolate between @p left and @p right values, where
		 *							0 corresponds to the @p left value, 1 to the @p right value and values in-between
		 *							interpolate linearly between the two.
		 */
		void evaluate(float t, const float*& left, const float*& right, float& fraction) const;

		/** Returns a sample at the specified index. Returns last available sample if index is out of range. */
		const float* getSample(uint32_t idx) const;

	private:
		Vector<float> mValues;
		uint32_t mSampleSize;
		uint32_t mNumSamples;
		float mTimeStart;
		float mTimeScale;
	};

	/** @} */
}