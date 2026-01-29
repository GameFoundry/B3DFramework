#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"

shader PPSSAOBlur
{
	mixin PPBase;
	mixin PerCameraData;

	variations
	{
		DIR_HORZ = { true, false };
	};	
	
	code
	{
		[internal]
		cbuffer Input
		{
			float2 gPixelSize;
			float2 gPixelOffset;
			float gInvDepthThreshold;
		}		

		SamplerState gInputSamp;
		Texture2D gInputTex;
		Texture2D gDepthTex;
		
		static const int NUM_SAMPLES = 2;
		
		float fsmain(VStoFS input) : SV_Target0
		{
			float centerDepth = convertFromDeviceZ(gDepthTex.Sample(gInputSamp, input.uv0).r);
		
			float weightedSum = 0.0f;
			float weightSum = 0.0f;
			
			float centerAO = gInputTex.Sample(gInputSamp, input.uv0).r;
			weightedSum += centerAO;
			weightSum += 1.0f;
		
			// Note: Consider using normals as a weight as well
			[unroll]
			for(int i = 1; i < (NUM_SAMPLES + 1); ++i)
			{
				float2 sampleUV = gPixelSize * i + input.uv0;
			
				float sampleAO = gInputTex.Sample(gInputSamp, sampleUV).r;
				float sampleDepth = convertFromDeviceZ(gDepthTex.Sample(gInputSamp, sampleUV).r);
				
				float weight = saturate(1.0f - abs(sampleDepth - centerDepth) * gInvDepthThreshold);
				weightedSum += sampleAO * weight;
				weightSum += weight;
			}
			
			[unroll]
			for(int i = -NUM_SAMPLES; i < 0; ++i)
			{
				float2 sampleUV = gPixelSize * i + input.uv0;
			
				float sampleAO = gInputTex.Sample(gInputSamp, sampleUV).r;
				float sampleDepth = convertFromDeviceZ(gDepthTex.Sample(gInputSamp, sampleUV).r);
				
				float weight = saturate(1.0f - abs(sampleDepth - centerDepth) * gInvDepthThreshold);
				weightedSum += sampleAO * weight;
				weightSum += weight;
			}
					
			return weightedSum / weightSum;
		}	
	};
};
