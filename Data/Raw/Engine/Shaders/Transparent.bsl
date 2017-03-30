#include "$ENGINE$\BasePass.bslinc"
#include "$ENGINE$\LightingCommon.bslinc"
#include "$ENGINE$\LightGridCommon.bslinc"
#include "$ENGINE$\ReflectionCubemapCommon.bslinc"
#include "$ENGINE$\ImageBasedLighting.bslinc"
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
	
	float		gOpacity   = 1.0f;
};

Blocks =
{
	Block MaterialParams;
};

Transparent = true;

Technique 
 : inherits("LightingCommon")
 : inherits("LightGridCommon")
 : inherits("ReflectionCubemapCommon")
 : inherits("ImageBasedLighting")
 : base("Surface") =
{
	Language = "HLSL11";
	
	Pass =
	{
		DepthWrite = false;
	
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};	
	
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
			
			Buffer<uint3> gGridOffsetsAndSize : register(t4);

			cbuffer MaterialParams : register(b5)
			{
				float gOpacity;
			}
			
			float4 main(in VStoFS input) : SV_Target0
			{
				float3 normal = normalize(gNormalTex.Sample(gNormalSamp, input.uv0).xyz * 2.0f - float3(1, 1, 1));
				float3 worldNormal = calcWorldNormal(input, normal);
			
				SurfaceData surfaceData;
				surfaceData.albedo = gAlbedoTex.Sample(gAlbedoSamp, input.uv0);
				surfaceData.worldNormal.xyz = worldNormal;
				surfaceData.worldNormal.w = 0.0f;
				surfaceData.roughness = gRoughnessTex.Sample(gRoughnessSamp, input.uv0).x;
				surfaceData.metalness = gMetalnessTex.Sample(gMetalnessSamp, input.uv0).x;
				
				uint2 pixelPos = (uint2)input.position.xy;
				uint cellIdx = calcCellIdx(pixelPos, input.position.z);
				uint3 offsetAndSize = gGridOffsetsAndSize[cellIdx];
				
				uint4 lightOffsets;
				lightOffsets.x = gLightOffsets[0];
				lightOffsets.y = offsetAndSize.x;
				lightOffsets.z = lightOffsets.y + offsetAndSize.y;
				lightOffsets.w = lightOffsets.z + offsetAndSize.z;
				
				float3 V = normalize(gViewOrigin - input.worldPosition);
				float3 N = surfaceData.worldNormal.xyz;
				float3 R = 2 * dot(V, N) * N - V;
				float3 specR = getSpecularDominantDir(N, R, surfaceData.roughness);
				
				float4 directLighting = getDirectLighting(input.worldPosition, V, specR, surfaceData, lightOffsets);
				float3 indirectDiffuse = getSkyIndirectDiffuse(surfaceData.worldNormal) * surfaceData.albedo;
				float3 imageBasedSpecular = getImageBasedSpecular(input.worldPosition, V, specR, surfaceData);

				float3 totalLighting = directLighting.rgb;
				totalLighting.rgb += indirectDiffuse;
				totalLighting.rgb += imageBasedSpecular;

				return float4(totalLighting, gOpacity);
			}	
		};
	};
};

Technique 
 : inherits("LightingCommon")
 : inherits("LightGridCommon")
 : base("Surface") =
{
	Language = "GLSL";
	
	Pass =
	{
		DepthWrite = false;
		
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};
	
		Fragment =
		{
			layout(location = 0) in vec2 uv0;
			layout(location = 1) in vec3 worldPosition;
			layout(location = 2) in vec3 tangentToWorldZ;
			layout(location = 3) in vec4 tangentToWorldX;			
		
			layout(binding = 5) uniform sampler2D gAlbedoTex;
			layout(binding = 6) uniform sampler2D gNormalTex;
			layout(binding = 7) uniform sampler2D gRoughnessTex;
			layout(binding = 8) uniform sampler2D gMetalnessTex;
			
			layout(binding = 9) uniform usamplerBuffer gGridOffsetsAndSize;
			layout(binding = 10) uniform usamplerBuffer gGridLightIndices;
			layout(std430, binding = 11) readonly buffer gLights
			{
				LightData[] gLightsData;
			};
						
			layout(binding = 12, std140) uniform MaterialParams
			{
				float gOpacity;
			};
			
			layout(location = 0) out vec4 fragColor;
			
			void main()
			{
				vec3 normal = normalize(texture(gNormalTex, uv0).xyz * 2.0f - vec3(1, 1, 1));
				vec3 worldNormal = calcWorldNormal(tangentToWorldZ, tangentToWorldX, normal);
			
				SurfaceData surfaceData;
				surfaceData.albedo = texture(gAlbedoTex, uv0);
				surfaceData.worldNormal.xyz = worldNormal;
				surfaceData.roughness = texture(gRoughnessTex, uv0).x;
				surfaceData.metalness = texture(gMetalnessTex, uv0).x;
				
				// Directional lights
				vec3 lightAccumulator = vec3(0, 0, 0);
				for(uint i = 0; i < gLightOffsets[0]; ++i)
				{
					LightData lightData = gLightsData[i];
					lightAccumulator += getDirLightContibution(surfaceData, lightData);
				}
				
				uvec2 pixelPos = uvec2(gl_FragCoord.xy);
				int cellIdx = calcCellIdx(pixelPos, gl_FragCoord.z);
				uvec3 offsetAndSize = texelFetch(gGridOffsetsAndSize, cellIdx).xyz;
				
				// Radial lights
				int i = int(offsetAndSize.x);
				uint end = offsetAndSize.x + offsetAndSize.y;
				for(; i < end; i++)
				{
					uint lightIndex = texelFetch(gGridLightIndices, i).x;
					LightData lightData = gLightsData[lightIndex];
					
					lightAccumulator += getPointLightContribution(worldPosition, surfaceData, lightData);
				}
				
				// Spot lights
				end += offsetAndSize.z;
				for(; i < end; i++)
				{
					uint lightIndex = texelFetch(gGridLightIndices, i).x;
					LightData lightData = gLightsData[lightIndex];
					
					lightAccumulator += getSpotLightContribution(worldPosition, surfaceData, lightData);
				}
				
				lightAccumulator += surfaceData.albedo.xyz * gAmbientFactor;
				
				vec3 diffuse = surfaceData.albedo.xyz / PI; // TODO - Add better lighting model later
				fragColor = vec4(diffuse * lightAccumulator, gOpacity); 
			}	
		};
	};
};