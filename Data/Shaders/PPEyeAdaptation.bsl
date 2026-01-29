#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\PPEyeAdaptationCommon.bslinc"

shader PPEyeAdaptation
{
	mixin PPBase;
	mixin PPEyeAdaptationParams;

	code
	{
		#define NUM_BUCKETS (THREADGROUP_SIZE_X * THREADGROUP_SIZE_Y)
	
		Texture2D gHistogramTex;
		
		/** 
		 * Returns luminance of the histogram bucket.
		 *
		 * @param pos	Position of the histogram bucket in range [0, 1].
		 * @return		Luminance of the bucket.
		 */
		float calcHistogramLuminance(float pos)
		{
			return exp2((pos - gEyeAdaptationParams[0].y) / gEyeAdaptationParams[0].x);
		}	
		
		/**
		 * Returns value of the histogram bucket.
		 *
		 * @param	histogram	Texture containing the histogram buckets in the first row.
		 * @param	bucketIdx	Index of the bucket. Caller must ensure it is in valid range.
		 * @return				Value of the needed histogram bucket.
		 */
		float getHistogramValue(Texture2D histogram, uint bucketIdx)
		{
			uint texelIdx = bucketIdx / 4;
			
			float4 packedValue = histogram.Load(int3(texelIdx, 0, 0));
			float4 mask = float4(
				(bucketIdx % 4) == 0,
				(bucketIdx % 4) == 1,
				(bucketIdx % 4) == 2,
				(bucketIdx % 4) == 3);

			return dot(packedValue, mask);	
		}

		/** 
		 * Calculates the sum of all values in the histogram.
		 *
		 * @param	histogram	Texture containing the histogram buckets in the first row.
		 * @return				Sum of all the values in the histogram.
		 */
		float calcHistogramSum(Texture2D histogram)
		{
			float sum = 0;

			for(uint i = 0; i < NUM_BUCKETS; i++)
				sum += getHistogramValue(histogram, i);

			return sum;
		}	

		/**
		 * Calculates the average luminance in the histogram, while ignoring the outlier values that may skew the result.
		 *
		 * @param	histogram	Texture containing the histogram buckets in the first row.
		 * @param	low			Sum below which to ignore values (removing lower end outliers), in range [0, histogramSum].
		 * @param	high		Sum above which to ignore values (removing higher end outliers), in range [0, histogramSum]. 
		 *                      Must be higher than @low.
		 * @return				Average luminance in the histogram.
		 */
		float calcHistogramAverageLuminance(Texture2D histogram, float low, float high)
		{
			float2 sumAndWeight = float2(0.0f, 0.0f);

			for(uint i = 0; i < NUM_BUCKETS; i++)
			{
				float value = getHistogramValue(histogram, i);

				// Ignore any values below the @low parameter, and then shift the valid range
				// by the amount we ignored. Eventually the low end of the range reaches zero
				// and values are no longer ignored.
				float offset = min(value, low);
				value = value - offset;
				low -= offset;
				high -= offset;

				// Ignore any values above the @high parameter, and then shift the valid range.
				value = min(value, high);
				high -= value;

				float histogramPos = i / (float)NUM_BUCKETS;
				float luminance = calcHistogramLuminance(histogramPos);
				
				sumAndWeight += float2(luminance, 1) * value;
			}
			
			return sumAndWeight.x / max(0.0001f, sumAndWeight.y);
		}
		
		/**
		 * Calculates the eye adaptation from the luminance in the provided histogram. Eye adaptation value will be 
		 * used for automatically scaling expsure based on scene brightness.
		 *
		 * @param	histogram	Texture containing the histogram buckets in the first row.
		 * @return				Ideal eye adaptation value for the provided luminance.
		 */
		float calcEyeAdaptation(Texture2D histogram)
		{
			float sum = calcHistogramSum(histogram);
			float lowRange = gEyeAdaptationParams[0].z * sum;
			float highRange = gEyeAdaptationParams[0].w * sum;
			
			float avgLuminance = calcHistogramAverageLuminance(histogram, lowRange, highRange);
			avgLuminance = clamp(avgLuminance, gEyeAdaptationParams[1].x, gEyeAdaptationParams[1].y);

			return avgLuminance;
		}
				
		float4 fsmain(VStoFS input) : SV_Target0
		{
			float exposureScale = gEyeAdaptationParams[2].x;

			float targetAdaptation = calcEyeAdaptation(gHistogramTex);
			float oldExposure = gHistogramTex.Load(int3(0, 1, 0)).x;
			float oldAdaptation = exposureScale / oldExposure; // Assuming same exposure scale as last frame
			float frameDelta = gEyeAdaptationParams[2].y;
			
			float smoothAdaptation = smoothEyeAdaptation(oldAdaptation, targetAdaptation, frameDelta);
			return exposureScale / smoothAdaptation; // Returns exposure
		}	
	};
};
