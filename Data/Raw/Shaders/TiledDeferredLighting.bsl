#include "$ENGINE$\GBufferInput.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"
#define USE_COMPUTE_INDICES 1
#include "$ENGINE$\DirectLighting.bslinc"
#include "$ENGINE$\ReflectionCubemapCommon.bslinc"
#include "$ENGINE$\ImageBasedLighting.bslinc"

shader TiledDeferredLighting
{
	mixin GBufferInput;
	mixin PerCameraData;
	mixin DirectLighting;
	mixin LightAccumulatorIndexed;
	mixin ReflectionCubemapCommon;
	mixin ImageBasedLighting;

	featureset = HighEnd;
	
	variations
	{
		MSAA_COUNT = { 1, 2, 4, 8 };
	};	
	
	code
	{	
		[internal]
		cbuffer Params
		{
			// Number of lights per type in the lights buffer
			// x - directional lights, y - radial lights, z - spot lights, w - total number of lights
			uint4 gLightCounts;
			// Strides between different light types in the light buffer
			// x - stride to radial lights, y - stride to spot lights. Directional lights are assumed to start at 0.
			uint2 gLightStrides;
			
			uint2 gFramebufferSize;
		}
	
		#if MSAA_COUNT > 1
		RWTexture2DArray<float4> gOutput;
		Texture2D gMSAACoverage;
		#else
		RWTexture2D<float4>	gOutput;
		#endif
					
		groupshared uint sTileMinZ;
		groupshared uint sTileMaxZ;

		groupshared uint sNumLightsPerType[2];
		groupshared uint sTotalNumLights;

		float4 getLighting(float2 clipSpacePos, SurfaceData surfaceData)
		{
			// x, y are now in clip space, z, w are in view space
			// We multiply them by a special inverse view-projection matrix, that had the projection entries that effect
			// z, w eliminated (since they are already in view space)
			// Note: Multiply by depth should be avoided if using ortographic projection
			float4 mixedSpacePos = float4(clipSpacePos * -surfaceData.depth, surfaceData.depth, 1);
			float4 worldPosition4D = mul(gMatScreenToWorld, mixedSpacePos);
			float3 worldPosition = worldPosition4D.xyz / worldPosition4D.w;
			
			uint4 lightOffsets;
			lightOffsets.x = gLightCounts[0];
			lightOffsets.y = 0;
			lightOffsets.z = sNumLightsPerType[0];
			lightOffsets.w = sTotalNumLights;
			
			float3 V = normalize(gViewOrigin - worldPosition);
			float3 N = surfaceData.worldNormal.xyz;
			float3 R = 2 * dot(V, N) * N - V;
			float3 specR = getSpecularDominantDir(N, R, surfaceData.roughness);
			
			return getDirectLighting(worldPosition, V, specR, surfaceData, lightOffsets);				
		}
		
		[numthreads(TILE_SIZE, TILE_SIZE, 1)]
		void csmain(
			uint3 groupId : SV_GroupID,
			uint3 groupThreadId : SV_GroupThreadID,
			uint3 dispatchThreadId : SV_DispatchThreadID)
		{
			uint threadIndex = groupThreadId.y * TILE_SIZE + groupThreadId.x;
			uint2 pixelPos = dispatchThreadId.xy + gViewportRectangle.xy;
			
			// Note: To improve performance perhaps:
			//  - Use halfZ (split depth range into two regions for better culling)
			//  - Use parallel reduction instead of atomics
			//  - Use AABB instead of frustum (no false positives)
			//   - Increase tile size to 32x32 to amortize the cost of AABB calc (2x if using halfZ)
			
			// Get data for all samples, and determine per-pixel minimum and maximum depth values
			SurfaceData surfaceData[MSAA_COUNT];
			uint sampleMinZ = 0x7F7FFFFF;
			uint sampleMaxZ = 0;

			#if MSAA_COUNT > 1
			[unroll]
			for(uint i = 0; i < MSAA_COUNT; ++i)
			{
				surfaceData[i] = getGBufferData(pixelPos, i);
				
				sampleMinZ = min(sampleMinZ, asuint(-surfaceData[i].depth));
				sampleMaxZ = max(sampleMaxZ, asuint(-surfaceData[i].depth));
			}
			#else
			surfaceData[0] = getGBufferData(pixelPos);
			sampleMinZ = asuint(-surfaceData[0].depth);
			sampleMaxZ = asuint(-surfaceData[0].depth);
			#endif

			// Set initial values
			if(threadIndex == 0)
			{
				sTileMinZ = 0x7F7FFFFF;
				sTileMaxZ = 0;
				sNumLightsPerType[0] = 0;
				sNumLightsPerType[1] = 0;
				sTotalNumLights = 0;
			}
			
			GroupMemoryBarrierWithGroupSync();
			
			// Determine minimum and maximum depth values for a tile			
			InterlockedMin(sTileMinZ, sampleMinZ);
			InterlockedMax(sTileMaxZ, sampleMaxZ);
			
			GroupMemoryBarrierWithGroupSync();
			
			float minTileZ = asfloat(sTileMinZ);
			float maxTileZ = asfloat(sTileMaxZ);
			
			// Create a frustum for the current tile
			// First determine a scale of the tile compared to the viewport
			float2 tileScale = gViewportRectangle.zw * rcp(float2(TILE_SIZE, TILE_SIZE));

			// Now we need to use that scale to scale down the frustum.
			// Assume a projection matrix:
			// A, 0, C, 0
			// 0, B, D, 0
			// 0, 0, Q, QN
			// 0, 0, -1, 0
			//
			// Where A is = 2*n / (r - l)
			// and C = (r + l) / (r - l)
			// 
			// Q & QN are used for Z value which we don't need to scale. B & D are equivalent for the
			// Y value, we'll only consider the X values (A & C) from now on.
			//
			// Both and A and C are inversely proportional to the size of the frustum (r - l). Larger scale mean that
			// tiles are that much smaller than the viewport. This means as our scale increases, (r - l) decreases,
			// which means A & C as a whole increase. Therefore:
			// A' = A * tileScale.x
			// C' = C * tileScale.x
			
			// Aside from scaling, we also need to offset the frustum to the center of the tile.
			// For this we calculate the bias value which we add to the C & D factors (which control
			// the offset in the projection matrix).
			float2 tileBias = tileScale - 1 - groupId.xy * 2;

			// This will yield a bias ranging from [-(tileScale - 1), tileScale - 1]. Every second bias is skipped as
			// corresponds to a point in-between two tiles, overlapping existing frustums.
			
			float flipSign = 1.0f;
			
			// Adjust for OpenGL's upside down texture system
			#if OPENGL
				flipSign = -1;
			#endif
			
			float At = gMatProj[0][0] * tileScale.x;
			float Ctt = gMatProj[0][2] * tileScale.x - tileBias.x;
			
			float Bt = gMatProj[1][1] * tileScale.y * flipSign;
			float Dtt = (gMatProj[1][2] * tileScale.y + flipSign * tileBias.y) * flipSign;
			
			// Extract left/right/top/bottom frustum planes from scaled projection matrix
			// Note: Do this on the CPU? Since they're shared among all entries in a tile. Plus they don't change across frames.
			float4 frustumPlanes[6];
			frustumPlanes[0] = float4(At, 0.0f, gMatProj[3][2] + Ctt, 0.0f);
			frustumPlanes[1] = float4(-At, 0.0f, gMatProj[3][2] - Ctt, 0.0f);
			frustumPlanes[2] = float4(0.0f, -Bt, gMatProj[3][2] - Dtt, 0.0f);
			frustumPlanes[3] = float4(0.0f, Bt, gMatProj[3][2] + Dtt, 0.0f);
			
			// Normalize
			[unroll]
			for (uint i = 0; i < 4; ++i) 
				frustumPlanes[i] *= rcp(length(frustumPlanes[i].xyz));
			
			// Generate near/far frustum planes
			// Note: d gets negated in plane equation, this is why its in opposite direction than it intuitively should be
			frustumPlanes[4] = float4(0.0f, 0.0f, -1.0f, -minTileZ); 
			frustumPlanes[5] = float4(0.0f, 0.0f, 1.0f, maxTileZ);
			
			// Find radial & spot lights overlapping the tile
			for(uint type = 0; type < 2; type++)
			{
				uint lightsStart = gLightStrides[type];
				uint lightsEnd = lightsStart + gLightCounts[type + 1];
				for (uint i = threadIndex + lightsStart; i < lightsEnd && i < MAX_LIGHTS; i += TILE_SIZE * TILE_SIZE)
				{
					float4 lightPosition = mul(gMatView, float4(gLights[i].position, 1.0f));
					float lightRadius = gLights[i].boundRadius;
					
					// Note: The cull method can have false positives. In case of large light bounds and small tiles, it
					// can end up being quite a lot. Consider adding an extra heuristic to check a separating plane.
					bool lightInTile = true;
				
					// First check side planes as this will cull majority of the lights
					[unroll]
					for (uint j = 0; j < 4; ++j)
					{
						float dist = dot(frustumPlanes[j], lightPosition);
						lightInTile = lightInTile && (dist >= -lightRadius);
					}

					// Make sure to do an actual branch, since it's quite likely an entire warp will have the same value
					[branch]
					if (lightInTile)
					{
						bool inDepthRange = true;
				
						// Check near/far planes
						[unroll]
						for (uint j = 4; j < 6; ++j)
						{
							float dist = dot(frustumPlanes[j], lightPosition);
							inDepthRange = inDepthRange && (dist >= -lightRadius);
						}
						
						// In tile, add to branch
						[branch]
						if (inDepthRange)
						{
							InterlockedAdd(sNumLightsPerType[type], 1U);
							
							uint idx;
							InterlockedAdd(sTotalNumLights, 1U, idx);
							gLightIndices[idx] = i;
						}
					}
				}
			}

			GroupMemoryBarrierWithGroupSync();

			// Generate world position
			float2 screenUv = ((float2)(gViewportRectangle.xy + pixelPos) + 0.5f) / (float2)gViewportRectangle.zw;
			float2 clipSpacePos = (screenUv - gClipToUVScaleOffset.zw) / gClipToUVScaleOffset.xy;
		
			uint2 viewportMax = gViewportRectangle.xy + gViewportRectangle.zw;			
			
			// Ignore pixels out of valid range
			if (all(dispatchThreadId.xy < viewportMax))
			{
				#if MSAA_COUNT > 1
				float coverage = gMSAACoverage.Load(int3(pixelPos, 0)).r;
				
				float4 lighting = getLighting(clipSpacePos.xy, surfaceData[0]);
				gOutput[uint3(pixelPos.xy, 0)] = lighting;
				
				bool doPerSampleShading = coverage > 0.5f;
				if(doPerSampleShading)
				{
					[unroll]
					for(uint i = 1; i < MSAA_COUNT; ++i)
					{
						lighting = getLighting(clipSpacePos.xy, surfaceData[i]);
						gOutput[uint3(pixelPos.xy, i)] = lighting;
					}
				}
				else // Splat same information to all samples
				{
					// Note: The splatting step can be skipped if we account for coverage when resolving. However
					// the coverage texture potentially becomes invalid after transparent geometry is renedered, 
					// so we need to resolve all samples. Consider getting around this issue somehow.
					[unroll]
					for(uint i = 1; i < MSAA_COUNT; ++i)
						gOutput[uint3(pixelPos.xy, i)] = lighting;
				}
				
				#else
				float4 lighting = getLighting(clipSpacePos.xy, surfaceData[0]);
				gOutput[pixelPos] = lighting;
				#endif
			}
		}
	};
};