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
	Language = "HLSL11";
	
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

Technique : base("Surface") =
{
	Language = "GLSL";
	
	Pass =
	{
		Fragment =
		{
			layout(location = 0) in vec2 uv0;
			layout(location = 2) in vec3 tangentToWorldZ;
			layout(location = 3) in vec4 tangentToWorldX;
		
			layout(binding = 4) uniform sampler2D gAlbedoTex;
			layout(binding = 5) uniform sampler2D gNormalTex;
			layout(binding = 6) uniform sampler2D gRoughnessTex;
			layout(binding = 7) uniform sampler2D gMetalnessTex;
			
			layout(location = 0) out vec4 fragColor[3];
			
			void main()
			{
				vec3 normal = normalize(texture(gNormalTex, uv0).xyz * 2.0f - vec3(1, 1, 1));
				vec3 worldNormal = calcWorldNormal(tangentToWorldZ, tangentToWorldX, normal);
			
				SurfaceData surfaceData;
				surfaceData.albedo = texture(gAlbedoTex, uv0);
				surfaceData.worldNormal.xyz = worldNormal;
				surfaceData.roughness = texture(gRoughnessTex, uv0).x;
				surfaceData.metalness = texture(gMetalnessTex, uv0).x;
				
				encodeGBuffer(surfaceData, fragColor[0], fragColor[1], fragColor[2]);
			}	
		};
	};
};