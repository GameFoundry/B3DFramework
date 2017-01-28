#include "$ENGINE$\GBuffer.bslinc"
#include "$ENGINE$\LightingCommon.bslinc"

Parameters =
{
	Sampler2D 	gGBufferASamp : alias("gGBufferATex");
	Sampler2D 	gGBufferBSamp : alias("gGBufferBTex");
	Sampler2D 	gDepthBufferSamp : alias("gDepthBufferTex");
	
	Texture2D 	gGBufferATex : auto("GBufferA");
	Texture2D	gGBufferBTex : auto("GBufferB");
	Texture2D 	gDepthBufferTex : auto("GBufferDepth");
};

Technique 
  : inherits("GBuffer")
  : inherits("LightingCommon") =
{
	Language = "HLSL11";
	
	Pass =
	{
		Compute = 
		{
			SamplerState 	gGBufferASamp : register(s0);
			SamplerState 	gGBufferBSamp : register(s1);
			SamplerState 	gDepthBufferSamp : register(s2);
	
			Texture2D 		gGBufferATex : register(t0);
			Texture2D		gGBufferBTex : register(t1);
			Texture2D 		gDepthBufferTex : register(t2);
			
			SurfaceData decodeGBuffer(float4 GBufferAData, float4 GBufferBData, float deviceZ)
			{
				SurfaceData output;
				
				output.albedo.xyz = GBufferAData.xyz;
				output.albedo.w = 1.0f;
				output.worldNormal = GBufferBData * float4(2, 2, 2, 1) - float4(1, 1, 1, 0);
				output.worldNormal.xyz = normalize(output.worldNormal.xyz);
				output.depth = convertFromDeviceZ(deviceZ);
				
				return output;
			}			
			
			SurfaceData getGBufferData(float2 uv)
			{
				float4 GBufferAData = gGBufferATex.SampleLevel(gGBufferASamp, uv, 0);
				float4 GBufferBData = gGBufferBTex.SampleLevel(gGBufferBSamp, uv, 0);
				float deviceZ = gDepthBufferTex.SampleLevel(gDepthBufferSamp, uv, 0).r;
				
				return decodeGBuffer(GBufferAData, GBufferBData, deviceZ);
			}
			
			SurfaceData getGBufferData(uint2 pixelPos)
			{
				float4 GBufferAData = gGBufferATex.Load(int3(pixelPos, 0));
				float4 GBufferBData = gGBufferBTex.Load(int3(pixelPos, 0));
				float deviceZ = gDepthBufferTex.Load(int3(pixelPos, 0)).r;
				
				return decodeGBuffer(GBufferAData, GBufferBData, deviceZ);
			}	
			
			StructuredBuffer<LightData> gDirLights : register(t3);
			StructuredBuffer<LightData> gPointLights : register(t4);
			StructuredBuffer<LightData> gSpotLights  : register(t5);				
		
			RWTexture2D<float4>			gOutput : register(u0);
		
			cbuffer Params : register(b0)
			{
				// x - directional, y - point, z - spot
				uint3 gNumLightsPerType;
			}
		
			[numthreads(TILE_SIZE, TILE_SIZE, 1)]
			void main(
				uint3 groupId : SV_GroupID,
				uint3 groupThreadId : SV_GroupThreadID,
				uint3 dispatchThreadId : SV_DispatchThreadID,
				uint threadIndex : SV_GroupIndex)
			{
				uint2 pixelPos = dispatchThreadId.xy + gViewportRectangle.xy;
				SurfaceData surfaceData = getGBufferData(pixelPos);

				float3 lightAccumulator = 0;
				float alpha = 0.0f;
				if(surfaceData.worldNormal.w > 0.0f)
				{
					float2 screenUv = ((float2)(gViewportRectangle.xy + pixelPos) + 0.5f) / (float2)gViewportRectangle.zw;
					float2 clipSpacePos = (screenUv - gClipToUVScaleOffset.zw) / gClipToUVScaleOffset.xy;
				
					// x, y are now in clip space, z, w are in view space
					// We multiply them by a special inverse view-projection matrix, that had the projection entries that effect
					// z, w eliminated (since they are already in view space)
					// Note: Multiply by depth should be avoided if using ortographic projection
					float4 mixedSpacePos = float4(clipSpacePos.xy * -surfaceData.depth, surfaceData.depth, 1);
					float4 worldPosition4D = mul(gMatScreenToWorld, mixedSpacePos);
					float3 worldPosition = worldPosition4D.xyz / worldPosition4D.w;
					
					for(uint i = 0; i < gNumLightsPerType.x; i++)
						lightAccumulator += getDirLightContibution(surfaceData, gDirLights[i]);
					
					for(uint i = 0; i < gNumLightsPerType.y; i++)
						lightAccumulator += getPointLightContribution(worldPosition, surfaceData, gPointLights[i]);
					
					for(uint i = 0; i < gNumLightsPerType.z; i++)
						lightAccumulator += getSpotLightContribution(worldPosition, surfaceData, gSpotLights[i]);
						
					alpha = 1.0f;
				}
				
				float3 diffuse = surfaceData.albedo.xyz / PI; // TODO - Add better lighting model later
				uint2 viewportMax = gViewportRectangle.xy + gViewportRectangle.zw;

				// Ignore pixels out of valid range
				if (all(dispatchThreadId.xy < viewportMax)) 
					gOutput[pixelPos] = float4(gOutput[pixelPos].xyz + diffuse * lightAccumulator, alpha);
			}
		};
	};
};

