#include "$ENGINE$\BasePass.bslinc"
#include "$ENGINE$\Surface.bslinc"

Parameters =
{
	Sampler2D 	gAlbedoSamp : alias("gAlbedoTex");
	Sampler2D 	gNormalSamp : alias("gNormalTex");
	Sampler2D	gRoughnessSamp : alias("gRoughnessTex");
	Sampler2D	gMetalnessSamp : alias("gMetalnessTex");
	
	Texture2D 	gAlbedoTex;
	Texture2D	gNormalTex = "normal";
	Texture2D	gRoughnessTex = "white";
	Texture2D	gMetalnessTex = "black";
};

Technique : base("Surface") =
{
	Pass =
	{
		Fragment =
		{
			SamplerState gAlbedoSamp : register(s0);
			SamplerState gNormalSamp : register(s1);
			SamplerState gRoughnessSamp : register(s2);
			SamplerState gMetalnessSamp : register(s3);
			
			Texture2D gAlbedoTex : register(t0);
			Texture2D gNormalTex : register(t1);
			Texture2D gRoughnessTex : register(t2);
			Texture2D gMetalnessTex : register(t3);
			
			void main(
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
};