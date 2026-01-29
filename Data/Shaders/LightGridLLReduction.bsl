#include "$ENGINE$\PerCameraData.bslinc"
#include "$ENGINE$\LightGridCommon.bslinc"

shader LightGridLLReduction
{
	mixin PerCameraData;
	mixin LightGridCommon; 

	featureset = HighEnd;
	
	code
	{
		StructuredBuffer<uint> gLightsLLHeads;
		Buffer<uint4> gLightsLL;
					
		StructuredBuffer<uint> gProbesLLHeads;
		Buffer<uint2> gProbesLL;
		
		RWStructuredBuffer<uint> gGridDataCounter;
		
		RWBuffer<uint4> gGridLightOffsetAndSize;
		RWBuffer<uint> gGridLightIndices;

		RWBuffer<uint2> gGridProbeOffsetAndSize;
		RWBuffer<uint> gGridProbeIndices;
		
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
			
			// Reduce lights
			//// First count total number of lights affecting the tile
			uint currentIdx = gLightsLLHeads[cellIdx];
			uint numRadialLights = 0;
			uint numSpotLights = 0;
			while(currentIdx != 0xFFFFFFFF)
			{
				uint4 entry = gLightsLL[currentIdx];
			
				if(entry.y == 1) // Radial
					numRadialLights++;
				else // Spot
					numSpotLights++;

				currentIdx = entry.z;
			}
			
			//// Allocate enough room and remember the offset to indices
			uint numLights = numRadialLights + numSpotLights;
			uint indicesStart;
			InterlockedAdd(gGridDataCounter[0], numLights, indicesStart);
			gGridLightOffsetAndSize[cellIdx] = uint4(indicesStart, numRadialLights, numSpotLights, 0);
			
			//// Actually write light indices (reverse order, so that radial lights come first, as is the convention)
			currentIdx = gLightsLLHeads[cellIdx];
			uint lightIdx = 0;
			while(currentIdx != 0xFFFFFFFF)
			{
				uint4 entry = gLightsLL[currentIdx];
			
				gGridLightIndices[indicesStart + numLights - 1 - lightIdx] = entry.x;
				
				currentIdx = entry.z;
				lightIdx++;
			}
			
			// Reduce probes
			//// First count total number of probes affecting the tile
			currentIdx = gProbesLLHeads[cellIdx];
			uint numProbes = 0;
			while(currentIdx != 0xFFFFFFFF)
			{
				uint2 entry = gProbesLL[currentIdx];
			
				numProbes++;
				currentIdx = entry.y;
			}
			
			//// Allocate enough room and remember the offset to indices
			InterlockedAdd(gGridDataCounter[1], numProbes, indicesStart);
			gGridProbeOffsetAndSize[cellIdx] = uint2(indicesStart, numProbes);
			
			//// Actually write probe indices (reverse order, in order to restore original order since LL was formed in reverse)
			currentIdx = gProbesLLHeads[cellIdx];
			uint probeIdx = 0;
			while(currentIdx != 0xFFFFFFFF)
			{
				uint2 entry = gProbesLL[currentIdx];
			
				gGridProbeIndices[indicesStart + numProbes - 1 - probeIdx] = entry.x;
				
				currentIdx = entry.y;
				probeIdx++;
			}
		}
	};
};
