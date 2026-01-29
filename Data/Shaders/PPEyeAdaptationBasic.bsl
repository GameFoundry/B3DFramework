#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\PPEyeAdaptationCommon.bslinc"

shader PPEyeAdaptationBasic
{
	mixin PPBase;
	mixin PPEyeAdaptationParams;

	code
	{
		[internal]
		cbuffer Input
		{
			int2 gInputTexSize;
		}		
	
		Texture2D gCurFrameTex;
		Texture2D gPrevFrameTex;
		
		/** 
		 * Returns the value in range [0,1] on the triangle function with slope @p slope. 
		 * @p t must be in range [0,1].
		 */
		float triangleFunc(float t, float slope)
		{
			return max(1.0f - 2.0f * slope * abs(t - 0.5f), 0.0f);
		}		
		
		/** 
		 * Calculates the average value of all pixels in the input texture. The pixels are
		 * weighted using a triangle filter using @p slope as the line slope. @p slope of 0
		 * means all pixels will be uniformly weighted.
		 */
		float calcWeightedAverageAlpha(Texture2D input, int2 inputSize, float slope)
		{
			float2 invSize = 1.0f / inputSize;

			float sum = 0.0f;
			float weightSum = 0.0f;
			for (uint i = 0; i < inputSize.x; ++i)
			{
				float weightX = triangleFunc(i * invSize.x, slope);  
				for (uint j = 0; j < inputSize.y; ++j)
				{
					float weightY = triangleFunc(j * invSize.y, slope);
					float weight = max(weightX * weightY, 0.05f);
	
					float value = input.Load(int3(i, j, 0)).a;
					
					sum += value * weight;
					weightSum += weight;
				}
			}

			return sum / weightSum;
		}		
		
		float4 fsmain(VStoFS input) : SV_Target0
		{
			float slope = 0.0f; // TODO - Allow custom slope?
			float avgLuminance = calcWeightedAverageAlpha(gCurFrameTex, gInputTexSize, slope);
		
			// Scale back into normal range (was log2 encoded and scale into [0, 1] range)
			avgLuminance = exp2((avgLuminance - gEyeAdaptationParams[0].y) / gEyeAdaptationParams[0].x);
			avgLuminance /= 0.16f;
		
			// Clamp to valid range
			avgLuminance = clamp(avgLuminance, gEyeAdaptationParams[1].x, gEyeAdaptationParams[1].y);
			
			float exposureScale = gEyeAdaptationParams[2].x;
			float oldExposure = gPrevFrameTex.Load(int3(0, 0, 0)).x;
			float oldLuminance = exposureScale / oldExposure; // Assuming same exposure scale as last frame
			
			float frameDelta = gEyeAdaptationParams[2].y;
			float smoothAdaptation = smoothEyeAdaptation(oldLuminance, avgLuminance, frameDelta);
			
			return exposureScale / smoothAdaptation; // Returns exposure
		}	
	};
};
