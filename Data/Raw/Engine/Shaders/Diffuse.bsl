#include "$ENGINE$\DeferredBasePass.bslinc"

Parameters =
{
	Sampler2D 	gAlbedoSamp : alias("gAlbedoTex");
	Sampler2D 	gNormalSamp : alias("gNormalTex");
	
	Texture2D 	gAlbedoTex;
	Texture2D	gNormalTex = "normal";
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
			
			Texture2D gAlbedoTex : register(t0);
			Texture2D gNormalTex : register(t1);
			
			float4 main(
				in VStoFS input, 
				out float4 OutGBufferA : SV_Target1,
				out float4 OutGBufferB : SV_Target2) : SV_Target0
			{
				float3 normal = normalize(gNormalTex.Sample(gNormalSamp, input.uv0) * 2.0f - float3(1, 1, 1));
				float3 worldNormal = calcWorldNormal(input, normal);
			
				SurfaceData surfaceData;
				surfaceData.albedo = gAlbedoTex.Sample(gAlbedoSamp, input.uv0);
				surfaceData.worldNormal.xyz = worldNormal;
				
				encodeGBuffer(surfaceData, OutGBufferA, OutGBufferB);
				
				// TODO - Just returning a simple ambient term, use better environment lighting later
				return float4(surfaceData.albedo.rgb, 1.0f) * 0.01f; 
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
			layout(location = 1) in vec3 tangentToWorldZ;
			layout(location = 2) in vec4 tangentToWorldX;			
		
			layout(binding = 4) uniform sampler2D gAlbedoTex;
			layout(binding = 5) uniform sampler2D gNormalTex;

			layout(location = 0) out vec4 fragColor[3];
			
			void main()
			{
				vec3 normal = normalize(texture(gNormalTex, uv0).xyz * 2.0f - vec3(1, 1, 1));
				vec3 worldNormal = calcWorldNormal(tangentToWorldZ, tangentToWorldX, normal);
			
				SurfaceData surfaceData;
				surfaceData.albedo = texture(gAlbedoTex, uv0);
				surfaceData.worldNormal.xyz = worldNormal;
				
				encodeGBuffer(surfaceData, fragColor[1], fragColor[2]);
				
				// TODO - Just returning a simple ambient term, use better environment lighting later
				fragColor[0] = vec4(surfaceData.albedo.rgb, 1.0f) * 0.01f; 
			}	
		};
	};
};

#include "$ENGINE$\Surface.bslinc"