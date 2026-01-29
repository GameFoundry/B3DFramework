#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"

shader PPSSAO
{
	mixin PPBase;
	mixin PerCameraData;

	variations
	{
		MIX_WITH_UPSAMPLED = { true, false };
		FINAL_AO = { true, false };
		QUALITY = { 0, 1, 2, 3, 4 };
	};
	
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
			float2 gFadeMultiplyAdd;
			float gPower;
			float gIntensity;
		}		

		SamplerState gInputSamp;
		Texture2D gDepthTex;
		Texture2D gNormalsTex;
		Texture2D gDownsampledAO;
		Texture2D gSetupAO;
		
		SamplerState gRandomSamp;
		Texture2D gRandomTex;
		
		#if QUALITY < 3
			#define SAMPLE_STEPS 1
		#else
			#define SAMPLE_STEPS 3
		#endif
		
		#if QUALITY < 4
			#define SAMPLE_SET 0
		#else
			#define SAMPLE_SET 1
		#endif
		
		// Points within a disc, at equally separated angles from 0 to 2PI.
		// Each point is also placed further away from the disc center, up to unit disc radius.
		// f[x_, s_] := {((x + 1)/(s + 1))*Cos[(x/s)*2 Pi], (x + 1)/(s + 1)*Sin[(x/s)*2 Pi]}
		#if SAMPLE_SET == 0
			#define SAMPLE_COUNT 3
			static const float2 SAMPLES[3] =
			{
				float2( 0.250f,  0.000f),
				float2(-0.250f,  0.433f),
				float2(-0.375f, -0.649f)
			};
		#else
			#define SAMPLE_COUNT 6
			static const float2 SAMPLES[6] =
			{

				float2( 0.142f,  0.000f),
				float2( 0.142f,  0.247f),
				float2(-0.214f,  0.371f),
				float2(-0.571f,  0.000f),
				float2(-0.357f, -0.618f),
				float2( 0.428f, -0.742f)
			};		
		#endif
		
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
			
			float weightedSum = 0.00001f;
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
		
		float fsmain(VStoFS input, float4 pixelPos : SV_Position) : SV_Target0
		{
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
			#if QUALITY == 0
			float2 rotateDir = float2(0, 1); // No random rotation
			#else
			float2 rotateDir = gRandomTex.Sample(gRandomSamp, input.uv0 * gRandomTileScale) * 2 - 1;
			#endif
			
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
					
					float2 screenPosL = input.screenPos + sampleOffset * scale;
					float2 screenPosR = input.screenPos - sampleOffset * scale;
					
					// TODO - Sample HiZ here to minimize cache trashing (depending on quality)
					#if FINAL_AO // Final uses gbuffer input
					float depthL = gDepthTex.Sample(gInputSamp, ndcToDepthUV(screenPosL)).r;
					float depthR = gDepthTex.Sample(gInputSamp, ndcToDepthUV(screenPosR)).r;
					
					depthL = convertFromDeviceZ(depthL);
					depthR = convertFromDeviceZ(depthR);
					#else
					float depthL = gSetupAO.Sample(gInputSamp, ndcToDepthUV(screenPosL)).w;
					float depthR = gSetupAO.Sample(gInputSamp, ndcToDepthUV(screenPosR)).w;
					#endif
					
					float3 viewPosL = getViewSpacePos(screenPosL, depthL);
					float3 viewPosR = getViewSpacePos(screenPosR, depthR);
					
					float3 diffL = viewPosL - viewPos;
					float3 diffR = viewPosR - viewPos;
					
					float angleL = saturate(dot(diffL, viewNormal) * rsqrt(dot(diffL, diffL)));
					float angleR = saturate(dot(diffR, viewNormal) * rsqrt(dot(diffR, diffR)));
					
					// Avoid blending if depths are too different to avoid leaking
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
			
			float output = 0;
			
			// Divide by total weight to get the weighted average
			output = accumulator.x / accumulator.y;
			
			#if MIX_WITH_UPSAMPLED
			float upsampledAO = getUpsampledAO(input.uv0, sceneDepth, worldNormal);
			
			// Note: 0.6f just an arbitrary constant that looks good. Make this adjustable externally?
			output = lerp(output, upsampledAO, 0.6f);
			#endif
			
			#if FINAL_AO
			// Fade out far away AO
			// Reference: 1 - saturate((depth - fadeDistance) / fadeRange)
			output = lerp(output, 1.0f, saturate(-sceneDepth * gFadeMultiplyAdd.x + gFadeMultiplyAdd.y));
			
			// Adjust power and intensity
			output = 1.0f - saturate((1.0f - pow(output, gPower)) * gIntensity);
			#endif
			
			// On quality 0 we don't blur at all. At qualities higher than 1 we use a proper bilateral blur.
			#if QUALITY == 1
			// Perform a 2x2 ad-hoc blur to hide the dither pattern
			// Note: Ideally the blur would be 4x4 since the pattern is 4x4
			
			float4 myVal = float4(output, viewNormal);
			float4 dX = ddx(myVal);
			float4 dY = ddy(myVal);
			
			int2 mod = (int2)(pixelPos.xy) % 2;
			float4 horzVal = myVal - dX * (mod.x * 2 - 1);
			float4 vertVal = myVal - dY * (mod.y * 2 - 1);
			
			// Do weighted average depending on how similar the normals are
			float weightHorz = saturate(pow(saturate(dot(viewNormal, horzVal.yzw)), 4.0f));
			float weightVert = saturate(pow(saturate(dot(viewNormal, vertVal.yzw)), 4.0f));
			
			float myWeight = 1.0f;
			float invWeight = 1.0f / (myWeight + weightHorz + weightVert);
			
			myWeight *= invWeight;
			weightHorz *= invWeight;
			weightVert *= invWeight;
			
			output = output * myWeight + horzVal.r * weightHorz + vertVal.r * weightVert;
			#endif
			
			return output;
		}	
	};
};
