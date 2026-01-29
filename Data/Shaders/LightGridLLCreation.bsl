#include "$ENGINE$\PerCameraData.bslinc"
#define USE_LIGHT_GRID_INDICES 1
#include "$ENGINE$\DirectLighting.bslinc"
#include "$ENGINE$\ImageBasedLighting.bslinc"
#include "$ENGINE$\LightGridCommon.bslinc"

shader LightGridLLCreation
{
	mixin PerCameraData;
	mixin LightData;
	mixin LightGridCommon;
	mixin ImageBasedLighting;

	featureset = HighEnd;
	
	code
	{
		StructuredBuffer<LightData> gLights;
		StructuredBuffer<ReflProbeData> gReflectionProbes;
	
		RWStructuredBuffer<uint> gLightsCounter;
		RWStructuredBuffer<uint> gLightsLLHeads;
		RWBuffer<uint4> gLightsLL;
		
		RWStructuredBuffer<uint> gProbesCounter;
		RWStructuredBuffer<uint> gProbesLLHeads;
		RWBuffer<uint2> gProbesLL;
			
		// Generates a an axis aligned bounding box in NDC and transforms it to view space.
		// Note: This will overlap other cells, so it might be better to use frustum planes
		// instead of AABB, although frustum testing procedure could result in more false positive
		void calcCellAABB(uint3 cellIdx, out float3 center, out float3 extent)
		{
			// Note:: AABB calculation in tiled deferred image based lighting shader uses less instructions than this,
			// see if it can be applied here.
		
			// Convert grid XY coordinates to clip coordinates
			float2 a = 2.0f / gGridSize.xy;
		
			float3 ndcMin;
			float3 ndcMax;
		
			ndcMin.xy = cellIdx.xy * a - float2(1.0f, 1.0f);
			ndcMax.xy = (cellIdx.xy + 1) * a - float2(1.0f, 1.0f);
		
			// Flip Y depending on render API, depending if Y in NDC is facing up or down
			// (We negate the value because we want NDC with Y flipped, so origin is top left)
			float flipY = -sign(gMatProj[1].y);
			ndcMin.y *= flipY;
			ndcMax.y *= flipY;
		
			// Because we're viewing along negative Z, farther end is the minimum
			float viewZMin = calcViewZFromCellZ(cellIdx.z + 1);
			float viewZMax = calcViewZFromCellZ(cellIdx.z);
		
			ndcMin.z = convertToNDCZ(viewZMax);
			ndcMax.z = convertToNDCZ(viewZMin);
		
			float4 corner[8];
			// Near
			corner[0] = mul(gMatInvProj, float4(ndcMin.x, ndcMin.y, ndcMin.z, 1.0f));
			corner[1] = mul(gMatInvProj, float4(ndcMax.x, ndcMin.y, ndcMin.z, 1.0f));
			corner[2] = mul(gMatInvProj, float4(ndcMax.x, ndcMax.y, ndcMin.z, 1.0f));
			corner[3] = mul(gMatInvProj, float4(ndcMin.x, ndcMax.y, ndcMin.z, 1.0f));
		
			// Far
			corner[4] = mul(gMatInvProj, float4(ndcMin.x, ndcMin.y, ndcMax.z, 1.0f));
			corner[5] = mul(gMatInvProj, float4(ndcMax.x, ndcMin.y, ndcMax.z, 1.0f));
			corner[6] = mul(gMatInvProj, float4(ndcMax.x, ndcMax.y, ndcMax.z, 1.0f));
			corner[7] = mul(gMatInvProj, float4(ndcMin.x, ndcMax.y, ndcMax.z, 1.0f));
		
			[unroll]
			for(uint i = 0; i < 8; ++i)
				corner[i].xy /= corner[i].w;
		
			float3 viewMin = float3(corner[0].xy, viewZMin);
			float3 viewMax = float3(corner[0].xy, viewZMax);
			
			[unroll]
			for(uint i = 1; i < 8; ++i)
			{
				viewMin.xy = min(viewMin.xy, corner[i].xy);
				viewMax.xy = max(viewMax.xy, corner[i].xy);
			}
			
			extent = (viewMax - viewMin) * 0.5f;
			center = viewMin + extent;
		}
	
		[numthreads(THREADGROUP_SIZE, THREADGROUP_SIZE, THREADGROUP_SIZE)]
		void csmain(
			uint3 groupId : SV_GroupID,
			uint3 groupThreadId : SV_GroupThreadID,
			uint3 dispatchThreadId : SV_DispatchThreadID)
		{
			// Ignore pixels out of valid range
			if (any(dispatchThreadId.xy >= gGridSize.xy))
				return;
				
			uint maxNumLinks = gNumCells * gMaxNumLightsPerCell;	
			uint cellIdx = (dispatchThreadId.z * gGridSize.y + dispatchThreadId.y) * gGridSize.x + dispatchThreadId.x;
			
			float3 cellCenter;
			float3 cellExtent;
			calcCellAABB(dispatchThreadId, cellCenter, cellExtent);
			
			for(uint type = 1; type < 3; ++type)
			{
				uint lightsStart = gLightStrides[type - 1];
				uint lightsEnd = lightsStart + gLightCounts[type];
				for(uint i = lightsStart; i < lightsEnd; ++i)
				{
					float4 lightPosition = mul(gMatView, float4(gLights[i].position, 1.0f));
					float lightRadius = gLights[i].boundRadius;
					
					// Calculate distance from box to light
					float3 distances = max(abs(lightPosition - cellCenter) - cellExtent, 0);
					float distSqrd = dot(distances, distances);
					
					if(distSqrd <= (lightRadius * lightRadius))
					{
						uint nextLink;
						InterlockedAdd(gLightsCounter[0], 1U, nextLink);
						
						if(nextLink < maxNumLinks)
						{
							uint prevLink;
							InterlockedExchange(gLightsLLHeads[cellIdx], nextLink, prevLink);
							
							gLightsLL[nextLink] = uint4(i, type, prevLink, 0);
						}
					}
				}
			}
			
			for(uint i = 0; i < gNumReflProbes; ++i)
			{
				float4 probePosition = mul(gMatView, float4(gReflectionProbes[i].position, 1.0f));
				float probeRadius = gReflectionProbes[i].radius;
				
				// Calculate distance from box to light
				float3 distances = max(abs(probePosition - cellCenter) - cellExtent, 0);
				float distSqrd = dot(distances, distances);
				
				if(distSqrd <= (probeRadius * probeRadius))
				{
					uint nextLink;
					InterlockedAdd(gProbesCounter[0], 1U, nextLink);
					
					if(nextLink < maxNumLinks)
					{
						uint prevLink;
						InterlockedExchange(gProbesLLHeads[cellIdx], nextLink, prevLink);
						
						gProbesLL[nextLink] = uint2(i, prevLink);
					}
				}
			}
		}
	};
};
