#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"

technique PPSSAO
{
	mixin PPBase;
	mixin PerCameraData;

	code
	{
		[internal]
		cbuffer Input
		{
			float gSampleRadius;
			float gWorldSpaceRadiusMask;
			float2 gTanHalfFOV; // x - horz FOV, y - vert FOV
			float2 gRandomTileScale;
			float gCotHalfFOV;
			float gBias;
			float2 gDownsampledPixelSize;
		}		

		SamplerState gInputSamp;
		Texture2D gDepthTex;
		Texture2D gNormalsTex;
		Texture2D gDownsampledAO;
		Texture2D gSetupAO;
		
		SamplerState gRandomSamp;
		Texture2D gRandomTex;
		
		// TODO - Allow these to be controlled by a quality level
		#define SAMPLE_COUNT 6
		#define SAMPLE_STEPS 3
		
		static const float2 SAMPLES[6] =
		{
			// Points within a disc, at equally separated angles from 0 to 2PI.
			// Each point is also placed further away from the disc center, up to unit disc radius.
			float2( 0.000f,  0.166f),
			float2( 0.288f,  0.166f),
			float2( 0.433f, -0.250f),
			float2( 0.000f, -0.666f),
			float2(-0.721f, -0.416f),
			float2(-0.866,   0.500f)
		};
		
		float2 ndcToDepthUV(float2 ndc)
		{
			return NDCToUV(ndc);
		}
		
		float3 getViewSpacePos(float2 ndc, float depth)
		{
			float2 clipSpace = ndc * -depth;
			
			// Use the tan(FOV/2) & aspect to move from clip to view space (basically just scaling).
			// This is the equivalent of multiplying by mixedToView matrix that's used in most
			// depth -> world space calculations, but if we make some assumptions we can avoid the
			// matrix multiply and get the same result. We can also avoid division by .w since we know
			// the depth is in view space and the mixedToView matrix wouldn't affect it.
			// The only entries that effect the coordinate are 0,0 and 1,1 entries in the matrix
			// (if the matrix is symmetric, which we assume is true), which are just the cotangent
			// of the half of the two aspect ratios.
			
			return float3(clipSpace * gTanHalfFOV, depth);
		}
		
		float getUpsampledAO(float2 uv, float depth, float3 normal)
		{
			float2 uvs[9];
			uvs[0] = uv + float2(-1, -1) * gDownsampledPixelSize;
			uvs[1] = uv + float2( 0, -1) * gDownsampledPixelSize;
			uvs[2] = uv + float2( 1, -1) * gDownsampledPixelSize;
			uvs[3] = uv + float2(-1,  0) * gDownsampledPixelSize;
			uvs[4] = uv + float2( 0,  0) * gDownsampledPixelSize;
			uvs[5] = uv + float2( 1,  0) * gDownsampledPixelSize;
			uvs[6] = uv + float2(-1,  1) * gDownsampledPixelSize;
			uvs[7] = uv + float2( 0,  1) * gDownsampledPixelSize;
			uvs[8] = uv + float2( 1,  1) * gDownsampledPixelSize;
			
			float weightedSum = 0.0f;
			float weightSum = 0.00001f;
			
			[unroll]
			for(int i = 0; i < 9; ++i)
			{
				// Get AO from previous step (half-resolution buffer)
				float sampleAO = gDownsampledAO.Sample(gInputSamp, uvs[i]).r;
				
				// Get filtered normal/depth
				float4 sampleNormalAndDepth = gSetupAO.Sample(gInputSamp, uvs[i]);
				float3 sampleNormal = sampleNormalAndDepth.xyz * 2.0f - 1.0f;
				float sampleDepth = sampleNormalAndDepth.w;
				
				// Compute sample contribution depending on how close it is to current
				// depth and normal
				float weight = saturate(1.0f - abs(sampleDepth - depth) * 0.3f);
				weight *= saturate(dot(sampleNormal, normal));
				
				weightedSum += sampleAO * weight;
				weightSum += weight;
			}
			
			return weightedSum / weightSum;
		}
		
		float4 fsmain(VStoFS input) : SV_Target0
		{
			// TODO - Support MSAA (most likely don't require all samples)
		
			#if FINAL_AO // Final uses gbuffer input
			float sceneDepth = convertFromDeviceZ(gDepthTex.Sample(gInputSamp, input.uv0).r);
			float3 worldNormal = gNormalsTex.Sample(gInputSamp, input.uv0).xyz * 2.0f - 1.0f;
			#else // Input from AO setup pass
			float4 aoSetup = gSetupAO.Sample(gInputSamp, input.uv0);
			float sceneDepth = aoSetup.w;
			float3 worldNormal = aoSetup.xyz * 2.0f - 1.0f;
			#endif
			
			float3 viewNormal = normalize(mul((float3x3)gMatView, worldNormal));
			float3 viewPos = getViewSpacePos(input.screenPos, sceneDepth);
			
			// Apply bias to avoid false occlusion due to depth quantization or other precision issues
			viewPos += viewNormal * gBias * -sceneDepth;
			// Note: Do I want to recalculate screen position from this new view position?
			
			// Project sample radius to screen space (approximately), using the formula:
			// screenRadius = worldRadius * 1/tan(fov/2) / z
			// The formula approximates sphere projection and is more accurate the closer to the screen center
			// the sphere origin is.
			float sampleRadius = gSampleRadius * lerp(-sceneDepth, 1, gWorldSpaceRadiusMask) * gCotHalfFOV / -sceneDepth;
			
			// Get random rotation
			float2 rotateDir = gRandomTex.Sample(gRandomSamp, input.uv0 * gRandomTileScale) * 2 - 1;
			
			// Scale by screen space sample radius
			rotateDir *= sampleRadius;
			
			// Construct rotation matrix
			float2 rotateDir90 = float2(-rotateDir.y, rotateDir.x); // Rotate 90 degrees
			float2x2 rotateTfrm = float2x2(
				rotateDir.x, rotateDir90.x,
				rotateDir.y, rotateDir90.y
			);
						
			float invRange = 1.0f / gSampleRadius;
			
			// For every sample, find the highest horizon angle in the direction of the sample
			float2 accumulator = 0.00001f;
			[unroll]
			for(int i = 0; i < SAMPLE_COUNT; ++i)
			{
				float2 sampleOffset = mul(rotateTfrm, SAMPLES[i]);
			
				// Step along the direction of the sample offset, looking for the maximum angle in two directions
				// (positive dir of the sample offset, and negative). Steps are weighted so that those that are
				// further away from the origin contribute less.
				float3 stepAccum = 0;
				
				[unroll]
				for(int j = 1; j <= SAMPLE_STEPS; ++j)
				{
					float scale = j / (float)SAMPLE_STEPS;
					
					float2 screenPosL = input.screenPos + sampleOffset;
					float2 screenPosR = input.screenPos - sampleOffset;
					
					// TODO - Sample HiZ here to minimize cache trashing (depending on quality)
					float depthL = gDepthTex.Sample(gInputSamp, ndcToDepthUV(screenPosL)).r;
					float depthR = gDepthTex.Sample(gInputSamp, ndcToDepthUV(screenPosR)).r;
					
					depthL = convertFromDeviceZ(depthL);
					depthR = convertFromDeviceZ(depthR);
					
					float3 viewPosL = getViewSpacePos(screenPosL, depthL);
					float3 viewPosR = getViewSpacePos(screenPosR, depthR);
					
					float3 diffL = viewPosL - viewPos;
					float3 diffR = viewPosR - viewPos;
					
					float angleL = saturate(dot(diffL, viewNormal) * rsqrt(dot(diffL, diffL)));
					float angleR = saturate(dot(diffR, viewNormal) * rsqrt(dot(diffR, diffR)));
					
					float weight = saturate(1.0f - length(diffL) * invRange);
					weight *= saturate(1.0f - length(diffR) * invRange);
					
					float2 angles = float2(angleL, angleR);
					stepAccum = lerp(stepAccum, float3(max(angles, stepAccum.xy), 1), weight);
				}
				
				// Negate since higher angle means more occlusion
				float2 weightedValue = 1.0f - stepAccum.xy;
				
				// Square to reduce impact on areas with low AO, and increase impact on areas with high AO
				weightedValue *= weightedValue;
				
				// Multiply by weight since we calculate the weighted average
				weightedValue *= stepAccum.z;
				
				// Accumulate sum total and weight total
				accumulator += float2(weightedValue.x + weightedValue.y, 2.0f * stepAccum.z);
			}
			
			float4 output = 0;
			
			// Divide by total weight to get the weighted average
			output.r = accumulator.x / accumulator.y;
			
			#if MIX_WITH_UPSAMPLED
			float upsampledAO = getUpsampledAO(input.uv0, sceneDepth, worldNormal);
			
			// Note: 0.6f just an arbitrary constant that looks good. Make this adjustable externally?
			output.r = lerp(output.r, upsampledAO, 0.6f);
			#endif
			
			#if FINAL_AO
			// TODO - Fade out far away AO
			// TODO - Adjust power/intensity
			#endif
			
			// TODO - Perform filtering over 2x2 pixels using derivatives (on quality levels with no upsampling)
			return output;
		}	
	};
};