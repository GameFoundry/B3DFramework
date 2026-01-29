#include "$ENGINE$/GBufferInput.bslinc"
#include "$ENGINE$/ShadowProjectionCommon.bslinc"

shader ShadowProject
{
	mixin GBufferInput;
	mixin ShadowProjectionCommon;

	variations
	{
		SHADOW_QUALITY = { 1, 2, 3, 4 };
		CASCADING = { true, false };
		NEEDS_TRANSFORM = { true, false };
		MSAA_COUNT = { 1, 2 };
	};			
	
	depth
	{
		read = false;
		write = false;
	};
	
	stencil
	{
		enabled = true;
		
		// This clears the stencil at the same time as performing the test
		// Note: Need to test performance clearing the stencil this way vs. clearing it separately,
		//   as this disables HiStencil optimization.
		front = { zero, zero, zero, neq };
		readmask = 0x7F;
		writemask = 0x7F;
	};
	
	#if CASCADING
	blend
	{
		target
		{
			enabled = true;
			writemask = R;
			color = { srcA, srcIA, add };
		};
	};
	#else
	blend
	{
		target
		{
			enabled = true;
			writemask = R;
			color = { one, one, max };
		};
	};	
	#endif
	
	code
	{
		#if CASCADING
		Texture2DArray gShadowTex;
		#else
		Texture2D gShadowTex;
		#endif
		
		SamplerState gShadowSampler;
	
		[internal]
		cbuffer Params
		{
			// Transform a point in mixed space (xy - clip space, z - view space) to a point
			// in shadow space
			float4x4 gMixedToShadowSpace;
			float2 gShadowMapSize;
			float2 gShadowMapSizeInv;
			float gSoftTransitionScale;
			float gFadePercent;
			
			float gFadePlaneDepth;
			float gInvFadePlaneRange;
			
			float gFace;
		};
		
		// Converts a set of shadow depths into occlusion values, where 1 means scene object is occluded and 0
		// not occluded. Values between 1 and 0 are used for soft transitions on receivers that are near casters.
		float4 getOcclusion(float4 shadowDepth, float sceneDepth)
		{
			// Offset the shadow a bit to reduce shadow acne and use scale for soft transitions.
			// Visualization (Mathematica): Plot[1.0 - Clip[(500 - x)*0.5 + 1, {0, 1}], {x, 480, 520}]
			return 1.0f - saturate((shadowDepth - sceneDepth) * gSoftTransitionScale + 1);
		}
		
		// Takes UV coordinates as input and returns a location to sample from, and a fraction
		// that can be used for bilinear interpolation between the samples. Returned sample
		// center is always located on a border between texels, in UV space.
		float2 getFilteringInfo(float2 uv, out float2 fraction)
		{
			// UV to texel position
			float2 texelXY = uv * gShadowMapSize;

			// -0.5f offset because UV (0, 0) maps to (-0.5, -0.5) texel position
			texelXY -= 0.5f;

			// Get fraction to use for interpolation
			fraction = frac(texelXY);

			// Get center location to gather from (in UV coordinates)
			return (floor(texelXY) + 0.5f)  * gShadowMapSizeInv;
		}
		
		float PCF1x1(float2 uv, float sceneDepth)
		{
			#if CASCADING
			float3 sampleCenter = float3(uv, gFace);
			#else
			float2 sampleCenter = uv;
			#endif
			
			float depthSample = gShadowTex.Sample(gShadowSampler, sampleCenter).r;
			return getOcclusion(depthSample.rrrr, sceneDepth).r;
		}
	
		float PCF2x2(float2 uv, float sceneDepth)
		{
			float2 fraction;
			#if CASCADING
			float3 sampleCenter = float3(getFilteringInfo(uv, fraction), gFace);
			#else
			float2 sampleCenter = getFilteringInfo(uv, fraction);
			#endif			
				
			// Gather four samples. Samples are returned in counter-clockwise order, starting with lower left
			float4 depthSamples = gShadowTex.GatherRed(gShadowSampler, sampleCenter);
			
			// Convert samples to occlusion
			float4 occlusion = getOcclusion(depthSamples, sceneDepth);
			
			// Perform bilinear interpolation
			float2 lerpHorz = lerp(occlusion.wx, occlusion.zy, fraction.xx);
			return lerp(lerpHorz.x, lerpHorz.y, fraction.y);				
		}
		
		float PCF4x4(float2 uv, float sceneDepth)
		{
			float2 fraction;
			#if CASCADING
			float3 sampleCenter = float3(getFilteringInfo(uv, fraction), gFace);
			#else
			float2 sampleCenter = getFilteringInfo(uv, fraction);
			#endif	
							
			// Gather 16 samples in four 2x2 gathers. Samples are returned in counter-clockwise order, starting with lower left.
			// Gathers are performed in clockwise order, starting with top left block.
			float4 topLeftSamples = gShadowTex.GatherRed(gShadowSampler, sampleCenter, int2(-1, -1));
			float4 topRightSamples = gShadowTex.GatherRed(gShadowSampler, sampleCenter, int2(1, -1));
			float4 botLeftSamples = gShadowTex.GatherRed(gShadowSampler, sampleCenter, int2(-1, 1));
			float4 botRightSamples = gShadowTex.GatherRed(gShadowSampler, sampleCenter, int2(1, 1));
			
			// Convert samples to occlusion
			float4 topLeftOcclusion = getOcclusion(topLeftSamples, sceneDepth);
			float4 topRightOcclusion = getOcclusion(topRightSamples, sceneDepth);
			float4 botLeftOcclusion = getOcclusion(botLeftSamples, sceneDepth);
			float4 botRightOcclusion = getOcclusion(botRightSamples, sceneDepth);
			
			// Get the average occusion value. Fraction only needs to be applied to edge samples.
			//// Acculate occlusion per row
			float4 rowOcclusion;
			
			//// Add column 1, top to bottom
			rowOcclusion.x = topLeftOcclusion.w * (1.0f - fraction.x);
			rowOcclusion.y = topLeftOcclusion.x * (1.0f - fraction.x);
			rowOcclusion.z = botLeftOcclusion.w * (1.0f - fraction.x);
			rowOcclusion.w = botLeftOcclusion.x * (1.0f - fraction.x);
			
			//// Add column 2 & 3, top to bottom
			rowOcclusion.x += topLeftOcclusion.z + topRightOcclusion.w;
			rowOcclusion.y += topLeftOcclusion.y + topRightOcclusion.x;
			rowOcclusion.z += botLeftOcclusion.z + botRightOcclusion.w;
			rowOcclusion.w += botLeftOcclusion.y + botRightOcclusion.x;
			
			//// Add column 4, top to bottom
			rowOcclusion.x += topRightOcclusion.z * fraction.x;
			rowOcclusion.y += topRightOcclusion.y * fraction.x;
			rowOcclusion.z += botRightOcclusion.z * fraction.x;
			rowOcclusion.w += botRightOcclusion.w * fraction.x;
			
			//// Accumulate occlusion per columns
			float4 occlusionAccumulator = dot(rowOcclusion, float4(1.0f - fraction.y, 1.0f, 1.0f, fraction.y));
			
			// Calc average occlusion using a 3x3 area and return
			return occlusionAccumulator * (1.0f / 9.0f);				
		}
		
		// Accumulates samples for all columns in a row, for 6x2 samples. Samples are provided in three 2x2
		// blocks. Samples in a block are in counter-clockwise order, starting with lower left. Returns two
		// rows with their accumulated values, starting with top row.
		float2 accumulateRows6x2(float fraction, float4 left, float4 mid, float4 right)
		{
			float2 row;
			
			// Column 1, top to bottom
			row.x = left.w * (1.0f - fraction);
			row.y = left.x * (1.0f - fraction);
			
			// Columns 2, 3, 4, 5, top to bottom
			row.x += left.z + mid.w + mid.z + right.w;
			row.y += left.y + mid.x + mid.y + right.x;
			
			// Column 6, top to bottom
			row.x += right.z * fraction;
			row.y += right.y * fraction;
			
			return row;
		}
		
		float PCF6x6(float2 uv, float sceneDepth)
		{
			float2 fraction;
			#if CASCADING
			float3 sampleCenter = float3(getFilteringInfo(uv, fraction), gFace);
			#else
			float2 sampleCenter = getFilteringInfo(uv, fraction);
			#endif	
							
			// Gather 36 samples in nine 2x2 gathers. Gathers are performed in clockwise order, starting with top left block.
			// Every three gathers (one row), the values are accumulated to their corresponding row.
			// Samples for individual gather operations are returned in counter-clockwise order, starting with lower left.
			float2 rows[3];
			{
				float4 left = getOcclusion(gShadowTex.GatherRed(gShadowSampler, sampleCenter, int2(-2, -2)), sceneDepth);
				float4 middle = getOcclusion(gShadowTex.GatherRed(gShadowSampler, sampleCenter, int2(0, -2)), sceneDepth);
				float4 right = getOcclusion(gShadowTex.GatherRed(gShadowSampler, sampleCenter, int2(2, -2)), sceneDepth);
				
				rows[0] = accumulateRows6x2(fraction.x, left, middle, right);
			}

			{
				float4 left = getOcclusion(gShadowTex.GatherRed(gShadowSampler, sampleCenter, int2(-2, 0)), sceneDepth);
				float4 middle = getOcclusion(gShadowTex.GatherRed(gShadowSampler, sampleCenter, int2(0, 0)), sceneDepth);
				float4 right = getOcclusion(gShadowTex.GatherRed(gShadowSampler, sampleCenter, int2(2, 0)), sceneDepth);

				rows[1] = accumulateRows6x2(fraction.x, left, middle, right);
			}

			{
				float4 left = getOcclusion(gShadowTex.GatherRed(gShadowSampler, sampleCenter, int2(-2, 2)), sceneDepth);
				float4 middle = getOcclusion(gShadowTex.GatherRed(gShadowSampler, sampleCenter, int2(0, 2)), sceneDepth);
				float4 right = getOcclusion(gShadowTex.GatherRed(gShadowSampler, sampleCenter, int2(2, 2)), sceneDepth);

				rows[2] = accumulateRows6x2(fraction.x, left, middle, right);
			}

			// Accumulate all rows
			float occlusionAccumulator;
			occlusionAccumulator = rows[0].x * (1.0f - fraction.y);
			occlusionAccumulator += rows[0].y + rows[1].x + rows[1].y + rows[2].x;
			occlusionAccumulator += rows[2].y * fraction.y;
							
			// Calc average occlusion using 5x5 area and return
			return occlusionAccumulator * (1.0f / 25.0f);				
		}

		float4 fsmain(VStoFS input, uint sampleIdx : SV_SampleIndex) : SV_Target0
		{
			float2 ndcPos = input.clipSpacePos.xy / input.clipSpacePos.w;
			uint2 pixelPos = NDCToScreen(ndcPos);
		
			// Get depth & calculate world position
			#if MSAA_COUNT > 1
			float deviceZ = gDepthBufferTex.Load(pixelPos, sampleIdx).r;
			#else			
			float deviceZ = gDepthBufferTex.Load(int3(pixelPos, 0)).r;
			#endif
			
			float depth = convertFromDeviceZ(deviceZ);
			float4 mixedSpacePos = float4(ndcPos * -depth, depth, 1);
			
			float4 shadowPosition = mul(gMixedToShadowSpace, mixedSpacePos); 
			shadowPosition.xy /= shadowPosition.w;
			
			// Clamp depth range because pixels in the shadow map that haven't been rendered to will have a value of 1,
			// and we want those to remain unshadowed.
			float lightSpaceDepth = min(shadowPosition.z, 0.999999f);
			
			float occlusion = 0.0f;
			#if SHADOW_QUALITY <= 1
				occlusion = PCF1x1(shadowPosition.xy, lightSpaceDepth);
			#elif SHADOW_QUALITY == 2
				occlusion = PCF2x2(shadowPosition.xy, lightSpaceDepth);
			#elif SHADOW_QUALITY == 3
				occlusion = PCF4x4(shadowPosition.xy, lightSpaceDepth);
			#else
				occlusion = PCF6x6(shadowPosition.xy, lightSpaceDepth);
			#endif
			
			float alpha = 1.0f;
			#if CASCADING
				alpha = 1.0f - saturate((-depth - gFadePlaneDepth) * gInvFadePlaneRange);
			#endif

			occlusion *= gFadePercent;
			
			// Encode to get better precision in the blacks, similar to gamma correction but cheaper to execute
			return float4(sqrt(occlusion), 0.0f, 0.0f, alpha);
		}
	};
};
