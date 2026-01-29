#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\GBufferInput.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"
#include "$ENGINE$\ImportanceSampling.bslinc"
#include "$ENGINE$\ColorSpace.bslinc"

#define HI_Z 1
#include "$ENGINE$\RayMarch.bslinc"

shader PPSSRTrace
{
	mixin PPBase;
	mixin PerCameraData;
	mixin GBufferInput;
	mixin RayMarch;
	mixin ImportanceSampling;
	mixin ColorSpace;

	variations
	{
		MSAA_COUNT = { 1, 2 };
		QUALITY = { 0, 1, 2, 3, 4 };
		MSAA_RESOLVE_0TH = { true, false };
	};	
	
	stencil
	{
		enabled = true;
		reference = 0;
		front = { keep, keep, keep, eq };
		readmask = 0x7F;
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
			int gTemporalJitter;
		}
		
		#if QUALITY == 0
			#define NUM_RAYS 1
		#elif QUALITY == 1
			#define NUM_RAYS 4
		#elif QUALITY == 2
			#define NUM_RAYS 8
		#elif QUALITY == 3
			#define NUM_RAYS 12
		#else
			#define NUM_RAYS 16
		#endif
		
		Texture2D gSceneColor;
		SamplerState gSceneColorSamp;
		
		Texture2D gHiZ;
		SamplerState gHiZSamp;
		
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

		float4 fsmain(VStoFS input, float4 pixelPos : SV_Position
			#if MSAA_COUNT > 1 && !MSAA_RESOLVE_0TH
			, uint sampleIdx : SV_SampleIndex
			#endif
		) : SV_Target0
		{
			#if MSAA_RESOLVE_0TH
			uint sampleIdx = 0;
			#endif
		
			#if MSAA_COUNT > 1
			SurfaceData surfData = getGBufferData(trunc(input.uv0.xy), sampleIdx);
			#else
			SurfaceData surfData = getGBufferData(input.uv0);
			#endif
			
			float3 P = NDCToWorld(input.screenPos, surfData.depth);
			float3 V = normalize(gViewOrigin - P);
			float3 N = surfData.worldNormal.xyz;
			
			float roughness = surfData.roughness;
			
			float roughness2 = roughness * roughness;
			float roughness4 = roughness2 * roughness2;
			
			// Jitter ray offset in 4x4 tile, in order to avoid stairstep artifacts
			uint pixelIdx = mortonCode4x4((uint)pixelPos.x, (uint)pixelPos.y);
			
			RayMarchParams rayMarchParams;
			rayMarchParams.bufferSize = gHiZSize;
			rayMarchParams.numMips = gHiZNumMips;
			rayMarchParams.NDCToHiZUV = gNDCToHiZUV;
			rayMarchParams.HiZUVToScreenUV = gHiZUVToScreenUV;
			rayMarchParams.rayOrigin = P;

			// Make sure each pixel chooses different ray directions (noise looks better than repeating patterns)
			//// Magic integer is arbitrary, in order to convert from [0, 1] float
			uint2 pixRandom = random(pixelPos.xy + gTemporalJitter * uint2(61, 85)) * uint2(0x36174842, 0x15249835);

			float4 sum = 0;
			[loop]
			for(int i = 0; i < NUM_RAYS; ++i)
			{
				uint rayRandom = (pixelIdx + (gTemporalJitter + i * 207) & 15);
				rayMarchParams.jitterOffset = rayRandom / 15.0f - 0.5f;
			
				float2 e = hammersleySequence(i, NUM_RAYS, pixRandom);
				float2 sphericalH = importanceSampleGGX(e, roughness4);
				
				float cosTheta = sphericalH.x;
				float phi = sphericalH.y;
				float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
				
				float3 H = sphericalToCartesian(cosTheta, sinTheta, phi);

				// Transform H to world space
				float3 up = abs(H.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
				float3 tangentX = normalize(cross(up, N));
				float3 tangentY = cross(N, tangentX);
				
				H = tangentX * H.x + tangentY * H.y + N * H.z; 
				float3 R = 2 * dot( V, H ) * H - V;
				
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

				float4 rayHit = rayMarch(gHiZ, gHiZSamp, rayMarchParams);
				if(rayHit.w < 1.0f) // Hit
				{
					float4 color = gSceneColor.Sample(gSceneColorSamp, rayHit.xy);
					color.a = 1.0f; // Marks the pixel as SSR
					
					// Fade out near screen edges
					float2 rayHitNDC = UVToNDC(rayHit.xy);
					float2 vignette = saturate(abs(rayHitNDC) * 5.0f - 4.0f);
		
					color = color * saturate(1.0f - dot(vignette, vignette));
					
					// Tonemap the color to get a nicer visual average
					color.rgb /= (1 + LuminanceRGB(color.rgb));
					
					sum += color * dirFade;
				}
			}

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