Technique 
  : inherits("GBuffer")
  : inherits("LightingCommon") =
{
	Language = "GLSL";
	
	Pass =
	{
		Compute = 
		{
			layout (local_size_x = TILE_SIZE, local_size_y = TILE_SIZE) in;
		
			layout(binding = 0) uniform sampler2D gGBufferATex;
			layout(binding = 1) uniform sampler2D gGBufferBTex;
			layout(binding = 2) uniform sampler2D gDepthBufferTex;
			
			SurfaceData decodeGBuffer(vec4 GBufferAData, vec4 GBufferBData, float deviceZ)
			{
				SurfaceData surfaceData;
				
				surfaceData.albedo.xyz = GBufferAData.xyz;
				surfaceData.albedo.w = 1.0f;
				surfaceData.worldNormal = GBufferBData * vec4(2, 2, 2, 1) - vec4(1, 1, 1, 0);
				surfaceData.worldNormal.xyz = normalize(surfaceData.worldNormal.xyz);
				surfaceData.depth = convertFromDeviceZ(deviceZ);
				
				return surfaceData;
			}			
			
			SurfaceData getGBufferData(vec2 uv)
			{
				vec4 GBufferAData = textureLod(gGBufferATex, uv, 0);
				vec4 GBufferBData = textureLod(gGBufferBTex, uv, 0);
				float deviceZ = textureLod(gDepthBufferTex, uv, 0).r;
				
				return decodeGBuffer(GBufferAData, GBufferBData, deviceZ);
			}	
			
			SurfaceData getGBufferData(ivec2 pixelPos)
			{
				vec4 GBufferAData = texelFetch(gGBufferATex, pixelPos, 0);
				vec4 GBufferBData = texelFetch(gGBufferBTex, pixelPos, 0);
				float deviceZ = texelFetch(gDepthBufferTex, pixelPos, 0).r;
				
				return decodeGBuffer(GBufferAData, GBufferBData, deviceZ);
			}	
			
			layout(std430, binding = 3) buffer gDirLights
			{
				LightData[] gDirLightsData;
			};
			
			layout(std430, binding = 4) buffer gPointLights
			{
				LightData[] gPointLightsData;
			};
			
			layout(std430, binding = 5) buffer gSpotLights
			{
				LightData[] gSpotLightsData;
			};	
			
			layout(binding = 6, rgba16f) uniform image2D gOutput;
			
			layout(binding = 7, std140) uniform Params
			{
				// x - directional, y - point, z - spot
				uvec3 gNumLightsPerType;
			};
			
			void main()
			{
				ivec2 pixelPos = ivec2(gl_GlobalInvocationID.xy) + gViewportRectangle.xy;
				SurfaceData surfaceData = getGBufferData(pixelPos);

				float alpha = 0.0f;
				vec3 lightAccumulator = vec3(0, 0, 0);
				if(surfaceData.worldNormal.w > 0.0f)
				{
					vec2 screenUv = (vec2(gViewportRectangle.xy + pixelPos) + 0.5f) / vec2(gViewportRectangle.zw);
					vec2 clipSpacePos = (screenUv - gClipToUVScaleOffset.zw) / gClipToUVScaleOffset.xy;
				
					// x, y are now in clip space, z, w are in view space
					// We multiply them by a special inverse view-projection matrix, that had the projection entries that effect
					// z, w eliminated (since they are already in view space)
					// Note: Multiply by depth should be avoided if using ortographic projection
					vec4 mixedSpacePos = vec4(clipSpacePos.xy * -surfaceData.depth, surfaceData.depth, 1);
					vec4 worldPosition4D = gMatScreenToWorld * mixedSpacePos;
					vec3 worldPosition = worldPosition4D.xyz / worldPosition4D.w;
					
					for(uint i = 0; i < gNumLightsPerType.x; i++)
						lightAccumulator += getDirLightContibution(surfaceData, gDirLightsData[i]);
					
					for(uint i = 0; i < gNumLightsPerType.y; i++)
						lightAccumulator += getPointLightContribution(worldPosition, surfaceData, gPointLightsData[i]);
					
					for(uint i = 0; i < gNumLightsPerType.z; i++)
						lightAccumulator += getSpotLightContribution(worldPosition, surfaceData, gSpotLightsData[i]);
						
					alpha = 1.0f;
				}
				
				vec3 diffuse = surfaceData.albedo.xyz / PI; // TODO - Add better lighting model later
				
				uvec2 viewportMax = gViewportRectangle.xy + gViewportRectangle.zw;

				// Ignore pixels out of valid range
				if (all(lessThan(gl_GlobalInvocationID.xy, viewportMax))) 
				{
					vec4 existingValue = imageLoad(gOutput, pixelPos);
					imageStore(gOutput, pixelPos, vec4(diffuse * lightAccumulator + existingValue.xyz, alpha));
				}
			}
		};
	};
};