#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\GBufferInput.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"

#define HI_Z 1
#include "$ENGINE$\RayMarch.bslinc"

technique PPSSRTrace
{
	mixin PPBase;
	mixin PerCameraData;
	mixin GBufferInput;
	mixin RayMarch;

	stencil
	{
		enabled = true;
		reference = 0;
		front = { keep, keep, keep, eq };
	};	
	
	code
	{
		[internal]
		cbuffer Input
		{
			float4 gNDCToHiZUV;
			float2 gHiZUVToScreenUV;
			int2 gHiZSize;
			int gHiZNumMips;
		}
		
		Texture2D gSceneColor;
		SamplerState gSceneColorSamp;
		
		Texture2D gHiZ;
		
		float random (float2 st) 
		{
			// From https://thebookofshaders.com/10/
			return frac(sin(dot(st.xy, float2(12.9898, 78.233))) * 43758.5453123);
		}
		
		// Specialized morton code for 4x4 tiles
		uint mortonCode4x4(uint x, uint y)
		{
			return (x & 0x1) 		| ((x << 1) & 0x4)
				 | (y << 1) & 0x2 	| ((y << 2) & 0x8);
		}		

		float4 fsmain(VStoFS input, float4 pixelPos : SV_Position) : SV_Target0
		{
			// TODO - Support MSAA?
		
			SurfaceData surfData = getGBufferData(input.uv0);
			float3 P = NDCToWorld(input.screenPos, surfData.depth);
			float3 V = normalize(P - gViewOrigin);
			float3 N = surfData.worldNormal.xyz;
			
			// TODO - DEBUG ONLY - Only handle reflections on up facing surfaces
			if(dot(N, float3(0,1,0)) < 0.8)
				return gSceneColor.Sample(gSceneColorSamp, input.uv0);	
			else
				N = float3(0,1,0);
			
			// TODO - Allow number of steps and rays be customized using a quality level
			//  - And HiZ vs linear search
			
			// TODO - Use Hammersley + random to generate ray directions based on GGX BRDF
			//  - Clip BRDF lobe? And renormalize PDF?
			// TODO - Reject rays pointing under the surface
			
			// Eliminate rays pointing towards the viewer. They won't hit anything, plus they can screw up precision
			// and cause ray step offset to be too small, causing self-intersections.
			float3 R = normalize(reflect(V, N));
			if(dot(R, gViewDir) < 0.0f)
				return 0.0f;
			
			// Jitter ray offset in 4x4 tile, in order to avoid stairstep artifacts
			uint pixelIdx = mortonCode4x4((uint)pixelPos.x, (uint)pixelPos.y);
			float jitterOffset = (pixelIdx & 15) / 15.0f - 0.5f; // TODO - Also add per-frame jitter
			
			RayMarchParams rayMarchParams;
			rayMarchParams.bufferSize = gHiZSize;
			rayMarchParams.numMips = gHiZNumMips;
			rayMarchParams.NDCToHiZUV = gNDCToHiZUV;
			rayMarchParams.HiZUVToScreenUV = gHiZUVToScreenUV;
			rayMarchParams.rayOrigin = P;
			rayMarchParams.rayDir = R;
			rayMarchParams.jitterOffset = jitterOffset;
			
			// TODO - Fade based on roughness
			
			float4 rayHit = rayMarch(gHiZ, gDepthBufferSamp, rayMarchParams);
			if(rayHit.w < 1.0f) // Hit
			{
				float4 output = gSceneColor.Sample(gSceneColorSamp, rayHit.xy);
				
				// Fade out near screen edges
				float2 rayHitNDC = UVToNDC(rayHit.xy);
				float2 vignette = saturate(abs(rayHitNDC) * 5.0f - 4.0f);
	
				return output * (1.0f - dot(vignette, vignette));
			}
			
			return 0.0f;
		}	
	};
};