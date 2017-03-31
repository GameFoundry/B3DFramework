#include "$ENGINE$\PerCameraData.bslinc"
#define USE_LIGHT_GRID_INDICES
#include "$ENGINE$\LightingCommon.bslinc"
#include "$ENGINE$\LightGridCommon.bslinc"

Blocks =
{
	Block PerCamera : auto("PerCamera");
	Block GridParams : auto("GridParams");
};

Technique
 : inherits("PerCameraData")
 : inherits("LightingCommon")
 : inherits("LightGridCommon") =
{
	Language = "HLSL11";
	
	Pass =
	{
		Compute = 
		{
			RWBuffer<uint> gLinkedListCounter : register(u0);
			RWBuffer<uint> gLinkedListHeads : register(u1);
			RWBuffer<uint4> gLinkedList : register(u2);
					
			// Generates a an axis aligned bounding box in NDC and transforms it to view space.
			// Note: This will overlap other cells, so it might be better to use frustum planes
			// instead of AABB, although frustum testing procedure could result in more false positive
			void calcCellAABB(uint3 cellIdx, out float3 center, out float3 extent)
			{
				// Convert grid XY coordinates to clip coordinates
				float2 a = 2.0f / gGridSize.xy;
			
				float3 ndcMin;
				float3 ndcMax;
			
				ndcMin.xy = cellIdx.xy * a - float2(1.0f, 1.0f);
				ndcMax.xy = (cellIdx.xy + 1) * a - float2(1.0f, 1.0f);
			
				// Flip Y depending on render API, depending if Y in NDC is facing up or down
				// (We negate the value because we want NDC with Y flipped, so origin is top left)
				float flipY = -sign(gMatProj[1][1]);
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
			void main(
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
					uint lightOffset = gLightOffsets[type - 1];
					uint lightEnd = gLightOffsets[type];
					for(uint i = lightOffset; i < lightEnd; ++i)
					{
						float4 lightPosition = mul(gMatView, float4(gLights[i].position, 1.0f));
						float lightRadius = gLights[i].attRadius;
						
						// Calculate distance from box to light
						float3 distances = max(abs(lightPosition - cellCenter) - cellExtent, 0);
						float distSqrd = dot(distances, distances);
						
						if(distSqrd <= (lightRadius * lightRadius))
						{
							uint nextLink;
							InterlockedAdd(gLinkedListCounter[0], 1U, nextLink);
							
							if(nextLink < maxNumLinks)
							{
								uint prevLink;
								InterlockedExchange(gLinkedListHeads[cellIdx], nextLink, prevLink);
								
								gLinkedList[nextLink] = uint4(i, type, prevLink, 0);
							}
						}
					}
				}
			}
		};
	};
};

