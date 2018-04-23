#include "$ENGINE$\BasePass.bslinc"
#include "$ENGINE$\GBufferOutput.bslinc"

shader Surface
{
	mixin BasePass;
	mixin GBufferOutput;

	code
	{
		SamplerState gAlbedoSamp;
		SamplerState gNormalSamp;
		SamplerState gRoughnessSamp;
		SamplerState gMetalnessSamp;
		
		Texture2D gAlbedoTex = white;
		Texture2D gNormalTex = normal;
		Texture2D gRoughnessTex = white;
		Texture2D gMetalnessTex = black;
		
		void fsmain(
			in VStoFS input, 
			out float4 OutGBufferA : SV_Target0,
			out float4 OutGBufferB : SV_Target1,
			out float2 OutGBufferC : SV_Target2)
		{
			float3 normal = normalize(gNormalTex.Sample(gNormalSamp, input.uv0) * 2.0f - float3(1, 1, 1));
			float3 worldNormal = calcWorldNormal(input, normal);
		
			SurfaceData surfaceData;
			surfaceData.albedo = gAlbedoTex.Sample(gAlbedoSamp, input.uv0);
			surfaceData.worldNormal.xyz = worldNormal;
			surfaceData.roughness = gRoughnessTex.Sample(gRoughnessSamp, input.uv0).x;
			surfaceData.metalness = gMetalnessTex.Sample(gMetalnessSamp, input.uv0).x;
			
			encodeGBuffer(surfaceData, OutGBufferA, OutGBufferB, OutGBufferC);
		}	
	};
};