//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup General
	 *  @{
	 */

	/**
	 * Contains a set of samples resulting from sampling some function at equal intervals. The table can then be used
	 * for sampling that function at arbitrary time intervals. The sampling is fast but precision is limited to the number
	 * of samples.
	 */
	class B3D_EXPORT LookupTable
	{
	public:
		/**
		 * Constructs a lookup table from the provided set of values.
		 *
		 * @param	values		Buffer containing information about all the samples. Total buffer size must be divisble
		 *						by @p sampleSize.
		 * @param	startTime	Time at which the first provided sample has been evaluated at.
		 * @param	endTime		Time at which the last provided sample has been evaluate at. All samples in-between
		 *						first and last are assumed to be evaluated to equal intervals in the
		 *						[startTime, endTime] range.
		 * @param	sampleSize	Number of 'float's each sample requires. This number must divide the number of elements
		 *						in the @p values buffer.
		 */
		LookupTable(Vector<float> values, float startTime = 0.0f, float endTime = 1.0f, uint32_t sampleSize = 1);

		/**
		 * Evaluates the lookup table at the specified time.
		 *
		 * @param	t				Time to evaluate the lookup table at.
		 * @param	outLeft			Pointer to the set of values contained in the sample left to the time value.
		 * @param	outRight		Pointer to the set of values contained in the sample right to the time value.
		 * @param	outFraction		Fraction that determines how to interpolate between @p outLeft and @p outRight values, where
		 *							0 corresponds to the @p outLeft value, 1 to the @p outRight value and values in-between
		 *							interpolate linearly between the two.
		 */
		void Evaluate(float t, const float*& outLeft, const float*& outRight, float& outFraction) const;

		/** Returns a sample at the specified index. Returns last available sample if index is out of range. */
		const float* GetSample(uint32_t index) const;

	private:
		Vector<float> mValues;
		uint32_t mSampleSize;
		uint32_t mSampleCount;
		float mTimeStart;
		float mTimeScale;
	};

	/** @} */
} // namespace b3d
