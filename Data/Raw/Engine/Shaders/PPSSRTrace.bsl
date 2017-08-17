#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\GBufferInput.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"
#include "$ENGINE$\ImportanceSampling.bslinc"
#include "$ENGINE$\ColorSpace.bslinc"

#define HI_Z 1
#include "$ENGINE$\RayMarch.bslinc"

technique PPSSRTrace
{
	mixin PPBase;
	mixin PerCameraData;
	mixin GBufferInput;
	mixin RayMarch;
	mixin ImportanceSampling;
	mixin ColorSpace;

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
			float gIntensity;
			float2 gRoughnessScaleBias;
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
			float3 V = normalize(gViewOrigin - P);
			float3 N = surfData.worldNormal.xyz;
			
			float roughness = surfData.roughness;
			
			
			//roughness = 0.3f;//DEBUG ONLY
			roughness = 0.0f;
			
			
			float roughness2 = roughness * roughness;
			float roughness4 = roughness2 * roughness2;
			
			// TODO - DEBUG ONLY - Only handle reflections on up facing surfaces
			if(dot(N, float3(0,1,0)) < 0.8)
				return gSceneColor.Sample(gSceneColorSamp, input.uv0);	
			else
				N = float3(0,1,0);

			// Jitter ray offset in 4x4 tile, in order to avoid stairstep artifacts
			uint pixelIdx = mortonCode4x4((uint)pixelPos.x, (uint)pixelPos.y);
			float jitterOffset = (pixelIdx & 15) / 15.0f - 0.5f; // TODO - Also add per-frame jitter			
			
			RayMarchParams rayMarchParams;
			rayMarchParams.bufferSize = gHiZSize;
			rayMarchParams.numMips = gHiZNumMips;
			rayMarchParams.NDCToHiZUV = gNDCToHiZUV;
			rayMarchParams.HiZUVToScreenUV = gHiZUVToScreenUV;
			rayMarchParams.rayOrigin = P;
			rayMarchParams.jitterOffset = jitterOffset;			
			
			int NUM_RAYS = 1; // DEBUG ONLY
			
			float4 sum = 0;
			[loop]
			for(int i = 0; i < NUM_RAYS; ++i)
			{
				// TODO - Add per-frame random? (for temporal filtering)
				float2 random = hammersleySequence(i, NUM_RAYS);
				float2 sphericalH = importanceSampleGGX(random, roughness4);
				
				float cosTheta = sphericalH.x;
				float phi = sphericalH.y;
				float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
				
				float3 H = sphericalToCartesian(cosTheta, sinTheta, phi);

				// Transform H to world space
				float3 up = abs(H.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
				float3 tangentX = normalize(cross(up, N));
				float3 tangentY = cross(N, tangentX);
				
				H = tangentX * H.x + tangentY * H.y + N * H.z; 
				//float3 R = 2 * dot( V, H ) * H - V;
				float3 R = 2 * dot( V, N ) * N - V;
				
				// Eliminate rays pointing towards the viewer. They won't hit anything, plus they can screw up precision
				// and cause ray step offset to be too small, causing self-intersections.
				R = normalize(R); // Note: Normalization required?
				float dirFade = saturate(dot(R, gViewDir) * 10);
				
				if(dirFade < 0.00001f)
					continue;
				
				// Eliminate rays pointing below the surface
				if(dot(R, N) < 0.0005f)
					continue;
					
				rayMarchParams.rayDir = R;

				float4 rayHit = rayMarch(gHiZ, gDepthBufferSamp, rayMarchParams);
				if(rayHit.w < 1.0f) // Hit
				{
					float4 color = gSceneColor.Sample(gSceneColorSamp, rayHit.xy);
					color.a = 1.0f; // Marks the pixel as SSR
					
					// Fade out near screen edges
					float2 rayHitNDC = UVToNDC(rayHit.xy);
					float2 vignette = saturate(abs(rayHitNDC) * 5.0f - 4.0f);
		
					color = color * saturate(1.0f - dot(vignette, vignette));
					
					// Note: Not accounting for PDF here since we don't evaluate BRDF until later. Looks good though.
					
					// Tonemap the color to get a nicer visual average
					color.rgb /= (1 + LuminanceRGB(color.rgb));
					
					sum += color * dirFade;
				}
			}
			
			// Divide by total number of rays, instead of actual number of rays that passed the test. This scales down the
			// contribution for pixels for which many rays failed the test and might not be accurate.
			float4 output = sum / NUM_RAYS; 
			
			// Move back to high range (reverse tonemap)
			output.rgb /= (1 - LuminanceRGB(output.rgb));
			
			// Fade based on roughness
			float fadeValue = 1.0f - saturate(surfData.roughness * gRoughnessScaleBias.x + gRoughnessScaleBias.y);
			
			output *= fadeValue;
			output *= gIntensity;
			
			return output;
		}	
	};
};