Technique
 : inherits("PerCameraData")
 : inherits("LightingCommon")
 : inherits("LightGridCommon") =
{
	Language = "GLSL";
	
	Pass =
	{
		Compute = 
		{
			layout (local_size_x = THREADGROUP_SIZE, local_size_y = THREADGROUP_SIZE, local_size_z = THREADGROUP_SIZE) in;
		
			layout(std430, binding = 1) readonly buffer gLights
			{
				LightData[] gLightsData;
			};
			
			layout(binding = 2, r32ui) uniform uimageBuffer gLinkedListCounter;
			layout(binding = 3, r32ui) uniform uimageBuffer gLinkedListHeads;
			layout(binding = 5, rgba32ui) uniform uimageBuffer gLinkedList;
			
			void calcCellAABB(uvec3 cellIdx, out vec3 center, out vec3 extent)
			{
				// Convert grid XY coordinates to clip coordinates
				vec2 a = 2.0f / gGridSize.xy;
			
				vec3 ndcMin;
				vec3 ndcMax;
			
				ndcMin.xy = cellIdx.xy * a - vec2(1.0f, 1.0f);
				ndcMax.xy = (cellIdx.xy + 1) * a - vec2(1.0f, 1.0f);
			
				// Flip Y depending on render API, depending if Y in NDC is facing up or down
				// (We negate the value because we want NDC with Y flipped, so origin is top left)
				float flipY = -sign(gMatProj[1][1]);
				ndcMin.y *= flipY;
				ndcMax.y *= flipY;
			
				// Because we're viewing along negative Z, farther end is the minimum
				float viewZMin = calcViewZFromCellZ(cellIdx.z + 1);
				float viewZMax = calcViewZFromCellZ(cellIdx.z);
			
				ndcMin.z = convertToNDCZ(viewZMax);
				ndcMax.z = convertToNDCZ(viewZMin);
							
				vec4 corner[8];
				// Near
				corner[0] = gMatInvProj * vec4(ndcMin.x, ndcMin.y, ndcMin.z, 1.0f);
				corner[1] = gMatInvProj * vec4(ndcMax.x, ndcMin.y, ndcMin.z, 1.0f);
				corner[2] = gMatInvProj * vec4(ndcMax.x, ndcMax.y, ndcMin.z, 1.0f);
				corner[3] = gMatInvProj * vec4(ndcMin.x, ndcMax.y, ndcMin.z, 1.0f);
			
				// Far
				corner[4] = gMatInvProj * vec4(ndcMin.x, ndcMin.y, ndcMax.z, 1.0f);
				corner[5] = gMatInvProj * vec4(ndcMax.x, ndcMin.y, ndcMax.z, 1.0f);
				corner[6] = gMatInvProj * vec4(ndcMax.x, ndcMax.y, ndcMax.z, 1.0f);
				corner[7] = gMatInvProj * vec4(ndcMin.x, ndcMax.y, ndcMax.z, 1.0f);
			
				for(uint i = 0; i < 8; ++i)
					corner[i].xy /= corner[i].w;
			
				vec3 viewMin = vec3(corner[0].xy, viewZMin);
				vec3 viewMax = vec3(corner[0].xy, viewZMax);
				
				for(uint i = 1; i < 8; ++i)
				{
					viewMin.xy = min(viewMin.xy, corner[i].xy);
					viewMax.xy = max(viewMax.xy, corner[i].xy);
				}		
				
				extent = (viewMax - viewMin) * 0.5f;
				center = viewMin + extent;
			}

			void main()
			{
				// Ignore pixels out of valid range
				if (any(greaterThanEqual(gl_GlobalInvocationID.xy, gGridSize.xy)))
					return;
					
				uint maxNumLinks = gNumCells * gMaxNumLightsPerCell;	
				uint cellIdx = (gl_GlobalInvocationID.z * gGridSize.y + gl_GlobalInvocationID.y) * gGridSize.x + gl_GlobalInvocationID.x;
				
				vec3 cellCenter;
				vec3 cellExtent;
				calcCellAABB(gl_GlobalInvocationID, cellCenter, cellExtent);
				
				for(uint type = 1; type < 3; ++type)
				{
					uint lightOffset = gLightOffsets[type - 1];
					uint lightEnd = gLightOffsets[type];
					for(uint i = lightOffset; i < lightEnd; ++i)
					{
						vec4 lightPosition = gMatView * vec4(gLightsData[i].position, 1.0f);
						float lightRadius = gLightsData[i].attRadius;
						
						// Calculate distance from box to light
						vec3 distances = max(abs(lightPosition.xyz - cellCenter) - cellExtent, 0);
						float distSqrd = dot(distances, distances);
						
						if(distSqrd <= (lightRadius * lightRadius))
						{
							uint nextLink = imageAtomicAdd(gLinkedListCounter, 0, 1U);
							if(nextLink < maxNumLinks)
							{
								uint prevLink = imageAtomicExchange(gLinkedListHeads, int(cellIdx), nextLink);
								
								imageStore(gLinkedList, int(nextLink), uvec4(i, type, prevLink, 0));
							}
						}
					}
				}
			}
		};
	};
};