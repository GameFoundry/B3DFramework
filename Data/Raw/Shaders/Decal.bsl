#define CLIP_POS 1
#include "$ENGINE$\BasePass.bslinc"
#include "$ENGINE$\GBufferOutput.bslinc"
#include "$ENGINE$\DepthInput.bslinc"

shader Surface
{
	mixin BasePass;
	mixin GBufferOutput;
	mixin DepthInput;

	variations
	{
		// 0 - Transparent
		// 1 - Stain
		// 2 - Normal
		// 3 - Emissive
		BLEND_MODE = { 0, 1, 2, 3 };
	};
	
	blend
	{
		// Scene color
		target	
		{
			enabled = true;
			color = { srcA, srcIA, add };
			
			#if BLEND_MODE == 3
				writemask = empty;
			#endif
		};
		
		// Albedo
		target	
		{
			enabled = true;
			
			#if BLEND_MODE == 1
				color = { dstRGB, zero, add };
			#else
				color = { srcA, srcIA, add };
			#endif
			
			#if BLEND_MODE != 0
			#if BLEND_MODE != 1
				writemask = empty;
			#endif
			#endif
		};
		
		// Normal
		target	
		{
			enabled = true;
			color = { srcA, srcIA, add };
			
			#if BLEND_MODE == 3
				writemask = empty;
			#endif
		};
		
		// Metallic & roughness
		target	
		{
			enabled = true;
			color = { srcA, srcIA, add };
			
			#if BLEND_MODE != 0
			#if BLEND_MODE != 1
				writemask = empty;
			#endif
			#endif
		};
	};	
	
	depth
	{
		write = false;
	};
	
	code
	{
		#ifndef MSAA_COUNT
			#define MSAA_COUNT 1
		#endif
	
		[alias(gOpacityTex)]
		SamplerState gOpacitySamp;	
	
		Texture2D gOpacityTex = white;
	
		#if BLEND_MODE == 3
			[alias(gEmissiveMaskTex)]
			SamplerState gEmissiveMaskSamp;	
		
			Texture2D gEmissiveMaskTex = black;
		#endif
		
		#if BLEND_MODE != 3
			[alias(gNormalTex)]
			SamplerState gNormalSamp;
			
			Texture2D gNormalTex = normal;
		#endif
	
		#if BLEND_MODE == 0 || BLEND_MODE == 1
			[alias(gAlbedoTex)]
			SamplerState gAlbedoSamp;
		
			[alias(gRoughnessTex)]
			SamplerState gRoughnessSamp;
			
			[alias(gMetalnessTex)]
			SamplerState gMetalnessSamp;
		
			Texture2D gAlbedoTex = white;
			Texture2D gRoughnessTex = white;
			Texture2D gMetalnessTex = black;
		#endif

		cbuffer MaterialParams
		{
			float2 gUVOffset = { 0.0f, 0.0f };
			float2 gUVTile = { 1.0f, 1.0f };
			
			#if BLEND_MODE == 3
				[color]
				float3 gEmissiveColor = { 1.0f, 1.0f, 1.0f };
			#endif
			
			#if BLEND_MODE == 0 || BLEND_MODE == 1
				[spriteuv(gAlbedoTex)]
				float4 gSpriteUV;
			#endif
		};
	
		[internal]
		cbuffer DecalParams
		{
			float4x4 gWorldToDecal;
			float3 gDecalNormal;
			float gNormalTolerance;
		}
		
		float3x3 getWorldToTangent(float3 N, float3 p, float2 uv)
		{
			float3 dp1 = ddx(p);
			float3 dp2 = ddy(p);
			float2 duv1 = ddx(uv);
			float2 duv2 = ddy(uv);
			
			float3 dp2perp = cross(dp2, N);
			float3 dp1perp = cross(N, dp1);
			float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
			float3 B = dp2perp * duv1.y + dp1perp * duv2.y;
		 
			float invmax = rsqrt(max(dot(T,T), dot(B,B)));
			return float3x3(T * invmax, B * invmax, N);
		}
		
		void fsmain(
			in VStoFS input, 
			in float4 screenPos : SV_Position,
			out float4 OutSceneColor : SV_Target0,
			out float4 OutGBufferA : SV_Target1,
			out float4 OutGBufferB : SV_Target2,
			out float2 OutGBufferC : SV_Target3)
		{		
			#if MSAA_COUNT > 1
				// Note: Always sampling from 0th MSAA sample, could impact quality/aliasing
				float deviceZ = gDepthBufferTex.Load(screenPos.xy, 0).r;
			#else
				float deviceZ = gDepthBufferTex.Load(int3(screenPos.xy, 0)).r;
			#endif	
		
			float depth = convertFromDeviceZ(deviceZ);
			float2 ndcPos = input.clipPos.xy / input.clipPos.w;
		
			// x, y are now in clip space, z, w are in view space
			// We multiply them by a special inverse view-projection matrix, that had the projection entries that effect
			// z, w eliminated (since they are already in view space)
			// Note: Multiply by depth should be avoided if using ortographic projection
			float4 mixedSpacePos = float4(ndcPos.xy * -depth, depth, 1);
			float4 worldPosition4D = mul(gMatScreenToWorld, mixedSpacePos);
			float3 worldPosition = worldPosition4D.xyz / worldPosition4D.w;
			
			float4 decalPos = mul(gWorldToDecal, float4(worldPosition, 1.0f));
			float3 decalUV = (decalPos.xyz + 1.0f) * 0.5f;
						
			float alpha = 0.0f;
			if(any(decalUV < 0.0f) || any(decalUV > 1.0f))
				discard;
				
			float3 worldNormal = normalize(cross(ddy(worldPosition), ddx(worldPosition)));
			if(dot(worldNormal, gDecalNormal) > gNormalTolerance)
				discard;
				
			float3x3 worldToTangent = getWorldToTangent(worldNormal, worldPosition, decalUV.xy);
				
			float2 uv = (decalUV.xy * gUVTile + gUVOffset) * gSpriteUV.zw + gSpriteUV.xy;
			float opacity = gOpacityTex.Sample(gOpacitySamp, uv);
				
			#if BLEND_MODE == 3
				OutSceneColor = float4(gEmissiveColor * gEmissiveMaskTex.Sample(gEmissiveMaskSamp, uv).x, opacity);
			#elif BLEND_MODE == 2
				float3 normal = normalize(gNormalTex.Sample(gNormalSamp, uv) * 2.0f - float3(1, 1, 1));
				
				// Flip multiplication order since we need to transform with tangentToWorld, which is the transpose
				worldNormal = mul(normal, worldToTangent);
				OutGBufferB = float4(worldNormal * 0.5f + 0.5f, opacity);
			#else
				OutGBufferA = float4(gAlbedoTex.Sample(gAlbedoSamp, uv).xyz, opacity);
				
				float3 normal = normalize(gNormalTex.Sample(gNormalSamp, uv) * 2.0f - float3(1, 1, 1));
				
				// Flip multiplication order since we need to transform with tangentToWorld, which is the transpose
				worldNormal = mul(normal, worldToTangent);
				OutGBufferB = float4(worldNormal * 0.5f + 0.5f, opacity);
				
				float roughness = gRoughnessTex.Sample(gRoughnessSamp, uv).x;
				float metalness = gMetalnessTex.Sample(gMetalnessSamp, uv).x;
				
				OutGBufferC = float4(roughness, metalness, 0.0f, opacity);
			#endif
		}	
	};
};