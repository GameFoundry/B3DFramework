#include "$ENGINE$\DeferredBasePass.bslinc"

Parameters = 
{
	Sampler2D 	samp;
	Texture2D 	tex;
};

Technique = 
{
	Language = "HLSL11";
	
	Pass = 
	{
		Fragment =
		{
			SamplerState samp : register(s0);
			Texture2D tex : register(t0);
		
			float4 main(
				in VStoFS input, 
				out float4 OutGBufferA : SV_Target1,
				out float4 OutGBufferB : SV_Target2) : SV_Target0
			{
				GBufferData gbufferData;
				gbufferData.albedo = float4(tex.Sample(samp, input.uv0).xyz, 1.0f);
				gbufferData.worldNormal.xyz = input.tangentToWorldZ;
				
				encodeGBuffer(gbufferData, OutGBufferA, OutGBufferB);
				
				// TODO - Just returning a simple ambient term, use better environment lighting later
				return float4(gbufferData.albedo.rgb, 1.0f) * 0.2f; 
			}	
		};
	};
};