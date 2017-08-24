#include "$ENGINE$\GBufferInput.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"
#include "$ENGINE$\ReflectionCubemapCommon.bslinc"
#define USE_COMPUTE_INDICES 1
#include "$ENGINE$\LightingCommon.bslinc"
#include "$ENGINE$\ImageBasedLighting.bslinc"

technique TiledDeferredImageBasedLighting
{
	mixin GBufferInput;
	mixin PerCameraData;
	mixin LightingCommon;
	mixin ReflectionCubemapCommon;
	mixin ImageBasedLighting;

	code
	{			
		[internal]
		cbuffer Params : register(b0)
		{
			uint2 gFramebufferSize;
		}
	
		#if MSAA_COUNT > 1
		Texture2DMS<float4> gInColor;
		RWBuffer<float4> gOutput;
		
		uint getLinearAddress(uint2 coord, uint sampleIndex)
		{
			return (coord.y * gFramebufferSize.x + coord.x) * MSAA_COUNT + sampleIndex;
		}
		
		void writeBufferSample(uint2 coord, uint sampleIndex, float4 color)
		{
			uint idx = getLinearAddress(coord, sampleIndex);
			gOutput[idx] = color;
		}

		#else
		Texture2D<float4> gInColor;
		RWTexture2D<float4>	gOutput;
		#endif
					
		groupshared uint sTileMinZ;
		groupshared uint sTileMaxZ;

		void getTileZBounds(uint threadIndex, SurfaceData surfaceData[MSAA_COUNT], out float minTileZ, out float maxTileZ)
		{
			// Note: To improve performance perhaps:
			//  - Use halfZ (split depth range into two regions for better culling)
			//  - Use parallel reduction instead of atomics
		
			uint sampleMinZ = 0x7F7FFFFF;
			uint sampleMaxZ = 0;

			#if MSAA_COUNT > 1
			[unroll]
			for(uint i = 0; i < MSAA_COUNT; ++i)
			{
				sampleMinZ = min(sampleMinZ, asuint(-surfaceData[i].depth));
				sampleMaxZ = max(sampleMaxZ, asuint(-surfaceData[i].depth));
			}
			#else
			sampleMinZ = asuint(-surfaceData[0].depth);
			sampleMaxZ = asuint(-surfaceData[0].depth);
			#endif

			// Set initial values
			if(threadIndex == 0)
			{
				sTileMinZ = 0x7F7FFFFF;
				sTileMaxZ = 0;
			}
			
			GroupMemoryBarrierWithGroupSync();
			
			// Determine minimum and maximum depth values for a tile			
			InterlockedMin(sTileMinZ, sampleMinZ);
			InterlockedMax(sTileMaxZ, sampleMaxZ);
			
			GroupMemoryBarrierWithGroupSync();
			
			minTileZ = -asfloat(sTileMinZ);
			maxTileZ = -asfloat(sTileMaxZ);
		}
		
		void calcTileAABB(uint2 tileId, float viewZMin, float viewZMax, out float3 center, out float3 extent)
		{
			uint2 pixelPos = tileId * TILE_SIZE;
		
			// Convert thread XY coordinates to NDC coordinates
			float2 uvTopLeft = (pixelPos + 0.5f) / gFramebufferSize;
			float2 uvBottomRight = (pixelPos + uint2(TILE_SIZE, TILE_SIZE) - 0.5f) / gFramebufferSize;
		
			float3 ndcMin;
			float3 ndcMax;
		
			ndcMin.xy = uvTopLeft * 2.0f - float2(1.0f, 1.0f);
			ndcMax.xy = uvBottomRight * 2.0f - float2(1.0f, 1.0f);
		
			// Flip Y depending on render API, depending if Y in NDC is facing up or down
			// (We negate the value because we want NDC with Y flipped, so origin is top left)
			float flipY = -sign(gMatProj[1][1]);
			ndcMin.y *= flipY;
			ndcMax.y *= flipY;
		
			ndcMin.z = convertToNDCZ(viewZMin);
			ndcMax.z = convertToNDCZ(viewZMax);
		
			float4 corner[5];
			// Far
			corner[0] = mul(gMatInvProj, float4(ndcMin.x, ndcMin.y, ndcMax.z, 1.0f));
			corner[1] = mul(gMatInvProj, float4(ndcMax.x, ndcMin.y, ndcMax.z, 1.0f));
			corner[2] = mul(gMatInvProj, float4(ndcMax.x, ndcMax.y, ndcMax.z, 1.0f));
			corner[3] = mul(gMatInvProj, float4(ndcMin.x, ndcMax.y, ndcMax.z, 1.0f));
			
			// Near (only one point, as the far away face is guaranteed to be larger in XY extents)
			corner[4] = mul(gMatInvProj, float4(ndcMin.x, ndcMin.y, ndcMin.z, 1.0f));
		
			[unroll]
			for(uint i = 0; i < 5; ++i)
				corner[i].xy /= corner[i].w;
		
			float3 viewMin = float3(corner[0].xy, viewZMin);
			float3 viewMax = float3(corner[0].xy, viewZMax);
			
			[unroll]
			for(uint i = 1; i < 4; ++i)
			{
				viewMin.xy = min(viewMin.xy, corner[i].xy);
				viewMax.xy = max(viewMax.xy, corner[i].xy);
			}
			
			extent = (viewMax - viewMin) * 0.5f;
			center = viewMin + extent;
		}
		
		bool intersectSphereBox(float3 sCenter, float sRadius, float3 bCenter, float3 bExtents)
		{
			float3 closestOnBox = max(0, abs(bCenter - sCenter) - bExtents);
			return dot(closestOnBox, closestOnBox) < sRadius * sRadius;
		}
		
		float4 getLighting(uint2 pixelPos, uint sampleIdx, float2 clipSpacePos, SurfaceData surfaceData, uint probeOffset, uint numProbes)
		{
			// x, y are now in clip space, z, w are in view space
			// We multiply them by a special inverse view-projection matrix, that had the projection entries that effect
			// z, w eliminated (since they are already in view space)
			// Note: Multiply by depth should be avoided if using ortographic projection
			float4 mixedSpacePos = float4(clipSpacePos * -surfaceData.depth, surfaceData.depth, 1);
			float4 worldPosition4D = mul(gMatScreenToWorld, mixedSpacePos);
			float3 worldPosition = worldPosition4D.xyz / worldPosition4D.w;
			
			float3 V = normalize(gViewOrigin - worldPosition);
			float3 N = surfaceData.worldNormal.xyz;
			float3 R = 2 * dot(V, N) * N - V;
			float3 specR = getSpecularDominantDir(N, R, surfaceData.roughness);
			
			float4 existingColor;
			#if MSAA_COUNT > 1
			existingColor = gInColor.Load(pixelPos.xy, sampleIdx);
			#else
			existingColor = gInColor.Load(int3(pixelPos.xy, 0));
			#endif				
			
			float ao = gAmbientOcclusionTex.Load(int3(pixelPos.xy, 0));
			float4 ssr = 0;//gSSRTex.Load(int3(pixelPos.xy, 0));
			float3 imageBasedSpecular = getImageBasedSpecular(worldPosition, V, specR, surfaceData, ao, ssr, probeOffset, numProbes);

			float4 totalLighting = existingColor;
			totalLighting.rgb += imageBasedSpecular;
			
			return totalLighting;				
		}
					
		groupshared uint gUnsortedProbeIndices[MAX_PROBES];
		groupshared uint sNumProbes;
		
		[numthreads(TILE_SIZE, TILE_SIZE, 1)]
		void csmain(
			uint3 groupId : SV_GroupID,
			uint3 groupThreadId : SV_GroupThreadID,
			uint3 dispatchThreadId : SV_DispatchThreadID)
		{
			uint threadIndex = groupThreadId.y * TILE_SIZE + groupThreadId.x;
			uint2 pixelPos = dispatchThreadId.xy + gViewportRectangle.xy;
			
			// Get data for all samples
			SurfaceData surfaceData[MSAA_COUNT];
			
			#if MSAA_COUNT > 1
			[unroll]
			for(uint i = 0; i < MSAA_COUNT; ++i)
				surfaceData[i] = getGBufferData(pixelPos, i);
			#else
			surfaceData[0] = getGBufferData(pixelPos);
			#endif

			// Set initial values
			if(threadIndex == 0)
				sNumProbes = 0;				
			
			// Determine per-pixel minimum and maximum depth values
			float minTileZ, maxTileZ;
			getTileZBounds(threadIndex, surfaceData, minTileZ, maxTileZ);
			
			// Create AABB for the current tile
			float3 center, extent;
			calcTileAABB(groupId.xy, minTileZ, maxTileZ, center, extent);
							
			// Find probes overlapping the tile
			for (uint i = threadIndex; i < gNumProbes && i < MAX_LIGHTS; i += TILE_SIZE)
			{
				float4 probePosition = mul(gMatView, float4(gReflectionProbes[i].position, 1.0f));
				float probeRadius = gReflectionProbes[i].radius;
			
				if(intersectSphereBox(probePosition, probeRadius, center, extent))
				{
					uint idx;
					InterlockedAdd(sNumProbes, 1U, idx);
					gUnsortedProbeIndices[idx] = i;
				}
			}

			GroupMemoryBarrierWithGroupSync();

			// Sort based on original indices. Using parallel enumeration sort (n^2) - could be faster
			const uint numThreads = TILE_SIZE * TILE_SIZE;
			for (uint i = threadIndex; i < sNumProbes; i += numThreads)
			{
				int idx = gUnsortedProbeIndices[i];
				uint smallerCount = 0;

				for (uint j = 0; j < sNumProbes; j++) 
				{
					int otherIdx = gUnsortedProbeIndices[j];
					if (otherIdx < idx)
						smallerCount++;
				}

				gReflectionProbeIndices[smallerCount] = gUnsortedProbeIndices[i];
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
				float4 lighting = getLighting(pixelPos, 0, clipSpacePos.xy, surfaceData[0], 0, sNumProbes);
				writeBufferSample(pixelPos, 0, lighting);

				bool doPerSampleShading = needsPerSampleShading(surfaceData);
				if(doPerSampleShading)
				{
					[unroll]
					for(uint i = 1; i < MSAA_COUNT; ++i)
					{
						lighting = getLighting(pixelPos, i, clipSpacePos.xy, surfaceData[i], 0, sNumProbes);
						writeBufferSample(pixelPos, i, lighting);
					}
				}
				else // Splat same information to all samples
				{
					[unroll]
					for(uint i = 1; i < MSAA_COUNT; ++i)
						writeBufferSample(pixelPos, i, lighting);
				}
				
				#else
				float4 lighting = getLighting(pixelPos, 0, clipSpacePos.xy, surfaceData[0], 0, sNumProbes);
				gOutput[pixelPos] = lighting;
				#endif
			}
		}
	};
};