#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\DepthInput.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"

shader PPMotionBlur
{
	mixin PPBase;
	mixin DepthInput;
	mixin PerCameraData;
	
	code
	{
		cbuffer Params
		{
			uint gHalfNumSamples;
		};
		
		Texture2D gInputTex;
		SamplerState gInputSampler 
		{
			Filter = MIN_MAG_MIP_POINT;
		};
	
		float4 fsmain(VStoFS input) : SV_Target0
		{
			// TODO - Deal with MSAA inputs?
		
			float2 currentUV = input.uv0;
			float2 ndcPos = input.screenPos;
			float curDepth = gDepthBufferTex.Sample(gDepthBufferSamp, currentUV).r;
		
			float4 currentNDC = float4(ndcPos, curDepth, 1);
			float4 prevClip = mul(gNDCToPrevNDC, currentNDC);
			float2 prevNdcPos = prevClip.xy / prevClip.w;
			float2 prevUV = NDCToUV(prevNdcPos);
			
			// TODO - Scale blue length by framerate
			// TODO - Clamp to maximum blur radius
			float2 blurDir = (prevUV - currentUV) * 0.5;
			
			// TODO - Jitter the samples
			
			float4 output = gInputTex.Sample(gInputSampler, currentUV);
			for (int i = -gHalfNumSamples; i < 0; ++i) 
			{
				float2 offset = blurDir * (i / (float)gHalfNumSamples);
				output += gInputTex.Sample(gInputSampler, currentUV + offset);
			}
			
			for (int i = 1; i <= gHalfNumSamples; ++i) 
			{
				float2 offset = blurDir * (i / (float)gHalfNumSamples);
				output += gInputTex.Sample(gInputSampler, currentUV + offset);
			}
			
			output /= gHalfNumSamples * 2 + 1;
			return output;
		}	
	};
};