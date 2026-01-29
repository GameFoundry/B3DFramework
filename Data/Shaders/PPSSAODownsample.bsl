#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"

shader PPSSAODownsample
{
	mixin PPBase;
	mixin PerCameraData;

	code
	{
		[internal]
		cbuffer Input
		{
			float2 gPixelSize;
			float gInvDepthThreshold;
		}		

		SamplerState gInputSamp;
		Texture2D gDepthTex;
		Texture2D gNormalsTex;
		
		float4 fsmain(VStoFS input) : SV_Target0
		{
			float2 uvs[4];
			uvs[0] = input.uv0 + float2(-0.5f, -0.5f) * gPixelSize;
			uvs[1] = input.uv0 + float2(-0.5f,  0.5f) * gPixelSize;
			uvs[2] = input.uv0 + float2( 0.5f, -0.5f) * gPixelSize;
			uvs[3] = input.uv0 + float2( 0.5f,  0.5f) * gPixelSize;
			
			float4 samples[4];
			[unroll]
			for(int i = 0; i < 4; i++)
			{
				samples[i].xyz = gNormalsTex.Sample(gInputSamp, uvs[i]).xyz;
				samples[i].w = convertFromDeviceZ(gDepthTex.Sample(gInputSamp, uvs[i]).r);
			}
		
			float maxZ = max(max(samples[0].w, samples[1].w), max(samples[2].w, samples[3].w));
			
			float3 weightedSum = 0.0f;
			float weightSum = 0.00001f; // Avoid division by 0
			[unroll]
			for(int i = 0; i < 4; i++)
			{
				float weight = saturate(1.0f - abs(samples[i].w - maxZ) * gInvDepthThreshold);
			
				weightedSum += samples[i].xyz * weight;
				weightSum += weight;
			}
			
			return float4(weightedSum / weightSum, maxZ);
		}	
	};
};
